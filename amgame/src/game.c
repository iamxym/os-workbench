#include <game.h>

// Operating system is a C program!
extern int bx,by,bw,bh,score;
extern int stacks[105];
extern int fpx[105],fpy[105];
extern char vis[205][205];
 //uint32_t ran();
int game_sec,game_h_sec;
uint32_t FPS=40;
void game_progress(int key)
{
  for (int i=1;i<=100;++i)
    if (fpx[i]!=-1&&fpy[i]!=-1)
    {
      if (fpy[i]==by&&bx-1<=fpx[i]&&fpx[i]<=bx+1)
      {
        ++score;
        printf("Your score is %d\n",score);
        vis[fpx[i]][fpy[i]]=0;
        fpx[i]=-1;fpy[i]=-1;
        stacks[++stacks[0]]=i;
      }
    }
  if (++game_h_sec>=(FPS/2))
  {
    game_h_sec=0;
    for (int i=1;i<=100;++i)
      if (fpx[i]!=-1&&fpy[i]!=-1)
      {
        vis[fpx[i]][fpy[i]]=0;
        if (++fpy[i]>=bh)
        {
          fpx[i]=-1;fpy[i]=-1;
          stacks[++stacks[0]]=i;
        }
        else vis[fpx[i]][fpy[i]]=1;
      }
  }
  if (++game_sec>=FPS)
  {
    
    game_sec=0;
    int id=stacks[stacks[0]--];
    fpx[id]=ran()%(bw);
    fpy[id]=1;
    vis[fpx[id]][fpy[id]]=1;
  }
  //left 75 right 76 down 74 up 73
  if (key==73)
    if (by>1) --by;
  if (key==74)
    if (by<bh-1) ++by;
  if (key==75)
    if (bx>1) --bx;
  if (key==76)
    if (bx<bw-2) ++bx;
}
int main(const char *args) {
  _ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");
  
  splash();

  puts("Press any key to see its key code...\n");
  game_sec=0;game_h_sec=0;
  int key;
  //printf("%d %d %d %d\n",bx,by,bw,bh);
  uint32_t next_frame=uptime();
  printf("Your score is %d\n",0);
  while (1) {
    while (uptime()<next_frame);
    key=do_key_event();
    if (key==1) _halt(-1);
    game_progress(key);
    screen_update();
    //print_key();
    next_frame+=1000/FPS;
  }
  return 0;
}
