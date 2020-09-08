ret=vfs->mkdir("/etw1ld63cgzmq");
ret=vfs->chdir("etw1ld63cgzmq");
ret=vfs->mkdir("/etw1ld63cgzmq/d9afza5vpuemop");
ret=vfs->chdir("d9afza5vpuemop");
{
  fd = vfs->open("/etw1ld63cgzmq/d9afza5vpuemop/b3.txt", O_CREAT | O_WRONLY);
	ret=vfs->write(fd,"12345",5);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/d9afza5vpuemop/a-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/d9afza5vpuemop/a2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/etw1ld63cgzmq/jcf8z7r7pn0r2");
ret=vfs->chdir("jcf8z7r7pn0r2");
{
  fd = vfs->open("/etw1ld63cgzmq/jcf8z7r7pn0r2/b1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/jcf8z7r7pn0r2/a3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/jcf8z7r7pn0r2/a2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/jcf8z7r7pn0r2/a-1.txt", O_CREAT | O_RDWR);
ret=vfs->write(fd,"12345678",8);
	ret=vfs->lseek(fd,-4,SEEK_END);
	ret=vfs->read(fd,ss,4);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/jcf8z7r7pn0r2/a-3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
vfs->mkdir("1dgjoc8ebhmhzf");
ret=vfs->chdir("/etw1ld63cgzmq/1dgjoc8ebhmhzf");
{
  fd = vfs->open("/etw1ld63cgzmq/1dgjoc8ebhmhzf/a3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/1dgjoc8ebhmhzf/a-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/1dgjoc8ebhmhzf/a1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
{
  fd = vfs->open("/etw1ld63cgzmq/a-1.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/a-3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/etw1ld63cgzmq/a1.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("h51ctvjkyfefmod");
ret=vfs->chdir("/h51ctvjkyfefmod");
ret=vfs->mkdir("vc8rip4wswit");
ret=vfs->chdir("/h51ctvjkyfefmod/vc8rip4wswit");
{
  fd = vfs->open("/h51ctvjkyfefmod/vc8rip4wswit/c-1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/vc8rip4wswit/c3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/vc8rip4wswit/b3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/vc8rip4wswit/c-2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/h51ctvjkyfefmod/2759ac5ut3d0m");
ret=vfs->chdir("2759ac5ut3d0m");
{
  fd = vfs->open("/h51ctvjkyfefmod/2759ac5ut3d0m/c-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/2759ac5ut3d0m/a2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/2759ac5ut3d0m/b3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/2759ac5ut3d0m/b-2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/h51ctvjkyfefmod/mtrlg4zfbr2h");
ret=vfs->chdir("mtrlg4zfbr2h");
{
  fd = vfs->open("/h51ctvjkyfefmod/mtrlg4zfbr2h/b1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/mtrlg4zfbr2h/a-1.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/h51ctvjkyfefmod/xc531x8lnysaij");
ret=vfs->chdir("xc531x8lnysaij");
{
  fd = vfs->open("/h51ctvjkyfefmod/xc531x8lnysaij/a1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/xc531x8lnysaij/c3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/h51ctvjkyfefmod/4pdtl7etzvt0");
ret=vfs->chdir("4pdtl7etzvt0");
{
  fd = vfs->open("/h51ctvjkyfefmod/4pdtl7etzvt0/a3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/4pdtl7etzvt0/a-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/h51ctvjkyfefmod/4pdtl7etzvt0/c-2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
{
  fd = vfs->open("/h51ctvjkyfefmod/b1.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/hsiysh7mcz2xm3");
ret=vfs->chdir("hsiysh7mcz2xm3");
ret=vfs->mkdir("cnjgm3yx8jg5j");
ret=vfs->chdir("/hsiysh7mcz2xm3/cnjgm3yx8jg5j");
{
  fd = vfs->open("/hsiysh7mcz2xm3/cnjgm3yx8jg5j/b-2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/cnjgm3yx8jg5j/a-3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/cnjgm3yx8jg5j/b2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/hsiysh7mcz2xm3/ef7ceoicta2vkj3");
ret=vfs->chdir("ef7ceoicta2vkj3");
{
  fd = vfs->open("/hsiysh7mcz2xm3/ef7ceoicta2vkj3/b2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/ef7ceoicta2vkj3/c-1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/ef7ceoicta2vkj3/a-2.txt", O_CREAT | O_RDWR);
	ret=vfs->write(fd,"abcd",4);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/ef7ceoicta2vkj3/c3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("dkipsvdc40j5fjw");
ret=vfs->chdir("/hsiysh7mcz2xm3/dkipsvdc40j5fjw");
{
  fd = vfs->open("/hsiysh7mcz2xm3/dkipsvdc40j5fjw/a-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/dkipsvdc40j5fjw/c2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/dkipsvdc40j5fjw/c-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/dkipsvdc40j5fjw/b2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
{
  fd = vfs->open("/hsiysh7mcz2xm3/b-1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/a-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/c1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/hsiysh7mcz2xm3/a3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/jw3f4nsa23antht8");
ret=vfs->chdir("jw3f4nsa23antht8");
ret=vfs->mkdir("6706thj11fg0");
ret=vfs->chdir("/jw3f4nsa23antht8/6706thj11fg0");
{
  fd = vfs->open("/jw3f4nsa23antht8/6706thj11fg0/a-1.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("b5uqfwehwbwwlao");
ret=vfs->chdir("/jw3f4nsa23antht8/b5uqfwehwbwwlao");
{
  fd = vfs->open("/jw3f4nsa23antht8/b5uqfwehwbwwlao/a2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/jw3f4nsa23antht8/asxbojl3h4wq");
ret=vfs->chdir("asxbojl3h4wq");
{
  fd = vfs->open("/jw3f4nsa23antht8/asxbojl3h4wq/a-3.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("ulfg8tkyjiou6ppl");
ret=vfs->chdir("/jw3f4nsa23antht8/ulfg8tkyjiou6ppl");
{
  fd = vfs->open("/jw3f4nsa23antht8/ulfg8tkyjiou6ppl/b2.txt", O_CREAT | O_RDWR);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
ret=vfs->mkdir("/jw3f4nsa23antht8/fp2x7dy0a0u2nx");
ret=vfs->chdir("fp2x7dy0a0u2nx");
{
  fd = vfs->open("/jw3f4nsa23antht8/fp2x7dy0a0u2nx/c-2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/jw3f4nsa23antht8/fp2x7dy0a0u2nx/b3.txt", O_CREAT | O_RDWR);
	ret=vfs->write(fd,"ppqqrrrng",9);
	ret=vfs->lseek(fd,6,SEEK_SET);
	ret=vfs->read(fd,ss,3);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
{
  fd = vfs->open("/jw3f4nsa23antht8/a1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/jw3f4nsa23antht8/b1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/jw3f4nsa23antht8/a2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
ret=vfs->chdir("..");
{
  fd = vfs->open("/b-3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/b3.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/c-1.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
{
  fd = vfs->open("/c2.txt", O_CREAT | O_WRONLY);
  if (fd > 0) vfs->close(fd);
}
