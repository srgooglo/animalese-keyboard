#include "mixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Sound {
  int16_t *data;
  size_t length_samples;
  size_t cursor;
  struct Sound *next;
} Sound;

struct Mixer {
  pa_threaded_mainloop *mainloop;
  pa_context *context;
  pa_stream *stream;
  Sound *active_sounds;
  int ready; // 0=Pending, 1=Ready, -1=Error
};

static void stream_write_callback(pa_stream *stream, size_t length, void *userdata) {
  Mixer *mixer = (Mixer *)userdata;
  size_t n_samples = length / sizeof(int16_t);
  int16_t *mix_buffer = calloc(n_samples, sizeof(int16_t));

  // if no sound is playing, send silence to keep the stream alive
  if (mixer->active_sounds == NULL) {
    pa_stream_write(stream, mix_buffer, length, NULL, 0, PA_SEEK_RELATIVE);
    free(mix_buffer);

    return;
  }

  Sound *current = mixer->active_sounds;
  Sound *prev = NULL;

  while (current != NULL) {
    size_t samples_left = current->length_samples - current->cursor;
    size_t to_mix = (n_samples < samples_left) ? n_samples : samples_left;

    for (size_t i = 0; i < to_mix; i++) {
      int32_t mixed_value = mix_buffer[i];
      int16_t sound_value = current->data[current->cursor + i];

      mixed_value += sound_value;

      if (mixed_value > 32767) {
        mixed_value = 32767;
      } else if (mixed_value < -32768) {
        mixed_value = -32768;
      }

      mix_buffer[i] = (int16_t)mixed_value;
    }

    current->cursor += to_mix;

    if (current->cursor >= current->length_samples) {
      Sound *finished = current;

      if (prev == NULL) {
        mixer->active_sounds = current->next;
        current = mixer->active_sounds;
      } else {
        prev->next = current->next;
        current = current->next;
      }

      free(finished->data);
      free(finished);
    } else {
      prev = current;
      current = current->next;
    }
  }

  pa_stream_write(stream, mix_buffer, length, NULL, 0, PA_SEEK_RELATIVE);
  free(mix_buffer);
}

static void context_state_callback(pa_context *context, void *userdata) {
  Mixer *mixer = (Mixer *)userdata;

  switch (pa_context_get_state(context)) {
  case PA_CONTEXT_READY:
    mixer->ready = 1;
    break;
  case PA_CONTEXT_FAILED:
  case PA_CONTEXT_TERMINATED:
    mixer->ready = -1;
    break;
  default:
    break;
  }

  pa_threaded_mainloop_signal(mixer->mainloop, 0);
}

Mixer *mixer_create(const char *socket_path, const char *cookie_path) {
  Mixer *mixer = calloc(1, sizeof(Mixer));

  mixer->mainloop = pa_threaded_mainloop_new();

  if (!mixer->mainloop) {
    return NULL;
  }

  pa_mainloop_api *api = pa_threaded_mainloop_get_api(mixer->mainloop);

  pa_proplist *proplist = pa_proplist_new();
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "bingus");
  mixer->context = pa_context_new_with_proplist(api, "bingus", proplist);
  pa_proplist_free(proplist);

  pa_context_set_state_callback(mixer->context, context_state_callback, mixer);

  if (cookie_path) {
    pa_context_load_cookie_from_file(mixer->context, cookie_path);
  }

  if (pa_context_connect(mixer->context, socket_path, PA_CONTEXT_NOFLAGS, NULL) <
      0) {
    free(mixer);
    return NULL;
  }

  pa_threaded_mainloop_start(mixer->mainloop);
  pa_threaded_mainloop_lock(mixer->mainloop);

  while (mixer->ready == 0) {
    pa_threaded_mainloop_wait(mixer->mainloop);
  }

  if (mixer->ready < 0) {
    pa_threaded_mainloop_unlock(mixer->mainloop);
    return NULL;
  }

  pa_sample_spec ss = {.format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2};
  mixer->stream = pa_stream_new(mixer->context, "playback", &ss, NULL);

  // set the write callback
  pa_stream_set_write_callback(mixer->stream, stream_write_callback, mixer);

  pa_buffer_attr bufferAttribute;
  bufferAttribute.tlength = pa_usec_to_bytes(50 * 1000, &ss);
  bufferAttribute.maxlength = (uint32_t)-1;
  bufferAttribute.minreq = (uint32_t)-1;
  bufferAttribute.prebuf = (uint32_t)-1;
  bufferAttribute.fragsize = (uint32_t)-1;

  pa_stream_connect_playback(mixer->stream, NULL, &bufferAttribute, PA_STREAM_ADJUST_LATENCY,
                             NULL, NULL);
  pa_threaded_mainloop_unlock(mixer->mainloop);

  return mixer;
}

void mixer_play_buffer(Mixer *mixer, const int16_t *data, size_t length_bytes) {
  if (!mixer || !mixer->stream) {
    return;
  }

  Sound *sound = malloc(sizeof(Sound));
  sound->data = malloc(length_bytes);

  memcpy(sound->data, data, length_bytes);

  sound->length_samples = length_bytes / sizeof(int16_t);
  sound->cursor = 0;

  pa_threaded_mainloop_lock(mixer->mainloop);

  sound->next = mixer->active_sounds;
  mixer->active_sounds = sound;

  pa_threaded_mainloop_unlock(mixer->mainloop);
}

void mixer_stop(Mixer *mixer) {
  if (!mixer) {
    return;
  }

  pa_threaded_mainloop_lock(mixer->mainloop);

  Sound *current = mixer->active_sounds;

  while (current != NULL) {
    Sound *next = current->next;

    if (current->data) {
      free(current->data);
    }

    free(current);

    current = next;
  }

  mixer->active_sounds = NULL;

  pa_threaded_mainloop_unlock(mixer->mainloop);
}

void mixer_destroy(Mixer *mixer) {
  if (!mixer) {
    return;
  }

  if (mixer->mainloop) {
    pa_threaded_mainloop_stop(mixer->mainloop);
  }

  free(mixer);
}
