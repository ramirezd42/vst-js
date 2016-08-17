#include "PluginList.h"

vstjs::PluginList::PluginList(std::string searchPath) {
  AudioPluginFormatManager formatManager;
  formatManager.addDefaultFormats();

  VST3PluginFormat pluginFormat;
  FileSearchPath pluginSearchPath(searchPath);
  ScopedPointer<KnownPluginList> pluginList = new KnownPluginList();
  File deadMansFile;

  PluginDirectoryScanner directoryScanner(*pluginList, pluginFormat, pluginSearchPath, true, deadMansFile);
  String nameOfPluginBeingScanned;
  while(directoryScanner.scanNextFile(true, nameOfPluginBeingScanned)) {}

  this->list = pluginList;
}

NAN_METHOD(vstjs::PluginList::New) {
  if(info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments");
  }

  if(!info[0]->IsString()) {
    Nan::ThrowTypeError("Wrong argument type");
  }

  auto searchPath = Nan::To<v8::String>(info[0]).ToLocalChecked();
  auto pluginList = new vstjs::PluginList(*Nan::Utf8String(searchPath));
  pluginList->Wrap(info.This());
}

NAN_METHOD(vstjs::PluginList::GetPlugins) {
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments");
  }

  if(!info[0]->IsFunction()) {
    Nan::ThrowTypeError("Wrong argument type(s)");
  }
  v8::Local<v8::Function> cb = info[1].As<v8::Function>();
  unsigned argc = 1;
  auto pluginList = Nan::ObjectWrap::Unwrap<vstjs::PluginList>(info.Holder());
  ScopedPointer<KnownPluginList::PluginTree> pluginTree =  pluginList->list->createTree(KnownPluginList::sortAlphabetically);
  Array<const juce::PluginDescription*> pluginDescriptions = pluginTree->plugins;

  v8::Local<v8::Array> pluginArr = v8::Array::New(Nan::GetCurrentContext()->GetIsolate(), pluginDescriptions.size());
  for(int i=0; i< pluginDescriptions.size(); i++) {
    v8::Local<v8::Object> plugin = Nan::New<v8::Object>();
    plugin->Set(Nan::New("name").ToLocalChecked(), Nan::New(pluginDescriptions[i]->name.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("descriptiveName").ToLocalChecked(), Nan::New(pluginDescriptions[i]->descriptiveName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("pluginFormatName").ToLocalChecked(), Nan::New(pluginDescriptions[i]->pluginFormatName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("category").ToLocalChecked(), Nan::New(pluginDescriptions[i]->category.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("manufacturerName").ToLocalChecked(), Nan::New(pluginDescriptions[i]->manufacturerName.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("version").ToLocalChecked(), Nan::New(pluginDescriptions[i]->version.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("fileOrIdentifier").ToLocalChecked(), Nan::New(pluginDescriptions[i]->fileOrIdentifier.toStdString()).ToLocalChecked());
    plugin->Set(Nan::New("identifierString").ToLocalChecked(), Nan::New(pluginDescriptions[i]->createIdentifierString().toStdString()).ToLocalChecked());
    pluginArr->Set(i, plugin);
  }

  v8::Local<v8::Value> argv[argc] = { pluginArr };
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb, argc, argv);
}

NAN_MODULE_INIT(vstjs::PluginList::Init) {
  auto cname = Nan::New("PluginList").ToLocalChecked();
  auto ctor = Nan::New<v8::FunctionTemplate>(New);
  auto ctorInst = ctor->InstanceTemplate();
  ctor->SetClassName(cname);
  ctorInst->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(ctor, "getPlugins", GetPlugins);
  Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());
}

NAN_MODULE_INIT(InitAll) {
  vstjs::PluginList::Init(target);
}

NODE_MODULE(NanObject, InitAll)

