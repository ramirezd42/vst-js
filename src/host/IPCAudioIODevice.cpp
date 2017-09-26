//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::interprocess;
using namespace std;
namespace bip = boost::interprocess;

void printBuffer(LockFreeSharedMemoryBuffer* buffer) {
  for (int channel = 0; channel < buffer->NumChannels; ++channel) {
    printf("Channel %d:", channel);
    for (int sample = 0; sample < buffer->BufferSize; ++sample) {
      printf("%f ", buffer->buffer[channel][sample]);
    }
    cout << "\n";
  }
}

void prepareInputData(LockFreeSharedMemoryBuffer& buf, float **destination) {
  for (int channel = 0; channel < buf.NumChannels; ++channel) {
    for (int sample = 0; sample < buf.BufferSize; ++sample) {
      destination[channel][sample] = buf.buffer[channel][sample];
    }
  }
}

void copyOutputData( float **data, LockFreeSharedMemoryBuffer& destination) {
  for (int channel = 0; channel < destination.NumChannels; ++channel) {
    for (int sample = 0; sample < destination.BufferSize; ++sample) {
       destination.buffer[channel][sample] = data[channel][sample];
    }
  }
}

IPCAudioIODevice::IPCAudioIODevice(const String &deviceName, const String _inputShmemFile, const String _outputShmemFile)
    : AudioIODevice(deviceName, "IPC"),
      Thread(deviceName),
      inputShmemFile(_inputShmemFile),
      outputShmemFile(_outputShmemFile),
      deviceIsOpen(false),
      deviceIsPlaying(false),
      inputBuffer(IPCAudioIOBuffer(SharedMemoryBuffer::NumChannels, SharedMemoryBuffer::BufferSize)),
      outputBuffer(IPCAudioIOBuffer(SharedMemoryBuffer::NumChannels, SharedMemoryBuffer::BufferSize)) {
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
}

String IPCAudioIODevice::open(const BigInteger &inputChannels,
                              const BigInteger &outputChannels,
                              double sampleRate, int bufferSizeSamples) {
  if (this->deviceIsOpen) {
    return "";
  }
  this->deviceIsOpen = true;
  this->startThread(9);
  return "";
}

void IPCAudioIODevice::close() {
  this->deviceIsOpen = false;
  this->stopThread(10000);

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
  return BigInteger(3);
}

BigInteger IPCAudioIODevice::getActiveInputChannels() const {
  return BigInteger(3);
}

int IPCAudioIODevice::getOutputLatencyInSamples() {
  return 0;
}

int IPCAudioIODevice::getInputLatencyInSamples() {
  return 0;
}

void IPCAudioIODevice::run() {

  //Create shared memory objects.
  shared_memory_object inputShmObj
    (open_only                    //only create
      , inputShmemFile.toRawUTF8()
      ,read_write                   //read-write mode
    );

  shared_memory_object outputShmObj
    (open_only                    //only create
      , outputShmemFile.toRawUTF8()
      ,read_write                   //read-write mode
    );

  try{
    //Map the whole shared memory in this process
    mapped_region inputRegion
      (inputShmObj                       //What to map
        ,read_write //Map it as read-write
      );

    mapped_region outputRegion
      (outputShmObj                       //What to map
        ,read_write //Map it as read-write
      );
    //Get the address of the mapped region
    void * inputAddr  = inputRegion.get_address();
    void * outputAddr = outputRegion.get_address();

    //Obtain a pointer to the shared structure
    ipc_audio_buffer* inputQueue = static_cast<ipc_audio_buffer*>(inputAddr);
    ipc_audio_buffer* outputQueue = static_cast<ipc_audio_buffer*>(outputAddr);
    LockFreeSharedMemoryBuffer tempBuffer;

    //Print messages until the other process marks the end
    bool end_loop = false;
    do{
      // init input buffer
      while(!inputQueue->pop(tempBuffer)){}

      prepareInputData(tempBuffer, inputBuffer.data);

      // init output buffer
      callback->audioDeviceIOCallback(
        const_cast<const float**>(inputBuffer.data),
        LockFreeSharedMemoryBuffer::NumChannels, outputBuffer.data,
        LockFreeSharedMemoryBuffer::NumChannels, LockFreeSharedMemoryBuffer::BufferSize
      );

      copyOutputData(outputBuffer.data, tempBuffer);
      outputQueue->push(tempBuffer);
    }
    while(!end_loop);
  }
  catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
}
