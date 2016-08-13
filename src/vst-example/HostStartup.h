#include "../JuceLibraryCode/JuceHeader.h"
#include "InternalFilters.h"
#include "MainHostWindow.h"

#if !(JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU)
#error                                                                         \
    "If you're building the audio plugin host, you probably want to enable VST and/or AU support"
#endif

class PluginHostApp : public JUCEApplication, private AsyncUpdater {
public:
  PluginHostApp() {}
  void initialise(const String &) override;
  void handleAsyncUpdate() override;
  void shutdown() override;
  void systemRequestedQuit() override;
  const String getApplicationName() override;
  const String getApplicationVersion() override;
  bool moreThanOneInstanceAllowed() override;

  ApplicationCommandManager commandManager;
  ScopedPointer<ApplicationProperties> appProperties;
  LookAndFeel_V3 lookAndFeel;

private:
  ScopedPointer<MainHostWindow> mainWindow;
};
