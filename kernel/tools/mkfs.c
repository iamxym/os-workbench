#include <user.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define MKFS_INODE_OFFSET (1024*1024)
#define MKFS_TABLE_OFFSET (32*1024+MKFS_INODE_OFFSET)
#define MKFS_DATA_OFFSET (256*1024+MKFS_TABLE_OFFSET)
#define PG_SIZE 4096
#define INF_32 0xffffffff//跳转表中的结束位
#define INF_64 0xffffffffffffffff
#define PROC_INODE 1
#define DEV_INODE 2
#define RANDOM_INODE 3
#define NULL_INODE 4
#define ZERO_INODE 5



//#define IMG_SIZE (64 * 1024 * 1024)
int str_to_num(char *str)
{
  int ret=0;
  while (*str!='\0') ret=ret*10+*str-'0',str++;
  return ret;
}
struct inode{
    int size;//文件大小bytes
    uint8_t type;//T_DIR或T_FILE
    uint8_t acc;//文件权限
    int links;//链接次数（有多少个文件名以及多少个fd指向它）
    uint32_t inode_id;//inode的编号，用于识别文件
    uint16_t blocks;//block个数
    int begin_block,end_block;//起始/末尾block的编号
    intptr_t sem;
}__attribute__((packed));
typedef struct inode inode_t;
uint32_t jmp_table[64*1024];
inode_t arr[1024];
int arr_cnt,block_cnt;//分别用来对inode个数，inode编号和block个数来计数
uint32_t ed_flag[1024];
uint8_t *disk;
struct ufs_dirent now_dir;
char name[257],data[4097];
void mmap_write(uintptr_t addr,void* data,int num)
{
  uint8_t *dat=(uint8_t*)data;
  for (int i=0;i<num;++i) *(disk+addr+i)=*(dat+i);
}
void mkfs_init()
{
  for (int i=0;i<1024;++i) ed_flag[i]=0xffffffff;
  for (int i=0;i<64*1024;++i) jmp_table[i]=0xffffffff;
  memset(arr,0xff,sizeof(arr));
  for (int i=0;i<1024;++i) arr[i].inode_id=i;
  // /
  arr[0].size=3*sizeof(struct ufs_dirent);
  arr[0].type=T_DIR;
  //arr[0].access=O_RDWR;
  arr[0].links=1;
  arr[0].inode_id=0;
  arr[0].blocks=1;
  arr[0].begin_block=arr[0].end_block=0;
  jmp_table[0]=0;
  // /proc
  arr[1].size=sizeof(struct ufs_dirent);
  arr[1].type=T_DIR;
  //arr[1].access=O_RDONLY;
  arr[1].links=1;
  arr[1].inode_id=PROC_INODE;
  arr[1].blocks=1;
  arr[1].begin_block=arr[1].end_block=1;
  jmp_table[1]=1;
  // /dev
  arr[2].size=4*sizeof(struct ufs_dirent);
  arr[2].type=T_DIR;
  //arr[2].access=O_RDONLY;
  arr[2].links=1;
  arr[2].inode_id=DEV_INODE;
  arr[2].blocks=1;
  arr[2].begin_block=arr[1].end_block=2;
  jmp_table[2]=2;
  // /dev/random
  arr[3].size=0;
  arr[3].type=T_FILE;
  //arr[3].access=O_RDONLY;
  arr[3].links=1;
  arr[3].inode_id=RANDOM_INODE;
  arr[3].blocks=0;
  // /dev/null
  arr[4].size=0;
  arr[4].type=T_FILE;
  //arr[4].access=O_RDWR;
  arr[4].links=1;
  arr[4].inode_id=NULL_INODE;
  arr[4].blocks=0;
  // /dev/zero
  arr[5].size=0;
  arr[5].type=T_FILE;
  //arr[5].access=O_RDONLY;
  arr[5].links=1;
  arr[5].inode_id=ZERO_INODE;
  arr[5].blocks=0;

  mmap_write(MKFS_DATA_OFFSET,ed_flag,PG_SIZE);
  now_dir.inode=0;strncpy(now_dir.name,"..\0",3);
  mmap_write(MKFS_DATA_OFFSET,&now_dir,sizeof(struct ufs_dirent));
  now_dir.inode=PROC_INODE;strncpy(now_dir.name,"proc\0",5);
  mmap_write(MKFS_DATA_OFFSET+sizeof(struct ufs_dirent),&now_dir,sizeof(struct ufs_dirent));
  now_dir.inode=DEV_INODE;strncpy(now_dir.name,"dev\0",4);
  mmap_write(MKFS_DATA_OFFSET+2*sizeof(struct ufs_dirent),&now_dir,sizeof(struct ufs_dirent));

  //对/proc目录下的内容初始化
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE,ed_flag,PG_SIZE);
  now_dir.inode=0;strncpy(now_dir.name,"..\0",3);
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE,&now_dir,sizeof(struct ufs_dirent));
  //对/dev目录下的内容初始化
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE*2,ed_flag,PG_SIZE);//新目录创建时要往里面全填1，判定起始下没有文件在目录中

  now_dir.inode=0;strncpy(now_dir.name,"..\0",3);
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE*2,&now_dir,sizeof(struct ufs_dirent));
  now_dir.inode=RANDOM_INODE;strncpy(now_dir.name,"random\0",7);
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE*2+sizeof(struct ufs_dirent),&now_dir,sizeof(struct ufs_dirent));
  now_dir.inode=NULL_INODE;strncpy(now_dir.name,"null\0",5);
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE*2+sizeof(struct ufs_dirent)*2,&now_dir,sizeof(struct ufs_dirent));
  now_dir.inode=ZERO_INODE;strncpy(now_dir.name,"zero\0",5);
  mmap_write(MKFS_DATA_OFFSET+PG_SIZE*2+sizeof(struct ufs_dirent)*3,&now_dir,sizeof(struct ufs_dirent));
  
  arr_cnt=6;
  block_cnt=3;
  
}
void search_dir(int pid)
{
  FILE *fp=popen("ls -F","r");
  char buf[64];
  int len,sid;
  while (fgets(buf,64,fp)!=NULL)
  {
    
    len=strlen(buf);
    //printf("%d %s",len,buf);
    --len;
    //i~last
    sid=arr_cnt++;
    if (buf[len-1]=='/')//是目录
    {
      /*TODO：向我们的文件的该目录中mkdir该目录项*/
      
      arr[sid].size=sizeof(struct ufs_dirent);
      arr[sid].type=T_DIR;
      //arr[sid].access=O_RDWR;
      arr[sid].links=1;
      arr[sid].inode_id=sid;
      arr[sid].blocks=1;
      arr[sid].begin_block=arr[sid].end_block=block_cnt;
      jmp_table[block_cnt]=block_cnt;//分配对应块
      mmap_write(MKFS_DATA_OFFSET+arr[sid].begin_block*PG_SIZE,ed_flag,PG_SIZE);//将对应数据区清零
      now_dir.inode=arr[pid].inode_id;strncpy(now_dir.name,"..\0",3);
      mmap_write(MKFS_DATA_OFFSET+arr[sid].begin_block*PG_SIZE,&now_dir,sizeof(struct ufs_dirent));
      ++block_cnt;

      

      strncpy(name,buf,len-1);
      name[len-1]='\0';
      
      //向其中添加该目录项
      memcpy(now_dir.name,name,len);
      //printf("dir=%s\n",now_dir.name);
      now_dir.inode=arr[sid].inode_id;
      mmap_write(MKFS_DATA_OFFSET+arr[pid].begin_block*PG_SIZE+arr[pid].size,&now_dir,sizeof(struct ufs_dirent));
      arr[pid].size+=sizeof(struct ufs_dirent);

      assert(chdir(name)==0);
      search_dir(sid);
    }
    else
    {
      /*TODO：向我们的文件的该目录中open/create该文件项*/
      arr[sid].size=0;
      arr[sid].type=T_FILE;
      //arr[sid].access=O_RDWR;
      arr[sid].links=1;
      arr[sid].inode_id=sid;
      arr[sid].blocks=1;
      arr[sid].begin_block=arr[sid].end_block=block_cnt;
      jmp_table[block_cnt]=block_cnt;//分配对应块
      ++block_cnt;
      

      strncpy(name,buf,len);
      name[len]='\0';
      
      //向其中添加该目录项
      memcpy(now_dir.name,name,len+1);
      //printf("file=%s\n",now_dir.name);
      now_dir.inode=arr[sid].inode_id;
      mmap_write(MKFS_DATA_OFFSET+arr[pid].begin_block*PG_SIZE+arr[pid].size,&now_dir,sizeof(struct ufs_dirent));
      arr[pid].size+=sizeof(struct ufs_dirent);
      //printf("%s\n",name);
      int fd=open(name,O_RDONLY),rd_size;
      do 
      {
        //memset(data,0,sizeof(data));
        rd_size=read(fd,data,4096);
        //printf("rd_size=%d\n",rd_size);
        //printf("data=%s\n",data);
        /*TODO：向我们的文件中写入大小为rd_size的data*/
        data[rd_size]=0;
        //printf("%s\n",data);
        mmap_write(MKFS_DATA_OFFSET+arr[sid].end_block*PG_SIZE,data,rd_size);
        arr[sid].size+=rd_size;
        
        if (rd_size==4096)
        {
          jmp_table[arr[sid].end_block]=block_cnt;
          arr[sid].end_block=block_cnt;
          ++arr[sid].blocks;  
          jmp_table[block_cnt]=block_cnt;//分配对应块
          
          ++block_cnt;
        }
      }while(rd_size==4096);
    }
    
  }
  //free(buf);
  pclose(fp);
  chdir("..");
}
char ss[4096];
void mkfs_wrback()
{
  
  mmap_write(MKFS_INODE_OFFSET,arr,sizeof(arr));
  mmap_write(MKFS_TABLE_OFFSET,jmp_table,sizeof(jmp_table));
  //printf("%d\n",arr_cnt);
  /*uintptr_t ptr=((uintptr_t)disk+MKFS_DATA_OFFSET+sizeof(struct ufs_dirent));
  struct ufs_dirent* now=(void*)ptr;
  printf("%d %d\n",MKFS_DATA_OFFSET-MKFS_TABLE_OFFSET,(int)sizeof(jmp_table));
  printf("%d %s\n",now->inode,now->name);*/
}
int main(int argc, char *argv[]) {
  int fd;
  

  // TODO: argument parsing
  if (argc!=4) {puts("error argc!");assert(0);}
  
  assert((fd = open(argv[2], O_RDWR)) > 0);
  int img_size=str_to_num(argv[1])*1024*1024;
  //printf("%d\n",img_size);
  assert((ftruncate(fd, img_size)) == 0);
  assert((disk = mmap(NULL, img_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) != (void *)-1);
  mkfs_init();
  
  char init_path[128];
  strcpy(init_path,argv[3]);
  assert(chdir(init_path)==0);
  search_dir(0);
  mkfs_wrback();
  // TODO: mkfs
  
  munmap(disk, img_size);
  close(fd);
}
