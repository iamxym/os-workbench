#include <am.h>
#include <amdev.h>
#include<klib.h>

void splash();
void print_key();
void screen_update();
int do_key_event();
int bw,bh,bx,by,score;
int stacks[105];
int fpx[105],fpy[105];
char vis[205][205];
static uint32_t seed=19240817;
static inline uint32_t ran()
{
  seed=((seed<<8)^(seed>>8)^uptime());
  return seed;
}
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
