#ifndef SHAREDMEMORYBUFFER_H
#define SHAREDMEMORYBUFFER_H

#include <boost/interprocess/sync/interprocess_condition.hpp>

struct SharedMemoryBuffer {
    static const int NumChannels = 2;
    static const int BufferSize = 512;

    SharedMemoryBuffer(): message_in(false) {}

    float buffer[NumChannels][BufferSize];
    boost::interprocess::interprocess_mutex mutex;
    boost::interprocess::interprocess_condition  cond_full;
    boost::interprocess::interprocess_condition  cond_empty;
    bool message_in;
};

#endif //VSTJS_SHAREDMEMORYBUFFER_H
