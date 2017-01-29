const AudioContext = require('web-audio-api').AudioContext
  , Speaker = require('speaker')

const fs = require('fs')
const spawn = require('child_process').spawn

const grpc = require('grpc')
const vstjsProto = grpc.load('./shared/proto/iobuffer.proto').vstjs
const Sync = require('sync')

const bufferSize = 512
const numInputChannels = 2
const numOutputChannels = 2

// launch a new plugin Process
const pluginPath = '/Library/Audio/Plug-Ins/VST3/PrimeEQ.vst3'
const hostAddress = 'tcp://127.0.0.1:5561'
const rpcAudioIO = new vstjsProto.rpcAudioIO(hostAddress,grpc.credentials.createInsecure())
const proc = spawn('/Users/dxr224/Projects/vst-js/vstjs-bin', [pluginPath, hostAddress])

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

let loop = 0;
scriptNode.onaudioprocess = function(audioProcessingEvent) {

  // The input buffer is the song we loaded earlier
  const inputBuffer = audioProcessingEvent.inputBuffer
  let inputData = new Float32Array()
  for (var channel = 0; channel < inputBuffer.numberOfChannels; channel++) {
    const channelData = inputBuffer.getChannelData(channel)
    const merged = new Float32Array(inputData.length + channelData.length)
    merged.set(inputData)
    merged.set(channelData, inputData.length)
    inputData = merged
  }

  rpcAudioIO.processAudioBlock({
    sampleSize: 512,
    numChannels: 2,
    inputData: Array.from(inputData),
  }, function(response) {
    console.log(response.outputData)
  })

  audioProcessingEvent.outputBuffer = inputBuffer

  loop++
  console.log(`Block ${loop} processed`)
}


fs.readFile(__dirname + '/test.wav', function(err, fileBuf) {
  console.log('reading file..')
  if (err) throw err
  audioContext.decodeAudioData(fileBuf, function (audioBuffer) {
    sourceNode.buffer = audioBuffer
    sourceNode.start(0)
  }, function (err) { throw err })
})

proc.stdout.on('data', (data) => {
  console.log(`stdout: ${data}`);
});

proc.stderr.on('data', (data) => {
  console.log(`stderr: ${data}`);
});

proc.on('close', (code) => {
  console.log(`child process exited with code ${code}`);
});
