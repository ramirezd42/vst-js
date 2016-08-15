// hello.cc
#include "JuceLibraryCode/JuceHeader.h"
#include "vst-example/HostStartup.h"
#include <nan.h>

using namespace juce;

namespace vstjs {

ScopedPointer<KnownPluginList> FetchPlugins(String pluginPath) {
  AudioPluginFormatManager formatManager;
  formatManager.addDefaultFormats();
  formatManager.addFormat (new InternalPluginFormat());

  VST3PluginFormat pluginFormat;
  FileSearchPath pluginSearchPath(pluginPath);
  ScopedPointer<KnownPluginList> pluginList = new KnownPluginList();
  File deadMansFile;

  PluginDirectoryScanner directoryScanner(*pluginList, pluginFormat, pluginSearchPath, true, deadMansFile);
  String nameOfPluginBeingScanned;
  while(directoryScanner.scanNextFile(true, nameOfPluginBeingScanned)) {}
  return pluginList;
}

void ListPlugins(const Nan::FunctionCallbackInfo<v8::Value> &info) {

  if (info.Length() < 2) {
    Nan::ThrowTypeError("Wrong number of arguments");
  }

  if(!info[0]->IsString() || !info[1]->IsFunction()) {
    Nan::ThrowTypeError("Wrong argument type(s)");
  }

  v8::String::Utf8Value searchArg(info[0]->ToString());
  const char* searchPath = *searchArg;

  v8::Local<v8::Function> cb = info[1].As<v8::Function>();
  const unsigned argc = 1;
  ScopedPointer<KnownPluginList> pluginList = FetchPlugins(searchPath);
  ScopedPointer<KnownPluginList::PluginTree> pluginTree =  pluginList->createTree(KnownPluginList::sortAlphabetically);
  Array<const PluginDescription*> pluginDescripions =  pluginTree->plugins;

  v8::Local<v8::Array> pluginArr = v8::Array::New(Nan::GetCurrentContext()->GetIsolate(), pluginDescripions.size());
  for(int i=0; i< pluginDescripions.size(); i++) {
    v8::Local<v8::Object> plugin = Nan::New<v8::Object>();
    plugin->Set(Nan::New("name").ToLocalChecked(), Nan::New(pluginDescripions[i]->name.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("descriptiveName").ToLocalChecked(), Nan::New(pluginDescripions[i]->descriptiveName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("pluginFormatName").ToLocalChecked(), Nan::New(pluginDescripions[i]->pluginFormatName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("category").ToLocalChecked(), Nan::New(pluginDescripions[i]->category.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("manufacturerName").ToLocalChecked(), Nan::New(pluginDescripions[i]->manufacturerName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("version").ToLocalChecked(), Nan::New(pluginDescripions[i]->version.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("fileOrIdentifier").ToLocalChecked(), Nan::New(pluginDescripions[i]->fileOrIdentifier.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("identifierString").ToLocalChecked(), Nan::New(pluginDescripions[i]->createIdentifierString().toStdString()).ToLocalChecked());
    pluginArr->Set(i, plugin);
  }

  v8::Local<v8::Value> argv[argc] = { pluginArr };
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb, argc, argv);
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
  exports->Set(Nan::New("listPlugins").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(ListPlugins)->GetFunction());
}

NODE_MODULE(addon, Init)
}
