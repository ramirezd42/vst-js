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


IPCAudioIODevice::IPCAudioIODevice(const String &deviceName,
                                   const String _shmemSegmentId)
    : AudioIODevice(deviceName, "IPC"),
      Thread(deviceName),
      shmemSegmentId(_shmemSegmentId),
      deviceIsOpen(false),
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

void IPCAudioIODevice::run() {

  //Create a shared memory object.
  shared_memory_object shm
    (open_only                    //only create
      , shmemSegmentId.toRawUTF8()
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
      if(!data->message_in){
        data->cond_empty.wait(lock);
      }
      else{
        //Print the message
//        cout << "Buffer\n---\ninput:" << endl;
//        printBuffer(data);


        // init input buffer
        // todo: have a single outputbuffer as a private class member and just re-use that every time
        float **inputBuffer;
        inputBuffer = new float *[data->NumChannels];
        for (int i = 0; i < data->NumChannels; i++) {
          inputBuffer[i] = new float[data->BufferSize];
        }
        prepareInputData(data, inputBuffer);

        // init output buffer
        // todo: have a single outputbuffer as a private class member and just re-use that every time
        float **outputBuffer;
        outputBuffer = new float *[data->NumChannels];
        for (int i = 0; i < data->NumChannels; i++) {
          outputBuffer[i] = new float[data->BufferSize];
        }

        callback->audioDeviceIOCallback(
          const_cast<const float**>(inputBuffer),
          data->NumChannels, outputBuffer,
          data->NumChannels, data->BufferSize
        );
//        cout << "output:" << endl;
//        printBuffer(data);
//        cout << endl << endl;

        //Notify the other process that the buffer is empty
        data->message_in = false;
        data->cond_full.notify_one();

        //cleanup output buffer
        for (int i = 0; i < data->NumChannels; ++i) {
          delete[] outputBuffer[i];
        }
        delete[] outputBuffer;
      }
    }
    while(!end_loop);
  }
  catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
}

//void IPCAudioIODevice::prepareInputData(const vstjs::AudioBlock *buffer,
//                                        float **destination) {
//  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
//    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
//      destination[channel][sample] = buffer->audiodata(channel * buffer->samplesize() + sample);
//    }
//  }
//}

//
//void IPCAudioIODevice::prepareOutputData(const vstjs::AudioBlock *buffer,
//                                         float **destination) {
//  for (int channel = 0; channel < buffer->numchannels(); ++channel) {
//    for (int sample = 0; sample < buffer->samplesize(); ++sample) {
//      destination[channel][sample] = 0;
//    }
//  }
//}

//
//grpc::Status IPCAudioIODevice::ProcessAudioBlock (grpc::ServerContext* context, const vstjs::AudioBlock* request,
//                                vstjs::AudioBlock* reply) {
//
//  if ( isPlaying() && callback != nullptr) {
//
//    // init i/o buffers
//    float **nextInputBuffer;
//    nextInputBuffer = new float *[request->numchannels()];
//    for (int i = 0; i < request->numchannels(); i++) {
//      nextInputBuffer[i] = new float[request->samplesize()];
//    }
//    this->prepareInputData(request, nextInputBuffer);
//
//    float **nextOutputBuffer;
//    nextOutputBuffer = new float *[request->numchannels()];
//    for (int i = 0; i < request->numchannels(); i++) {
//      nextOutputBuffer[i] = new float[request->samplesize()];
//    }
//    this->prepareOutputData(request, nextOutputBuffer);
//
//    // pass i/o data to audio device callback
//    callback->audioDeviceIOCallback(
//      const_cast<const float **>(nextInputBuffer),
//      request->numchannels(), nextOutputBuffer,
//      request->numchannels(), request->samplesize());
//
//    // copy new output data to protobuf object
//    for (int channel = 0; channel < request->numchannels();
//         ++channel) {
//      for (int sample = 0; sample < request->samplesize(); ++sample) {
//        reply->add_audiodata(nextOutputBuffer[channel][sample]);
//      }
//    }
//  }
//}
