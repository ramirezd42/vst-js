//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
using vstjs::RpcAudioIO;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName,
                                   const String _socketAddress)
    : AudioIODevice(deviceName, "IPC"),
      socketAddress(_socketAddress), deviceIsOpen(false), context(1),
      deviceIsPlaying(false) {
  inputChannelNames = new StringArray();
  inputChannelNames->add("Input 1");
  inputChannelNames->add("Input 2");

  outputChannelNames = new StringArray();
  outputChannelNames->add("Output 1");
  outputChannelNames->add("Output 2");

  sampleRates = new Array<double>();
  sampleRates->add(44000);

  bufferSizes = new Array<int>();
  bufferSizes->add(512);

  bitDepths = new Array<int>();
  bitDepths->add(24);

  serverBuilder = new ServerBuilder();
}

String IPCAudioIODevice::open(const BigInteger &inputChannels,
                              const BigInteger &outputChannels,
                              double sampleRate, int bufferSizeSamples) {
  if (this->deviceIsOpen) {
    return "";
  }
  this->deviceIsOpen = true;

  //start listening on socket address and register rpc service
  this->serverBuilder->AddListeningPort(this->socketAddress.toRawUTF8(), grpc::InsecureServerCredentials());
  this->serverBuilder->RegisterService(this);
  this->serviceInstance = serverBuilder->BuildAndStart();
  std::cout << "Server listening on " << this->socketAddress.toRawUTF8() << std::endl;
  return "";
}

void IPCAudioIODevice::close() {
  this->deviceIsOpen = false;
  deviceIsOpen = false;
}

bool IPCAudioIODevice::isOpen() {
  return this->deviceIsOpen;
}

void IPCAudioIODevice::start(AudioIODeviceCallback *callback) {
  if (!deviceIsPlaying) {
    if (callback != nullptr) {
      callback->audioDeviceAboutToStart(this);
    }

    deviceIsPlaying = true;
    this->callback = callback;
  }
  deviceIsPlaying = true;
}

void IPCAudioIODevice::stop() {
  this->deviceIsPlaying = false;
  if (callback != nullptr) {
    callback->audioDeviceStopped();
  }
}

bool IPCAudioIODevice::isPlaying() { return this->deviceIsPlaying; }

String IPCAudioIODevice::getLastError() {
  return "";
}

int IPCAudioIODevice::getCurrentBufferSizeSamples() {
  return this->getDefaultBufferSize();
}

double IPCAudioIODevice::getCurrentSampleRate() {
  return this->sampleRates->getFirst();
}

int IPCAudioIODevice::getCurrentBitDepth() {
  return this->bitDepths->getFirst();
}

BigInteger IPCAudioIODevice::getActiveOutputChannels() const {
  return 11;
}

BigInteger IPCAudioIODevice::getActiveInputChannels() const {
  return 11;
}

int IPCAudioIODevice::getOutputLatencyInSamples() {
  return 0;
}

int IPCAudioIODevice::getInputLatencyInSamples() {
  return 0;
}

void IPCAudioIODevice::prepareInputData(const vstjs::AudioBlock *buffer,
                                        float **destination) {
  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] = buffer->audiodata(channel * buffer->samplesize() + sample);
    }
  }
}

void IPCAudioIODevice::prepareOutputData(const vstjs::AudioBlock *buffer,
                                         float **destination) {
  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] = 0;
    }
  }
}

grpc::Status IPCAudioIODevice::ProcessAudioBlock (grpc::ServerContext* context, const vstjs::AudioBlock* request,
                                vstjs::AudioBlock* reply) {

  if ( isPlaying() && callback != nullptr) {

    // init i/o buffers
    float **nextInputBuffer;
    nextInputBuffer = new float *[request->numchannels()];
    for (int i = 0; i < request->numchannels(); i++) {
      nextInputBuffer[i] = new float[request->samplesize()];
    }
    this->prepareInputData(request, nextInputBuffer);

    float **nextOutputBuffer;
    nextOutputBuffer = new float *[request->numchannels()];
    for (int i = 0; i < request->numchannels(); i++) {
      nextOutputBuffer[i] = new float[request->samplesize()];
    }
    this->prepareOutputData(request, nextOutputBuffer);

    // pass i/o data to audio device callback
    callback->audioDeviceIOCallback(
      const_cast<const float **>(nextInputBuffer),
      request->numchannels(), nextOutputBuffer,
      request->numchannels(), request->samplesize());

    // copy new output data to protobuf object
    for (int channel = 0; channel < request->numchannels();
         ++channel) {
      for (int sample = 0; sample < request->samplesize(); ++sample) {
        reply->add_audiodata(nextOutputBuffer[channel][sample]);
      }
    }
  }
}
