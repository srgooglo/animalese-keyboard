#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int run_mainloop(struct libinput *libinput);
int handle_events(struct libinput *libinput);
void emit_input_event(int kind, uint32_t code, int pressed, const char *keyname);
int initialize();

#ifdef __cplusplus
}
#endif
