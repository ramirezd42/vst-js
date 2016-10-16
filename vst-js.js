const vstjs = require('./build/Debug/vst-js.node')
const AudioContext = require('web-audio-api').AudioContext
  , Speaker = require('speaker')

const fs = require('fs')

const bufferSize = 4096
const numInputChannels = 2
const numOutputChannels = 2

// setup webaudio stuff
const audioContext = new AudioContext
var sourceNode = audioContext.createBufferSource();
const scriptNode = audioContext.createScriptProcessor(bufferSize, numInputChannels, numOutputChannels)

audioContext.outStream = new Speaker({
  channels: audioContext.format.numberOfChannels,
  bitDepth: audioContext.format.bitDepth,
  sampleRate: audioContext.sampleRate
})

sourceNode.connect(scriptNode)
scriptNode.connect(audioContext.destination)

// launch a new plugin Process
const host = vstjs.createHost()
host.start()

const instance = host.launchPlugin('/Library/Audio/Plug-Ins/VST3')

// display gui window to user
instance.displayGUI()

scriptNode.onaudioprocess = function(audioProcessingEvent) {
  // The input buffer is the song we loaded earlier
  var inputBuffer = audioProcessingEvent.inputBuffer

  // The output buffer contains the samples that will be modified and played
  audioProcessingEvent.outputBuffer = instance.processAudioBlock(inputBuffer)

  // // Loop through the output channels (in this case there is only one)
  // for (var channel = 0; channel < outputBuffer.numberOfChannels; channel++) {
  //   var inputData = inputBuffer.getChannelData(channel);
  //   var outputData = outputBuffer.getChannelData(channel);

  //   // Loop through the 4096 samples
  //   for (var sample = 0; sample < inputBuffer.length; sample++) {
  //     // make output equal to the same as the input
  //     outputData[sample] = inputData[sample];
  //   }
  // }
}

fs.readFile(__dirname + '/test.wav', function(err, fileBuf) {
  console.log('reading file..')
  if (err) throw err
  audioContext.decodeAudioData(fileBuf, function (audioBuffer) {
    console.log(sourceNode)
    sourceNode.buffer = audioBuffer
    sourceNode.start(0)
  }, function (err) { throw err })
})

// hide gui window (processing will still happen)
instance.hideGUI()

// stop plugin instance when you're done
instance.stop()

// stop plugin host when you're done
setTimeout(() => host.stop(), 5000)
