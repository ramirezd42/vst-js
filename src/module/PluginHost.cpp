#include "PluginHost.h"
#include <iostream>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/atomic.hpp>

using namespace std;
using namespace boost::process;
using namespace boost::interprocess;

PluginHost::PluginHost(std::string _inputShmemFile, std::string _outputShmemFile, std::string _pluginPath) :
    inputShmemFile(_inputShmemFile),
    outputShmemFile(_outputShmemFile),
    pluginPath(_pluginPath),
    inputShmemRemover(inputShmemFile.data()),
    outputShmemRemover(outputShmemFile.data()),
    inputShmemObject(open_or_create, inputShmemFile.data(), read_write),
    outputShmemObject(open_or_create, outputShmemFile.data(), read_write)
{

  try {

    //Set size
    inputShmemObject.truncate(sizeof(ipc_audio_buffer));
    outputShmemObject.truncate(sizeof(ipc_audio_buffer));

    inputRegion = std::unique_ptr<mapped_region> (new mapped_region(
      inputShmemObject,
      read_write
    ));

    outputRegion = std::unique_ptr<mapped_region> (new mapped_region(
      outputShmemObject,
      read_write
    ));

    //Construct the shared structure in memory
    inputQueue = new(inputRegion->get_address()) ipc_audio_buffer;
    outputQueue = new(outputRegion->get_address()) ipc_audio_buffer;
  } catch(interprocess_exception &ex){
    cout << ex.what() << endl;
  }
};
PluginHost::~PluginHost(){};

void PluginHost::Start(std::string moduleDirectory) {
  // launch child process, with specified plugin and memory file
  processManager.open_process(moduleDirectory, pluginPath, inputShmemFile, outputShmemFile);
}

void PluginHost::Stop() {
  processManager.terminate_process();
}

void PluginHost::ProcessAudioBlock(int numChannels, int numSamples, float** buffer) {

  for (int channel = 0; channel < numChannels; channel++) {
    for (int sample = 0; sample < numSamples; sample++) {
      tempBuffer.buffer[channel][sample] = buffer[channel][sample];
    }
  }
  inputQueue->push(tempBuffer);

  while(!outputQueue->pop(tempBuffer)){}

  for (int channel = 0;  channel < numChannels; channel++) {
    for (int sample = 0; sample < numSamples; sample++) {
      buffer[channel][sample] = tempBuffer.buffer[channel][sample];
    }
  }
}

