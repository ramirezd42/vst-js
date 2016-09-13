/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include "IPCAudioIODevice.h"

//==============================================================================
class vstjshostApplication : public JUCEApplication {
public:
  //==============================================================================
  vstjshostApplication() {
  }

  const String getApplicationName() override {
    return ProjectInfo::projectName;
  }

  const String getApplicationVersion() override {
    return ProjectInfo::versionString;
  }

  bool moreThanOneInstanceAllowed() override {
    return true;
  }

  //==============================================================================
  void initialise(const String &commandLine) override {
    // This method is where you should put your application's initialisation code..

    mainWindow = new MainWindow("Plugin Host");
  }

  void shutdown() override {
    // Add your application's shutdown code here..

    mainWindow = nullptr; // (deletes our window)
  }

  //==============================================================================
  void systemRequestedQuit() override {
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app to close.
    quit();
  }

  void anotherInstanceStarted(const String &commandLine) override {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
  }

  //==============================================================================
  /*
      This class implements the desktop window that contains an instance of
      our MainContentComponent class.
  */
  class MainWindow : public DocumentWindow {
  public:
    MainWindow(String name) : DocumentWindow(name,
      Colours::lightgrey,
      DocumentWindow::allButtons) {
      formatManager.addDefaultFormats();
      setUsingNativeTitleBar(true);
      setResizable(true, true);

      centreWithSize(getWidth(), getHeight());
      setVisible(true);



      AudioDeviceManager::AudioDeviceSetup setup;
      deviceManager.getAudioDeviceSetup(setup);

      PropertiesFile::Options options;
      options.applicationName     = "Juce Audio Plugin Host";
      options.filenameSuffix      = "settings";
      options.osxLibrarySubFolder = "Preferences";

      appProperties = new ApplicationProperties();
      appProperties->setStorageParameters (options);

//    ScopedPointer<XmlElement> savedAudioState (appProperties->getUserSettings() ->getXmlValue ("audioDeviceState"));
      ScopedPointer<XmlElement> savedAudioState = appProperties->getUserSettings()->getXmlValue("audioDeviceState");

      IPCAudioIODeviceType* ipcType = new IPCAudioIODeviceType();
      deviceManager.addAudioDeviceType(ipcType);
      deviceManager.setCurrentAudioDeviceType("IPC", true);
      deviceManager.initialise (256, 256, savedAudioState, true);

      OwnedArray<juce::PluginDescription> foundPlugins;
      VST3PluginFormat format;
      format.findAllTypesForFile(foundPlugins, "/Library/Audio/Plug-Ins/VST3/PrimeEQ.vst3");

      description = foundPlugins[0];
      AudioPluginInstance* instance = format.createInstanceFromDescription(*description, setup.sampleRate, setup.bufferSize);

      //i/o graph nodes
      inputProcessor = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
      outputProcessor = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

      // Add all nodes to graph
      inputNode = graph.addNode(inputProcessor);
      pluginNode = graph.addNode(instance);
      outputNode = graph.addNode(outputProcessor);

      graph.addConnection(inputNode->nodeId, 0, pluginNode->nodeId, 0);
      graph.addConnection(inputNode->nodeId, 1, pluginNode->nodeId, 1);

      graph.addConnection(pluginNode->nodeId, 0, outputNode->nodeId, 0);
      graph.addConnection(pluginNode->nodeId, 1, outputNode->nodeId, 1);

      // Get UI and add it to main window
      editor = instance->createEditor();
      this->setContentOwned(editor, true);

      graphPlayer.setProcessor(&graph);
      deviceManager.addAudioCallback(&graphPlayer);
    }

    void closeButtonPressed() override {
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
    AudioDeviceManager deviceManager;
    AudioPluginFormatManager formatManager;
    ScopedPointer<PluginDescription> description;
    ScopedPointer<AudioProcessorEditor> editor;
    AudioProcessorGraph graph;
    AudioProcessorPlayer graphPlayer;

    ScopedPointer<AudioProcessor> inputProcessor;
    ScopedPointer<AudioProcessor> outputProcessor;

    ScopedPointer<AudioProcessorGraph::Node> inputNode;
    ScopedPointer<AudioProcessorGraph::Node> outputNode;

    ScopedPointer<AudioProcessorGraph::Node> pluginNode;

    ScopedPointer<ApplicationProperties> appProperties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
  };

private:
  ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (vstjshostApplication)
