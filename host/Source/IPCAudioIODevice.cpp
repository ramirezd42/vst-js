//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
#include <vector>

using namespace boost::interprocess;

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName) :
AudioIODevice(deviceName, "IPC"),
Thread (deviceName),
deviceIsOpen(false),
deviceIsPlaying(false),
sharedMemorySize(10000)
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
  
  sharedMemory =  managed_shared_memory(
    boost::interprocess::open_or_create,
    "ipc-audio-io",
    65536 // TODO: this is a random number that was used in the docs. This should be something that makes sense.
  );

  std::cout << "Mapped shared memory segment (ipc-audio-io)" << std::endl;

  inputData = sharedMemory.find<IPCAudioData>("inputData").first;
  outputData = sharedMemory.find<IPCAudioData>("outputData").first;
  this->startThread(9);


  return "";
}

void IPCAudioIODevice::close() {
  // TODO: implement stub
  this->deviceIsOpen = false;
  this->stopThread(10000);

  shared_memory_object::remove("ipc-audio-io");
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

  const float** inputChannelData = getInputChannelData(inputData, numInputChannels, inputData->size()/numInputChannels);
  float** outputChannelData = getOutputChannelData(outputData, numOutputChannels, outputData->size()/numOutputChannels);
  while(! threadShouldExit()) {
  std::cout << "Input Samples (PreProcess): " << std::endl;
  for (int channel=0; channel < numInputChannels; ++channel) {
    std::cout << "Input Channel " << channel << ": " << std::endl;
    for (int sample=0; sample < inputData->size()/numInputChannels; ++sample) {
      std::cout << inputChannelData[channel][sample] << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "Output Samples (PreProcess): " << std::endl;
  for (int channel=0; channel < numOutputChannels; ++channel) {
    std::cout << "Output Channel " << channel << ": " << std::endl;
    for (int sample=0; sample < outputData->size()/numOutputChannels; ++sample) {
      std::cout << outputChannelData[channel][sample] << std::endl;
    }
  }
  std::cout << std::endl;

    if(isPlaying() && callback != nullptr) {
      callback->audioDeviceIOCallback(
        inputChannelData,
        numInputChannels,
        outputChannelData,
        numOutputChannels,
        inputData->size()/numInputChannels
      );
    }

  std::cout << "Input Samples (PostProcess): " << std::endl;
  for (int channel=0; channel < numInputChannels; ++channel) {
    std::cout << "Input Channel " << channel << ": " << std::endl;
    for (int sample=0; sample < inputData->size()/numInputChannels; ++sample) {
      std::cout << inputChannelData[channel][sample] << std::endl;
    }
  }
  std::cout << std::endl;

  std::cout << "Output Samples (PostProcess): " << std::endl;
  for (int channel=0; channel < numOutputChannels; ++channel) {
    std::cout << "Output Channel " << channel << ": " << std::endl;
    for (int sample=0; sample < outputData->size()/numOutputChannels; ++sample) {
      std::cout << outputChannelData[channel][sample] << std::endl;
    }
  }
  std::cout << std::endl;

  }

}


const float** IPCAudioIODevice::getInputChannelData(IPCAudioData *data, int numChannels, int numSamples) {
  //TODO: WHY DOES THIS NOT WORK?!?!?!
  const float* dataToCopy = data->data();
  std::vector<const float*>* returnData = new std::vector<const float*>();
  for(int channel = 0; channel < numChannels; ++channel) {
    std::vector<float>* channelData = new std::vector<float>;
    for(int sample=0; sample<numSamples; ++sample) {
      channelData->push_back(dataToCopy[(channel*numSamples) + sample]);
    }
    returnData->push_back(channelData->data());
  }
  return returnData->data();
}


float** IPCAudioIODevice::getOutputChannelData(IPCAudioData *data, int numChannels, int numSamples) {
  //TODO: WHY DOES THIS NOT WORK?!?!?!
  float* dataToCopy = data->data();
  std::vector<float*>* returnData = new std::vector<float*>();
  for(int channel = 0; channel < numChannels; ++channel) {
    std::vector<float>* channelData = new std::vector<float>;
    for(int sample=0; sample<numSamples; ++sample) {
      channelData->push_back(dataToCopy[(channel*numSamples) + sample]);
    }
    returnData->push_back(channelData->data());
  }
  return returnData->data();
}
