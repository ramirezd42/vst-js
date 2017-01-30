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
  bufferSizes->add(100);

  bitDepths = new Array<int>();
  bitDepths->add(24);
}

String IPCAudioIODevice::open(const BigInteger &inputChannels,
                              const BigInteger &outputChannels,
                              double sampleRate, int bufferSizeSamples) {
  if (this->deviceIsOpen) {
    return "";
  }
  // TODO: implement stub
  this->deviceIsOpen = true;

  //start listening on socket address and register rpc service
  serverBuilder->AddListeningPort(socketAddress.toRawUTF8(), grpc::InsecureServerCredentials());
  serverBuilder->RegisterService(this);
  serviceInstance = serverBuilder->BuildAndStart();
  std::cout << "Server listening on " << socketAddress.toRawUTF8() << std::endl;
  return "";
}

void IPCAudioIODevice::close() {
  // TODO: implement stub
  this->deviceIsOpen = false;
  deviceIsOpen = false;
}

bool IPCAudioIODevice::isOpen() {
  // TODO: implement stub
  return this->deviceIsOpen;
}

void IPCAudioIODevice::start(AudioIODeviceCallback *callback) {
  // TODO: implement stub
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
  // TODO: implement stub
  this->deviceIsPlaying = false;
  if (callback != nullptr) {
    callback->audioDeviceStopped();
  }
}

bool IPCAudioIODevice::isPlaying() { return this->deviceIsPlaying; }

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

void IPCAudioIODevice::prepareInputData(vstjs::AudioBlock *buffer,
                                        float **destination) {
  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] =
          buffer->audiodata(channel * buffer->samplesize() + sample);
    }
  }
}

void IPCAudioIODevice::prepareOutputData(vstjs::AudioBlock *buffer,
                                         float **destination) {
  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
      destination[channel][sample] =
          buffer->audiodata(channel * buffer->samplesize() + sample);
    }
  }
}

grpc::Status IPCAudioIODevice::ProcessAudioBlock (grpc::ServerContext* context, const vstjs::AudioBlock* request,
                                vstjs::AudioBlock* reply) {
  // todo:: what was previously in run method should be in here now
  return grpc::Status::OK;
}

//
//void IPCAudioIODevice::run() {
//
//  while (!threadShouldExit()) {
//    std::string message;
//    while(true) {
//      try {
////        message = s_recv (socket);
//        break;
//      } catch(zmq::error_t err){
//        std::cout << "ZMQ Receive Error: " << zmq_strerror(err.num()) << std::endl;
//      }
//    }
//
//    vstjs::AudioBlock nextBuffer;
//    if (nextBuffer.ParseFromString(message) && isPlaying() &&
//        callback != nullptr) {
//
//      // init i/o buffers
//      float **nextInputBuffer;
//      nextInputBuffer = new float *[nextBuffer.numchannels()];
//      for (int i = 0; i < nextBuffer.numchannels(); i++) {
//        nextInputBuffer[i] = new float[nextBuffer.samplesize()];
//      }
//      this->prepareInputData(&nextBuffer, nextInputBuffer);
//
//      float **nextOutputBuffer;
//      nextOutputBuffer = new float *[nextBuffer.numchannels()];
//      for (int i = 0; i < nextBuffer.numchannels(); i++) {
//        nextOutputBuffer[i] = new float[nextBuffer.samplesize()];
//      }
//      this->prepareOutputData(&nextBuffer, nextOutputBuffer);
//
//      // pass i/o data to audio device callback
//      callback->audioDeviceIOCallback(
//          const_cast<const float **>(nextInputBuffer),
//          nextBuffer.numchannels(), nextOutputBuffer,
//          nextBuffer.numchannels(), nextBuffer.samplesize());
//
//      // copy new output data to protobuf object
//      for (int channel = 0; channel < nextBuffer.numchannels();
//           ++channel) {
//        for (int sample = 0; sample < nextBuffer.samplesize(); ++sample) {
//          nextBuffer.set_audiodata((channel * nextBuffer.samplesize()) +
//                                        sample,
//                                    nextOutputBuffer[channel][sample]);
//        }
//      }
//
//      // reserialize protobuf object and send back
//      std::string message;
//      nextBuffer.SerializeToString(&message);
//
//      while(true) {
//        try {
////          s_send (socket, message);
//          break;
//        } catch(zmq::error_t err){
//          std::cout << "ZMQ Send Error: " << zmq_strerror(err.num()) << std::endl;
//        }
//      }
//
//      // clean up i/o buffer memory
//      for (int i = 0; i < nextBuffer.numchannels(); ++i) {
//        delete[] nextInputBuffer[i];
//      }
//      delete[] nextInputBuffer;
//
//      for (int i = 0; i < nextBuffer.numchannels(); ++i) {
//        delete[] nextOutputBuffer[i];
//      }
//      delete[] nextOutputBuffer;
//    }
//  }
//}
