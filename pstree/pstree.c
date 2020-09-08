#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "dirent.h"
#include <string.h>
#include <sys/types.h>
char str_status[256]="/proc/";
int fa[65536],first[65536],tot,p_stack[65536],p_spa[65536],p_tmp[65536],sort_tmp[65536];
char fstr[65536][256];
struct edge
{
  int u,v,next;
};
struct edge e[65536];
int cal_num(char *sta)
{
  int ret=0;
  while (*sta<'0'||*sta>'9') ++sta;
  while (*sta>='0'&&*sta<='9') ret=ret*10+(int)(*sta)-48,++sta;
  return ret;
}
int parent_check(char* s)
{
  if (s[0]=='P'&&s[1]=='P'&&s[2]=='i'&&s[3]=='d') return 1;
  else return 0;
}
void add(int x,int y)
{
  e[++tot].v=y;e[tot].u=x;e[tot].next=first[x];first[x]=tot;
}
void mergesort(int l,int r)
{
  if (l>=r) return;
  int mid=(l+r)/2;
  mergesort(l,mid);
  mergesort(mid+1,r);
  //for (int i=l;i<=r;++i) printf("%d%c",p_tmp[i]," \n"[i==r]);
  for (int i=l;i<=r;++i) sort_tmp[i]=p_tmp[i];
  int pl=l,pr=mid+1,pn=l;
  while (pl<=mid&&pr<=r)
    if (sort_tmp[pl]>sort_tmp[pr]) p_tmp[pn++]=sort_tmp[pr++];
    else p_tmp[pn++]=sort_tmp[pl++];
  while (pl<=mid) p_tmp[pn++]=sort_tmp[pl++];
  while (pr<=r) p_tmp[pn++]=sort_tmp[pr++];
}
void openstatus(int id,char *pos)//打开id目录下的status，获得他的parent编号
{
  int len=6;
//  putchar(*pos);
  while (*pos>='0'&&*pos<='9') str_status[len]=*pos++,++len;
  str_status[len++]='/';
  str_status[len++]='s';
  str_status[len++]='t';
  str_status[len++]='a';
  str_status[len++]='t';
  str_status[len++]='u';
  str_status[len++]='s';
  str_status[len]='\0';
  //printf("%d %s\n",id,str_status);
  FILE *fp=fopen(str_status,"r");
  char fpline[256],fname[256];
  int name_len=0;
  char *chp;
  if (fp)
  {
      fgets(fpline,255,fp);
      chp=fpline+4;
      while (!((*chp>='a'&&*chp<='z')||(*chp>='A'&&*chp<='Z'))) ++chp;
      while (*chp!='\0'&&*chp!='\n') fname[name_len++]=*chp,++chp;
      fname[name_len]='\0';
      while (parent_check(fpline)==0) fgets(fpline,255,fp);
     // printf("%s",fpline);
      
      fa[id]=cal_num(fpline+4);
      strcpy(fstr[id],fname);
      //printf("%d %s\n",id,fstr[id]);
      if (fa[id]!=0) add(fa[id],id);
      fclose(fp);
  }
  else
  {
    puts("read error!");
  }
  
}
void openproc()//打开"/proc"，检索所有数字开头的文件
{
  DIR *dir=opendir("/proc");
  if (dir==NULL) return;
  struct dirent *entry=readdir(dir);
  while (entry!=NULL)
  {
    if (entry->d_name[0]>='0'&&entry->d_name[0]<='9')
    {
      openstatus(cal_num(entry->d_name),entry->d_name);
    //  printf("%s\n",entry->d_name);
    }
    entry=readdir(dir);
  }
  closedir(dir);
}
void Print(int flag)//pid是当前进程编号，spa为前方空格数量，flag用于标示输出配置
{
  int top=1,u,v,cnt;
  p_stack[1]=1;
  while (top)
  {
    u=p_stack[top--];
    for (int i=1;i<=p_spa[u];++i) putchar(' ');
    printf("%s",fstr[u]);
    if (flag&1) printf("(%d)\n",u);
    else putchar('\n');
    cnt=0;
    for (int i=first[u];i;i=e[i].next)
    {
      v=e[i].v;
      p_spa[v]=p_spa[u]+2;
      //p_stack[++top]=v;
      p_tmp[++cnt]=v;
    }
    if (flag&2) {mergesort(1,cnt);}
    for (int i=cnt;i>=1;--i) p_stack[++top]=p_tmp[i];
  }
}


int main(int argc, char *argv[]) {
  tot=0;
  int flag=0;
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
    if (argv[i][1]=='p'||(argv[i][1]=='-'&&argv[i][2]=='s')) flag|=1;
    else if (argv[i][1]=='n'||(argv[i][1]=='-'&&argv[i][2]=='n')) flag|=2;
    else if (argv[i][1]=='V'||(argv[i][1]=='-'&&argv[i][2]=='v')) flag|=4;
  }
  if (flag&4) {fprintf(stderr,"pstree (PSmisc) UNKNOWN\nCopyright (C) 1993-2017 Werner Almesberger and Craig Small\n\nPSmisc comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it under the terms of the GNU General Public License.\nFor more information about these matters, see the files named COPYING.\n");return 0;}
  assert(!argv[argc]);
  openproc();
  Print(flag);
  
  
  
  return 0;
}
