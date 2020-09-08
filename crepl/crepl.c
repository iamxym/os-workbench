#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
char test_name[]="/tmp/file_test.c";
char pro_name[]="/tmp/pro_name.c";
char test_str[100005],pro_str[100005];
char buf[1024];
int pi[2];
int main(int argc, char *argv[]) {
  static char line[4096];
  
  strcpy(pro_str,"#include<stdio.h>\n");
  //printf("%s\n",pro_str);
  if (pipe(pi)==-1) {puts("pipe create error");assert(0);}
  int test_fd=mkstemp(test_name);
  while (1) {
    memset(test_str,0,sizeof(test_str));
    memset(line,0,sizeof(line));
    if (!fgets(line, sizeof(line), stdin)) break;
    int len=strlen(line),line_type;
    if (len>2&&line[0]=='i'&&line[1]=='n'&&line[2]=='t')//function
    {
      line_type=1;
      strcpy(test_str,pro_str);
      strcat(test_str,line);
      strcat(test_str,"int main(){printf(\"OK\\n\");}\0");
    }
    else//expression
    {
      line_type=2;
      strcpy(test_str,pro_str);
      strcat(test_str,"int main(){printf(\"%d\\n\",");
      strcat(test_str,line);
      strcat(test_str,");}\0");
    }
    int pid=fork();
    if (pid==0)
    {
      close(pi[0]);
      close(fileno(stdout));
      close(fileno(stderr));
      FILE* test_fp=fopen(test_name,"w+");
      fprintf(test_fp,"%s",test_str);
	    fflush(test_fp);
      dup2(pi[1],fileno(stdout));
      dup2(pi[1],fileno(stderr));
      char *exec_argv[]={"gcc","-o","/tmp/file_test","/tmp/file_test.c",NULL};
      execvp("gcc",exec_argv);
      assert(0);
    }
    int status;
    wait(&status);
    fcntl(pi[0],F_SETFL,O_NONBLOCK);
    int cnt=0;
    while (read(pi[0],buf,sizeof(buf))>0) ++cnt;
    if (cnt>0) puts("Compile error");
    else
    {
      if (line_type==1) strcat(pro_str,line);
      int ppid=fork();
      if (ppid==0)//执行该文件
      {
        char *pp_argv[]={NULL};
        execvp("/tmp/file_test",pp_argv);
        assert(0);
      }
      wait(&status);
    }
    fflush(stdout);
  }
}
/*

*/