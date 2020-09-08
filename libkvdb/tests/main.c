#include<stdio.h>
#include<stdlib.h>
#include"kvdb.h"
#include<string.h>
#include<assert.h>
void panic_on(int jud,const char* warn)
{
	if (jud!=0)
	{
		puts(warn);
		assert(0);
	}
}
int main() {
  struct kvdb *db;
  const char *key = "operating-systems";
  char *value;

  panic_on(!(db = kvdb_open("a.db")), "cannot open db"); // 打开数据库

  kvdb_put(db, key, "three-easy-pieces"); // db[key] = "three-easy-pieces"
  value = kvdb_get(db, key); // value = db[key];
  kvdb_close(db); // 关闭数据库
  printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}
