# vst.js
## What Is It?
**vst.js** is a small native node addon that allows for instantiation of natively installed **VST3** audio plugins for use with audio processing applications written for NodeJS. In it's current form it is essentially a wrapper of a handful of [JUCE](juce.com) classes.

## WARNING
This library is in an extremely experimental state. Large portions of functionally have yet to be implemented and is currently only buildable for OSX devices.

I am currently focused on building out functionality and haven't tested the build on any device other than my MacBook Pro so it would be a small miracle if you were able to build it without any trouble.

Additionally, I'm learning C++ just to develop this library. If you have any criticisms on my approach or code quality I'd love to hear them.

## Requirements
### Steinberg VST3 SDK
Due to licensing concerns I am currently not bundling the VST3 SDK along with this project. You will need to download the SDK from [Steinbergs's Website](http://www.steinberg.net/en/company/developers.html) and place it at `~/SDKs/VST3` (This may be automated or at least pulled from an environment variable in the future).

## Usage Examples
### Getting a list of available plugins:
```javascript
const vstjs = require('./build/Debug/vst-js.node');
const list = new vstjs.PluginList('/Library/Audio/Plug-Ins/VST3');
list.getPlugins(plugins => plugins.each(
  plugin => console.log(plugin.getName())
)));
```
