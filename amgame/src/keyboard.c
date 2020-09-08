#include <game.h>
int do_key_event()
{
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  /*if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    printf("%d\n",event.keycode);
    puts("\n");
  }*/
  return event.keydown?event.keycode:0;
  
}
void print_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    printf("%d\n",event.keycode);
    puts("\n");
  }
}