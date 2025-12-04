#ifndef MIXER_CORE_H
#define MIXER_CORE_H

#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mixer Mixer;

Mixer *mixer_create(const char *socket_path, const char *cookie_path);
void mixer_destroy(Mixer *m);
void mixer_stop(Mixer *m);
void mixer_play_buffer(Mixer *m, const int16_t *data, size_t length_bytes);
int mixer_is_ready(Mixer *m);

int16_t* mixer_resample_audio(const int16_t* data, size_t in_bytes, float pitch, size_t* out_bytes);
int16_t* mixer_apply_fadeout(const int16_t* data, size_t in_bytes, int duration_ms, size_t* out_bytes);

#ifdef __cplusplus
}
#endif

#endif
