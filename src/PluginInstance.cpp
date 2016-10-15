#include "PluginInstance.h"
#include <nan.h>

using namespace v8;

PluginInstance::PluginInstance(){};
PluginInstance::~PluginInstance(){};

Nan::Persistent<v8::Function> PluginInstance::constructor;

void PluginInstance::Init() {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PluginInstance").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeTemplate(
      tpl, "displayGUI",
      Nan::New<v8::FunctionTemplate>(DisplayGUI)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "hideGUI", Nan::New<v8::FunctionTemplate>(HideGUI)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "stop", Nan::New<v8::FunctionTemplate>(Stop)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "processAudioBlock",
      Nan::New<v8::FunctionTemplate>(ProcessAudioBlock)->GetFunction());

  constructor.Reset(tpl->GetFunction());
}

void PluginInstance::DisplayGUI(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(Nan::New("Displaying GUI...").ToLocalChecked());
}

void PluginInstance::HideGUI(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(Nan::New("Hiding GUI...").ToLocalChecked());
}

void PluginInstance::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(Nan::New("Plugin Stopped...").ToLocalChecked());
}

void PluginInstance::ProcessAudioBlock(
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

void PluginInstance::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginInstance *obj = new PluginInstance();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Object> PluginInstance::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 0;
  v8::Local<v8::Value> argv[argc] = {};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
