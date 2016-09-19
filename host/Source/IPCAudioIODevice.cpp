//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName) :
AudioIODevice(deviceName, "IPC"),
Thread (deviceName),
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
  // TODO: implement stub
  this->deviceIsOpen = true;
  this->startThread(9);
  return "";
}

void IPCAudioIODevice::close() {
  // TODO: implement stub
  this->deviceIsOpen = false;
  this->stopThread(10000);
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
  int numSamples = 3;
  int numInputChannels = 2;
  int numOutputChannels = 2;

  Array<const float*> inputChannels;
  Array<float*> outputChannels;

  AudioSampleBuffer buffer (numInputChannels + numOutputChannels, numSamples);

  for (int i=0; i < numInputChannels; ++i) {
    inputChannels.add(buffer.getReadPointer(i));
  }

  for (int i=0; i < numOutputChannels; ++i) {
    outputChannels.add(buffer.getWritePointer(i + numInputChannels));
  }

  while(! threadShouldExit()) {
    if(isPlaying() && callback != nullptr) {
      getNextAudioBlock(&buffer, numInputChannels, numSamples);
      callback->audioDeviceIOCallback(
        inputChannels.getRawDataPointer(),
        numInputChannels,
        outputChannels.getRawDataPointer(),
        numOutputChannels,
        numSamples
      );
    }
  }
}

void IPCAudioIODevice::getNextAudioBlock(AudioSampleBuffer* buffer, int numInputChannels, int numSamples) {
  for (int channel = 0; channel < numInputChannels; ++channel)
  {
     float* writeBuffer = buffer->getWritePointer (channel);

    for (int sample = 0; sample < numSamples; ++sample)
      writeBuffer[sample] = randomGen.nextFloat() * 0.25f - 0.125f;
  }
}

