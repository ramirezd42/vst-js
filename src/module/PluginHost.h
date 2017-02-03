#ifndef PLUGINHOST_H
#define PLUGINHOST_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/optional/optional.hpp>
#include <boost/process.hpp>
#include <nan.h>
#include "SharedMemoryBuffer.h"

//Erase previous shared memory and schedule erasure on exit
struct shm_remove
{
  const char *segmentId;
  shm_remove(const char *id): segmentId(id) { boost::interprocess::shared_memory_object::remove(segmentId); }
  ~shm_remove(){ boost::interprocess::shared_memory_object::remove(segmentId); }
};

struct process_manager
{
public:
  ~process_manager() { if (child_) terminate_process(); }

  void open_process(std::string pluginPath, std::string shmemSegmentId)
  {
    std::string exec = "/Users/dxr224/Projects/vst-js/cmake-build-debug/vstjs-bin";
    child_ = boost::process::child(exec, pluginPath, shmemSegmentId);
  }

  void terminate_process() {
    if(child_) {
      child_.value().terminate();
    }
  }

private:
  boost::optional<boost::process::child> child_;
};

class PluginHost : public Nan::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  static void Start(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void ProcessAudioBlock(const Nan::FunctionCallbackInfo<v8::Value> &info);

  std::string shMemSegmentId;
  std::string pluginPath;
  shm_remove shmemRemover;
  std::unique_ptr<boost::interprocess::shared_memory_object> shm;
  std::unique_ptr<boost::interprocess::mapped_region> region;
  std::unique_ptr<SharedMemoryBuffer> shmemBuffer;
  process_manager processManager;

private:
  PluginHost(std::string _shmemSegmentId, std::string pluginPath);
  ~PluginHost();

  static Nan::Persistent<v8::Function> constructor;
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
};

#endif
