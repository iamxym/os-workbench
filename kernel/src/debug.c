
#include <common.h>
#include <klib.h>
static uint64_t now=20000506;
void paic(int val,char s[])
{
    if (val){assert(0);printf("%s\n",s);}
}
int rand()
{
    //0~32766
    now=((now<<16)^now)*19230817+131;
    return now;
}

void srand(unsigned int seed)
{
  now=seed;
}