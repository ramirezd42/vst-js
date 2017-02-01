const spawn = require('child_process').spawn
const grpc = require('grpc')
const path = require('path')
const deasync = require('deasync')
const EventEmitter = require('events')

const hostBinPath = './cmake-build-debug/vstjs-bin'
const vstjsProto = grpc.load(path.join(__dirname, '/shared/proto/iobuffer.proto')).vstjs

function splitByChannel(audioData, numChannels, sampleSize) {
  return [...Array(numChannels).keys()].map((i) => {
    const start = i * sampleSize
    const end = (i * sampleSize) + sampleSize
    return audioData.slice(start, end)
  })
}

function concatenateChannels(audioData) {
  return audioData.reduce((prev, cur) => {
    const merged = new Float32Array(prev.length + cur.length)
    merged.set(prev)
    merged.set(cur, prev.length)
    return merged
  }, new Float32Array())
}

module.exports.PluginHost = class extends EventEmitter {
  constructor(pluginPath, hostAddress) {
    super()
    const rpcAudioIO = new vstjsProto.RpcAudioIO(hostAddress, grpc.credentials.createInsecure())
    this.pluginPath = pluginPath
    this.hostAddress = hostAddress
    this.processAudioBlockSync = deasync(rpcAudioIO.processAudioBlock).bind(rpcAudioIO)
  }

  launchProcess() {
    this.proc = spawn(hostBinPath, [this.pluginPath, this.hostAddress])
    this.proc.on('close', code => this.emit('close', code))
    this.proc.stdout.on('data', data => this.emit('info', data))
    this.proc.stderr.on('data', data => this.emit('error', data))
  }

  processAudioBlock(audioData) {
    const numChannels = audioData.length
    const sampleSize = audioData[0].length
    const concatenated = concatenateChannels(audioData)

    try {
      const result = this.processAudioBlockSync({
        sampleSize,
        numChannels,
        audiodata: Array.from(concatenated),
      })
      return splitByChannel(result.audiodata, numChannels, sampleSize)
    } catch (err) {
      return audioData
    }
  }

  shutDown() {
    this.proc.kill()
  }
}
