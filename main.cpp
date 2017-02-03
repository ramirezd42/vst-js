#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include "SharedMemoryBuffer.h"

using namespace boost::interprocess;
using namespace std;

void getNextInputBuffer(SharedMemoryBuffer* buffer) {
  for(int channel = 0; channel < buffer->NumChannels; ++channel) {
    for(int sample =0; sample < buffer->BufferSize; ++sample) {
      buffer->buffer[channel][sample] = rand() / (float)RAND_MAX;
    }
  }
}

void printBuffer(SharedMemoryBuffer* buffer) {
  for(int channel = 0; channel < buffer->NumChannels; ++channel) {
    printf("Channel %d: ", channel);
    for(int sample =0; sample < buffer->BufferSize; ++sample) {
      printf("%f ", buffer->buffer[channel][sample]);
    }
    cout << "\n";
  }
}

int main ()
{

  //Erase previous shared memory and schedule erasure on exit
  struct shm_remove
  {
    shm_remove() { shared_memory_object::remove("MySharedMemory"); }
    ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
  } remover;

  //Create a shared memory object.
  shared_memory_object shm
    (create_only               //only create
      ,"MySharedMemory"           //name
      ,read_write                //read-write mode
    );
  try{
    //Set size
    shm.truncate(sizeof(SharedMemoryBuffer));

    //Map the whole shared memory in this process
    mapped_region region
      (shm                       //What to map
        ,read_write //Map it as read-write
      );

    int numBuffers = 1000;
    //Get the address of the mapped region
    void * addr       = region.get_address();

    //Construct the shared structure in memory
    SharedMemoryBuffer * data = new (addr) SharedMemoryBuffer;

    for(int i = 0; i < numBuffers; ++i){
      scoped_lock<interprocess_mutex> lock(data->mutex);
      if(data->message_in){
        cout << "Full. Waiting" << endl;
        data->cond_full.wait(lock);
      }
      if(i > 0) {
        cout << "output:" << endl;
        printBuffer(data);
        cout << endl << endl;
      }

      getNextInputBuffer(data);
      printf("Buffer %d:\n---\n", i);
      cout << "input:" << endl;
      printBuffer(data);


      //Notify to the other process that there is a message
      data->cond_empty.notify_one();

      //Mark message buffer as full
      data->message_in = true;
    }
  }
  catch(interprocess_exception &ex){
    cout << ex.what() << endl;
    return 1;
  }

  return 0;
}
