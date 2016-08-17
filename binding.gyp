{
  "targets": [
    {
      "target_name": "vst-js",
      "include_dirs": [
        "src",
        "include/JUCE/modules",
        "include/VST3",
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "src/vst-js.cc",
        "src/PluginInstance.cc",
        "src/JuceLibraryCode/juce_audio_basics.mm",
        "src/JuceLibraryCode/juce_audio_devices.mm",
        "src/JuceLibraryCode/juce_audio_processors.mm",
        "src/JuceLibraryCode/juce_audio_utils.mm",
        "src/JuceLibraryCode/juce_core.mm",
        "src/JuceLibraryCode/juce_cryptography.mm",
        "src/JuceLibraryCode/juce_data_structures.mm",
        "src/JuceLibraryCode/juce_events.mm",
        "src/JuceLibraryCode/juce_graphics.mm",
        "src/JuceLibraryCode/juce_gui_basics.mm",
        "src/JuceLibraryCode/juce_gui_extra.mm",
        "src/JuceLibraryCode/juce_opengl.mm",
        "src/JuceLibraryCode/juce_video.mm"
      ],
      "conditions": [[
        'OS=="mac"',
        {
          'defines': [
            '__MACOSX_CORE__'
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Accelerate.framework',
              '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
              '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
              '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreAudioKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreMIDI.framework',
              '$(SDKROOT)/System/Library/Frameworks/DiscRecording.framework',
              '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
              '$(SDKROOT)/System/Library/Frameworks/QTKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuickTime.framework',
              '$(SDKROOT)/System/Library/Frameworks/WebKit.framework'
            ]
          },
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES'
          }
        }
      ]]
    }
  ]
}
