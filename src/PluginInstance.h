#ifndef PLUGININSTANCE_H
#define PLUGININSTANCE_H

#include <nan.h>

class PluginInstance : public Nan::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  static void DisplayGUI(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void HideGUI(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void
  ProcessAudioBlock(const Nan::FunctionCallbackInfo<v8::Value> &info);

private:
  PluginInstance();
  ~PluginInstance();

  static Nan::Persistent<v8::Function> constructor;
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
};

#endif
