//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
#include <vector>
#include "zhelpers.hpp"

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName) :
AudioIODevice(deviceName, "IPC"),
Thread (deviceName),
deviceIsOpen(false),
context(1),
socket(context, ZMQ_REP),
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

  socket.bind("tcp://127.0.0.1:5560");
  this->startThread(9);
  return "";
}

void IPCAudioIODevice::close() {
  // TODO: implement stub
  this->deviceIsOpen = false;
  this->stopThread(10000);

  deviceIsOpen = false;
  stopThread(10000);
  socket.close();
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

void IPCAudioIODevice::prepareInputData(vstjs::IOBuffer* buffer) {
  nextInputBuffer.clear();
  for(int channel = 0; channel < buffer->numinputchannels(); ++channel) {
    Array<float>* channelData = new Array<float>;
    for(int sample = 0; sample < buffer->samplesize(); ++sample) {
      channelData->add(buffer->inputdata(channel * buffer->samplesize() + sample));
    }
    nextInputBuffer.add(channelData->getRawDataPointer());
  }
}

void IPCAudioIODevice::prepareOutputData(vstjs::IOBuffer* buffer) {
  nextOutputBuffer.clear();
  for(int channel = 0; channel < buffer->numoutputchannels(); ++channel) {
    Array<float>* channelData = new Array<float>;
    for(int sample = 0; sample < buffer->samplesize(); ++sample) {
      channelData->add(buffer->outputdata(channel * buffer->samplesize() + sample));
    }
    nextOutputBuffer.add(channelData->getRawDataPointer());
  }
}
  
void IPCAudioIODevice::run() {

  

  while(! threadShouldExit()) {
      std::string message = s_recv (socket);
      vstjs::IOBuffer nextBuffer;
      if(nextBuffer.ParseFromString(message) && isPlaying() && callback != nullptr) {

        prepareInputData(&nextBuffer);
        prepareOutputData(&nextBuffer);

        callback->audioDeviceIOCallback(
          nextInputBuffer.getRawDataPointer(),
          nextBuffer.numinputchannels(),
          nextOutputBuffer.getRawDataPointer(),
          nextBuffer.numoutputchannels(),
          nextBuffer.samplesize()
        );

        for(int channel = 0; channel < nextBuffer.numoutputchannels(); ++channel) {
          for (int sample = 0; sample < nextBuffer.samplesize(); ++sample) {
            nextBuffer.set_outputdata((channel * nextBuffer.samplesize()) + sample, nextOutputBuffer[channel][sample]);
          }
        }

        std::string message;
        nextBuffer.SerializeToString(&message);

        s_send (socket, message);
    }
  }
}
