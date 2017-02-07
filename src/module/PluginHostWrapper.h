#ifndef VSTJS_PLUGINHOSTWRAPPER_H
#define VSTJS_PLUGINHOSTWRAPPER_H

#include <nan.h>
#include "PluginHost.h"

class PluginHostWrapper : public Nan::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  static void Start(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void ProcessAudioBlock(const Nan::FunctionCallbackInfo<v8::Value> &info);

private:
  PluginHostWrapper(std::string _shmemFile, std::string pluginPath);
  static Nan::Persistent<v8::Function> constructor;
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
  PluginHost host;
};

#endif //VSTJS_PLUGINHOSTWRAPPER_H
