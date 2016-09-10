//
// Created by David Ramirez on 9/9/16.
//

#ifndef VST_JS_HOST_AUDIOPROCESSORIPC_H
#define VST_JS_HOST_AUDIOPROCESSORIPC_H

#include "../../shared/JuceLibraryCode/JuceHeader.h"

class AudioProcessorIPC {
public:
  AudioProcessorIPC (ScopedPointer<AudioProcessor> _processor, double _sampleRate, int _blockSize, bool _supportDoublePrecision, int _numInputChans, int _numOutputChans) :
  processor(_processor),
  sampleRate(_sampleRate),
  blockSize(_blockSize),
  supportDoublePrecision(_supportDoublePrecision),
  numInputChans(_numInputChans),
  numOutputChans(_numOutputChans)
  {
    const int numInBuses = processor->busArrangement.inputBuses.size();
    const int numOutBuses = processor->busArrangement.outputBuses.size();
    for (int i = 1; i < numInBuses; ++i)
    {
      bool success = processor->setPreferredBusArrangement (true, i, AudioChannelSet::disabled());

      // if using in audio processor player, it must be possible to disable sidechains
      jassert (success);

      ignoreUnused (success);
    }

    for (int i = 1; i < numOutBuses; ++i)
    {
      bool success = processor->setPreferredBusArrangement (false, i, AudioChannelSet::disabled());

      // if using in audio processor player, it must be possible to disable aux outputs
      jassert (success);

      ignoreUnused(success);
    }

    if (numInBuses > 0 && processor->busArrangement.inputBuses.getReference(0).channels.size() != numInputChans)
      processor->setPreferredBusArrangement (true,  0, AudioChannelSet::canonicalChannelSet(numInputChans));

    if (numOutBuses > 0 && processor->busArrangement.outputBuses.getReference(0).channels.size() != numOutputChans)
      processor->setPreferredBusArrangement (false,  0, AudioChannelSet::canonicalChannelSet(numOutputChans));

    jassert (processor->getTotalNumInputChannels()  == numInputChans);
    jassert (processor->getTotalNumOutputChannels() == numOutputChans);

    processor->setRateAndBufferSizeDetails (sampleRate, blockSize);

    const bool supportsDouble = processor->supportsDoublePrecisionProcessing() && supportDoublePrecision;
    AudioProcessor::ProcessingPrecision precision = supportsDouble ? AudioProcessor::doublePrecision
                                                                   : AudioProcessor::singlePrecision;
    processor->setProcessingPrecision(precision);
    processor->prepareToPlay(sampleRate, blockSize);
  }
  bool supportDoublePrecision;
private:
  void audioIOCallback (
    const float ** const inputChannelData,
    const int numInputChannels,
    float** const outputChannelData,
    const int numOutputChannels,
    const int numSamples
  );
  ScopedPointer<AudioProcessor> processor;
  CriticalSection lock;

  double sampleRate;
  int blockSize;
  int numInputChans;
  int numOutputChans;

  MidiBuffer incomingMidi;
  HeapBlock<float*> channels;
  AudioBuffer<float> tempBuffer;
  AudioBuffer<double> conversionBuffer;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorIPC)
};


#endif //VST_JS_HOST_AUDIOPROCESSORIPC_H
