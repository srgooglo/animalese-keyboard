#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libinput.h>
#include <libudev.h>

#include "main.h"

extern void emit_input_event(int kind, uint32_t code, int pressed,
                             const char *keyname);

#define MAX_BUFFER_LENGTH 512

enum error_code {
  NO_ERROR,
  UDEV_FAILED,
  LIBINPUT_FAILED,
  SEAT_FAILED,
  PERMISSION_FAILED
};

struct input_handler_data {
  struct udev *udev;
  struct libinput *libinput;
};

static int open_restricted(const char *path, int flags, void *user_data) {
  (void)user_data;
  int fd = open(path, flags);

  if (fd < 0) {
    fprintf(stderr, "Failed to open %s because of %s.\n", path,
            strerror(errno));
  }

  return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data) {
  (void)user_data;
  close(fd);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static void *handle_input(void *user_data) {
  struct input_handler_data *input_handler_data = user_data;

  char line[MAX_BUFFER_LENGTH];
  while (fgets(line, MAX_BUFFER_LENGTH, stdin) != NULL) {
    if (strcmp(line, "stop\n") == 0) {
      libinput_unref(input_handler_data->libinput);
      udev_unref(input_handler_data->udev);
      exit(EXIT_SUCCESS);
    }
  }

  return NULL;
}

int handle_events(struct libinput *libinput) {
  struct libinput_event *event;
  int result = -1;

  if (libinput_dispatch(libinput) < 0) {
    fprintf(stderr, "Failed to dispatch libinput events\n");
    return result;
  }

  while ((event = libinput_get_event(libinput)) != NULL) {
    enum libinput_event_type type = libinput_event_get_type(event);

    if (type == LIBINPUT_EVENT_KEYBOARD_KEY) {
      struct libinput_event_keyboard *kb =
          libinput_event_get_keyboard_event(event);
      uint32_t key = libinput_event_keyboard_get_key(kb);

      enum libinput_key_state state = libinput_event_keyboard_get_key_state(kb);
      const char *state_str =
          (state == LIBINPUT_KEY_STATE_PRESSED) ? "pressed" : "released";

      const char *evname = libevdev_event_code_get_name(EV_KEY, (int)key);
      char keyname_buf[64];

      if (evname != NULL) {
        strncpy(keyname_buf, evname, sizeof(keyname_buf) - 1);
        keyname_buf[sizeof(keyname_buf) - 1] = '\0';
      }

      emit_input_event(0, key, state == LIBINPUT_KEY_STATE_PRESSED,
                       keyname_buf);
    }

    libinput_event_destroy(event);
    result = 0;
  }

  return result;
}

int run_mainloop(struct libinput *libinput) {
  struct pollfd fd;

  fd.fd = libinput_get_fd(libinput);
  fd.events = POLLIN;
  fd.revents = 0;

  printf("run_mainloop: libinput fd=%d\n", fd.fd);
  fflush(stdout);

  // do an initial dispatch so device-added events are processed before polling
  if (libinput_dispatch(libinput) < 0) {
    fprintf(stderr,
            "run_mainloop: libinput_dispatch failed on initial dispatch\n");
    fflush(stderr);
  } else {
    handle_events(libinput);
  }

  while (poll(&fd, 1, -1) > -1) {
    handle_events(libinput);
  }

  return 0;
}

int initialize() {
  struct udev *udev = udev_new();

  if (udev == NULL) {
    fprintf(stderr, "Failed to initialize udev.\n");
    return UDEV_FAILED;
  }

  struct libinput *libinput =
      libinput_udev_create_context(&interface, NULL, udev);

  if (!libinput) {
    fprintf(stderr, "Failed to initialize libinput from udev.\n");
    return LIBINPUT_FAILED;
  }

  // assign seat so libinput will monitor input devices
  int seat_rc = libinput_udev_assign_seat(libinput, "seat0");
  printf("initialize: libinput_udev_assign_seat returned %d\n", seat_rc);
  fflush(stdout);

  pthread_t input_handler;
  struct input_handler_data input_handler_data = {udev, libinput};
  pthread_create(&input_handler, NULL, handle_input, &input_handler_data);

  // perform an initial dispatch and handle events so devices are detected now
  if (libinput_dispatch(libinput) < 0) {
    fprintf(stderr,
            "initialize: libinput_dispatch failed on initial dispatch\n");
    fflush(stderr);
  } else {
    handle_events(libinput);
  }

  if (run_mainloop(libinput) < 0) {
    return PERMISSION_FAILED;
  }

  libinput_unref(libinput);
  udev_unref(udev);

  return NO_ERROR;
}
