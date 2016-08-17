#include "JuceLibraryCode/JuceHeader.h"
#include <nan.h>

namespace vstjs {
  class PluginList: public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);
    juce::ScopedPointer<juce::KnownPluginList> list;
  private:
    static NAN_METHOD(New);
    static NAN_METHOD(GetPlugin);

    PluginList(std::string);
  };
}