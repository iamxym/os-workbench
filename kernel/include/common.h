#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#include <sys/types.h>
#include <amdev.h>
#include<user.h>
#define INT_MIN (-2147483647-1)
#define INT_MAX (2147483647)
enum task_state{
    state_null=1,state_wait,state_enable,state_dead
    //分别表示空任务 处于信号量等待 正在执行 被中断保持睡眠 被teardown
};
#define STACK_SIZE 4096
typedef struct file_desc{
    uint32_t inode;//对应文件的inode编号
    int offset;//文件游标相对于开头的总偏移量
    int block_id;//文件游标所在的block编号
    int share;//共享游标的家族编号
    int access;//该文件描述符的读写权限
}fd_t;
typedef struct task{
    struct {
        char* name;
        _Context  context;
    };
    fd_t fd[16];
    uint8_t *stack;
    int fd_cnt,state,running,susp,id;//susp表示是否挂机（用于多CPU切换），running表示是否在某一CPU上运行
    int current_inode;//当前工作路径的inode编号
}task_t;
typedef struct spinlock{
    char name[128];
    int lock,last_inter,belong;
}spinlock_t;
typedef struct semaphore{
    struct spinlock lk;
    char name[128];
    int val,head,tail,qlen;
    struct task* queue[64];
}sem_t;
struct irq_list
{
    int seq,event;
    handler_t handler;
    struct irq_list* next;
};
