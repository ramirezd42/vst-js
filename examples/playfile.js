/* eslint-disable import/no-extraneous-dependencies */
/* eslint-disable no-console */

const { AudioContext } = require('web-audio-api')
const Speaker = require('speaker')
const fs = require('fs')
const path = require('path')
const vstjs = require('../build/Release/vstjs.node')

const bufferSize = 512
const numChannels = 2
const pluginPath = process.argv[2]
const filePath = process.argv[3]

const pluginHost = vstjs.launchPlugin(pluginPath)
pluginHost.start()

// setup webaudio stuff
const audioContext = new AudioContext()
const sourceNode = audioContext.createBufferSource()
const scriptNode = audioContext.createScriptProcessor(bufferSize, numChannels, numChannels)


audioContext.outStream = new Speaker({
  channels: audioContext.format.numberOfChannels,
  bitDepth: audioContext.format.bitDepth,
  sampleRate: audioContext.sampleRate,
})

sourceNode.connect(scriptNode)
scriptNode.connect(audioContext.destination)


scriptNode.onaudioprocess = function onaudioprocess(audioProcessingEvent) {
  const inputBuffer = audioProcessingEvent.inputBuffer
  const channels = [...Array(numChannels).keys()]
    .map(i => audioProcessingEvent.inputBuffer.getChannelData(i))

  // process audio block via pluginHost
  pluginHost.processAudioBlock(numChannels, bufferSize, channels)
  audioProcessingEvent.outputBuffer = inputBuffer // eslint-disable-line no-param-reassign
}

fs.readFile(filePath, (err, fileBuf) => {
  console.log('reading file..')
  if (err) throw err
  audioContext.decodeAudioData(fileBuf, (audioBuffer) => {
    sourceNode.buffer = audioBuffer
    sourceNode.start(0)
  }, (e) => { throw e })
})
