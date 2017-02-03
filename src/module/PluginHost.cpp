 #include "PluginHost.h"
#include <nan.h>
#include <iostream>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace v8;
using namespace std;
using namespace boost::process;
using namespace boost::interprocess;

// THIS JUNK IS TEMPORARY TO TEST WITH WHITE NOISE DATA
//
//void getNextInputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels, int numSamples, float * inputChannelData) {
//  for (int channel = 0; channel < numInputChannels; ++channel) {
//    for (int sample= 0; sample < numSamples * numInputChannels; ++sample) {
//      buffer.add_inputdata(inputChannelData[channel+sample]);
//    }
//  }
//}

//void getNextInputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels,
//                            int numSamples, Random &randomGen) {
//  for (int i = 0; i < numInputChannels; ++i) {
//    for (int i = 0; i < numSamples * numInputChannels; ++i) {
//      buffer.add_inputdata(randomGen.nextFloat() * 0.25f - 0.125f);
//    }
//  }
//}

//
//void getNextOutputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels,
//                             int numSamples) {
//  for (int i = 0; i < numInputChannels; ++i) {
//    for (int i = 0; i < numSamples * numInputChannels; ++i) {
//      buffer.add_outputdata(0);
//    }
//  }
//}
//
//void copyNextOutputAudioBlock(const float* data, float* output, int numInputChannels, int numSamples) {
//  for (int channel = 0; channel < numInputChannels; ++channel) {
//    for (int sample= 0; sample < numSamples * numInputChannels; ++sample) {
//      output[channel+sample] = data[channel+sample];
//    }
//  }
//}
//
//juce::String bufferToString(vstjs::IOBuffer &buffer) {
//  juce::String returnStr = "";
//  for (int channel = 0; channel < buffer.numinputchannels(); ++channel) {
//    for (int sample = 0; sample < buffer.samplesize(); ++ sample) {
//      returnStr += "Channel " + channel;
//      returnStr += ", Sample " + sample;
//      returnStr += std::to_string(buffer.inputdata().Get(buffer.samplesize()*channel + sample));
//      returnStr += "\n";
////      std::cout << "Channel " << channel << ", Sample " << sample << ": " << buffer.inputdata().Get(buffer.samplesize()*channel + sample) << std::endl;
//    }
//  }
//  return returnStr;
//}

 void getNextInputBuffer(SharedMemoryBuffer* buffer) {
   for(int channel = 0; channel < buffer->NumChannels; ++channel) {
     for(int sample =0; sample < buffer->BufferSize; ++sample) {
       buffer->buffer[channel][sample] = rand() / (float)RAND_MAX;
     }
   }
 }

 void printBuffer(SharedMemoryBuffer* buffer) {
   for(int channel = 0; channel < buffer->NumChannels; ++channel) {
     printf("Channel %d: ", channel);
     for(int sample =0; sample < buffer->BufferSize; ++sample) {
       printf("%f ", buffer->buffer[channel][sample]);
     }
     cout << "\n";
   }
 }

// END TEMPORARY JUNK

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

  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());

  scoped_lock<interprocess_mutex> lock(obj->shmemBuffer.get()->mutex);
  if(obj->shmemBuffer.get()->message_in){
    cout << "Full. Waiting" << endl;
    obj->shmemBuffer.get()->cond_full.wait(lock);
  }

  getNextInputBuffer(obj->shmemBuffer.get());

  //Notify to the other process that there is a message
  obj->shmemBuffer.get()->cond_empty.notify_one();

  //Mark message buffer as full
  obj->shmemBuffer.get()->message_in = true;

  info.GetReturnValue().Set(info[2]);
}

