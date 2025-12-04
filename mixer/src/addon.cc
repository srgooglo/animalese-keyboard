#include "mixer.h"
#include <nan.h>

class PulseMixer : public Nan::ObjectWrap {
public:
  static void Init(v8::Local<v8::Object> exports);

private:
  explicit PulseMixer(const char *socket, const char *cookie);
  ~PulseMixer();

  Mixer *mixer_handle;

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Play(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Stop(const Nan::FunctionCallbackInfo<v8::Value> &info);
};

PulseMixer::PulseMixer(const char *socket, const char *cookie) {
  this->mixer_handle = mixer_create(socket, cookie);
}

PulseMixer::~PulseMixer() {
  if (this->mixer_handle) {
    mixer_destroy(this->mixer_handle);
  }
}

void PulseMixer::Init(v8::Local<v8::Object> exports) {
  v8::Local<v8::Context> context = exports->GetCreationContextChecked();

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Mixer").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "play", Play);
  Nan::SetPrototypeMethod(tpl, "stop", Stop);

  Nan::Set(exports, Nan::New("Mixer").ToLocalChecked(),
           tpl->GetFunction(context).ToLocalChecked());
}

void PulseMixer::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowError("Mixer must be instantiated using 'new'");
  }

  // Extraer argumentos
  Nan::Utf8String socket(info[0]);
  Nan::Utf8String cookie(info[1]);

  const char *socket_ptr = info[0]->IsString() ? *socket : NULL;
  const char *cookie_ptr = info[1]->IsString() ? *cookie : NULL;

  PulseMixer *obj = new PulseMixer(socket_ptr, cookie_ptr);

  if (!obj->mixer_handle) {
    return Nan::ThrowError(
        "Failed to connect to PulseAudio/Pipewire. Check socket path.");
  }

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void PulseMixer::Play(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PulseMixer *obj = ObjectWrap::Unwrap<PulseMixer>(info.Holder());

  if (info.Length() < 1 || !node::Buffer::HasInstance(info[0])) {
    return Nan::ThrowTypeError("Argument must be a Buffer");
  }

  v8::Local<v8::Object> bufferObj = info[0].As<v8::Object>();

  const char *data = node::Buffer::Data(bufferObj);
  size_t length = node::Buffer::Length(bufferObj);

  mixer_play_buffer(obj->mixer_handle, (const int16_t *)data, length);

  info.GetReturnValue().SetUndefined();
}

void PulseMixer::Stop(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  PulseMixer *obj = ObjectWrap::Unwrap<PulseMixer>(info.Holder());

  mixer_stop(obj->mixer_handle);

  info.GetReturnValue().SetUndefined();
}

void InitModule(v8::Local<v8::Object> exports) { PulseMixer::Init(exports); }

NODE_MODULE(addon, InitModule)
