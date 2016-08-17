#include "JuceLibraryCode/JuceHeader.h"
#include <nan.h>
class PluginList : public Nan::ObjectWrap {
public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("PluginList").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "getHandle", GetHandle);
    SetPrototypeMethod(tpl, "getPlugins", GetPlugins);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("PluginList").ToLocalChecked(),
      Nan::GetFunction(tpl).ToLocalChecked());
  }
protected:
  void Ref() override {}
  void Unref() override {}
private:
  explicit PluginList(const char* _searchPath):searchPath(_searchPath) {
    this->pluginList = new KnownPluginList();
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();
    VST3PluginFormat pluginFormat;
    File deadMansFile;
    FileSearchPath pluginSearchPath(this->searchPath);
    PluginDirectoryScanner directoryScanner(*this->pluginList, pluginFormat, pluginSearchPath, true, deadMansFile);
    String nameOfPluginBeingScanned;
    while(directoryScanner.scanNextFile(true, nameOfPluginBeingScanned)) {}
  }
  ~PluginList() {}

  const char* searchPath;
  ScopedPointer<KnownPluginList> pluginList;

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      if (info.Length() < 1) {
        Nan::ThrowTypeError("Wrong number of arguments");
      }

      if(!info[0]->IsString()) {
        Nan::ThrowTypeError("Wrong argument type(s)");
      }

      v8::String::Utf8Value searchArg(info[0]->ToString());
      const char* searchPath = *searchArg;
      PluginList *obj = new PluginList(searchPath);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = {info[0]};
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }

  static NAN_METHOD(GetHandle) {
    PluginList* obj = Nan::ObjectWrap::Unwrap<PluginList>(info.Holder());
    info.GetReturnValue().Set(obj->handle());
  }

  static NAN_METHOD(GetPlugins) {
    if (info.Length() < 1) {
      Nan::ThrowTypeError("Wrong number of arguments");
    }

    if(!info[0]->IsFunction()) {
      Nan::ThrowTypeError("Wrong argument type(s)");
    }

    v8::Local<v8::Function> cb = info[0].As<v8::Function>();

    PluginList* obj = Nan::ObjectWrap::Unwrap<PluginList>(info.Holder());
    obj->Ref();
    const unsigned argc = 1;
//    ScopedPointer<KnownPluginList::PluginTree> pluginTree = obj->pluginList->createTree(KnownPluginList::sortAlphabetically);
    ScopedPointer<KnownPluginList::PluginTree> pluginTree = obj->pluginList->createTree(KnownPluginList::sortAlphabetically);
    Array<const PluginDescription*> pluginDescriptions =  pluginTree->plugins;

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

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

NODE_MODULE(objectwrapper, PluginList::Init)
