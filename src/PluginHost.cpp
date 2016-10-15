#include "PluginHost.h"
#include "PluginInstance.h"
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
      tpl, "launchPlugin",
      Nan::New<v8::FunctionTemplate>(LaunchPlugin)->GetFunction());

  constructor.Reset(tpl->GetFunction());
}

void PluginHost::Start(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(Nan::New("Host Started...").ToLocalChecked());
}

void PluginHost::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(Nan::New("Host Stopped...").ToLocalChecked());
}

void PluginHost::LaunchPlugin(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(PluginInstance::NewInstance(info[0]));
}

void PluginHost::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = new PluginHost();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Object> PluginHost::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 0;
  v8::Local<v8::Value> argv[argc] = {};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
