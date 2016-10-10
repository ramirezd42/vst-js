export class PluginInstance {

  stop(): boolean {
    return true
  }

  displayGUI(): boolean {
    return true
  }

  hideGUI(): boolean {
    return true
  }

  processAudioBlock(inputBuffer: number[][]): number[][] {
    return inputBuffer
  }
}

export class PluginHost {
 start(): boolean {
   return true
 }

 stop(): boolean {
   return true
 }

 launchPlugin(pluginPath: string): PluginInstance {
  // todo: implement stub
  return new PluginInstance()
 }
}
