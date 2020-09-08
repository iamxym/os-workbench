#include <common.h>
#define current currents[_cpu()]
#define last_cur last_currents[_cpu()]

//_intr_write(0/1)时关/开中断
task_t* Task[256];
task_t* last_currents[10];
static task_t idle_task[10];
static spinlock_t task_lock;
static int task_cnt;
uint8_t core_state[10];
task_t* currents[10];//最大CPU数为8
void paic(int val,char s[]);
int sem_empty(sem_t* x)
{
    return (x->head==x->tail);
}
void sem_push(sem_t* x,struct task *Ta)
{
    x->queue[x->tail++]=Ta;
    if (x->tail>=x->qlen) x->tail-=x->qlen;
}
struct task* sem_pop(sem_t* x)
{
    task_t *ret=x->queue[x->head++];
    if (x->head>=x->qlen) x->head-=x->qlen;
    return ret;
}
static int xchg(volatile int *addr, int newval) {
  int result;
  asm volatile ("lock xchg %0, %1":
    "+m"(*addr), "=a"(result) : "1"(newval) : "cc");
  return result;
}

void lock_init(spinlock_t *lk, const char *name)
{
    strcpy(lk->name,name);
    lk->lock=0;lk->belong=-1;
}
void lock(spinlock_t *lk)
{   
    int t=_intr_read();
    _intr_write(0);
    while (xchg(&lk->lock, 1));
    lk->last_inter=t;
    lk->belong=_cpu();
}
void unlock(spinlock_t *lk)
{
    int t=lk->last_inter;
    xchg(&lk->lock, 0);
    if (t) _intr_write(1);
}
void sem_init(sem_t *sem, const char *name, int value)
{
    
    strcpy(sem->name,name);
    lock_init(&sem->lk,"sem_lock");
    sem->qlen=sizeof(sem->queue)/sizeof(task_t*);
    sem->val=value;
    sem->head=0;
    sem->tail=0;
}
void sem_wait(sem_t *sem)
{
    //if (sem->name[0]=='7'&&sem->name[1]=='7'&&sem->name[2]=='7') return;
    int fla=0;
    lock(&sem->lk);
    --sem->val;
    
    if (sem->val<0)
    {
        sem_push(sem,current);
        if (current->state==state_dead) assert(0);
        xchg(&current->state,state_wait);//进入信号量队列等待
        fla=1;
    }
    unlock(&sem->lk);
    if (fla)//切换进程
    {
        _yield();
        while (current->state!=state_enable);
    }
    
    if (current->state==state_dead) assert(0);
}
void sem_signal(sem_t *sem)
{
    //if (sem->name[0]=='7'&&sem->name[1]=='7'&&sem->name[2]=='7') return;
    lock(&sem->lk);
    if (++sem->val<=0)
    {
        struct task* Ta=sem_pop(sem);
        if (Ta->state==state_dead) assert(0);
        xchg(&Ta->state,state_enable);//可以运行
    
    }
    unlock(&sem->lk);
    
}
int task_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{
    
    task->stack=pmm->alloc(STACK_SIZE);
    task->name=pmm->alloc(strlen(name));
    task->context=*_kcontext((_Area){(void*)((uintptr_t)task->stack),(void*)((uintptr_t)task->stack+STACK_SIZE)},entry,arg);
    task->running=0;
    task->susp=0;
    task->current_inode=0;
    for (int i=0;i<16;++i) task->fd[i].inode=0xffffffff;
    task->fd_cnt=0;
    strcpy(task->name,name);
    

    lock(&task_lock);
    
    task->id=task_cnt;
    Task[task_cnt++]=task;
    //if (strncmp(name,"tty-task",8)!=0&&strncmp(name,"input-task",10)!=0)
    {
        
        char s[20],tm;
        int tmp=task_cnt,len=0;
        while (tmp) s[len++]=tmp%10+'0',tmp/=10;
        s[len]='/';s[len+1]='c';s[len+2]='o';
        s[len+3]='r';s[len+4]='p';s[len+5]='/';
        len+=6;
        for (int i=0;i*2<len;++i) tm=s[i],s[i]=s[len-i-1],s[len-i-1]=tm;
        s[len]=0;
        core_state[_cpu()]=0xff;
        //printf("%s\n",name);
        //if(current==NULL) assert(0);
        //
        //if (vfs->chdir("/proc")!=0) assert(0);
        vfs->mkdir(s);

        
        s[len]='/';s[len+1]='n';s[len+2]='a';s[len+3]='m';s[len+4]='e';
        len+=5;
        s[len]=0;
        //
        int fd=vfs->open(s,O_CREAT|O_RDONLY);
        
        vfs->write(fd,(void*)name,strlen(name));
        vfs->close(fd);
        
        core_state[_cpu()]=0;
    }
    
    task->state=state_enable;
    
    unlock(&task_lock);
    
    return task->id;
}
void task_teardown(task_t *task)
{
    lock(&task_lock);
    task->state=state_dead;
    if (task->running==1) assert(0); 
    pmm->free(task->stack);
    pmm->free(task->name);
    
    unlock(&task_lock);
    //_intr_write(1);
}
static _Context* kmt_context_save(_Event ev, _Context *context)//存储上下文
{
    //paic(current==NULL,"current NULL!");
    //paic(current&&current->state!=state_running,"current state error!");
    
    if (last_cur)
    {
        if (last_cur->susp==0) {assert(0);printf("state=%d\n",last_cur->state);}
        xchg(&last_cur->susp,0);//可以让last_cur被别的cpu调度了
        last_cur=NULL;
    }
    
    current->context=*context;
    return NULL;
}
static _Context* kmt_schedule(_Event ev, _Context *context)
{
    //printf("???");
    
    paic(_intr_read()!=0,"cli when schedule ");
    
    int id=0,ci=-1;
    lock(&task_lock);//这里的效率有问题，不过先不管
    //顺序多CPU
    
    if (task_cnt>0)
    {
        if (current==&idle_task[_cpu()]) id=0,ci=task_cnt;
        else id=current->id,ci=task_cnt-1;
        do
        {
            if (--ci<0) break;
            id=(id+1)%task_cnt;
            
        }while (Task[id]->state!=state_enable||Task[id]->running==1||xchg(&Task[id]->susp,1));
    }
    
    current->running=0;
    if (last_cur) assert(0);
    if (current!=&idle_task[_cpu()])
    {
        if (current->susp!=1) assert(0);
        last_cur=current;
    }
    if (ci>=0)
    {
        if (Task[id]->state!=state_enable) assert(0);
        Task[id]->running=1;
        current=Task[id];
    }
    else
    {
        idle_task[_cpu()].running=1;
        current=&idle_task[_cpu()];
    }
    unlock(&task_lock);
    return &(current->context);
}
static void idle_entry()
{
    _intr_write(1);
    while (1) _yield();
}
void kmt_init()
{
    
    os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
    
    os->on_irq(INT_MAX, _EVENT_NULL, kmt_schedule);
    task_cnt=0;
    
    lock_init(&task_lock,"task_lock");
    for(int i=0;i<_ncpu();++i)
    {
        core_state[i]=0;
        currents[i]=&idle_task[i];
        last_currents[i]=NULL;
        idle_task[i].state=state_enable;
        idle_task[i].running=0;
        idle_task[i].susp=0;
        idle_task[i].id=-1;
        idle_task[i].current_inode=-1;
        for (int j=0;j<16;++j) idle_task[i].fd[j].inode=0xffffffff;
        idle_task[i].fd_cnt=0;
        idle_task[i].stack=pmm->alloc(STACK_SIZE);
        idle_task[i].context= *_kcontext((_Area){(void*)((uintptr_t)idle_task[i].stack),(void*)((uintptr_t)idle_task[i].stack+STACK_SIZE)},idle_entry,NULL);
    }
}

MODULE_DEF(kmt)={
    .init=kmt_init,
    .create=task_create,
    .teardown=task_teardown,
    .spin_init=lock_init,
    .spin_lock=lock,
    .spin_unlock=unlock,
    .sem_init=sem_init,
    .sem_wait=sem_wait,
    .sem_signal=sem_signal,
};