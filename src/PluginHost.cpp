#include "PluginHost.h"
#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include <nan.h>

using namespace v8;

PluginHost::PluginHost(){};
PluginHost::~PluginHost(){};

Nan::Persistent<v8::Function> PluginHost::constructor;

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
  if (info.Length() < 2) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 1 argument");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected String");
    return;
  }

  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 2. Expected String");
    return;
  }

  v8::String::Utf8Value param1(info[0]->ToString());
  juce::String pluginPath = std::string(*param1);

  v8::String::Utf8Value param2(info[1]->ToString());
  juce::String socketAddress = std::string(*param2);

  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  StringArray args;
  args.add("/Users/dxr224/Projects/vst-js/build/Debug/vst-js-bin");
  args.add(pluginPath);
  args.add(socketAddress);
  obj->proc.start(args);
  info.GetReturnValue().Set(Nan::New("Host Started...").ToLocalChecked());
}

void PluginHost::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  obj->proc.kill();
  info.GetReturnValue().Set(Nan::New("Host Stopped...").ToLocalChecked());
}

void PluginHost::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = new PluginHost();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void PluginHost::ProcessAudioBlock(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 1 argument");
    return;
  }

  // if (!info[0]->IsArray()) {
  //   Nan::ThrowTypeError("Wrong argument. Expected float[][]");
  //   return;
  // }

  // just pass the input buffer as the output buffer for now (no-op)
  info.GetReturnValue().Set(info[0]);
}

v8::Local<v8::Object> PluginHost::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 0;
  v8::Local<v8::Value> argv[argc] = {};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
