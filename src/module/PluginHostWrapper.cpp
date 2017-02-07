#include "PluginHostWrapper.h"
#include <nan.h>

using namespace std;
using namespace v8;

PluginHostWrapper::PluginHostWrapper(string _shmemFile, string _pluginPath): host(_shmemFile, _pluginPath) {}
Nan::Persistent<v8::Function> PluginHostWrapper::constructor;

v8::Local<v8::Object> PluginHostWrapper::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;
  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {arg};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}

void PluginHostWrapper::Init() {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PluginHost").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeTemplate(
    tpl, "start", Nan::New<v8::FunctionTemplate>(Start));

  Nan::SetPrototypeTemplate(
    tpl, "stop", Nan::New<v8::FunctionTemplate>(Stop));

  Nan::SetPrototypeTemplate(
    tpl, "processAudioBlock",
    Nan::New<v8::FunctionTemplate>(ProcessAudioBlock));

  constructor.Reset(tpl->GetFunction());
}

void PluginHostWrapper::Start(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHostWrapper *obj = ObjectWrap::Unwrap<PluginHostWrapper>(info.This());
  obj->host.Start();
}

void PluginHostWrapper::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHostWrapper *obj = ObjectWrap::Unwrap<PluginHostWrapper>(info.This());
  obj->host.Stop();
}

void PluginHostWrapper::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
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

  PluginHostWrapper *obj = new PluginHostWrapper("test123", pluginPath);

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void PluginHostWrapper::ProcessAudioBlock(
  const Nan::FunctionCallbackInfo<v8::Value> &info) {

  PluginHostWrapper *obj = ObjectWrap::Unwrap<PluginHostWrapper>(info.This());

  if (info.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 3 arguments");
    return;
  }

  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected Number");
    return;
  }
  int numChannels = info[0]->Int32Value();

  if (!info[1]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 2. Expected Number");
    return;
  }
  int numSamples = info[1]->Int32Value();

  if (!info[2]->IsArray()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 3. Expected Typed Array");
    return;
  }

  // copy to buffer
  Local<Array> a = Local<Array>::Cast(info[2]);
  float** buffer = new float*[numChannels];
  for (int channel = 0;  channel < numChannels; channel++) {
    Local<Value> element = a->Get(channel);
    if (!element->IsFloat32Array()) {
      Nan::ThrowTypeError("Incorrect Type for channel data. Expected Float32Array");
    }
    Local<TypedArray> channel_handle = Local<TypedArray>::Cast(element);
    Nan::TypedArrayContents<float> dest(channel_handle);
    buffer[channel] = *dest;
  }

  obj->host.ProcessAudioBlock(numChannels, numSamples, buffer);

  // copy from buffer
  Local<Array> outputChannels = Array::New(v8::Isolate::GetCurrent(), numChannels);
  for (int channel = 0; channel < numChannels; ++channel) {
    Local<Float32Array> outChan = Float32Array::New(ArrayBuffer::New(v8::Isolate::GetCurrent(), sizeof(float) * numSamples), 0, numSamples);
    for (int sample = 0; sample < numSamples; ++sample) {
      outChan->Set(sample, Number::New(v8::Isolate::GetCurrent(), buffer[channel][sample]));
    }
    outputChannels->Set(channel, outChan);
  }
  // return result
  info.GetReturnValue().Set(outputChannels);
}

