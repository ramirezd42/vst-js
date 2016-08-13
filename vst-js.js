var vstjs
try {
  vstjs = require('./build/Release/vst-js.node');
} catch (err) {
  vstjs = require('./build/Debug/vst-js.node');
}

vstjs.listPlugins(r => console.log(r))
