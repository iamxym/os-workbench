#include <game.h>

#define SIDE 16
static int w, h;
extern int bx,by,bw,bh;
extern int stacks[105];
extern int fpx[105],fpy[105];
extern char vis[205][205];
static void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
  bw=w/SIDE;bh=h/SIDE;
  bx=bw/2;by=bh/2;
  for (int i=1;i<=100;++i) stacks[i]=i,fpx[i]=fpy[i]=-1;
  stacks[0]=100;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // careful! stack is limited!
  _DEV_VIDEO_FBCTRL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &event, sizeof(event));
}

void splash() {
  init();
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if (bx-1<=x&&x<=bx+1&&by==y) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}
void screen_update()
{
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if (bx-1<=x&&x<=bx+1&&by==y) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
      else if (vis[x][y])
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xff0000); // ?
      }
      else
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0); // black
      }
      
      
    }
  }
}
