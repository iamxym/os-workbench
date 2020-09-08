#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include<stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
const char MY_DIRECT[]="/home/zhanghaonan/zhn/tmp/";
//const char MY_DIRECT[]="/tmp/";
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define ATTR_LONG_NAME (ATTR_READ_ONLY |ATTR_HIDDEN |ATTR_SYSTEM |ATTR_VOLUME_ID)
#define SIZE_DIR (sizeof(struct DIR))
struct fat_header{
    //手册附录2
    uint8_t boot_jump[3];//boot code所在地址
    uint8_t OEMName[8];//MSDOS5.0
    uint16_t BytePerSec;//每个sector中的byte数，本实验中为512
    uint8_t SecPerClus;//每个cluster中的sector数，本实验中为8
    uint16_t ReserSec;//保留区的sector个数
    uint8_t fat_num;//0x2
    uint32_t rtentry_cnt:16;//must be 0
    uint32_t sec_16_cnt:16;//must be 0
    uint8_t MediaID;
    uint32_t FatSz16:16;//must be 0
    uint32_t SecPerTrack:16;//0x3f
    uint32_t head_num:16;//0xff
    uint32_t hidsec_num;//0x3f
    uint32_t sec_32_cnt;//volume上的sector总数
    uint32_t sec_32_Perfat_cnt;//每个FAT中setor个数
    uint32_t Exflags:16;//
    uint32_t version:16;//must be 0x0
    uint32_t start_clus;//根目录下第一个cluster的序号
    uint32_t info_sec:16;//关于FSInfo所在的sector，一般是1
    uint32_t backup_sec:16;//0或6
    uint32_t reserved[3];//must be 0
    uint8_t drive_num;//0x80或0
    uint8_t reserved2;//must be 0x0
    uint8_t Exsign;//详见P12
    uint32_t serial_num;//
    uint8_t label[11];//
    uint8_t FStype[8];//为字符串"FAT32   "
    uint8_t reserved3[420];//0x0
    uint32_t Sign:16;//设为0x55AA

}__attribute__((packed));
struct DIR{
    /*FAT directory
    存储其他文件和子目录，由一系列32字节的目录entry构成，
    每个目录entry表示一个文件/子目录
    见手册P23
    */
   uint8_t name[11];//short name 至多11个字符
   uint8_t attr;//表示文件属性
   uint8_t NTRes;//must be set to 0
   uint8_t crt_time_tenth;//文件创建时间，以10s为单位，范围0~199
   uint16_t crt_time;//文件创建时间，以2s为单位
   uint16_t crt_date;//创建日期
   uint16_t lst_acc_date;//最后一次访问日期
   uint16_t fstclus_high;//该entry描述的文件/目录的第一个data cluster的高位字
   uint16_t wrt_time;//最后一次修改时间
   uint16_t wrt_date;//最后一次修改日期
   uint16_t fstclus_low;//该entry描述的文件/目录的第一个data cluster的低位字
   uint32_t size;//描述文件/目录的大小，以byte为单位
}__attribute__((packed));
struct LDIR{
    //用于长命名的额外目录，必须紧跟在相应短名称目录(DIR)之前(?)
    //注意LDIR倒序存储，即最后一个entry先存储，接下来时entry n-1,entry n-2,...,entry 1
    uint8_t ord;//所有长命名部分中的序号，注意最后一个entry(最先存储)的内容被0x40 mask
    uint16_t name1[5];//字符1～5
    uint8_t attr;//必须设置为ATTR_LONG_NAME才知道是长命名目录，即attr==ATTR_LONG_NAME_MASK
    uint8_t type;//must be set to 0
    uint8_t checksum;//对应短命名entry的名称校验和(at end?)
    uint16_t name2[6];//字符6～11
    uint16_t fstclus_low;//must be set to 0
    uint16_t name3[2];//字符12～13
}__attribute__((packed));
struct bmp_header{
    //14字节
    uint16_t bfType;//必须为"BM",0x424D
    uint32_t bfSize;//bmp文件大小
    uint16_t bfReserverd1;
    uint16_t bfReserverd2;
    uint32_t bfOffset;//文件起始位置到像素数据的字节偏移量
}__attribute__((packed));
struct bmp_info{
    //40字节
    uint32_t biSize;//infoheader结构体大小
    uint32_t biWidth;//图像宽度
    uint32_t biHeight;//图像高度
    uint16_t biPlanes;    //2Bytes，图像数据平面，BMP存储RGB数据，因此总为1
    uint16_t biBitCount;   //2Bytes，图像像素位数
    uint32_t biCompression;     //4Bytes，0：不压缩，1：RLE8，2：RLE4
    uint32_t biSizeImage;       //4Bytes，4字节对齐的图像数据大小
    uint32_t biXPelsPerMeter;   //4 Bytes，用象素/米表示的水平分辨率
    uint32_t biYPelsPerMeter;   //4 Bytes，用象素/米表示的垂直分辨率
    uint32_t biClrUsed;          //4 Bytes，实际使用的调色板索引数，0：使用所有的调色板索引
    uint32_t biClrImportant;     //4 Bytes，重要的调色板索引数，0：所有的调色板索引都重要
}__attribute__((packed));
void sizetest();
unsigned char ChkSum (unsigned char *pFcbName)        
{               
    short FcbNameLen;               
    unsigned char Sum;               
    Sum = 0;               
    for (FcbNameLen=11; FcbNameLen!=0; FcbNameLen--) 
    {
        // NOTE: The operation is an unsigned char rotate right 
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    return (Sum); 
}
int ok_in(char a,char b,char c){return a<=b&&b<=c;}
int legal_chk(char *s)
{
    for (;*s;++s)
        if (ok_in('A',(*s),'Z')||ok_in('a',(*s),'z')||ok_in('0',*s,'9')||*s=='.');
        else return 0;
    return 1;
}

char bmp_name[305][305];
char bmp_path[305];
uint16_t tmp[128];
char sha1_str[305];
uint8_t bmp_buf[10000005];
uint8_t bmp_tmp[4096];
int bmp_cnt=0;

int bmp_check(uintptr_t begin,uintptr_t end,uint32_t byte_wid,int now_len)
{
 //   return 1;
    /*
    int kkk=0;
    for (int i=0;begin+i+2<end;++i)
    {
        if ((bmp_tmp[i]=='B'&&bmp_tmp[i+1]=='M'&&bmp_tmp[i+2]=='P')) ++kkk;
        
        //if (lianx>=byte_wid/3) return 0;
    }
    if (kkk>=2) {return 0;}*/
    if (now_len-54<=byte_wid) return 1;
    uint8_t c[3];
    int32_t dc[3];
    int sb=0,nz=0;
    for (int i=0;i+2<byte_wid;i+=3,++nz)
    {
        
        c[0]=bmp_tmp[i];
        c[1]=bmp_tmp[i+1];
        c[2]=bmp_tmp[i+2];
        dc[0]=bmp_buf[now_len+i-byte_wid];
        dc[1]=bmp_buf[now_len+i-byte_wid+1];
        dc[2]=bmp_buf[now_len+i-byte_wid+2];
        dc[0]-=c[0];
        dc[1]-=c[1];
        dc[2]-=c[2];
        dc[0]=dc[0]*dc[0];
        dc[1]=dc[1]*dc[1];
        dc[2]=dc[2]*dc[2];
        //if (bmp_cnt<=5) printf("%s=%d\n",bmp_name[bmp_cnt],dc[0]+dc[1]+dc[2]);
        if (dc[0]+dc[1]+dc[2]>10000) ++sb;
    }
    //printf("%d %d\n",sb,nz);
    return sb*3<=nz;
}
int main(int argc, char *argv[]) {
    //sizetest();
    if (argc!=2) {assert(0);puts("Wrong args!");}


    char *file_path=argv[1];
    
    FILE *fp=fopen(file_path,"rb");
    if (fp==NULL) assert(0);
    fseek(fp,0,SEEK_END);
    uint32_t file_size=ftell(fp);//镜像文件大小获得
    
    int fd=open(file_path,O_RDONLY);
    
    void *img_addr=mmap(NULL,file_size,PROT_READ,MAP_SHARED,fd,0);//现在addr~addr+file_size是镜像内容了
    //if ((uintptr_t)img_addr%4096!=0) {printf("%c %c\n",0xf4,0xff);assert(0);}
    struct fat_header* f_header=img_addr;

    if (img_addr==MAP_FAILED) {assert(0);puts("mmap error");}
    if (strncmp((char*)&f_header->FStype,"FAT32",5)==0&&f_header->Sign==0xaa55);
    else {assert(0);printf("%x\n",f_header->Sign);puts("fat header error!");}
    
    void *clus_addr=(void*)((uintptr_t)img_addr+(f_header->ReserSec+f_header->fat_num*f_header->sec_32_Perfat_cnt+f_header->hidsec_num)*f_header->BytePerSec);
    uintptr_t addr=((uintptr_t)clus_addr);
    close(fd);
    fclose(fp);
    struct DIR *dir;
    struct LDIR *ldir;
    struct bmp_header* b_header;
    struct bmp_info* b_info;
    
    for (;addr<(uintptr_t)img_addr+file_size;addr+=SIZE_DIR)
    {
        dir=(struct DIR*)addr;
        if (dir->attr==ATTR_LONG_NAME)//long dir
        {
            continue;
        }
        else if ((strncmp((char*)&dir->name[8],"bmp",3)==0||strncmp((char*)&dir->name[8],"BMP",3)==0)&&dir->NTRes==0)//short dir
        {
            memset(bmp_path,0,sizeof(bmp_path));
            memset(bmp_name,0,sizeof(bmp_name));
            memset(tmp,0,sizeof(tmp));
            
            int flag=0;
            for (int oo=0;oo<6;++oo)
                if ((dir->name[oo]>='A'&&dir->name[oo]<='Z')||(dir->name[oo]>='a'&&dir->name[oo]<='z')||(dir->name[oo]>='0'&&dir->name[oo]<='9'));
                else {flag=1;break;}
            //printf("%d %s\n",(int)dir->name[0]&127,dir->name);
            if (flag) continue;
            //从DIR中读出文件名
            int tmp_len=0;
            uint8_t chksum=ChkSum((unsigned char*)dir->name);
            
            ldir=(struct LDIR *)(addr-SIZE_DIR);
            
            if (ldir->attr!=ATTR_LONG_NAME) {continue;assert(0);}
            
            for (int oid=1,ii=1;(uintptr_t)((void*)ldir)>=(uintptr_t)clus_addr&&ii<=10;++ii)
            {
                if (ldir->checksum==chksum&&(ldir->ord&0x0f)==oid)
                {
                    ++oid;
                    for (int o=0;o<5;++o) tmp[tmp_len++]=ldir->name1[o];
                    for (int o=0;o<6;++o) tmp[tmp_len++]=ldir->name2[o];
                    for (int o=0;o<2;++o) tmp[tmp_len++]=ldir->name3[o];
                }
                if ((ldir->ord&0x40)&&ldir->checksum==chksum) break;
                else --ldir;
            }
            
            //bmp_name[++bmp_cnt]=(char*)malloc(tmp_len);
            ++bmp_cnt;
            for (int i=0;i<tmp_len;++i)
                if (tmp[i]==0x0000||tmp[i]==0xffff)
                {
                    bmp_name[bmp_cnt][i]='\0';
                    if (bmp_name[bmp_cnt][i-4]!='.'||bmp_name[bmp_cnt][i-3]!='b'||bmp_name[bmp_cnt][i-2]!='m'||bmp_name[bmp_cnt][i-1]!='p') flag=1;
                    break;
                }
                else bmp_name[bmp_cnt][i]=(char)tmp[i];
            //printf("name=%s\n",bmp_name[bmp_cnt]);
            
            //printf("%s\n",(char*)dir->name);
            //跳到对应的bmp_header
            void* bmp_addr=(void*)((((dir->fstclus_high<<16)|dir->fstclus_low)-f_header->start_clus)*f_header->SecPerClus*f_header->BytePerSec+(uintptr_t)clus_addr);
            b_header=(struct bmp_header*)bmp_addr;
            
            if (b_header->bfType!=0x4d42) {assert(0);puts("find bmp header error!");printf("%x\n",b_header->bfType);}
            
            //if (((uintptr_t)bmp_addr+54)/4096!=(uintptr_t)bmp_addr/4096) {flag=1;}
            
            
            uint32_t byte_wid,hei;
            int rem_size;
            b_info=(struct bmp_info*)((uintptr_t)bmp_addr+14);

            assert(b_info->biCompression==0);
            hei=b_info->biHeight;
            byte_wid=b_info->biWidth*3;
            if (byte_wid%4!=0) byte_wid+=(4-byte_wid%4);
            rem_size=hei*byte_wid;
            if (rem_size);
            //这里?
            if (rem_size+54!=b_header->bfSize||b_info->biSize!=40) continue;

            memcpy(bmp_buf,bmp_addr,14+40);//赋值bmp文件头
            
            uintptr_t now_ptr,now_addr;
            if (((uintptr_t)bmp_addr+54)%4096==0)
            now_ptr=(uintptr_t)bmp_addr+54;
            else
            {
                memcpy(bmp_buf+(b_header->bfSize-rem_size),(void*)((uintptr_t)bmp_addr+54),4096-((uintptr_t)bmp_addr+54)%4096);
                rem_size-=(4096-((uintptr_t)bmp_addr+54)%4096);
                if (rem_size<0) {printf("%c %c\n",0xf4,0xff);assert(0);}
                now_ptr=(uintptr_t)bmp_addr+54+(4096-((uintptr_t)bmp_addr+54)%4096);
            }
            if (now_ptr);
            for (int delta;rem_size>0&&now_ptr<(uintptr_t)img_addr+file_size;now_ptr+=4096)
            {
                delta=(rem_size>4096?4096:rem_size);
                memcpy(bmp_tmp,(void*)now_ptr,delta);
                if (bmp_check(now_ptr,now_ptr+delta,byte_wid,b_header->bfSize-rem_size))
                {
                    memcpy(bmp_buf+(b_header->bfSize-rem_size),(void*)now_ptr,delta);
                    rem_size-=delta;
                }
                //else puts("YES");
                
            }
            if (rem_size<0) {printf("%c %c\n",0xf4,0xff);assert(0);}
            if (rem_size>0) continue;
            //printf("%x %x\n",bmp_buf[0],bmp_buf[1]);
            //strcpy(bmp_path,"/home/zhanghaonan/zhn/tmp/");
            
            strcpy(bmp_path,"sha1sum ");
            strcat(bmp_path,MY_DIRECT);
            strcat(bmp_path,bmp_name[bmp_cnt]);
            //printf("fnnn=%s\n",bmp_path+8);
            FILE* bmp_fp=fopen(bmp_path+8,"w+");
            if (bmp_fp==NULL) {assert(0);}
            fwrite(bmp_buf,b_header->bfSize,1,bmp_fp);
            //fwrite((void*)((uintptr_t)bmp_addr),b_header->bfSize,1,bmp_fp);
            fclose(bmp_fp);
            FILE* num_fp = popen(bmp_path, "r");
            fscanf(num_fp, "%s", sha1_str); // Get it!
            pclose(num_fp);
            if (!flag&&strlen(sha1_str)==40&&legal_chk(bmp_path+strlen(MY_DIRECT)+8))
            printf("%s %s\n",sha1_str,bmp_path+strlen(MY_DIRECT)+8);
            
        }
        else//other
        {
            continue;
        }
        
        //fflush(stdout);
    }
}
//以下为debug相关函数
void sizetest()
{
    int siz=sizeof(struct fat_header);
    printf("%d\n",siz);
    if (sizeof(struct fat_header)!=512) assert(0);
    siz=sizeof(struct DIR);
    printf("%d\n",siz);
    siz=sizeof(struct LDIR);
    printf("%d\n",siz);
    siz=sizeof(struct bmp_header);
    printf("%d\n",siz);
    siz=sizeof(struct bmp_info);
    printf("%d\n",siz);
 
}