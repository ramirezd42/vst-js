#include "PluginHostWrapper.h"

void CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(PluginHostWrapper::NewInstance(info[0]));
}

void InitAll(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  Nan::HandleScope scope;

  // get the path of the .node file
  v8::Local<v8::String> modulePathHandle = module->Get(Nan::New("filename").ToLocalChecked()).As<v8::String>();
  std::string modulePath = *v8::String::Utf8Value(modulePathHandle);
  unsigned long lastSlash = modulePath.find_last_of("/");
  std::string moduleDir = modulePath.substr(0,lastSlash);

  PluginHostWrapper::Init(moduleDir);

  exports->Set(Nan::New("launchPlugin").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(CreateObject)->GetFunction());
}

NODE_MODULE(vstjs, InitAll)
