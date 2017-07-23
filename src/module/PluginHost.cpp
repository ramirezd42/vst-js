#include "PluginHost.h"
#include <iostream>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace std;
using namespace boost::process;
using namespace boost::interprocess;

PluginHost::PluginHost(std::string _shmemFile, std::string _pluginPath)
  : shmemFile(_shmemFile),
    pluginPath(_pluginPath),
    shmemRemover(_shmemFile.data()),
    shm(create_only, shmemFile.data(), read_write)
{

  try {

    //Set size
    shm.truncate(sizeof(SharedMemoryBuffer));

    region = unique_ptr<mapped_region> (new mapped_region(
      shm,
      read_write
    ));

    //Construct the shared structure in memory
    shmemBuffer = new(region->get_address()) SharedMemoryBuffer;
  } catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
};
PluginHost::~PluginHost(){};

void PluginHost::Start(std::string moduleDirectory) {
  // launch child process, with specified plugin and memory file
  processManager.open_process(moduleDirectory, pluginPath, shmemFile);
}

void PluginHost::Stop() {
  processManager.terminate_process();
}

void PluginHost::ProcessAudioBlock(int numChannels, int numSamples, float** buffer) {

  scoped_lock<interprocess_mutex> lock(shmemBuffer->mutex);

  for (int channel = 0; channel < numChannels; channel++) {
    for (int sample = 0; sample < numSamples; sample++) {
      shmemBuffer->buffer[channel][sample] = buffer[channel][sample];
    }
  }

  // wait for child process to write to the buffer
  shmemBuffer->cond_empty.notify_one();
  shmemBuffer->message_in = true;

  if(shmemBuffer->message_in) {
    shmemBuffer->cond_full.wait(lock);
  }

  for (int channel = 0;  channel < numChannels; channel++) {
    for (int sample = 0; sample < numSamples; sample++) {
      buffer[channel][sample] = shmemBuffer->buffer[channel][sample];
    }
  }
}

