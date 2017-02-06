 #include "PluginHost.h"
#include <nan.h>
#include <iostream>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace v8;
using namespace std;
using namespace boost::process;
using namespace boost::interprocess;

PluginHost::PluginHost(std::string _shmemFile, std::string _pluginPath)
  : shmemFile(_shmemFile),
    pluginPath(_pluginPath),
    shmemRemover(_shmemFile.data())

{
  shm = unique_ptr<shared_memory_object> (new shared_memory_object(create_only               //only create
    ,shmemFile.data() //name
    ,read_write                //read-write mode
  ));

  try {

    //Set size
    shm->truncate(sizeof(SharedMemoryBuffer));

    region = unique_ptr<mapped_region> (new mapped_region(
      *shm,
      read_write
    ));

    //Construct the shared structure in memory
    shmemBuffer = unique_ptr<SharedMemoryBuffer>(new(region->get_address()) SharedMemoryBuffer);
  } catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
};
PluginHost::~PluginHost(){};

Nan::Persistent<v8::Function> PluginHost::constructor;

v8::Local<v8::Object> PluginHost::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;
  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {arg};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}

void PluginHost::Init() {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PluginHost").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeTemplate(
      tpl, "start", Nan::New<v8::FunctionTemplate>(Start)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "stop", Nan::New<v8::FunctionTemplate>(Stop)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "processAudioBlock",
      Nan::New<v8::FunctionTemplate>(ProcessAudioBlock)->GetFunction());

  constructor.Reset(tpl->GetFunction());
}

void PluginHost::Start(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());

  // launch child process, with specified plugin and memory file
  obj->processManager.open_process(obj->pluginPath, obj->shmemFile);
}

void PluginHost::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  obj->processManager.terminate_process();
  info.GetReturnValue().Set(Nan::New("Host Stopped...").ToLocalChecked());
}

void PluginHost::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 1 argument");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected String");
    return;
  }
  v8::String::Utf8Value param1(info[0]->ToString());
  string pluginPath = string(*param1);

  PluginHost *obj = new PluginHost("test123", pluginPath);

    obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void PluginHost::ProcessAudioBlock(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {

  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());

  if (info.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 3 arguments");
    return;
  }

  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected Number");
    return;
  }
  double numChannels = info[0]->NumberValue();

  if (!info[1]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 2. Expected Number");
    return;
  }
  double numSamples = info[1]->NumberValue();

  if (!info[2]->IsArray()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 3. Expected Typed Array");
    return;
  }

  scoped_lock<interprocess_mutex> lock(obj->shmemBuffer->mutex);


  // copy to buffer
  // todo: THIS IS WHERE THE PROBLEM IS. THE BUFFER IS NOT BEING ASSIGNED THE RIGHT FLOAT VALUE?
  Local<Array> a = Local<Array>::Cast(info[2]);
  for (int channel = 0, size = a->Length(); channel < size; channel++) {
    Local<Value> element = a->Get(channel);
    if (!element->IsFloat32Array()) {
      Nan::ThrowTypeError("Incorrect Type for channel data. Expected Float32Array");
    }
    Local<TypedArray> channel_handle = Local<TypedArray>::Cast(element);
    Nan::TypedArrayContents<float> dest(channel_handle);
    float* samples = *dest;
    for (int sample = 0; sample < channel_handle->Length(); sample++) {
      obj->shmemBuffer->buffer[channel][sample] = samples[sample];
    }
    int foo = 0;
  }

  // wait for child process to write to the buffer
  obj->shmemBuffer->cond_empty.notify_one();
  obj->shmemBuffer->message_in = true;

  if(obj->shmemBuffer->message_in) {
    obj->shmemBuffer->cond_full.wait(lock);
  }


  // copy from buffer
  Local<Array> outputChannels = Array::New(v8::Isolate::GetCurrent(), obj->shmemBuffer->NumChannels);
  for (int channel = 0; channel < obj->shmemBuffer->NumChannels; ++channel) {
    Local<Float32Array> outChan = Float32Array::New(ArrayBuffer::New(v8::Isolate::GetCurrent(), sizeof(float) * obj->shmemBuffer->BufferSize), 0, obj->shmemBuffer->BufferSize);
    for (int sample = 0; sample < obj->shmemBuffer->BufferSize; ++sample) {
      outChan->Set(sample, Number::New(v8::Isolate::GetCurrent(), obj->shmemBuffer->buffer[channel][sample]));
    }
    outputChannels->Set(channel, outChan);
  }
//  // return result
  info.GetReturnValue().Set(outputChannels);
}

