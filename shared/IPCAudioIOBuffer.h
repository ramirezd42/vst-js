#ifndef IPCAUDIOIOBUFFER_H
#define IPCAUDIOIOBUFFER_H

struct IPCAudioIOBuffer {
  float** data;
  IPCAudioIOBuffer(int numChannels, int bufferSize) {
    data = new float *[numChannels];
    for (int i = 0; i < numChannels; i++) {
      data[i] = new float[bufferSize];
    }
  }
  ~IPCAudioIOBuffer() {
    delete [] data;
  }
};

#endif //VSTJS_IPCAUDIOIOBUFFER_H
