const vstjs = require('./build/Debug/vst-js.node')
const bufferSize = 4096
const numInputChannels = 2
const numOutputChannels = 2

const AudioContext = require('web-audio-api').AudioContext
const audioCtx = new AudioContext()
const scriptNode = audioCtx.createScriptProcessor('/Library/Audio/Plug-Ins/VST3', bufferSize, numInputChannels, numOutputChannels)
const pluginInstance = vstjs.launchPlugin('./build/Debug/vst-js.node', bufferSize, numInputChannels, numOutputChannels)

scriptNode.onaudioprocess = function(audioProcessingEvent) {
  const inputBuffer = [].concat.apply([], audioProcessingEvent.inputBuffer)
  const outputBuffer = [].concat.apply([], audioProcessingEvent.outputBuffer)
  pluginInstance.processAudio(inputBuffer, outputBuffer)
}

scriptNode.onaudioprocess = function(audioProcessingEvent) {
  const inputBuffer = audioProcessingEvent.inputBuffer;
  let outputBuffer = audioProcessingEvent.outputBuffer;
}

pluginInstance.start()
pluginInstance.displayGUI()
pluginInstance.hideGUI()
pluginInstance.stop()
pluginInstance.destroy()

// const vstjs = require('./build/Debug/vst-js.node');
// console.log(vstjs);
// const list = new vstjs.PluginList('/Library/Audio/Plug-Ins/VST3');
//
// const bufferSize = 4096;
// const numInputChannels = 2;
// const numOutputChannels = 2;
// const audioCtx = new AudioContext();
//
// list.getPlugins(pluginList => {
//   const pluginDescription = pluginList[0]
//   const pluginInstance = pluginDescription.createInstance(bufferSize, numInputChannels, numOutputChannels)
//   var scriptNode = audioCtx.createScriptProcessor(bufferSize, numInputChannels, numOutputChannels);
//
//   scriptNode.onaudioprocess = function(audioProcessingEvent) {
//     const inputBuffer = [].concat.apply([], audioProcessingEvent.inputBuffer);
//     const outputBuffer = [].concat.apply([], audioProcessingEvent.outputBuffer);
//     pluginInstance.processAudio(inputBuffer, outputBuffer)
//   }
//
//   pluginInstance.displayGUI()
//
// );
//
// setInterval(_ => console.log('ah ah ah ah stayin alive, stayin alive'), 1000)
