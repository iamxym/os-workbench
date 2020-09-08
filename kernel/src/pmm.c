#include <common.h>
/*
分配2 4 8 16 ... 2^12 共12种分配方式
每一种分配方式建立一个链表，链表中的每一项要包含一个4KiB的分配空间和一定的相关信息（例如bitmap至多2k bit=256B）
*/
#define page_size ((1<<12)+(1<<11)+(1<<10))
typedef struct _lock_t
{
  int flag;
}spin_lock;
spin_lock big_lock,test_lock;
uintptr_t now_heap;
struct page_info{
  uint8_t page[page_size];//4+2+1=7KiB的page，最多存的块数少于4096
  int p_cpu,type;//归属的cpu和分配大小2 4 ... 4096
  uint64_t bitmap[64];//64*8=512B=4096 bit
  struct page_info* next;
};
struct page_info* info_head[9][13];//不同的cpu和分配大小

static int xchg(volatile int *addr, int newval) {
  int result;
  asm volatile ("lock xchg %0, %1":
    "+m"(*addr), "=a"(result) : "1"(newval) : "cc");
  return result;
}

static inline void lock_init()
{
  big_lock.flag=0;
  test_lock.flag=0;
}
static inline void lock(spin_lock* big_lock){while (xchg(&(big_lock->flag),1));}
static inline void unlock(spin_lock* big_lock){xchg(&(big_lock->flag),0);}
static inline void new_info(struct page_info* now,int b_id,int now_cpu)//创建一个新的页信息
{
  if (now==NULL) return;
  now->p_cpu=now_cpu;
  now->type=(1<<b_id);
  for (int i=0;i<64;++i) now->bitmap[i]=0;
  now->next=NULL;
}
static inline int page_full(struct page_info* now)//判断当前页是否已满
{
  if (now==NULL) return 0;
  int i,bit_cnt=page_size/(now->type);
  uint64_t tmp=0;
  for (i=0;i<64;++i)
    if (now->bitmap[i]+1ULL==0ULL);
    else {tmp=now->bitmap[i];break;}
  assert(tmp+1ULL!=0ULL);
  i=i*64;//前面的块都不合适
  while (tmp&(1ULL)) tmp>>=(1ULL),++i;
  if (i+1<=bit_cnt) return 0;//块数还够
  else return 1;
}
static inline void* slow_alloc(size_t size)
{
  lock(&big_lock);
  void* ret;
  if (now_heap<size+(uintptr_t)_heap.start) ret=NULL;
  else now_heap-=size,ret=(void*)now_heap;
  unlock(&big_lock);
  return ret;
}
//以上为lock部分
static void *kalloc(size_t size) {
  int inter_flag = _intr_read();
  _intr_write(0);
  assert(size>=1&&size<=4096);
  int b_id=1,b_size;
  int now_cpu=_cpu();
  for (;(1<<b_id)<size;++b_id);
  b_size=(1<<b_id);
  
  assert(b_size<=4096);
  struct page_info* now=info_head[now_cpu][b_id],*pre=NULL;
  if (now==NULL)
  {
    info_head[now_cpu][b_id]=(struct page_info*)slow_alloc(8192);
    new_info(info_head[now_cpu][b_id],b_id,now_cpu);
    now=info_head[now_cpu][b_id];
    //申请一块4k page
  }
  else
  {
    while (now&&page_full(now)) pre=now,now=now->next;
    if (now==NULL)
    {
      
      now=(struct page_info*)slow_alloc(8192);
      new_info(now,b_id,now_cpu);
      pre->next=now;
    }
  }
  if (now==NULL) return NULL;
  uint64_t i,j;
  for (i=0;now->bitmap[i]+1ULL==0ULL;++i);
  assert(i<64);
  for (j=0;(1ULL<<j)&(now->bitmap[i]);++j);
  lock(&test_lock);
  now->bitmap[i]|=(1ULL<<j);//把对应位置设为1
  unlock(&test_lock);
  assert(j<64);
  assert((now->type)==b_size);
  if (inter_flag) _intr_write(1);
  return (void*)((uintptr_t)((uintptr_t)now+(i*64+j)*b_size));
}

static void kfree(void *ptr) {
  int inter_flag=_intr_read();
  _intr_write(0);
  uintptr_t ptr_addr=(uintptr_t)ptr,start_addr=(ptr_addr/8192)*8192;
  struct page_info *now=(struct page_info*)start_addr;
  uint64_t cnt=(ptr_addr-start_addr)/(now->type),i=cnt/64,j=cnt%64;
  assert((i*64+j)*(now->type)+start_addr==ptr_addr);
  assert(now->bitmap[i]&(1ULL<<j));
  lock(&test_lock);
  now->bitmap[i]^=(1ULL<<j);
  unlock(&test_lock);
  if (inter_flag) _intr_write(1);
}

static void pmm_init() {
  //uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  //printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);
  now_heap=(uintptr_t)_heap.end;
  lock_init();
  for (int i=0;i<9;++i)
    for (int j=1;j<=12;++j) info_head[i][j]=NULL;
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};