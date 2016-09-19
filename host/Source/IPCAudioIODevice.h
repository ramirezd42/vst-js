//
// Created by David Ramirez on 9/12/16.
//

#ifndef VST_JS_HOST_IPCAUDIOIODEVICE_H
#define VST_JS_HOST_IPCAUDIOIODEVICE_H

//#ifndef BOOST_DATE_TIME_NO_LIB
//#define BOOST_DATE_TIME_NO_LIB

#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include <boost/interprocess/managed_shared_memory.hpp>

class IPCAudioIODevice : public AudioIODevice, private Thread {
public:
  IPCAudioIODevice(const String & deviceName);
  ~IPCAudioIODevice() {}
  StringArray getOutputChannelNames() override { return *inputChannelNames; };
  StringArray getInputChannelNames() override { return *outputChannelNames; };
  Array<double> getAvailableSampleRates() override { return *sampleRates; };
  Array<int> getAvailableBufferSizes() { return *bufferSizes; };
  int getDefaultBufferSize() { return bufferSizes->getFirst(); };
  String open (const BigInteger &inputChannels, const BigInteger &outputChannels, double sampleRate, int bufferSizeSamples) override;
  void close() override;
  bool isOpen() override;
  void start(AudioIODeviceCallback *callback) override;
  void stop() override;
  bool isPlaying() override;
  String getLastError() override;
  int getCurrentBufferSizeSamples() override;
  double getCurrentSampleRate() override;
  int getCurrentBitDepth() override;
  BigInteger getActiveOutputChannels() const override;
  BigInteger getActiveInputChannels() const override;
  int getOutputLatencyInSamples() override;
  int getInputLatencyInSamples() override;
  void run() override;

  bool hasControlPanel() const override { return false; }
  bool showControlPanel() override { return false; }
  bool setAudioPreprocessingEnabled(bool shouldBeEnabled) override { return false; }
private:
  ScopedPointer<StringArray> inputChannelNames;
  ScopedPointer<StringArray> outputChannelNames;
  ScopedPointer<Array<double>> sampleRates;
  ScopedPointer<Array<int>> bufferSizes;
  ScopedPointer<Array<int>> bitDepths;

  ScopedPointer<AudioIODeviceCallback> callback;
  void getNextAudioBlock(AudioSampleBuffer* buffer, int numInputChannels, int numSamples);

  bool deviceIsOpen;
  bool deviceIsPlaying;
  int bufferSize = 0;

  Random randomGen;
};

class IPCAudioIODeviceType : public AudioIODeviceType
{
public:
  IPCAudioIODeviceType() : AudioIODeviceType ("IPC") {
    deviceNames = new StringArray();
    deviceNames->add("default");
  }
  void scanForDevices() override { /* todo: implement stub */ }
  StringArray getDeviceNames(bool wantInputNames = false) const override { return *deviceNames; }
  int getDefaultDeviceIndex(bool forInput) const override { return 0; }
  int getIndexOfDevice (AudioIODevice* device, bool asInput) const override { return 0; }
  bool hasSeparateInputsAndOutputs() const override { return false; }
  AudioIODevice* createDevice(const String& outputDeviceName, const String& inputDeviceName) override {
    return new IPCAudioIODevice(deviceNames->getReference(0));
  }
private:
  ScopedPointer<StringArray> deviceNames;
};

#endif //VST_JS_HOST_IPCAUDIOIODEVICE_H
