//
// Created by David Ramirez on 9/12/16.
//

#include "IPCAudioIODevice.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::interprocess;
using namespace std;

void printBuffer(SharedMemoryBuffer* buffer) {
  for (int channel = 0; channel < buffer->NumChannels; ++channel) {
    printf("Channel %d:", channel);
    for (int sample = 0; sample < buffer->BufferSize; ++sample) {
      printf("%f ", buffer->buffer[channel][sample]);
    }
    cout << "\n";
  }
}

void prepareInputData(SharedMemoryBuffer *buf, float **destination) {
  for (int channel = 0; channel < buf->NumChannels; ++channel) {
    for (int sample = 0; sample < buf->BufferSize; ++sample) {
      destination[channel][sample] = buf->buffer[channel][sample];
    }
  }
}

void copyOutputData( float **data, SharedMemoryBuffer *destination) {
  for (int channel = 0; channel < destination->NumChannels; ++channel) {
    for (int sample = 0; sample < destination->BufferSize; ++sample) {
       destination->buffer[channel][sample]= data[channel][sample];
    }
  }
}


IPCAudioIODevice::IPCAudioIODevice(const String &deviceName,
                                   const String _shmemFile)
    : AudioIODevice(deviceName, "IPC"),
      Thread(deviceName),
      shmemFile(_shmemFile),
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

  //Create a shared memory object.
  shared_memory_object shm
    (open_only                    //only create
      , shmemFile.toRawUTF8()
      ,read_write                   //read-write mode
    );
  try{
    //Map the whole shared memory in this process
    mapped_region region
      (shm                       //What to map
        ,read_write //Map it as read-write
      );

    //Get the address of the mapped region
    void * addr       = region.get_address();

    //Obtain a pointer to the shared structure
    SharedMemoryBuffer * data = static_cast<SharedMemoryBuffer*>(addr);

    //Print messages until the other process marks the end
    bool end_loop = false;
    do{
      scoped_lock<interprocess_mutex> lock(data->mutex);

      if(!data->message_in) {
        data->cond_empty.wait(lock);
      }
      // init input buffer
      prepareInputData(data, inputBuffer.data);

      // init output buffer
      callback->audioDeviceIOCallback(
        const_cast<const float**>(inputBuffer.data),
        data->NumChannels, outputBuffer.data,
        data->NumChannels, data->BufferSize
      );
      copyOutputData(outputBuffer.data, data);

      //Notify the other process that the buffer is empty
      data->message_in = false;
      data->cond_full.notify_one();
    }
    while(!end_loop);
  }
  catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
}
