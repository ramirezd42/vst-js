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

  void open_process(std::string modulePath, std::string pluginPath, std::string shmemFile)
  {
    std::string exec = modulePath + "/vstjs-bin";
    child_ = boost::process::child(exec, pluginPath, shmemFile);
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
  PluginHost(std::string _shmemFile, std::string pluginPath);
  ~PluginHost();
  void Start(std::string moduleDirectory);
  void Stop();
  void ProcessAudioBlock(int numChannels, int numSamples, float** inputBuffer);

  std::string shmemFile;
  std::string pluginPath;
  shm_remove shmemRemover;
  boost::interprocess::shared_memory_object shm;
  std::unique_ptr<boost::interprocess::mapped_region> region;
  SharedMemoryBuffer* shmemBuffer;
  process_manager processManager;
};

#endif
