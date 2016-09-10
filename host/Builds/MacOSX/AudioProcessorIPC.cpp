//
// Created by David Ramirez on 9/9/16.
//

#include "AudioProcessorIPC.h"

void AudioProcessorIPC::audioIOCallback (
  const float** const inputChannelData,
  const int numInputChannels,
  float** const outputChannelData,
  const int numOutputChannels,
  const int numSamples
) {
  int totalNumChans = 0;
  if(numInputChannels > numOutputChannels) {
    // if there aren't enough output channels for the number of
    // inputs, we need to create some temporary extra ones (can't
    // use the input data in case it gets written to)
    tempBuffer.setSize (numInputChannels - numOutputChannels, numSamples,
      false, false, true);

    for (int i = 0; i < numOutputChannels; ++i)
    {
      channels[totalNumChans] = outputChannelData[i];
      memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
      ++totalNumChans;
    }

    for (int i = numOutputChannels; i < numInputChannels; ++i)
    {
      channels[totalNumChans] = tempBuffer.getWritePointer (i - numOutputChannels);
      memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
      ++totalNumChans;
    }
  }
  else
  {
    for (int i = 0; i < numInputChannels; ++i)
    {
      channels[totalNumChans] = outputChannelData[i];
      memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
      ++totalNumChans;
    }

    for (int i = numInputChannels; i < numOutputChannels; ++i)
    {
      channels[totalNumChans] = outputChannelData[i];
      zeromem (channels[totalNumChans], sizeof (float) * (size_t) numSamples);
      ++totalNumChans;
    }
  }
  AudioSampleBuffer buffer (channels, totalNumChans, numSamples);

  {
    const ScopedLock sl (lock);

    if (processor != nullptr)
    {
      const ScopedLock sl2 (processor->getCallbackLock());

      if (! processor->isSuspended())
      {
        if (processor->isUsingDoublePrecision())
        {
          conversionBuffer.makeCopyOf (buffer);
          processor->processBlock (conversionBuffer, incomingMidi);
          buffer.makeCopyOf (conversionBuffer);
        }
        else
        {
          processor->processBlock (buffer, incomingMidi);
        }

        return;
      }
    }
  }

  for (int i = 0; i < numOutputChannels; ++i)
    FloatVectorOperations::clear (outputChannelData[i], numSamples);
}