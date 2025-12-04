#include "mixer.h"
#include <cstdint>
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
  static void Resample(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void FadeOut(const Nan::FunctionCallbackInfo<v8::Value> &info);
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
  Nan::SetPrototypeMethod(tpl, "resample", Resample);
  Nan::SetPrototypeMethod(tpl, "fadeout", FadeOut);

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

void PulseMixer::Resample(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 2 || !node::Buffer::HasInstance(info[0]) ||
      !info[1]->IsNumber()) {
    return Nan::ThrowTypeError("Usage: resample(buffer, pitch)");
  }

  v8::Local<v8::Object> bufferObj = info[0].As<v8::Object>();
  const char *in_data = node::Buffer::Data(bufferObj);
  size_t in_len = node::Buffer::Length(bufferObj);
  float pitch =
      (float)info[1]->NumberValue(Nan::GetCurrentContext()).FromMaybe(1.0);

  size_t out_len = 0;
  int16_t *out_data =
      mixer_resample_audio((const int16_t *)in_data, in_len, pitch, &out_len);

  if (!out_data) {
    return Nan::ThrowError("Memory allocation failed during resampling");
  }

  v8::Local<v8::Object> resultBuffer =
      Nan::CopyBuffer((char *)out_data, out_len).ToLocalChecked();

  free(out_data);

  info.GetReturnValue().Set(resultBuffer);
}

void PulseMixer::FadeOut(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 2 || !node::Buffer::HasInstance(info[0]) ||
      !info[1]->IsNumber()) {
    return Nan::ThrowTypeError("Usage: fade(buffer, duration_ms)");
  }

  v8::Local<v8::Object> bufferObj = info[0].As<v8::Object>();
  const char *in_data = node::Buffer::Data(bufferObj);
  size_t in_len = node::Buffer::Length(bufferObj);
  int duration =
      (int)info[1]->NumberValue(Nan::GetCurrentContext()).FromMaybe(0);

  size_t out_len = 0;
  int16_t *out_data =
      mixer_apply_fadeout((const int16_t *)in_data, in_len, duration, &out_len);

  if (!out_data) {
    return Nan::ThrowError("Memory allocation failed during fade");
  }

  v8::Local<v8::Object> resultBuffer =
      Nan::CopyBuffer((char *)out_data, out_len).ToLocalChecked();

  free(out_data);

  info.GetReturnValue().Set(resultBuffer);
}

void InitModule(v8::Local<v8::Object> exports) { PulseMixer::Init(exports); }

NODE_MODULE(addon, InitModule)
