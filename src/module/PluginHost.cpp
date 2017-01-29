 #include "PluginHost.h"
#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include "zhelpers.hpp"
#include <nan.h>

using namespace v8;

// THIS JUNK IS TEMPORARY TO TEST WITH WHITE NOISE DATA

void getNextInputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels, int numSamples, float * inputChannelData) {
  for (int channel = 0; channel < numInputChannels; ++channel) {
    for (int sample= 0; sample < numSamples * numInputChannels; ++sample) {
      buffer.add_inputdata(inputChannelData[channel+sample]);
    }
  }
}

//void getNextInputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels,
//                            int numSamples, Random &randomGen) {
//  for (int i = 0; i < numInputChannels; ++i) {
//    for (int i = 0; i < numSamples * numInputChannels; ++i) {
//      buffer.add_inputdata(randomGen.nextFloat() * 0.25f - 0.125f);
//    }
//  }
//}


void getNextOutputAudioBlock(vstjs::IOBuffer &buffer, int numInputChannels,
                             int numSamples) {
  for (int i = 0; i < numInputChannels; ++i) {
    for (int i = 0; i < numSamples * numInputChannels; ++i) {
      buffer.add_outputdata(0);
    }
  }
}

void copyNextOutputAudioBlock(const float* data, float* output, int numInputChannels, int numSamples) {
  for (int channel = 0; channel < numInputChannels; ++channel) {
    for (int sample= 0; sample < numSamples * numInputChannels; ++sample) {
      output[channel+sample] = data[channel+sample];
    }
  }
}

juce::String bufferToString(vstjs::IOBuffer &buffer) {
  juce::String returnStr = "";
  for (int channel = 0; channel < buffer.numinputchannels(); ++channel) {
    for (int sample = 0; sample < buffer.samplesize(); ++ sample) {
      returnStr += "Channel " + channel;
      returnStr += ", Sample " + sample;
      returnStr += std::to_string(buffer.inputdata().Get(buffer.samplesize()*channel + sample));
      returnStr += "\n";
//      std::cout << "Channel " << channel << ", Sample " << sample << ": " << buffer.inputdata().Get(buffer.samplesize()*channel + sample) << std::endl;
    }
  }
  return returnStr;
}

// END TEMPORARY JUNK

PluginHost::PluginHost(juce::String _socketAddress)
    : context(1), socket(this->context, ZMQ_REQ),
      socketAddress(_socketAddress){};
PluginHost::~PluginHost(){};

Nan::Persistent<v8::Function> PluginHost::constructor;

void PluginHost::Init() {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PluginHost").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeTemplate(
      tpl, "start", Nan::New<v8::FunctionTemplate>(Start)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "stop", Nan::New<v8::FunctionTemplate>(Stop)->GetFunction());

  Nan::SetPrototypeTemplate(
      tpl, "processAudioBlock",
      Nan::New<v8::FunctionTemplate>(ProcessAudioBlock)->GetFunction());

  constructor.Reset(tpl->GetFunction());
}

void PluginHost::Start(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());

  // connect socket to requested address
  obj->socket.setsockopt(ZMQ_RCVTIMEO, -1);
  obj->socket.setsockopt(ZMQ_LINGER, 0);
  std::cout << "Socket Address: " << obj->socketAddress << std::endl;
  obj->socket.connect(obj->socketAddress.toRawUTF8());
  info.GetReturnValue().Set(Nan::New("Host Started...").ToLocalChecked());
}

void PluginHost::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());
  obj->socket.disconnect(obj->socketAddress.toRawUTF8());
  info.GetReturnValue().Set(Nan::New("Host Stopped...").ToLocalChecked());
}

void PluginHost::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 1 argument");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected String");
    return;
  }

  v8::String::Utf8Value param1(info[0]->ToString());
  juce::String socketAddress = std::string(*param1);

  PluginHost *obj = new PluginHost(socketAddress);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void PluginHost::ProcessAudioBlock(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {

  if (info.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments. Expected 3 arguments");
    return;
  }

  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 1. Expected Number");
    return;
  }
  double numChannels = info[0]->NumberValue();

  if (!info[1]->IsNumber()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 2. Expected Number");
    return;
  }
  double numSamples = info[1]->NumberValue();

  if (!info[2]->IsTypedArray()) {
    Nan::ThrowTypeError("Incorrect Type for Argument 3. Expected Typed Array");
    return;
  }

  Local<TypedArray> inputDataArg = info[2].As<TypedArray>();
  Nan::TypedArrayContents<float> inputChannelData(inputDataArg);
//
//  v8::Local<v8::Array> inputChannelData = v8::Local<v8::Array>::Cast(info[2]);
//  for (uint32_t channel = 0; channel < inputChannelData->Length(); ++channel) {
//    v8::Float32Array::Cast(*inputChannelData->Get(channel));
//  }

  PluginHost *obj = ObjectWrap::Unwrap<PluginHost>(info.This());

  // THIS JUNK IS TEMPORARY TO TEST WHITE NOISE DATA
  Random randomGen;
  vstjs::IOBuffer nextBuffer;
  nextBuffer.set_samplesize(numSamples);
  nextBuffer.set_numinputchannels(numChannels);
  nextBuffer.set_numoutputchannels(numChannels);

  // generate next blocks of IO data
//  getNextInputAudioBlock(nextBuffer, numChannels, numSamples, randomGen);
  getNextInputAudioBlock(nextBuffer, numChannels, numSamples, (*inputChannelData));
  getNextOutputAudioBlock(nextBuffer, numChannels, numSamples);
//

  std::string message;
  nextBuffer.SerializeToString(&message);

  while(true) {
    try {
      s_send(obj->socket, message);
      break;
    } catch(zmq::error_t err){
      std::cout << zmq_strerror(err.num());;
    }
  }

  while(true) {
    try {
//      std::cout << bufferToString(nextBuffer);
      std::string response = s_recv(obj->socket);
      nextBuffer.ParseFromString(response);
      break;
    } catch(zmq::error_t err){
      std::cout << zmq_strerror(err.num());
    }
  }

  const float* foo = nextBuffer.inputdata().data();
//  copyNextOutputAudioBlock(foo, (*inputChannelData), numChannels, numSamples);

//  // just pass the input buffer as the output buffer for now (no-op)
  info.GetReturnValue().Set(info[2]);
}

v8::Local<v8::Object> PluginHost::NewInstance(v8::Local<v8::Value> arg) {
  Nan::EscapableHandleScope scope;
  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {arg};
  v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

  return scope.Escape(instance);
}
