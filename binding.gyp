{
  "targets": [
    {
      "target_name": "vst-js-bin",
      "type": "executable",
      "include_dirs": [
        "src/host",
        "shared",
        "shared/proto/build",
        "shared/JuceLibraryCode",
        "include",
        "include/JUCE/modules",
        "include/VST3",
        "include/zmq",
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "src/host/IPCAudioIODevice.cpp",
        "src/host/MainComponent.cpp",
        "src/host/Main.cpp",
        "shared/proto/build/iobuffer.pb.cc",
        "shared/JuceLibraryCode/juce_audio_basics.mm",
        "shared/JuceLibraryCode/juce_audio_devices.mm",
        "shared/JuceLibraryCode/juce_audio_formats.mm",
        "shared/JuceLibraryCode/juce_audio_processors.mm",
        "shared/JuceLibraryCode/juce_audio_utils.mm",
        "shared/JuceLibraryCode/juce_core.mm",
        "shared/JuceLibraryCode/juce_cryptography.mm",
        "shared/JuceLibraryCode/juce_data_structures.mm",
        "shared/JuceLibraryCode/juce_events.mm",
        "shared/JuceLibraryCode/juce_graphics.mm",
        "shared/JuceLibraryCode/juce_gui_basics.mm",
        "shared/JuceLibraryCode/juce_gui_extra.mm",
        "shared/JuceLibraryCode/juce_opengl.mm",
        "shared/JuceLibraryCode/juce_video.mm"
      ],
      "conditions": [[
        'OS=="mac"',
        {
          'defines': [
            '__MACOSX_CORE__'
          ],
          'link_settings': {
            'libraries': [
              '../lib/libprotobuf.a',
              '../lib/libzmq.a',
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
            'LIBRARY_SEARCH_PATHS': [ "$(inherited)", "$(SRCROOT)/lib" ],
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES',
            "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++11", "-stdlib=libc++" ],
            "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
            "MACOSX_DEPLOYMENT_TARGET": "10.7"
          }
        }
      ]]
    },
    {
      "target_name": "vst-js",
      "include_dirs": [
        "src/module",
        "shared",
        "shared/proto/build",
        "shared/JuceLibraryCode",
        "include",
        "include/JUCE/modules",
        "include/VST3",
        "include/zmq",
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "src/module/Module.cpp",
        "src/module/PluginHost.cpp",
        "shared/proto/build/iobuffer.pb.cc",
        "shared/JuceLibraryCode/juce_audio_basics.mm",
        "shared/JuceLibraryCode/juce_audio_devices.mm",
        "shared/JuceLibraryCode/juce_audio_formats.mm",
        "shared/JuceLibraryCode/juce_audio_processors.mm",
        "shared/JuceLibraryCode/juce_audio_utils.mm",
        "shared/JuceLibraryCode/juce_core.mm",
        "shared/JuceLibraryCode/juce_cryptography.mm",
        "shared/JuceLibraryCode/juce_data_structures.mm",
        "shared/JuceLibraryCode/juce_events.mm",
        "shared/JuceLibraryCode/juce_graphics.mm",
        "shared/JuceLibraryCode/juce_gui_basics.mm",
        "shared/JuceLibraryCode/juce_gui_extra.mm",
        "shared/JuceLibraryCode/juce_opengl.mm",
        "shared/JuceLibraryCode/juce_video.mm"
      ],
      "conditions": [[
        'OS=="mac"',
        {
          'defines': [
            '__MACOSX_CORE__'
          ],
          'link_settings': {
            'libraries': [
              '../lib/libprotobuf.a',
              '../lib/libzmq.a',
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
            'LIBRARY_SEARCH_PATHS': [ "$(inherited)", "$(SRCROOT)/lib" ],
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES',
            "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++11", "-stdlib=libc++" ],
            "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
            "MACOSX_DEPLOYMENT_TARGET": "10.7"
          }
        }
      ]]
    }
  ]
}
