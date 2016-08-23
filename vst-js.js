const vstjs = require('./build/Debug/vst-js.node');
console.log(vstjs);
const list = new vstjs.PluginList('/Library/Audio/Plug-Ins/VST3');
list.getPlugins(e => e[0].createInstance());

setInterval(_ => console.log('ah ah ah ah stayin alive, stayin alive'), 1000)