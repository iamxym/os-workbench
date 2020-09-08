#include<devices.h>

#define current currents[_cpu()]
#define cur_inode current->current_inode
#define INIT_INODE_OFFSET (1024*1024)//应该是1024*1024
#define INIT_TABLE_OFFSET (32*1024+INIT_INODE_OFFSET)//64KiB作为磁盘中跳转表区的开始，方便测试采用32K
#define INIT_DATA_OFFSET (256*1024+INIT_TABLE_OFFSET)//256KiB作为磁盘中data区的起始偏移,方便测试采用128K
#define INF_32 0xffffffff//跳转表中的结束位
#define INF_64 0xffffffffffffffff
#define PAGE_SIZE 4096
#define PROC_INODE 1
#define DEV_INODE 2
#define NULL_INODE 3
#define ZERO_INODE 4
#define RANDOM_INODE 5
//四个东西：写回writeback、/proc创建/删除task相关信息、mkfs、并行处理（信号量/锁）

/*
初步考虑/目录的inode编号为0，/proc的inode编号为2200 /dev的inode编号为7777

磁盘中主要区域分为3块：inode信息区(1024*32B)，跳转表(128K*4B)，data区（4K为一个block）

跳转表中每一项指向该文件的下一个block编号，如果该项编号指向其本身，则为最后一个block
而INF_32则表示该block空闲
1 修改/检索inode          lock_inode 可用于id_to_inode,del_fl,open,mkdir,link,data_alloc,wrback_inode等
2-1data区目录相关cpu_buf   使用lock_dirent或关中断 可用于dir_to_inode
2-2文件读写file            lock_file 可用于read,write,lseek
3 跳转表jmp_table         lock_jmp修改跳转表 可用于find_table
4 sda自身的read/write操作需要枷锁
*/
struct inode{
    int size;//文件大小bytes
    uint8_t type;//T_DIR或T_FILE
    uint8_t acc;//文件权限
    int links;//链接次数（有多少个文件名以及多少个fd指向它）
    uint32_t inode_id;//inode的编号，用于识别文件
    uint16_t blocks;//block个数，小于256M/4K=64K=2^16
    int begin_block,end_block;//起始/末尾block的编号
    sem_t *sem;
}__attribute__((packed));
typedef struct inode inode_t;
inode_t* inode_arr[1024];
extern task_t* currents[10];//最大CPU数为8
extern uint8_t core_state[10];
char cp_buf[PAGE_SIZE],ed_flag[PAGE_SIZE];
uint32_t jmp_table[PAGE_SIZE/sizeof(uint32_t)];
device_t* my_sda;
uint64_t inode_mex[64];//64*64=4096
sem_t inode_sem,jmp_sem,sda_sem,cpbuf_sem;//四个信号量分别对应inode访问，跳转表访问，sda的read/write和作为目录缓存的cpbuf
//int test_wat;
//int xiaoyimi;
static int min(int a,int b){return a>b?b:a;}
void vfs_init()
{
    //需要创建根目录‘/’,完成对线程目录/proc,设备目录/dev的挂载(mount)（可以考虑硬编码
    //printf("%d\n",sizeof(inode_t));
    //printf("???????\n");
    inode_t* tmp=NULL;
    for (int i=0;i<1024;++i)
    {
        inode_arr[i]=(inode_t *)pmm->alloc(sizeof(inode_t));
        memset(inode_arr[i],0xff,sizeof(inode_t));
        inode_arr[i]->inode_id=i;
    }
    for (int i=0;i<64;++i) inode_mex[i]=INF_64;
    for (int i=0;i<PAGE_SIZE;++i) ed_flag[i]=0xff;
    my_sda=dev->lookup("sda");
    if (my_sda==NULL) assert(0);
    
    
    for (int inode_cnt=0,i=INIT_INODE_OFFSET;i<INIT_TABLE_OFFSET&&inode_cnt<1024;i+=PAGE_SIZE)
    {
        
        my_sda->ops->read(my_sda,i,jmp_table,PAGE_SIZE);
        for (int j=0;j<PAGE_SIZE&&inode_cnt<1024;j+=sizeof(inode_t),++inode_cnt)
        {
            //xiaoyimi++;
            tmp=(inode_t*)(((void*)jmp_table)+j);
            
            if (tmp->size==INF_32) continue;
            inode_mex[tmp->inode_id/64]^=(1LL<<(tmp->inode_id%64));
            memcpy(inode_arr[inode_cnt],tmp,sizeof(inode_t));
            inode_arr[inode_cnt]->sem=(sem_t*)pmm->alloc(sizeof(sem_t));
            kmt->sem_init(inode_arr[inode_cnt]->sem,"777some inode",1);
            
        }
        
    }
    kmt->sem_init(&inode_sem,"777inode sem",1);
    kmt->sem_init(&jmp_sem,"777jmptable sem",1);
    kmt->sem_init(&sda_sem,"777sda sem",1);
    kmt->sem_init(&cpbuf_sem,"777cp_buf sem",1);
    //while (1);
    //TODO:把inode_mex处理一下
    /*//将跳转表各项初始化为0xffffffff
    for (int i=0;i<PAGE_SIZE/sizeof(uint32_t);++i) jmp_table[i]=INF_32;
    for (int i=0;i<(INIT_DATA_OFFSET-INIT_TABLE_OFFSET)/PAGE_SIZE;++i)
        my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+i*PAGE_SIZE,jmp_table,PAGE_SIZE);*/

    
}
void wrback_inode(int i)//定时向磁盘的inode区中写回inode链表
{
    //TODO
    kmt->sem_wait(&inode_sem);
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_INODE_OFFSET+i*sizeof(inode_t),inode_arr[i],sizeof(inode_t));
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(&inode_sem);
    /*kmt->sem_wait(&inode_sem);
    struct list_inode *now=inode_head;
    int addr=INIT_INODE_OFFSET;
    while (now!=NULL)
    {
        kmt->sem_wait(now->x.sem);
        kmt->sem_wait(&sda_sem);
        my_sda->ops->write(my_sda,addr,&(now->x),sizeof(inode_t));//末尾表项指向的下一block应为ret_page
        kmt->sem_signal(&sda_sem);
        kmt->sem_signal(now->x.sem);
        now=now->next;
        addr+=sizeof(inode_t);
    }
    kmt->sem_signal(&inode_sem);
    //末尾填0xff
    inode_t end_now;
    memset(&end_now,0xff,sizeof(inode_t));
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,addr,&end_now,sizeof(inode_t));
    kmt->sem_signal(&sda_sem);*/
}
//解析路径时，如果不是/开头，则相对于该线程的当前路径进行解析，否则绝对路径
//int xiaoyimi=-2;
static inode_t* id_to_inode(uint32_t id)
{
    //寻找以id为inode编号的文件，并返回其inode结构体
    inode_t* ret=NULL;
    kmt->sem_wait(&inode_sem);
    if (inode_arr[id]->size!=INF_32&&inode_arr[id]->inode_id==id)
       ret=inode_arr[id];
    kmt->sem_signal(&inode_sem);
    return ret;
}

static uint32_t dir_to_inode(inode_t * now,const char* path,int len)
{
    //从now指向的DIR寻找以path为文件名的FILE，并返回对应文件的inode编号
    
    uint32_t retr=INF_32;
    struct ufs_dirent* chk=NULL;
    kmt->sem_wait(&cpbuf_sem);
    kmt->sem_wait(now->sem);
    kmt->sem_wait(&sda_sem);
    my_sda->ops->read(my_sda,INIT_DATA_OFFSET+now->begin_block*PAGE_SIZE,cp_buf,PAGE_SIZE);
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(now->sem);
    for (int i=0;i<PAGE_SIZE;i+=sizeof(struct ufs_dirent))
    {
        chk=(struct ufs_dirent*)(cp_buf+i);
        //xiaoyimi=chk->inode+i;
        if (chk->inode!=INF_32&&strncmp(chk->name,path,len)==0) {retr=chk->inode;break;}
        
    }
    kmt->sem_signal(&cpbuf_sem);
    return retr;
}

static uint32_t find_path(const char* path,int len)
{
    //解析path的路径，并返回最终指向文件的inode编号
    if (len==0) return cur_inode;
    uint32_t tmp;
    int i;
    assert(path!=NULL);
    
    if (path[0]=='/') tmp=0,i=1;
    else tmp=cur_inode,i=0;
    
    inode_t* now;
    for (int last;i<len;i=last+2)
    {
        //++xiaoyimi;
        now=id_to_inode(tmp);
        
        assert(now!=NULL);
        last=i;
        while (last<len&&path[last]!='/') ++last;
        --last;
        if (!(now->type==T_DIR&&now->blocks==1)) return INF_32;
        
        
        tmp=dir_to_inode(now,path+i,last-i+1);
        
        //printf("%s %d\n",path+i,last-i+1);
        if (tmp==INF_32) return INF_32;
        
    }
    
    return tmp;

}
static int find_table(int id)
{
    kmt->sem_wait(&sda_sem);
    my_sda->ops->read(my_sda,INIT_TABLE_OFFSET+id*PAGE_SIZE,jmp_table,PAGE_SIZE);
    kmt->sem_signal(&sda_sem);
    for (int i=0;i<PAGE_SIZE/sizeof(uint32_t);++i)
        if (jmp_table[i]==INF_32)
            return i;//该block空闲
    return INF_32;
}
static void del_fl(inode_t* fl)
{
    int block_id=fl->begin_block,tmp_id=block_id,ed_flag=INF_32;
    kmt->sem_wait(&jmp_sem);
    do
    {
        block_id=tmp_id;
        kmt->sem_wait(&sda_sem);
        my_sda->ops->read(my_sda,INIT_TABLE_OFFSET+block_id*PAGE_SIZE,&tmp_id,4);
        my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+block_id*PAGE_SIZE,&ed_flag,4);
        kmt->sem_signal(&sda_sem);
    } while (block_id!=fl->end_block);
    kmt->sem_signal(&jmp_sem);
    kmt->sem_wait(&inode_sem);
    int iddd=fl->inode_id,di=fl->inode_id/64,rm=fl->inode_id%64;
    if (inode_mex[di]&(1<<rm)) assert(0);
    inode_mex[di]|=(1<<rm);
    pmm->free(fl->sem);
    memset(fl,0xff,sizeof(inode_t));
    fl->inode_id=iddd;
    kmt->sem_signal(&inode_sem);
    //pmm->free(fl);
    
    //xiaoyimi需要写回
    //wrback_inode();
}
static uint32_t data_alloc(inode_t* now)//申请一块空闲的data区并建立list
{
    uint32_t ret_page=INF_32;
    kmt->sem_wait(&jmp_sem);
    for (int i=0;i<(INIT_DATA_OFFSET-INIT_TABLE_OFFSET)/PAGE_SIZE;++i)
    {
        
        ret_page=find_table(i);
        if (ret_page!=INF_32)
        {
            ret_page+=+i*(PAGE_SIZE/sizeof(uint32_t));
            break;
        }
    }
    if (ret_page==INF_32) return INF_32;
    //kmt->sem_wait(now->x.sem);//???
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+now->end_block*4,&ret_page,sizeof(uint32_t));//末尾表项指向的下一block应为ret_page
    my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+ret_page*4,&ret_page,sizeof(uint32_t));//ret_page作为新的末尾表项
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(&jmp_sem);
    
    ++now->blocks;
    now->end_block=ret_page;
    //kmt->sem_signal(now->x.sem);
    //xiaoyimi需要写回
    //wrback_inode();
    return ret_page;
}
static void share_work(int fd)
{
    //dup的文件描述符共享偏移量
    for (int i=0;i<16;++i)
        if (current->fd[i].inode==current->fd[fd].inode&&current->fd[i].share==current->fd[fd].share)
        {
            current->fd[i].offset=current->fd[fd].offset;
            current->fd[i].block_id=current->fd[fd].block_id;
        }
}
//int xiaoyimi;
int vfs_chdir(const char *path)
{
    uint32_t goal=find_path(path,strlen(path));
    //xiaoyimi=goal;
    if (goal==INF_32) return -1;
    cur_inode=goal;
    return 0;
}
int vfs_open(const char *pathname, int flags)
{
    //打开 path,如打开成功，返回一个非负整数编号的文件描述符 (最小的未使用的文件描述符），否则返回-1
    
    uint32_t goal=find_path(pathname,strlen(pathname));
    inode_t* fl;
    if (goal==INF_32)//该目录下没有该文件
    {
        
        if (flags&O_CREAT)
        {
            //创建文件
            //如/home/zhn/a.txt不存在，则退回到/home/zhn的目录上去
            //寻找空闲的inode编号
            uint64_t nid=0;
            uint32_t cc=0;
            kmt->sem_wait(&inode_sem);
            for (int i=0;i<64;++i)
                if (inode_mex[i]!=0)
                {
                    nid=(inode_mex[i]&(-inode_mex[i]));
                    inode_mex[i]^=nid;
                    cc=i*64;
                    break;
                }
            kmt->sem_signal(&inode_sem);
            while (nid) ++cc,nid>>=1;
            --cc;

            inode_t *new_fl=inode_arr[cc];
            new_fl->inode_id=cc;//inode的编号，用于识别文件
            new_fl->size=0;
            new_fl->type=T_FILE;
            new_fl->links=1;
            new_fl->sem=(sem_t*)pmm->alloc(sizeof(sem_t));
            kmt->sem_init(new_fl->sem,"777 some inode",1);
            

            new_fl->blocks=1;//block个数
            //寻找空闲的block
            uint32_t ret_page=INF_32;
            kmt->sem_wait(&jmp_sem);
            for (int i=0;i<(INIT_DATA_OFFSET-INIT_TABLE_OFFSET)/PAGE_SIZE;++i)
            {
                
                ret_page=find_table(i);
                if (ret_page!=INF_32)
                {
                    ret_page+=+i*(PAGE_SIZE/sizeof(uint32_t));
                    break;
                }
            }
            if (ret_page==INF_32) return -1;
            new_fl->begin_block=new_fl->end_block=ret_page;//起始block的编号
            kmt->sem_wait(&sda_sem);
            my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+ret_page*4,&ret_page,sizeof(uint32_t));//向跳转表中写入
            kmt->sem_signal(&sda_sem);
            kmt->sem_signal(&jmp_sem);
            int len=strlen(pathname);
            while (len>0&&pathname[len-1]!='/') --len;
            if (len!=0)//类似/home/a.txt的形式
                goal=find_path(pathname,len);
            else//类似“a.txt”的形式
                goal=cur_inode;
            if (goal==INF_32) return -1;
            fl=id_to_inode(goal);
            assert(fl->type==T_DIR);//fl对应文件应为目录
            //if (fl->x.access==O_RDONLY&&core_state[_cpu()]==0) return -1;//不是内核态且为只读目录
            

            //寻找父目录fl中空的项
            kmt->sem_wait(fl->sem);
            fl->size+=sizeof(struct ufs_dirent);
            kmt->sem_signal(fl->sem);
            kmt->sem_wait(&cpbuf_sem);
            kmt->sem_wait(&sda_sem);
            my_sda->ops->read(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE,cp_buf,PAGE_SIZE);
            kmt->sem_signal(&sda_sem);
            struct ufs_dirent* chk;
            struct ufs_dirent tcp;
            tcp.inode=new_fl->inode_id;
            memset(tcp.name,0,sizeof(tcp.name));
            strcpy(tcp.name,pathname+len);
            ret_page=INF_32;
            for (int i=0;i<PAGE_SIZE;i+=sizeof(struct ufs_dirent))
            {
                chk=(struct ufs_dirent*)(cp_buf+i);
                if (chk->inode==INF_32) {ret_page=i;break;}
            }
            
            if (ret_page==INF_32) return -1;
            
            kmt->sem_wait(&sda_sem);
            my_sda->ops->write(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE+ret_page,&tcp,sizeof(struct ufs_dirent));
            kmt->sem_signal(&sda_sem);
            kmt->sem_signal(&cpbuf_sem);
            //可能要lock的地方
            wrback_inode(fl->inode_id);
            fl=new_fl;
        }
        else
            return -1;
        
    }
    else
    {
        fl=id_to_inode(goal);
        if (fl->type==T_DIR&&(flags&(~O_CREAT))!=O_RDONLY) return -1;
        //if (fl->x.access==O_RDONLY&&(flags&(~O_CREAT))!=O_RDONLY) return -1;
        //if (fl->x.access==O_WRONLY&&(flags&(~O_CREAT))!=O_WRONLY) return -1;
    }
    int fd=-1;
    for (int i=0;i<16;++i)
        if (current->fd[i].inode==INF_32)//空闲状态的fd
        {
            fd=i;
            current->fd[i].inode=fl->inode_id;
            current->fd[i].access=(flags&(~O_CREAT));
            current->fd[i].offset=0;
            current->fd[i].block_id=fl->begin_block;
            current->fd[i].share=++current->fd_cnt;
            break;
        }
    kmt->sem_wait(fl->sem);
    ++fl->links;
    kmt->sem_signal(fl->sem);
    //xiaoyimi需要写回
    wrback_inode(fl->inode_id);
        //else printf("%d\n",current->fd[i].inode);
    return fd;
}
int vfs_close(int fd)
{
    //关闭一个文件描述符
    if (fd>=0&&fd<=15);
    else return -1;
    if (current->fd[fd].inode!=INF_32)
    {
        inode_t* fl=id_to_inode(current->fd[fd].inode);
        kmt->sem_wait(fl->sem);
        if (--fl->links==0) del_fl(fl);
        kmt->sem_signal(fl->sem);
        //xiaoyimi需要写回
        wrback_inode(fl->inode_id);
        current->fd[fd].inode=INF_32;
        return 0;
    }
    return -1;
}
//通过调整游标offset来访问fd指向的文件数据
int vfs_write(int fd, void *buf, int count)
{
    
    if (current->fd[fd].access==O_RDONLY&&core_state[_cpu()]==0) return -1;
    if (current->fd[fd].inode==INF_32) return -1;
    if (current->fd[fd].inode==NULL_INODE)//    /dev/null文件
        return count;
    inode_t* now=id_to_inode(current->fd[fd].inode);
    kmt->sem_wait(now->sem);
    int buf_cnt=0,buf_rem=count,siz;
    while (buf_rem>0)
    {
        siz=min(PAGE_SIZE-current->fd[fd].offset%PAGE_SIZE,buf_rem);
        kmt->sem_wait(&sda_sem);
        my_sda->ops->write(my_sda,INIT_DATA_OFFSET+current->fd[fd].block_id*PAGE_SIZE+current->fd[fd].offset%PAGE_SIZE,buf+buf_cnt,siz);
        kmt->sem_signal(&sda_sem);
        buf_cnt+=siz;
        buf_rem-=siz;
        current->fd[fd].offset+=siz;
        if (current->fd[fd].offset/PAGE_SIZE>(current->fd[fd].offset-siz)/PAGE_SIZE)//到了下一页
        {
            if (current->fd[fd].block_id==now->end_block)//需要申请新的一页
            {
                current->fd[fd].block_id=data_alloc(now);//申请一块新的data block
                if (current->fd[fd].block_id==INF_32) assert(0);
            }
            else
            {
                kmt->sem_wait(&jmp_sem);
                kmt->sem_wait(&sda_sem);
                
                my_sda->ops->read(my_sda,INIT_TABLE_OFFSET+current->fd[fd].block_id*4,&(current->fd[fd].block_id),sizeof(uint32_t));
                kmt->sem_signal(&sda_sem);
                kmt->sem_signal(&jmp_sem);
            }
            
        }
    }
    //更改文件大小
    if (current->fd[fd].offset>now->size) now->size=current->fd[fd].offset;
    kmt->sem_signal(now->sem);
    share_work(fd);
    wrback_inode(current->fd[fd].inode);
    return count;
    
}

int vfs_read(int fd, void *buf, int count)
{
    //if (current->fd[fd].inode==INF_32) return 0;
    //assert(current->fd[fd].access!=O_WRONLY);
    if (current->fd[fd].inode==INF_32||current->fd[fd].access==O_WRONLY) return -1;
    if (current->fd[fd].inode==NULL_INODE) return 0;
    else if (current->fd[fd].inode==ZERO_INODE)
    {
        memset(buf,0,count);
        return count;
    }
    else if (current->fd[fd].inode==RANDOM_INODE)
    {
        uint8_t* ptr=buf;int n=count;
        while (n--) *ptr=rand()%255,++ptr;
        return count;
    }
    inode_t* now=id_to_inode(current->fd[fd].inode);
    kmt->sem_wait(now->sem);
    int buf_cnt=0,buf_rem=min(now->size-current->fd[fd].offset,count),siz;
    
    while (buf_rem>0)
    {
        
        siz=min(PAGE_SIZE-current->fd[fd].offset%PAGE_SIZE,buf_rem);
        kmt->sem_wait(&sda_sem);
        my_sda->ops->read(my_sda,INIT_DATA_OFFSET+current->fd[fd].block_id*PAGE_SIZE+current->fd[fd].offset%PAGE_SIZE,buf+buf_cnt,siz);
        kmt->sem_signal(&sda_sem);
        buf_cnt+=siz;
        buf_rem-=siz;
        current->fd[fd].offset+=siz;
        if (current->fd[fd].offset/PAGE_SIZE>(current->fd[fd].offset-siz)/PAGE_SIZE)//到了下一页
        {
            kmt->sem_wait(&jmp_sem);
            kmt->sem_wait(&sda_sem);
            my_sda->ops->read(my_sda,INIT_TABLE_OFFSET+current->fd[fd].block_id*4,&(current->fd[fd].block_id),sizeof(uint32_t));
            kmt->sem_signal(&sda_sem);
            kmt->sem_signal(&jmp_sem);
        }
    }
    kmt->sem_signal(now->sem);
    share_work(fd);
    wrback_inode(current->fd[fd].inode);
    return buf_cnt;
}

int vfs_lseek(int fd, int offset, int whence)
{
    if (current->fd[fd].inode==INF_32) return -1;
    if (current->fd[fd].inode==RANDOM_INODE||current->fd[fd].inode==NULL_INODE||current->fd[fd].inode==ZERO_INODE) return 0;
    //assert(current->fd[fd].inode!=INF_32);
    int true_off;
    inode_t* now=id_to_inode(current->fd[fd].inode);
    kmt->sem_wait(now->sem);
    if (whence==SEEK_CUR) true_off=current->fd[fd].offset+offset;
    else if (whence==SEEK_SET) true_off=offset;
    else true_off=now->size+offset;
    current->fd[fd].offset=true_off;

    if (true_off<((uint32_t)now->blocks)*PAGE_SIZE)//未超出我们已有的block
    {
        uint32_t blo=now->begin_block;
        int rem=true_off;
        while (rem>=PAGE_SIZE)
        {
            kmt->sem_wait(&jmp_sem);
            kmt->sem_wait(&sda_sem);
            my_sda->ops->read(my_sda,INIT_TABLE_OFFSET+blo*4,&blo,sizeof(uint32_t));
            kmt->sem_signal(&sda_sem);
            kmt->sem_signal(&jmp_sem);
            rem-=PAGE_SIZE;
        }
        current->fd[fd].block_id=blo;
    }
    else
    {
        int rem=true_off-(uint32_t)now->blocks*PAGE_SIZE;
        do
        {
            current->fd[fd].block_id=data_alloc(now);//申请一块新的data block
            if (current->fd[fd].block_id==INF_32) assert(0);
            rem-=PAGE_SIZE;
        } while (rem>=0);
        current->fd[fd].block_id=now->end_block;
    }
    kmt->sem_signal(now->sem);
    share_work(fd);
    //if (current->fd[2].offset!=true_off||current->fd[2].block_id!=now->x.end_block) assert(0);
    wrback_inode(current->fd[fd].inode);
    return true_off;
}
int vfs_dup(int fd)
{
    //assert(current->fd[fd].inode!=INF_32);
    if (current->fd[fd].inode==INF_32) return -1;
    //复制一份共享 offset 的文件描述符，返回最小可用的文件描述符。
    inode_t* fl=id_to_inode(current->fd[fd].inode);
    kmt->sem_wait(fl->sem);
    fl->links++;
    kmt->sem_signal(fl->sem);
    //xiaoyimi需要写回
    wrback_inode(fl->inode_id);
    for (int i=0;i<16;++i)
        if (current->fd[i].inode==INF_32)
        {
            current->fd[i].inode=current->fd[fd].inode;
            current->fd[i].access=current->fd[fd].access;
            current->fd[i].block_id=current->fd[fd].block_id;
            current->fd[i].offset=current->fd[fd].offset;
            current->fd[i].share=current->fd[fd].share;
            return i;
        }
    return -1;
}

int vfs_mkdir(const char *pathname)
{
       
    uint32_t goal=find_path(pathname,strlen(pathname));
    
    
    if (goal!=INF_32) return -1;
    //printf("%d\n",(int)goal);
    //创建目录
    //寻找空闲的inode编号
    uint64_t nid=0;
    uint32_t cc=0;
    //kmt->sem_wait(&inode_sem);
    for (int i=0;i<64;++i)
        if (inode_mex[i]!=0)
        {
            nid=(inode_mex[i]&(-inode_mex[i]));
            inode_mex[i]^=nid;
            cc=i*64;
            break;
        }
    //kmt->sem_signal(&inode_sem);
    while (nid) ++cc,nid>>=1;
    --cc;
    //printf("%d\n",cc);
    inode_t *new_fl=inode_arr[cc];
    new_fl->inode_id=cc;//inode的编号，用于识别文件
    new_fl->size=sizeof(struct ufs_dirent);
    new_fl->type=T_DIR;
    //new_fl->x.access=O_RDWR;
    new_fl->links=1;
    new_fl->sem=(sem_t*)pmm->alloc(sizeof(sem_t));
    kmt->sem_init(new_fl->sem,"777 some inode",1);
    
    
    //printf("new dir's inode is %d\n",cc);

    new_fl->blocks=1;//block个数
    //寻找空闲的block
    uint32_t ret_page=INF_32;
    kmt->sem_wait(&jmp_sem);
    for (int i=0;i<(INIT_DATA_OFFSET-INIT_TABLE_OFFSET)/PAGE_SIZE;++i)
    {
        
        ret_page=find_table(i);
        if (ret_page!=INF_32)
        {
            ret_page+=i*(PAGE_SIZE/sizeof(uint32_t));
            break;
        }
    }
    
    if (ret_page==INF_32) return -1;
    
    new_fl->begin_block=new_fl->end_block=ret_page;//起始block的编号
    //kmt->sem_wait(&cpbuf_sem);
    //memset(cp_buf,0xff,sizeof(cp_buf));//将对应该目录的block全部打上1，表示该目录为空
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_TABLE_OFFSET+ret_page*4,&ret_page,sizeof(uint32_t));//向跳转表中写入
    my_sda->ops->write(my_sda,INIT_DATA_OFFSET+ret_page*PAGE_SIZE,ed_flag,PAGE_SIZE);
    
    kmt->sem_signal(&sda_sem);
    //kmt->sem_signal(&cpbuf_sem);
    kmt->sem_signal(&jmp_sem);
    int len=strlen(pathname);
    while (len>0&&pathname[len-1]!='/') --len;
    //xiaoyimi=len;
    if (len!=0)//类似/home/zhn的形式，需要找到上层目录/home
        goal=find_path(pathname,len);
    else
        goal=cur_inode;
    
    //assert(goal!=INF_32);
    if (goal==INF_32) return -1;
    struct ufs_dirent tcp;
    strncpy(tcp.name,"..\0",3);tcp.inode=goal;
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_DATA_OFFSET+ret_page*PAGE_SIZE,&tcp,sizeof(struct ufs_dirent));
    kmt->sem_signal(&sda_sem);
    inode_t* fl=id_to_inode(goal);
    
    assert(fl->type==T_DIR);
    //if (fl->x.access==O_RDONLY&&core_state[_cpu()]==0) return -1;
    
    //assert(fl->x.type==T_DIR&&fl->x.access!=O_RDONLY);//fl对应文件应为目录

    //寻找父目录fl中空的项
    kmt->sem_wait(fl->sem);
    fl->size+=sizeof(struct ufs_dirent);
    kmt->sem_signal(fl->sem);
    kmt->sem_wait(&cpbuf_sem);
    kmt->sem_wait(&sda_sem);
    my_sda->ops->read(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE,cp_buf,PAGE_SIZE);
    kmt->sem_signal(&sda_sem);
    struct ufs_dirent* chk;
    
    tcp.inode=new_fl->inode_id;
    memset(tcp.name,0,sizeof(tcp.name));
    strcpy(tcp.name,pathname+len);
    ret_page=INF_32;
    for (int i=0;i<PAGE_SIZE;i+=sizeof(struct ufs_dirent))
    {
        chk=(struct ufs_dirent*)(cp_buf+i);
        if (chk->inode==INF_32) {ret_page=i;break;}
    }
    
    //assert(ret_page!=INF_32);
    if (ret_page==INF_32) return -1;
    
    kmt->sem_wait(&sda_sem);
    
    my_sda->ops->write(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE+ret_page,&tcp,sizeof(struct ufs_dirent));
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(&cpbuf_sem);
    
    //xiaoyimi需要写回
    wrback_inode(new_fl->inode_id);
    wrback_inode(fl->inode_id);
    return 0;
    
}
int vfs_link(const char *oldpath, const char *newpath)
{
    //创建链接
    uint32_t old_id=find_path(oldpath,strlen(oldpath)),new_id=find_path(newpath,strlen(newpath));
    if (old_id==INF_32||new_id!=INF_32) return -1;

    inode_t* ofl=id_to_inode(old_id);
    assert(ofl->type==T_FILE);
    kmt->sem_wait(ofl->sem);
    ofl->links++;
    kmt->sem_signal(ofl->sem);

    int len=strlen(newpath);
    while (len>0&&newpath[len-1]!='/') --len;
    if (len!=0)//类似/home/zhn的形式，需要找到上层目录/home，作为fl
        new_id=find_path(newpath,len);
    else
        new_id=cur_inode;
    if (new_id==INF_32) return -1;
    inode_t* fl=id_to_inode(new_id);
    //assert(fl->x.type==T_DIR&&fl->x.access!=O_RDONLY);//fl对应文件应为目录
    kmt->sem_wait(fl->sem);
    fl->size+=sizeof(struct ufs_dirent);
    kmt->sem_signal(fl->sem);
    //寻找fl目录中空的项
    kmt->sem_wait(&cpbuf_sem);
    kmt->sem_wait(&sda_sem);
    my_sda->ops->read(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE,cp_buf,PAGE_SIZE);
    kmt->sem_signal(&sda_sem);
    struct ufs_dirent* chk;
    struct ufs_dirent tcp;
    tcp.inode=ofl->inode_id;
    memset(tcp.name,0,sizeof(tcp.name));
    strcpy(tcp.name,newpath+len);
    int ret_page=INF_32;
    for (int i=0;i<PAGE_SIZE;i+=sizeof(struct ufs_dirent))
    {
        chk=(struct ufs_dirent*)(cp_buf+i);
        if (chk->inode==INF_32) {ret_page=i;break;}
    }
    
    if (ret_page==INF_32) return -1;
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE+ret_page,&tcp,sizeof(struct ufs_dirent));
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(&cpbuf_sem);
    //xiaoyimi需要写回
    wrback_inode(ofl->inode_id);
    wrback_inode(fl->inode_id);
    return 0;
}
int vfs_unlink(const char *pathname)
{
    //删除链接
    uint32_t goal=find_path(pathname,strlen(pathname)),fa_goal;
    if (goal==INF_32) return -1;
    int len=strlen(pathname);
    while (len>0&&pathname[len-1]!='/') --len;
    if (len!=0)//类似/home/zhn的形式，需要找到上层目录/home
        fa_goal=find_path(pathname,len);
    else
        fa_goal=cur_inode;
    if (fa_goal==INF_32||fa_goal==DEV_INODE||fa_goal==PROC_INODE) return -1;
    inode_t* fl=id_to_inode(fa_goal);
    
    kmt->sem_wait(fl->sem);
    fl->size-=sizeof(struct ufs_dirent);
    kmt->sem_signal(fl->sem);
    //将对应目录项清空
    kmt->sem_wait(&cpbuf_sem);
    kmt->sem_wait(&sda_sem);
    my_sda->ops->read(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE,cp_buf,PAGE_SIZE);
    kmt->sem_signal(&sda_sem);
    struct ufs_dirent* chk;
    struct ufs_dirent tcp;
    tcp.inode=INF_32;
    int ret_page=INF_32;
    for (int i=0;i<PAGE_SIZE;i+=sizeof(struct ufs_dirent))
    {
        chk=(struct ufs_dirent*)(cp_buf+i);
        if (chk->inode==goal) {ret_page=i;break;}
    }
    
    if (ret_page==INF_32) return -1;
    kmt->sem_wait(&sda_sem);
    my_sda->ops->write(my_sda,INIT_DATA_OFFSET+fl->begin_block*PAGE_SIZE+ret_page,&tcp,sizeof(struct ufs_dirent));
    kmt->sem_signal(&sda_sem);
    kmt->sem_signal(&cpbuf_sem);
    fl=id_to_inode(goal);
    kmt->sem_wait(fl->sem);
    if (--fl->links==0) del_fl(fl);//删除该文件
    kmt->sem_signal(fl->sem);
    //xiaoyimi需要写回
    wrback_inode(fl->inode_id);
    return 0;
}
int vfs_fstat(int fd, struct ufs_stat *buf)
{
    if (current->fd[fd].inode==INF_32) return -1;
    inode_t* now=id_to_inode(current->fd[fd].inode);
    kmt->sem_wait(now->sem);
    buf->id=now->inode_id+1;
    buf->type=now->type;
    buf->size=now->size;
    if (now->type==T_DIR) buf->size-=sizeof(struct ufs_dirent);
    kmt->sem_signal(now->sem);
    return 0;
}
//OJ会在kmt->create创建的线程中调用文件API，不会在操作系统启动/初始化/中断时执行对文件系统的操作。
MODULE_DEF(vfs) = {
  .init = vfs_init,
  .write =vfs_write,
  .read =vfs_read,
  .close =vfs_close,
  .open =vfs_open,
  .lseek =vfs_lseek,
  .link =vfs_link,
  .unlink =vfs_unlink,
  .fstat =vfs_fstat,
  .mkdir =vfs_mkdir,
  .chdir =vfs_chdir,
  .dup =vfs_dup,
  //.proc_mk=proc_mk,
};
