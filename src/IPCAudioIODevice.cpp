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

void IPCAudioIODevice::prepareInputData(vstjs::IOBuffer* buffer, float** destination) {
  for(int channel = 0; channel < buffer->numinputchannels(); ++channel) {
    for(int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] = buffer->inputdata(channel * buffer->samplesize() + sample);
    }
  }
}

void IPCAudioIODevice::prepareOutputData(vstjs::IOBuffer* buffer, float** destination) {
  for(int channel = 0; channel < buffer->numoutputchannels(); ++channel) {
    for(int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] = buffer->outputdata(channel * buffer->samplesize() + sample);
    }
  }
}
  
void IPCAudioIODevice::run() {

  

  while(! threadShouldExit()) {
      std::string message = s_recv (socket);
      vstjs::IOBuffer nextBuffer;
      if(nextBuffer.ParseFromString(message) && isPlaying() && callback != nullptr) {

        // init i/o buffers
        float** nextInputBuffer;
        nextInputBuffer = new float*[nextBuffer.numinputchannels()];
        for (int i = 0;i<nextBuffer.numinputchannels();i++) {
          nextInputBuffer[i] = new float[nextBuffer.samplesize()];
        }
        prepareInputData(&nextBuffer, nextInputBuffer);


        float** nextOutputBuffer;
        nextOutputBuffer = new float*[nextBuffer.numoutputchannels()];
        for (int i = 0;i<nextBuffer.numoutputchannels();i++) {
          nextOutputBuffer[i] = new float[nextBuffer.samplesize()];
        }
        prepareOutputData(&nextBuffer, nextOutputBuffer);


        // pass i/o data to audio device callback
        callback->audioDeviceIOCallback(
          const_cast<const float**>(nextInputBuffer),
          nextBuffer.numinputchannels(),
          nextOutputBuffer,
          nextBuffer.numoutputchannels(),
          nextBuffer.samplesize()
        );

        // copy new output data to protobuf object
        for(int channel = 0; channel < nextBuffer.numoutputchannels(); ++channel) {
          for (int sample = 0; sample < nextBuffer.samplesize(); ++sample) {
            nextBuffer.set_outputdata((channel * nextBuffer.samplesize()) + sample, nextOutputBuffer[channel][sample]);
          }
        }

        // reserialize protobuf object and send back
        std::string message;
        nextBuffer.SerializeToString(&message);

        s_send (socket, message);

        // clean up i/o buffer memory
        for (int i = 0; i < nextBuffer.numinputchannels(); ++i) {
          delete[] nextInputBuffer[i];
        }
        delete[] nextInputBuffer;

        for (int i = 0; i < nextBuffer.numoutputchannels(); ++i) {
          delete[] nextOutputBuffer[i];
        }
        delete[] nextOutputBuffer;
    }
  }
}
