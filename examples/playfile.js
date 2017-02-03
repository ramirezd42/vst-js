/* eslint-disable import/no-extraneous-dependencies */
/* eslint-disable no-console */

const AudioContext = require('web-audio-api').AudioContext
const AudioBuffer = require('web-audio-api').AudioBuffer
const Speaker = require('speaker')
const fs = require('fs')
const path = require('path')

const bufferSize = 512
const numChannels = 2
const pluginPath = '/Library/Audio/Plug-Ins/VST3/PrimeEQ.vst3'
// const hostAddress = '0.0.0.0:50051'
// const  = require('../index').PluginHost
//
// // launch plugin host process
// const pluginHost = new PluginHost(pluginPath, hostAddress)
// pluginHost.launchProcess()
const vstjs = require('../build/Release/vstjs.node')
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
  const hrstart = process.hrtime()
  const inputBuffer = audioProcessingEvent.inputBuffer
  const channels = [...Array(numChannels).keys()]
    .map(i => audioProcessingEvent.inputBuffer.getChannelData(i))

  // process audio block via pluginHost
  const output = pluginHost.processAudioBlock(numChannels, bufferSize, channels)
  const outputBuffer = AudioBuffer.fromArray(output, inputBuffer.sampleRate)
  audioProcessingEvent.outputBuffer = outputBuffer // eslint-disable-line no-param-reassign

  const hrend = process.hrtime(hrstart)
  console.info('Execution time (hr): %ds %dms', hrend[0], hrend[1] / 1000000)
}

fs.readFile(path.resolve(__dirname, './test.wav'), (err, fileBuf) => {
  console.log('reading file..')
  if (err) throw err
  audioContext.decodeAudioData(fileBuf, (audioBuffer) => {
    sourceNode.buffer = audioBuffer
    sourceNode.start(0)
  }, (e) => { throw e })
})

// pluginHost.on('info', (data) => {
//   console.log(`stdout: ${data}`)
// })
//
// pluginHost.on('error', (data) => {
//   console.log(`stderr: ${data}`)
// })
//
// pluginHost.on('close', (code) => {
//   console.log(`child process exited with code ${code}`)
// })
