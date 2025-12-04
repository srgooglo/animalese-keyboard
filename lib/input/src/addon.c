#include <node/js_native_api.h>
#include <node/node_api.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

static napi_threadsafe_function tsfn = NULL;

typedef struct {
  int kind;      // 0 keyboard 1 pointer_button
  uint32_t code; // keycode or button code
  int pressed;   // 1 pressed 0 released
  char *keyname; // null terminated key name, allocated dynamically
} input_event_t;

static void dispatchEventCallback(napi_env env, napi_value callback,
                                  void *context, void *data) {
  (void)context;
  input_event_t *event = (input_event_t *)data;

  if (event == NULL) {
    return;
  }

  // create the empty object
  napi_value obj;
  napi_create_object(env, &obj);

  // set the type property
  napi_value type_str;

  // type 0 is keyboard type
  if (event->kind == 0) {
    napi_create_string_utf8(env, "keyboard", NAPI_AUTO_LENGTH, &type_str);
  }

  // set the type to object
  napi_set_named_property(env, obj, "type", type_str);

  // set the code property
  napi_value code_val;
  napi_create_uint32(env, event->code, &code_val);
  napi_set_named_property(env, obj, "code", code_val);

  // set the pressed property
  napi_value pressed_val;
  napi_get_boolean(env, event->pressed ? true : false, &pressed_val);
  napi_set_named_property(env, obj, "pressed", pressed_val);

  // set the keyname property
  napi_value keyname_val;
  if (event->keyname != NULL) {
    napi_create_string_utf8(env, event->keyname, NAPI_AUTO_LENGTH,
                            &keyname_val);
    napi_set_named_property(env, obj, "keyname", keyname_val);
  }

  // invoke callback
  napi_value undefined;
  napi_get_undefined(env, &undefined);
  napi_call_function(env, undefined, callback, 1, &obj, NULL);

  // free allocated keyname then event
  if (event->keyname != NULL) {
    free(event->keyname);
  }
  free(event);
}

void emit_input_event(int kind, uint32_t code, int pressed,
                      const char *keyname) {
  if (tsfn == NULL) {
    return;
  }

  input_event_t *event = (input_event_t *)malloc(sizeof(input_event_t));

  if (event == NULL) {
    return;
  }

  event->kind = kind;
  event->code = code;
  event->pressed = pressed;
  event->keyname = NULL;

  if (keyname != NULL) {
    event->keyname = strdup(keyname);

    if (event->keyname == NULL) {
      free(event);
      return;
    }
  }

  napi_status threadsafe_state =
      napi_call_threadsafe_function(tsfn, event, napi_tsfn_nonblocking);

  if (threadsafe_state != napi_ok) {
    if (event->keyname != NULL) {
      free(event->keyname);
    }

    free(event);
    fprintf(stdout, "failed to queue event %d\n", (int)threadsafe_state);
    fflush(stdout);
  }
}

static void *input_thread_start(void *arg) {
  (void)arg;

  initialize();

  if (tsfn != NULL) {
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    tsfn = NULL;
  }

  return NULL;
}

napi_value StartLoop(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  // if no callback is provided throw an error
  if (argc < 1) {
    napi_throw_error(env, NULL, "Missing callback");
    return NULL;
  }

  // get the type of the first argument
  napi_valuetype argv0Type;
  napi_typeof(env, argv[0], &argv0Type);

  // if is not a function send a exception
  if (argv0Type != napi_function) {
    napi_throw_error(env, NULL, "callback is not a function");
    return NULL;
  }

  // check if tsfn is already initialized, if not create it
  if (tsfn == NULL) {
    napi_value resource_name;
    napi_create_string_utf8(env, "input_event", NAPI_AUTO_LENGTH,
                            &resource_name);
    napi_create_threadsafe_function(env, argv[0], NULL, resource_name, 0, 1,
                                    NULL, NULL, NULL, dispatchEventCallback,
                                    &tsfn);
  }

  pthread_t thread;
  int rc = pthread_create(&thread, NULL, input_thread_start, NULL);

  if (rc == 0) {
    pthread_detach(thread);
  }

  napi_value undefined;
  napi_get_undefined(env, &undefined);

  return undefined;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_value startLoopFunction;

  napi_create_function(env, "startLoop", NAPI_AUTO_LENGTH, StartLoop, NULL,
                       &startLoopFunction);
  napi_set_named_property(env, exports, "startLoop", startLoopFunction);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
