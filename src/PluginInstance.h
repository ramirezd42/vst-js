#include "JuceLibraryCode/JuceHeader.h"
#include <nan.h>

namespace vstjs {
  class PluginInstance:Nan::ObjectWrap {
  public:
    PluginInstance(juce::PluginDescription pluginDescription);
  protected:
    void Ref() override;
    void Unref() override;
  private:
    juce::ScopedPointer<juce::AudioPluginInstance> pluginInstance;
  };

}
