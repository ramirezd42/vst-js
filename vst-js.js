const vstjs = require('./build/Debug/vst-js.node');
console.log(vstjs);
const list = new vstjs.PluginList('/Library/Audio/Plug-Ins/VST3');
list.getPlugins(e => console.log(e[0].getName()));
