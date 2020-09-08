#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<ctype.h>
#define VAL_BIAS 300*(128+1+8+32+64)
struct kvdb {
  int header;//标志位，永远是4396777
  int fd;
  
  char buffer[VAL_BIAS+5];
  char chk_key[300];
  char buf[300];
  // your definition here
};

struct kvdb *kvdb_open(const char *filename) {
  struct kvdb* new_x=(struct kvdb*)malloc(sizeof(struct kvdb));
  
  if (new_x==NULL) {assert(0);return NULL;}
  new_x->header=4396777;
  new_x->fd=open(filename,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR);
  
  if (new_x->fd<=0) {return NULL;}
  
  return new_x;
}

int kvdb_close(struct kvdb *db) {
  if (db==NULL||db->header!=4396777) return -1;
  close(db->fd);
  
  db->header=0;
  free(db);
  return 0;
}
/*key 128 B
value 16MiB(绝大多是4KiB)
分为A、B两块，分别存储key-value信息与value的值
A存储方式为[key长度(8) key值(128) val长度(32) val末尾偏移(64) $(1)]
*/
int kvdb_put(struct kvdb *db, const char *key, const char *value) {

  flock(db->fd,LOCK_EX);
  
  int rem;
  long long bias=VAL_BIAS;//此时返回值为所有目录项大小

  int fd=db->fd,pointer=0;
  long long tmp;
  if (lseek(fd,0,SEEK_SET)!=0) assert(0);
  db->buffer[8+128+32+64]=' ';
  int lim=read(fd,db->buffer,VAL_BIAS);
  
  char ch;
  while(1)
  {
    pointer+=8+128+32+64;
    assert(pointer<VAL_BIAS);
    ch=db->buffer[pointer];
    if (ch=='$'&&pointer+1<=lim)
    {
      memcpy(db->buf,db->buffer+pointer-64,64);
      tmp=0;
      for (int i=0;i<64;++i) tmp=tmp*2+db->buf[i]-'0';
      bias=tmp;
      ++pointer;
    }
    else
    {
      pointer-=(8+128+32+64);
      break;
    }
  }
  rem=pointer;
  assert(rem<VAL_BIAS&&(pointer%(128+8+32+64+1)==0));
  int key_len=strlen(key),val_len=strlen(value);
  assert(key_len<=128&&val_len<=16*1024*1024);
  ch='$';
  memset(db->buf,' ',sizeof(db->buf));
  bias+=val_len;
  
  
  assert(bias>VAL_BIAS);
  for (int i=7;i>=0;--i) db->buf[7-i]=((key_len>>i)&1)+'0';
  memcpy(db->buf+8,key,key_len);
  for (int i=31;i>=0;--i) db->buf[128+8+31-i]=((val_len>>i)&1)+'0';
  for (int i=63;i>=0;--i) db->buf[128+8+32+63-i]=((bias>>i)&1)+'0';
  db->buf[8+128+32+64]=0;
  
  assert(bias-val_len>=VAL_BIAS);
  //printf("%s\n",db->buf);
  if (lseek(db->fd,rem,SEEK_SET)!=rem) assert(0);
  if (write(db->fd,db->buf,8+128+32+64)!=8+128+32+64) assert(0);
  fsync(db->fd);

  if (lseek(db->fd,bias-val_len,SEEK_SET)!=bias-val_len) assert(0);
  if (write(db->fd,value,val_len)!=val_len) assert(0);
  fsync(db->fd);

  assert(rem+(8+128+32+64+1)<VAL_BIAS);
  if (lseek(db->fd,rem+(8+128+32+64),SEEK_SET)!=rem+(8+128+32+64)) assert(0);
  
  if (write(db->fd,&ch,1)!=1) assert(0);//写入$
  fsync(db->fd);
  flock(db->fd,LOCK_UN);
  
  return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {

  flock(db->fd,LOCK_EX);
  int rem;//此时返回值为所有目录项大小

  int fd=db->fd,pointer=0;
  if (lseek(fd,0,SEEK_SET)!=0) assert(0);
  db->buffer[8+128+32+64]=' ';
  int lim=read(fd,db->buffer,VAL_BIAS);
  
  char ch;
  while(1)
  {
    pointer+=8+128+32+64;
    assert(pointer<VAL_BIAS);
    ch=db->buffer[pointer];
    if (ch=='$'&&pointer+1<=lim) ++pointer;
    else
    {
      pointer-=(8+128+32+64);
      break;
    }
  }
  rem=pointer;
  assert(rem<VAL_BIAS&&(pointer%(128+8+32+64+1)==0));
  //printf("%d\n",lseek(db->fd,0,SEEK_CUR));
  char* ret_val;
  int key_len,val_len,flag=0;
  long long bias;
  while (rem>0)
  {
    memset(db->chk_key,0,sizeof(db->chk_key));
    key_len=val_len=0;
    bias=0;
    rem-=(128+32+64+8+1);
    if (rem<0) assert(0);
    for (int i=0;i<8;++i) key_len=key_len*2+db->buffer[rem+i]-'0';
    memcpy(db->chk_key,db->buffer+rem+8,key_len);
    for (int i=0;i<32;++i) val_len=val_len*2+db->buffer[rem+8+128+i]-'0';
    for (int i=0;i<64;++i) bias=bias*2+db->buffer[rem+8+128+32+i]-'0';
    //printf("%d %d %d\n",bias,key_len,val_len);
    assert(key_len<=128&&val_len<=16*1024*1024&&bias-val_len>=VAL_BIAS);
    if (strcmp(key,db->chk_key)==0)
    {
      ret_val=malloc(val_len+5);
      if (ret_val==NULL) assert(0);
      if (lseek(db->fd,bias-val_len,SEEK_SET)!=bias-val_len) assert(0);
      if (read(db->fd,ret_val,val_len)!=val_len) assert(0);
      ret_val[val_len]='\0';
      flag=1;
      break;
    }
  }
  flock(db->fd,LOCK_UN);
  
  if (flag) return ret_val;
  else return NULL;
}
