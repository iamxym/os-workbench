#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
//#include<stdio.h>
#define STACK_SIZE 64*1024
//每个协程分配64KiB的堆栈


enum co_status{
  CO_NEW=1,   //新建
  CO_RUNNING, //执行中
  CO_WAITING, //co_wait等待状态
  CO_DEAD,    //已经结束但还没有free
};
struct co {
  char* name;
  void (*func)(void *); //指定入口
  void *arg;            //传入参数
  enum co_status status;//当前状态
  struct co* waiter;    //正在等待当前协程的携程（父协程）
  struct co* next;      //用于队列化线程，记录后继
  jmp_buf context;      //用于setjmp时保存寄存器现场
  uintptr_t stack_ptr;  //指向当前栈顶的位置指针
  uint8_t stack[STACK_SIZE];//协程堆栈
  
};
int tot_co;
struct co *current=NULL;
struct co *co_head=NULL;
struct co* co_start(const char *name, void (*func)(void *), void *arg);
void co_yield();
void co_wait(struct co *co);

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

void wrapper(struct co* co)
{
  //struct co* co=(struct co*)arg;
  co->status=CO_RUNNING;
  co->func(co->arg);
  co->status=CO_DEAD;
  if (co->waiter) co->waiter->status=CO_RUNNING;
  co_yield();
}
void entry_all(struct co* co)
{
      if (co->status==CO_NEW)
      {
        current=co;
        stack_switch_call((void*)co->stack_ptr,wrapper,(uintptr_t)co);
      }
      else if (co->status==CO_RUNNING)
      {
        current=co;
        longjmp(co->context,1);
      }
     
}
struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  if (co_head==NULL)
  {
    co_head=(struct co*)malloc(sizeof(struct co));
    co_head->status=CO_RUNNING;
    co_head->next=NULL;
    co_head->waiter=NULL;
    co_head->name="main";
    current=co_head;
    tot_co=1;
  }
  struct co* new_co=NULL;
  new_co=(struct co*)malloc(sizeof(struct co));
  if (new_co==NULL) while(1);
  //strncpy(new_co->name,name,strlen(name));
  new_co->name=(char*)name;
  //return new_co;
  new_co->arg=arg;
  new_co->func=func;
  new_co->status=CO_NEW;
  new_co->waiter=NULL;
  new_co->next=NULL;
  ++tot_co;
  //memset(new_co->stack,0,sizeof(new_co->stack));
  new_co->stack_ptr=(uintptr_t)((((uintptr_t)new_co->stack+sizeof(new_co->stack))>>4)<<4);
  //new_co->stack_ptr=(uintptr_t)(((uintptr_t)new_co->stack+sizeof(new_co->stack)));
  struct co* now=co_head;
  while (now->next!=NULL) now=now->next;
  now->next=new_co;
  
  
  return new_co;
}


void co_wait(struct co *co) {
  
  current->status=CO_WAITING;
  co->waiter=current;
  while (co->status!=CO_DEAD)
  {
      co_yield();
  }
  if (co->status!=CO_DEAD) assert(0);
  
  
  co->waiter->status=CO_RUNNING;
    struct co* now=co_head;
    while(now->next!=co) now=now->next;
    now->next=co->next;
  free(co);
  --tot_co;
}
//int flag;
void co_yield() {
  //if (current==co_head) while(1);
  int val=setjmp(current->context);
  struct co* pre=current;
  
  if (val==0)
  {
    //需要切换进程
    struct co* now;
    int goal;
    do
    {
      now=co_head;
      goal=rand()%tot_co;
      while (goal--) now=now->next;
    }while (now->status!=CO_RUNNING&&now->status!=CO_NEW);
    //if (ans==NULL) {current=co_head;assert(0);return;}//???????
    entry_all(now);
  }
  else current=pre;

}
