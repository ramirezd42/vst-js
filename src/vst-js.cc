// hello.cc
#include "JuceLibraryCode/JuceHeader.h"
#include "vst-example/HostStartup.h"
#include <nan.h>

// START OF MY GARBAGE
static juce::JUCEApplicationBase *juce_CreateApplication() {
  return new PluginHostApp();
}

// END OF MY GARBAGE
// static void launchHost() {
//   juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
//   juce::JUCEApplicationBase::main(0, NULL);
// }
//
// void Method(const FunctionCallbackInfo<Value> &args) {
//   launchHost();
//   Isolate *isolate = args.GetIsolate();
//   args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "world"));
// }
//
// void init(Local<Object> exports) { NODE_SET_METHOD(exports, "hello", Method);
// }
//

void ListPlugins(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  v8::Local<v8::Function> cb = info[0].As<v8::Function>();
  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {Nan::New("hello world").ToLocalChecked()};
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb, argc, argv);
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  exports->Set(Nan::New("listPlugins").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(ListPlugins)->GetFunction());
}

NODE_MODULE(addon, Init)
