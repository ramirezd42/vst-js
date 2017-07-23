<p align="center">
  <img width="200" src="https://raw.githubusercontent.com/ramirezd42/vst-js/master/logo.png">
</p>

## What Is It?
**vst.js** is a native NodeJS addon that can be used to launch VST3 plugins (including GUI interface) in a separate process.
 

## WARNING:
This library is in an extremely experimental state. Large portions of functionally have yet to be implemented and it is currently only buildable for OSX devices, although since everything is built with crossplatform libraries, building for other operating systems shouldn't be too difficult.

Additionally, I'm learning C++ by developing this project. If you have any criticisms on my approach or code quality I'd love to hear them.

---

## Installation

### Install CMake
vst.js uses CMake as its build system and requires it be present on your system before installing. On a Mac the easiest way to install CMake is via homebrew:
```
> brew install cmake
```

### Install Boost 3.6+
vst.js makes use of the popular [Boost C++ Framework](http://boost.org). It expects that version 3.6 or higher is available on your system prior to installation. The easiset way to install the Boost framework on a Mac is via homebrew:
```
> brew install boost
```

### Acquire the Steinberg VST3 SDK
Due to licensing concerns I am currently not bundling the VST3 SDK along with this project. You will need to download the SDK from [Steinbergs's Website](http://www.steinberg.net/en/company/developers.html) and place it at `~/SDKs/VST3` 

You can also specify the installed location of the VST3 SDK by setting a the `VST3_SDK_PATH` environment variable prior to installation

### NPM Install
Once all the above dependencies have been satisfied you can install via npm:

```
> npm install vstjs
```
---
## Usage Examples
The example below will play back an audio file via [node-web-audio-api](https://github.com/sebpiq/node-web-audio-api), and manipulate the audio via a VST3 plugin

```javascript
const { AudioContext } = require('web-audio-api')
const Speaker = require('speaker')
const fs = require('fs')
const path = require('path')
const vstjs = require('vstjs')

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
  audioProcessingEvent.outputBuffer = inputBuffer
}

fs.readFile(filePath, (err, fileBuf) => {
  console.log('reading file..')
  if (err) throw err
  audioContext.decodeAudioData(fileBuf, (audioBuffer) => {
    sourceNode.buffer = audioBuffer
    sourceNode.start(0)
  }, (e) => { throw e })
})
```
