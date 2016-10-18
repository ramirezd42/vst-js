#include "PluginHost.h"
#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include "zhelpers.hpp"
#include <nan.h>

using namespace v8;

PluginHost::PluginHost(juce::String _socketAddress)
    : context(1), socket(this->context, ZMQ_REQ),
      socketAddress(_socketAddress){};
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
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 1 argument");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected String");
    return;
  }

  v8::String::Utf8Value param1(info[0]->ToString());
  juce::String pluginPath = std::string(*param1);

  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  StringArray args;

  // connect socket to requested address
  std::cout << "Socket Address: " << std::endl;
  obj->socket.connect(obj->socketAddress.toRawUTF8());

  // launch child process, with specified plugin
  // listening on specified socket address
  args.add("/Users/dxr224/Projects/vst-js/build/Debug/vst-js-bin");
  args.add(pluginPath);
  args.add(obj->socketAddress);
  obj->proc.start(args);
  info.GetReturnValue().Set(Nan::New("Host Started...").ToLocalChecked());
}

void PluginHost::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  obj->proc.kill();
  obj->socket.disconnect(obj->socketAddress.toRawUTF8());
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
  juce::String socketAddress = std::string(*param1);

  PluginHost *obj = new PluginHost(socketAddress);
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
  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {arg};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
