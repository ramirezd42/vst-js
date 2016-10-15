#ifndef PLUGINHOST_H
#define PLUGINHOST_H

#include <nan.h>

class PluginHost : public Nan::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  static void Start(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void LaunchPlugin(const Nan::FunctionCallbackInfo<v8::Value> &info);

private:
  PluginHost();
  ~PluginHost();

  static Nan::Persistent<v8::Function> constructor;
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
};

#endif
