#include "JuceLibraryCode/JuceHeader.h"
#include <nan.h>
#include <thread>

namespace vstjs {

class NewProjectApplication  : public JUCEApplication
{
public:
  //==============================================================================
  NewProjectApplication() {}

  const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
  const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
  bool moreThanOneInstanceAllowed() override       { return true; }

  //==============================================================================
  void initialise (const String& commandLine) override
  {
    // This method is where you should put your application's initialisation code..

    mainWindow = new MainWindow (getApplicationName());
  }

  void shutdown() override
  {
    // Add your application's shutdown code here..

    mainWindow = nullptr; // (deletes our window)
  }

  //==============================================================================
  void systemRequestedQuit() override
  {
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app to close.
    quit();
  }

  void anotherInstanceStarted (const String& commandLine) override
  {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
  }

  //==============================================================================
  /*
      This class implements the desktop window that contains an instance of
      our MainContentComponent class.
  */
  class MainWindow    : public DocumentWindow
  {
  public:
    MainWindow (String name)  : DocumentWindow (name,
      Colours::lightgrey,
      DocumentWindow::allButtons)
    {
      setUsingNativeTitleBar (true);
//            setContentOwned (createMainContentComponent(), true);
      setResizable (true, true);

      centreWithSize (getWidth(), getHeight());
      setVisible (true);
    }

    void closeButtonPressed() override
    {
      // This is called when the user tries to close this window. Here, we'll just
      // ask the app to quit when this happens, but you can change this to do
      // whatever you need.
      JUCEApplication::getInstance()->systemRequestedQuit();
    }

    /* Note: Be careful if you override any DocumentWindow methods - the base
       class uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in your content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       subclass also calls the superclass's method.
    */

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
  };

private:
  ScopedPointer<MainWindow> mainWindow;
};

static juce::JUCEApplicationBase* juce_CreateApplication() { return new NewProjectApplication (); }
void callFromThread() {
  juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;
  juce::JUCEApplicationBase::main (0, nullptr);
}

class PluginDescription : public Nan::ObjectWrap {
public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("PluginDescription").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(2);
    // Prototype
    SetPrototypeMethod(tpl, "getName", GetName);
    SetPrototypeMethod(tpl, "createInstance", CreateInstance);

    constructor().Reset(tpl->GetFunction());
  }

  static v8::Local<v8::Object> NewInstance(juce::PluginDescription *plugin) {
    Nan::EscapableHandleScope scope;

    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor());
    v8::Local<v8::Object> instance = cons->NewInstance();

    instance->SetAlignedPointerInInternalField(0, plugin);
    return scope.Escape(instance);
  }

protected:
  void Ref() override {}
  void Unref() override {}

private:
  explicit PluginDescription(): t(std::thread(callFromThread)) {

  }
  ~PluginDescription() {
    t.join();
  }
  static NAN_METHOD(New) {
    vstjs::PluginDescription *desc = new vstjs::PluginDescription();
    desc->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }
  static NAN_METHOD(CreateInstance) {
    //TODO: see if I can open a window at all
//
//    MessageManager::getInstance()->setCurrentThreadAsMessageThread();
//    AudioDeviceManager deviceManager;
//    AudioDeviceManager::AudioDeviceSetup setup;
//    deviceManager.getAudioDeviceSetup(setup);
//    juce::PluginDescription *p = static_cast<juce::PluginDescription *>(
//        info.This()->GetAlignedPointerFromInternalField(0));
//    VST3PluginFormat format;
//    AudioPluginInstance *plugin = format.createInstanceFromDescription(
//        *p, setup.sampleRate, setup.bufferSize);
//    AudioProcessorEditor* editor = plugin->createEditor();
//
//    info.This()->SetAlignedPointerInInternalField(1, editor);
  }
  static NAN_METHOD(GetName) {
    juce::PluginDescription *p = static_cast<juce::PluginDescription *>(
      info.This()->GetAlignedPointerFromInternalField(0));
    info.GetReturnValue().Set(Nan::New(p->name.toStdString()).ToLocalChecked());
  }
  static inline Nan::Persistent<v8::Function> &constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
  std::thread t;
  ScopedPointer<NewProjectApplication::MainWindow> window;
};

class PluginList : public Nan::ObjectWrap {
public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("PluginList").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(2);

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
  explicit PluginList(const char *_searchPath) : searchPath(_searchPath) {
    this->pluginList = new KnownPluginList();
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();
    VST3PluginFormat pluginFormat;
    File deadMansFile;
    FileSearchPath pluginSearchPath(this->searchPath);
    PluginDirectoryScanner directoryScanner(
        *this->pluginList, pluginFormat, pluginSearchPath, true, deadMansFile);
    String nameOfPluginBeingScanned;
    while (directoryScanner.scanNextFile(true, nameOfPluginBeingScanned)) {
    }
  }
  ~PluginList() {}

  const char *searchPath;
  ScopedPointer<KnownPluginList> pluginList;

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      if (info.Length() < 1) {
        Nan::ThrowTypeError("Wrong number of arguments");
      }

      if (!info[0]->IsString()) {
        Nan::ThrowTypeError("Wrong argument type(s)");
      }

      v8::String::Utf8Value searchArg(info[0]->ToString());
      const char *searchPath = *searchArg;
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
    PluginList *obj = Nan::ObjectWrap::Unwrap<PluginList>(info.Holder());
    info.GetReturnValue().Set(obj->handle());
  }

  static NAN_METHOD(GetPluginInstance) {
    if (info.Length() < 1) {
      Nan::ThrowTypeError("Wrong number of arguments");
    }
    if (!info[0]->IsObject()) {
      Nan::ThrowTypeError("Wrong argument type");
    }
  }

  static NAN_METHOD(GetPlugins) {
    if (info.Length() < 1) {
      Nan::ThrowTypeError("Wrong number of arguments");
    }

    if (!info[0]->IsFunction()) {
      Nan::ThrowTypeError("Wrong argument type(s)");
    }

    v8::Local<v8::Function> cb = info[0].As<v8::Function>();

    PluginList *obj = Nan::ObjectWrap::Unwrap<PluginList>(info.Holder());
    obj->Ref();
    const unsigned argc = 1;
    //    ScopedPointer<KnownPluginList::PluginTree> pluginTree =
    //    obj->pluginList->createTree(KnownPluginList::sortAlphabetically);
    ScopedPointer<KnownPluginList::PluginTree> pluginTree =
        obj->pluginList->createTree(KnownPluginList::sortAlphabetically);
    Array<const juce::PluginDescription *> pluginDescriptions =
        pluginTree->plugins;

    v8::Local<v8::Array> pluginArr = v8::Array::New(
        Nan::GetCurrentContext()->GetIsolate(), pluginDescriptions.size());
    for (int i = 0; i < pluginDescriptions.size(); i++) {
      v8::Local<v8::Object> descObj = PluginDescription::NewInstance(
          const_cast<juce::PluginDescription *>(pluginDescriptions[i]));
      pluginArr->Set(i, descObj);
    }

    v8::Local<v8::Value> argv[argc] = {pluginArr};
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb, argc, argv);
  }

  static inline Nan::Persistent<v8::Function> &constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};
}

void InitAll(v8::Local<v8::Object> exports) {
  vstjs::PluginList::Init(exports);
  vstjs::PluginDescription::Init(exports);
}
NODE_MODULE(objectwrapper, InitAll)

