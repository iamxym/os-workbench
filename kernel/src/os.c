#include <common.h>
//#include<stdint.h>
/*
_intr_write(0/1)时关/开中断
*/
typedef struct irq_list irq_t;
void paic(int val,char s[]);
void irq_error(){paic(1,"find the irq head!");}
irq_t irq_head=(irq_t)
{
  .seq=INT32_MIN,
  .event=0,
  .handler=(handler_t)irq_error,
  .next=NULL
};

int yaq;
sem_t empty,fill;
spinlock_t* test_lk;
task_t* task1[10];
task_t* task2;
void producer() { while (1) { kmt->sem_wait(&empty); _putc('('); kmt->sem_signal(&fill);  } }
void consumer() { while (1) { kmt->sem_wait(&fill);  _putc(')'); kmt->sem_signal(&empty);} }
int tid;
const int sz=4096;
static void traverse(const char *root) {
  char *buf = pmm->alloc(sz); // asserts success
  struct ufs_stat s;
  //_yield();
  int fd = vfs->open(strcmp(root, "") == 0 ? "/" : root, O_RDONLY), nread;
  if (fd < 0) {++tid;goto release;}

  vfs->fstat(fd, &s);
  if (s.type == T_DIR) {
    while ( (nread = vfs->read(fd, buf, sz)) > 0) {
      for (int offset = 0;
           offset +  sizeof(struct ufs_dirent) <= nread;
           offset += sizeof(struct ufs_dirent)) {
        struct ufs_dirent *d = (struct ufs_dirent *)(buf + offset);
        if (d->name[0] != '.') { // 小彩蛋：你这下知道为什么
                                 // Linux 以 “.” 开头的文件是隐藏文件了吧
          char *fname = pmm->alloc(256); // assert success
          if (fname[0]=='/'&&fname[1]=='/')
          {
            while (1)
            {
              ++tid;
            }
          }
          sprintf(fname, "%s/%s", root, d->name);
          traverse(fname);
          pmm->free(fname);
        }
      }
    }
  }
  //else ++tid;

release:
  if (fd >= 0) vfs->close(fd);
  pmm->free(buf);
}

int ret=1;
char ss[4096];
void openclose_test(void* s)
{
  //printf("%d\n",vfs->unlink("/dev/null"));
  yaq=10;
  ret=vfs->open("t",O_RDONLY);
  vfs->close(ret);
  ret=vfs->open("t/tmp.txt",O_RDONLY);
  int fd=ret,fd2=vfs->open("copy.doc",O_RDWR|O_CREAT);
  if (fd2!=1) assert(0);
  while ((ret=vfs->read(fd,ss,4080))>0)
  {
    ++yaq;
    ret=vfs->write(fd2,ss,ret);
  }
  int fd3=vfs->dup(fd2),fd4=vfs->open("copy2.doc",O_WRONLY|O_CREAT);
  ret=vfs->lseek(fd2,0,SEEK_SET);
  while ((ret=vfs->read(fd3,ss,4094))>0)
  {
    ++yaq;
    ret=vfs->write(fd4,ss,ret);
  }
  ret=vfs->close(fd);
  ret=vfs->close(fd2);
  ret=vfs->close(fd3);
  ret=vfs->close(fd4);
  
  
  printf("mkdir=%d\n",ret=vfs->mkdir("home"));
  ret=2;
  printf("ret=%d\n",ret=vfs->chdir("home"));
  ret=3;
  printf("mkdir=%d\n",ret=vfs->mkdir("zhn"));
  ret=4;
  fd=vfs->open("zhn/a.txt",O_CREAT|O_RDWR);
  ret=fd+123;
  printf("write=%d\n",ret=vfs->write(fd,"0123456789",10));
  fd2=vfs->dup(fd);
  vfs->lseek(fd,5,SEEK_SET);
  
  printf("read=%d\n",vfs->read(fd2,ss,5));
  printf("%s\n",ss);
  vfs->close(fd);vfs->close(fd2);
  vfs->link("/home/zhn/a.txt","/home/b.txt");
  ret=vfs->chdir("/home");
  printf("ret=%d\n",ret);
  fd3=vfs->open("b.txt",O_RDONLY);
  printf("read=%d\n",vfs->read(fd3,ss,5));
  printf("%s\n",ss);
  ret=34958;
  printf("unlink=%d\n",ret=vfs->unlink("/home/b.txt"));
  printf("read=%d\n",ret=vfs->read(fd3,ss,5));
  printf("%d\n",ret=vfs->open("b.txt",O_RDONLY));
  printf("%s\n",ss);
  while(1);
}
void dir_test()
{
  //ret=chdir("/");
  ret=vfs->chdir("/proc");
  ret=vfs->chdir("1");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");

  ret=vfs->chdir("proc/1");
  ret=vfs->chdir("..");
  int fd=vfs->open("1/name",O_RDONLY);
  ret=vfs->read(fd,ss,10);
  ret=vfs->close(fd);
  ret=vfs->chdir("..");
  ret=vfs->chdir("proc/1");
  memset(ss,0,sizeof(ss));
  fd=vfs->open("name",O_RDONLY);
  ret=vfs->read(fd,ss,10);
  ret=vfs->close(fd);
  ret=vfs->chdir("..");
  ret=vfs->chdir("1");
  ret=vfs->chdir("..");
  ret=vfs->chdir("..");
  fd=vfs->open("proc/1/name",O_RDONLY);
  ret=vfs->close(fd);
  traverse("");
  while (1);
  
  
}
int fd;
void run_fs_test() {
#include "workload.inc"
//dir_test();
traverse("");
++fd;
++fd;
  while (1);
}
static void os_init() {
  
  pmm->init();
  
  kmt->init();
  
    
  /*
  现在问题：()问题 有时候正常 有时失常(连续3个左/右括号什么的)
  */
  dev->init();
  
  vfs->init();
  
  //while(1);
  //kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task123",openclose_test,NULL);
  //kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task456",dir_test,NULL);
  
  //kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task789",run_fs_test,NULL);
  /*test_lk=(spinlock_t*)((void*)pmm->alloc(sizeof(spinlock_t)));
  kmt->spin_init(test_lk,"123");
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task1",printer,"task 1");
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task2",printer,"task 2");
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task3",printer,"task 3");
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task4",printer,"task 4");*/
  //kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task5",printer,"task 5");
  //kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"task6",printer,"task 6");

  /*kmt->sem_init(&empty,"empty",2);
  kmt->sem_init(&fill,"fill",0);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"p",producer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"p",producer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"p",producer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"p",producer,NULL);
  
  //kmt->create(task1[_cpu()]=(task_t*)((void*)pmm->alloc(sizeof(task_t))),"y",yielder,NULL);

  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"c",consumer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"c",consumer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"c",consumer,NULL);
  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"c",consumer,NULL);*/

//  kmt->create((task_t*)((void*)pmm->alloc(sizeof(task_t))),"pr",printer,NULL);

  if (_intr_read()==1) assert(0);
}
static _Context* os_trap(_Event ev, _Context *context)
{
  _Context *next = NULL;
  for (irq_t* h=(&irq_head)->next;h!=NULL;h=h->next) {
    if (h->event == _EVENT_NULL || h->event == ev.event) {
      _Context *r = h->handler(ev, context);
      paic((r&&next), "returning multiple contexts");
      if (r) next = r;
    }
  }
  paic(!next, "returning NULL context");
  return next;
}
static void os_irq(int seq, int event, handler_t handler)
{
  irq_t* now=&irq_head;
  while (now->next!=NULL)
    if (now->next->seq>seq) break;
    else now=now->next;
  irq_t* node=(irq_t*)pmm->alloc(sizeof(irq_t));
  node->seq=seq;node->event=event;
  
  node->handler=handler;
  if (now->next) node->next=now->next;
  else node->next=NULL;
  now->next=node;
}

static void os_run() {
  _intr_write(1);
    
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq =os_irq,
};