#include "PluginHost.h"
#include "PluginInstance.h"
#include <nan.h>

void CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(PluginHost::NewInstance(info[0]));
}

void InitAll(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  Nan::HandleScope scope;

  PluginHost::Init();
  PluginInstance::Init();

  exports->Set(Nan::New("createHost").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(CreateObject)->GetFunction());
}

NODE_MODULE(vstjs, InitAll)
