//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
#include <vector>

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName) :
AudioIODevice(deviceName, "IPC"),
Thread (deviceName),
sharedMemorySize(10000),
deviceIsOpen(false),
deviceIsPlaying(false)
{
  inputChannelNames = new StringArray();
  inputChannelNames->add("Input 1");
  inputChannelNames->add("Input 2");

  outputChannelNames = new StringArray();
  outputChannelNames->add("Output 1");
  outputChannelNames->add("Output 2");

  sampleRates = new Array<double>();
  sampleRates->add(44000);

  bufferSizes = new Array<int>();
  bufferSizes->add(100);

  bitDepths = new Array<int>();
  bitDepths->add(24);
}

String IPCAudioIODevice::open(const BigInteger &inputChannels, const BigInteger &outputChannels, double sampleRate, int bufferSizeSamples) {
  if(this->deviceIsOpen) {
    return "";
  }
  // TODO: implement stub
  this->deviceIsOpen = true;
  this->startThread(9);
  return "";
}

void IPCAudioIODevice::close() {
  // TODO: implement stub
  this->deviceIsOpen = false;
  this->stopThread(10000);

  deviceIsOpen = false;
  stopThread(10000);
}

bool IPCAudioIODevice::isOpen() {
  // TODO: implement stub
  return this->deviceIsOpen;
}

void IPCAudioIODevice::start(AudioIODeviceCallback *callback) {
  // TODO: implement stub
  if (!deviceIsPlaying) {
    if(callback != nullptr) {
      callback->audioDeviceAboutToStart(this);
    }

    deviceIsPlaying = true;
    this->callback = callback;
  }
  deviceIsPlaying = true;
}

void IPCAudioIODevice::stop() {
  // TODO: implement stub
  this->deviceIsPlaying = false;
  if(callback != nullptr) {
    callback->audioDeviceStopped();
  }
}

bool IPCAudioIODevice::isPlaying() {
  return this->deviceIsPlaying;
}

String IPCAudioIODevice::getLastError() {
  // TODO: implement stub
  return "";
}

int IPCAudioIODevice::getCurrentBufferSizeSamples() {
  // TODO: implement stub
  return this->getDefaultBufferSize();
}

double IPCAudioIODevice::getCurrentSampleRate() {
  // TODO: implement stub
  return this->sampleRates->getFirst();
}

int IPCAudioIODevice::getCurrentBitDepth() {
  // TODO: implement stub
  return this->bitDepths->getFirst();
}

BigInteger IPCAudioIODevice::getActiveOutputChannels() const {
  // TODO: implement stub
  return 11;
}

BigInteger IPCAudioIODevice::getActiveInputChannels() const {
  // TODO: implement stub
  return 11;
}

int IPCAudioIODevice::getOutputLatencyInSamples() {
  // TODO: implement stub
  return 0;
}

int IPCAudioIODevice::getInputLatencyInSamples() {
  // TODO: implement stub
  return 0;
}

void IPCAudioIODevice::run() {
  int numInputChannels = 2;
  int numOutputChannels = 2;

  const float **inputChannelData;
  float **outputChannelData;
  int numSamples = 3;

  while(! threadShouldExit()) {
    // TODO: why is this not blocking even though the other proc has locked this mutex at this point
    {
      if(isPlaying() && callback != nullptr) {
        callback->audioDeviceIOCallback(
          inputChannelData,
          numInputChannels,
          outputChannelData,
          numOutputChannels,
          numSamples
        );
      }
    }
  }
}
