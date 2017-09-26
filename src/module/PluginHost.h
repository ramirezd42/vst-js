#ifndef PLUGINHOST_H
#define PLUGINHOST_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/optional/optional.hpp>
#include <boost/process.hpp>
#include <iostream>
#include "SharedMemoryBuffer.h"



//Erase previous shared memory and schedule erasure on exit
struct shm_remove
{
  const char *segmentFile;
  shm_remove(const char *id): segmentFile(id) { boost::interprocess::shared_memory_object::remove(segmentFile); }
  ~shm_remove(){ boost::interprocess::shared_memory_object::remove(segmentFile); }
};

struct shm_truncate
{
  const boost::interprocess::shared_memory_object shm;
  shm_truncate():shm(){}
};

struct process_manager
{
public:
  ~process_manager() { if (child_) terminate_process(); }

  void open_process(std::string modulePath, std::string pluginPath, std::string inputShmemFile, std::string outputShmemFile)
  {
    std::string exec = modulePath + "/vstjs-bin";
    child_ = boost::process::child(exec, pluginPath, inputShmemFile, outputShmemFile);
  }

  void terminate_process() {
    if(child_) {
      child_.value().terminate();
    }
  }

private:
  boost::optional<boost::process::child> child_;
};

class PluginHost {
public:
  PluginHost(std::string _inputShmemFile, std::string _outputShmemFile, std::string pluginPath);
  ~PluginHost();
  void Start(std::string moduleDirectory);
  void Stop();
  void ProcessAudioBlock(int numChannels, int numSamples, float** inputBuffer);

  std::string inputShmemFile;
  std::string outputShmemFile;
  std::string pluginPath;
  shm_remove inputShmemRemover;
  shm_remove outputShmemRemover;
  boost::interprocess::shared_memory_object inputShmemObject;
  boost::interprocess::shared_memory_object outputShmemObject;
  std::unique_ptr<boost::interprocess::mapped_region> region;
  std::unique_ptr<boost::interprocess::mapped_region> inputRegion;
  std::unique_ptr<boost::interprocess::mapped_region> outputRegion;
  ipc_audio_buffer* inputQueue;
  ipc_audio_buffer* outputQueue;
  process_manager processManager;
  LockFreeSharedMemoryBuffer tempBuffer;
};

#endif
