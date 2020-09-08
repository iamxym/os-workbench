#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>

#include <fcntl.h>
/*
strace -c ls可以获得执行ls时各系统调用的占用时间
strace -o test.txt ls可以将strace的结果输出到test.txt中
stdin=0 stdout=1 stderr=2
用fork/execve创建并运行子进程，子进程调用strace -c(或-T) COMMAND...，得到strace的输出
父进程从其中提取各系统调用运行时间，然后计算即可
*/
pid_t id;
char *exec_argv[512];
char api_name[512],api_num[512],file_path[512],buf[512];
char sys_path[512],str_path[512];
int pi[2],api_cnt,api_id[512];
struct node{
  char name[128];
  double time;
}api_arr[512];
double tot_time;
void swap(int *a,int *b)
{
  int t=*a;
  *a=*b;
  *b=t;
}
int dis_cnt;
void info_display()
{
  if (dis_cnt++) puts("====================");
  for (int i=1;i<=api_cnt;++i) api_id[i]=i;
  for (int i=1;i<=api_cnt;++i)
  {
    for (int j=1;j<api_cnt;++j)
      if (api_arr[api_id[j]].time<api_arr[api_id[j+1]].time) swap(&api_id[j],&api_id[j+1]);
  }
  for (int i=1;(i<=5)&&(i<=api_cnt);++i) printf("%s(%.0lf%%)\n",api_arr[api_id[i]].name,api_arr[api_id[i]].time*100/tot_time);
  for (int i=1;i<=80;++i) putchar('\0');
  fflush(stdout);
  fflush(stderr);
}
int main(int argc, char *argv[],char *envp[]) {
  if (argc<2) {puts("Too few arg!");assert(0);}
  if (pipe(pi)==-1){puts("pipe create error");assert(0);}
  id=fork();
  if (id==0)//子进程
  {
    exec_argv[0]="strace";
    exec_argv[1]="-T";
    exec_argv[2]="-o";
    exec_argv[3]=file_path;
    int exe_out=open("/dev/null",O_WRONLY);
    if (exe_out==-1) {puts("devnull open error");assert(0);}
    sprintf(file_path,"/proc/self/fd/%d",pi[1]);
    for (int i=1;i<argc;++i) exec_argv[i+3]=argv[i];
    exec_argv[argc+4]=NULL;
    strcpy(sys_path,getenv("PATH"));
    int sys_len=strlen(sys_path);
    memset(str_path,0,sizeof(str_path));
    strcpy(str_path,"strace");
    dup2(exe_out,fileno(stdout));
    dup2(exe_out,fileno(stderr));
    close(pi[0]);//子进程关闭输入端，只向其中输出结果
    int lp=0,rp;
    while (execve(str_path,exec_argv,envp)==-1&&lp<sys_len)
    {
      memset(str_path,0,sizeof(str_path));
      rp=lp;
      while (sys_path[rp]!=':'&&rp<sys_len) ++rp;
      strncpy(str_path,sys_path+lp,rp-lp);
      strcpy(str_path+(rp-lp),"/strace");
      lp=rp+1;
    }
    puts("not allowed to reach here");
    assert(0);
  }
  else
  {
    dup2(pi[0],fileno(stdin));
    int len=0,suc,sr=0;
    double api_time;
    clock_t start;
    api_cnt=0;
    tot_time=0;
    start=clock();
    while (1)
    {
      if (read(pi[0],buf+len,1)>0) suc=1;
      else suc=0;
      if (suc)
      {
        if (buf[len++]=='\n') {suc=1;buf[len]='\0';}
        else suc=0;
      }
      if (suc)
      {
        if (buf[0]=='+'&&buf[1]=='+'&&buf[2]=='+') {info_display();break;} 
        for (sr=0;sr<len&&buf[sr]!='(';++sr);
        strncpy(api_name,buf,sr);
        api_name[sr]='\0';
        for (sr=len-2;buf[sr]!='<'&&sr>=0;--sr);
        ++sr;
        strncpy(api_num,buf+sr,len-sr-2);
        api_num[len-sr-2]='\0';
        sscanf(api_num,"%lf",&api_time);
        int flag=0;
        for (int i=1;i<=api_cnt;++i)
          if (strcmp(api_name,api_arr[i].name)==0)
          {
            flag=1;
            api_arr[i].time+=api_time;
            break;
          }
        if (!flag)
        {
          api_arr[++api_cnt].time=api_time;
          strcpy(api_arr[api_cnt].name,api_name);
        }
        tot_time+=api_time;
        len=0;
      }
      if ((clock()-start)/CLOCKS_PER_SEC>=1) {info_display();start=clock();}
    }
  }
}
