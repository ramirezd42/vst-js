#ifndef PLUGINHOST_H
#define PLUGINHOST_H

#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include <nan.h>
#include "SharedMemoryBuffer.h"

class PluginHost : public Nan::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  static void Start(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void
  ProcessAudioBlock(const Nan::FunctionCallbackInfo<v8::Value> &info);
  String shMemSegmentId;

private:
  PluginHost(String _shmemSegmentId);
  ~PluginHost();

  static Nan::Persistent<v8::Function> constructor;
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
};

#endif
