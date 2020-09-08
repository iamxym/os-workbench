#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "co-test.h"

int g_count = 0;

static void add_count() {
    g_count++;
}

static int get_count() {
    return g_count;
}

static void work_loop(void *arg) {
    const char *s = (const char*)arg;
    
    for (int i = 0; i < 100; ++i) {
        printf("%s%d  ", s, get_count());
        add_count();
        co_yield();
    }
}

static void work(void *arg) {
    work_loop(arg);
}

static void test_1() {

    struct co *thd1 = co_start("thread-1", work, "X");
    
    struct co *thd2 = co_start("thread-2", work, "Y");
    
    co_wait(thd1);
    //return;
    co_wait(thd2);

//    printf("\n");
}

// -----------------------------------------------

static int g_running = 1;

static void do_produce(Queue *queue) {
    assert(!q_is_full(queue));
    Item *item = (Item*)malloc(sizeof(Item));
    if (!item) {
        fprintf(stderr, "New item failure\n");
        return;
    }
    item->data = (char*)malloc(10);
    if (!item->data) {
        fprintf(stderr, "New data failure\n");
        free(item);
        return;
    }
    memset(item->data, 0, 10);
    sprintf(item->data, "libco-%d", g_count++);
    q_push(queue, item);
}

static void producer(void *arg) {
    Queue *queue = (Queue*)arg;
    for (int i = 0; i < 100; ) {
        if (!q_is_full(queue)) {
            // co_yield();
            //printf("%d\n",i);
            do_produce(queue);
            i += 1;
        }
        co_yield();
    }
}

static void do_consume(Queue *queue) {
    assert(!q_is_empty(queue));

    Item *item = q_pop(queue);
    if (item) {
        printf("%s  ", (char *)item->data);
        free(item->data);
        free(item);
    }
}

static void consumer(void *arg) {
    Queue *queue = (Queue*)arg;
    while (g_running) {
        if (!q_is_empty(queue)) {
            do_consume(queue);
        }
        co_yield();
    }
}

static void test_2() {

    Queue *queue = q_new();

    struct co *thd1 = co_start("producer-1", producer, queue);
    struct co *thd2 = co_start("producer-2", producer, queue);
    struct co *thd3 = co_start("consumer-1", consumer, queue);
    struct co *thd4 = co_start("consumer-2", consumer, queue);

    co_wait(thd1);
    
    co_wait(thd2);
    
    g_running = 0;
    
    co_wait(thd3);
    co_wait(thd4);
    
    while (!q_is_empty(queue)) {
        do_consume(queue);
    }

    q_free(queue);
}

int tot=0;
void haro(void *arg)
{
    const char *s = (const char*)arg;
    for (int val,i=1;i<=8;++i)
    {
        
        printf("haro begin%s\n",s);
        co_yield();
        printf("haro end%d\n",++tot);
    }
}
void jiejie(void *arg)
{
    const char *s = (const char*)arg;
    for (int i=1;i<=3;++i)
    {
        printf("jiejie begin\n");
        co_yield();
        printf("jiejie end%s\n",s);
    }
    
}
void loi(void *arg)
{
    const char *s = (const char*)arg;
    for (int i=1;i<=4;++i)
    {
        printf("loi begin\n");
        //co_yield();
        printf("loi end%s\n",s);
    }
    
}
char my_str[10]="haro";
static void test_3()
{
    
   for (int i=1;i<=100;++i)
    {
        struct co *thd1=co_start(my_str, haro,NULL);
struct co *thd2=co_start(my_str, haro,NULL);
struct co *thd3=co_start(my_str, haro,NULL);
struct co *thd4=co_start(my_str, haro,NULL);
struct co *thd5=co_start(my_str, haro,NULL);
struct co *thd6=co_start(my_str, haro,NULL);
struct co *thd7=co_start(my_str, haro,NULL);
struct co *thd8=co_start(my_str, haro,NULL);
struct co *thd9=co_start(my_str, haro,NULL);
struct co *thd10=co_start(my_str, haro,NULL);
struct co *thd11=co_start(my_str, haro,NULL);
struct co *thd12=co_start(my_str, haro,NULL);
struct co *thd13=co_start(my_str, haro,NULL);
struct co *thd14=co_start(my_str, haro,NULL);
struct co *thd15=co_start(my_str, haro,NULL);
struct co *thd16=co_start(my_str, haro,NULL);
struct co *thd17=co_start(my_str, haro,NULL);
struct co *thd18=co_start(my_str, haro,NULL);
struct co *thd19=co_start(my_str, haro,NULL);
struct co *thd20=co_start(my_str, haro,NULL);
struct co *thd21=co_start(my_str, haro,NULL);
struct co *thd22=co_start(my_str, haro,NULL);
struct co *thd23=co_start(my_str, haro,NULL);
struct co *thd24=co_start(my_str, haro,NULL);
struct co *thd25=co_start(my_str, haro,NULL);
struct co *thd26=co_start(my_str, haro,NULL);
struct co *thd27=co_start(my_str, haro,NULL);
struct co *thd28=co_start(my_str, haro,NULL);
struct co *thd29=co_start(my_str, haro,NULL);
struct co *thd30=co_start(my_str, haro,NULL);
struct co *thd31=co_start(my_str, haro,NULL);
struct co *thd32=co_start(my_str, haro,NULL);
struct co *thd33=co_start(my_str, haro,NULL);
struct co *thd34=co_start(my_str, haro,NULL);
struct co *thd35=co_start(my_str, haro,NULL);
co_wait(thd1);
co_wait(thd2);
co_wait(thd3);
co_wait(thd4);
co_wait(thd5);
co_wait(thd6);
co_wait(thd7);
co_wait(thd8);
co_wait(thd9);
co_wait(thd10);
co_wait(thd11);
co_wait(thd12);
co_wait(thd13);
co_wait(thd14);
co_wait(thd15);
co_wait(thd16);
co_wait(thd17);
co_wait(thd18);
co_wait(thd19);
co_wait(thd20);
co_wait(thd21);
struct co *thd36=co_start(my_str, haro,NULL);
struct co *thd37=co_start(my_str, haro,NULL);
struct co *thd38=co_start(my_str, haro,NULL);
struct co *thd39=co_start(my_str, haro,NULL);
struct co *thd40=co_start(my_str, haro,NULL);
struct co *thd41=co_start(my_str, haro,NULL);
struct co *thd42=co_start(my_str, haro,NULL);
struct co *thd43=co_start(my_str, haro,NULL);
struct co *thd44=co_start(my_str, haro,NULL);
struct co *thd45=co_start(my_str, haro,NULL);
struct co *thd46=co_start(my_str, haro,NULL);
struct co *thd47=co_start(my_str, haro,NULL);
struct co *thd48=co_start(my_str, haro,NULL);
struct co *thd49=co_start(my_str, haro,NULL);
struct co *thd50=co_start(my_str, haro,NULL);
struct co *thd51=co_start(my_str, haro,NULL);
struct co *thd52=co_start(my_str, haro,NULL);
struct co *thd53=co_start(my_str, haro,NULL);
struct co *thd54=co_start(my_str, haro,NULL);
struct co *thd55=co_start(my_str, haro,NULL);
struct co *thd56=co_start(my_str, haro,NULL);
struct co *thd57=co_start(my_str, haro,NULL);
struct co *thd58=co_start(my_str, haro,NULL);
struct co *thd59=co_start(my_str, haro,NULL);
struct co *thd60=co_start(my_str, haro,NULL);
struct co *thd61=co_start(my_str, haro,NULL);
struct co *thd62=co_start(my_str, haro,NULL);
struct co *thd63=co_start(my_str, haro,NULL);
struct co *thd64=co_start(my_str, haro,NULL);
struct co *thd65=co_start(my_str, haro,NULL);
struct co *thd66=co_start(my_str, haro,NULL);
struct co *thd67=co_start(my_str, haro,NULL);
struct co *thd68=co_start(my_str, haro,NULL);
struct co *thd69=co_start(my_str, haro,NULL);
struct co *thd70=co_start(my_str, haro,NULL);
struct co *thd71=co_start(my_str, haro,NULL);
struct co *thd72=co_start(my_str, haro,NULL);
struct co *thd73=co_start(my_str, haro,NULL);
struct co *thd74=co_start(my_str, haro,NULL);
struct co *thd75=co_start(my_str, haro,NULL);
struct co *thd76=co_start(my_str, haro,NULL);
struct co *thd77=co_start(my_str, haro,NULL);
struct co *thd78=co_start(my_str, haro,NULL);
struct co *thd79=co_start(my_str, haro,NULL);
struct co *thd80=co_start(my_str, haro,NULL);
struct co *thd81=co_start(my_str, haro,NULL);
struct co *thd82=co_start(my_str, haro,NULL);
struct co *thd83=co_start(my_str, haro,NULL);
struct co *thd84=co_start(my_str, haro,NULL);
struct co *thd85=co_start(my_str, haro,NULL);
struct co *thd86=co_start(my_str, haro,NULL);
struct co *thd87=co_start(my_str, haro,NULL);
struct co *thd88=co_start(my_str, haro,NULL);
struct co *thd89=co_start(my_str, haro,NULL);
struct co *thd90=co_start(my_str, haro,NULL);
struct co *thd91=co_start(my_str, haro,NULL);
struct co *thd92=co_start(my_str, haro,NULL);
struct co *thd93=co_start(my_str, haro,NULL);
struct co *thd94=co_start(my_str, haro,NULL);
struct co *thd95=co_start(my_str, haro,NULL);
struct co *thd96=co_start(my_str, haro,NULL);
struct co *thd97=co_start(my_str, haro,NULL);
struct co *thd98=co_start(my_str, haro,NULL);
struct co *thd99=co_start(my_str, haro,NULL);
struct co *thd100=co_start(my_str, haro,NULL);
struct co *thd101=co_start(my_str, haro,NULL);
struct co *thd102=co_start(my_str, haro,NULL);
struct co *thd103=co_start(my_str, haro,NULL);
struct co *thd104=co_start(my_str, haro,NULL);
struct co *thd105=co_start(my_str, haro,NULL);
struct co *thd106=co_start(my_str, haro,NULL);
struct co *thd107=co_start(my_str, haro,NULL);
struct co *thd108=co_start(my_str, haro,NULL);
struct co *thd109=co_start(my_str, haro,NULL);
struct co *thd110=co_start(my_str, haro,NULL);
struct co *thd111=co_start(my_str, haro,NULL);
struct co *thd112=co_start(my_str, haro,NULL);
struct co *thd113=co_start(my_str, haro,NULL);
struct co *thd114=co_start(my_str, haro,NULL);
struct co *thd115=co_start(my_str, haro,NULL);
struct co *thd116=co_start(my_str, haro,NULL);
struct co *thd117=co_start(my_str, haro,NULL);
struct co *thd118=co_start(my_str, haro,NULL);
struct co *thd119=co_start(my_str, haro,NULL);
struct co *thd120=co_start(my_str, haro,NULL);
struct co *thd121=co_start(my_str, haro,NULL);
struct co *thd122=co_start(my_str, haro,NULL);
struct co *thd123=co_start(my_str, haro,NULL);
struct co *thd124=co_start(my_str, haro,NULL);
struct co *thd125=co_start(my_str, haro,NULL);
struct co *thd126=co_start(my_str, haro,NULL);
struct co *thd127=co_start(my_str, haro,NULL);
for (int j=1;j<=1024;++j) co_yield();

co_wait(thd22);
co_wait(thd23);
co_wait(thd24);
co_wait(thd25);
co_wait(thd26);
co_wait(thd27);
co_wait(thd28);
co_wait(thd29);
co_wait(thd30);
co_wait(thd31);
co_wait(thd32);
co_wait(thd33);
co_wait(thd34);
co_wait(thd35);
co_wait(thd36);
co_wait(thd37);
co_wait(thd38);
co_wait(thd39);
co_wait(thd40);
co_wait(thd41);
co_wait(thd42);
co_wait(thd43);
co_wait(thd44);
co_wait(thd45);
co_wait(thd46);
co_wait(thd47);
co_wait(thd48);
co_wait(thd49);
co_wait(thd50);
co_wait(thd51);
co_wait(thd52);
co_wait(thd53);
co_wait(thd54);
co_wait(thd55);
co_wait(thd56);
co_wait(thd57);
co_wait(thd58);
co_wait(thd59);
co_wait(thd60);
co_wait(thd61);
co_wait(thd62);
co_wait(thd63);
co_wait(thd64);
co_wait(thd65);
co_wait(thd66);
co_wait(thd67);
co_wait(thd68);
co_wait(thd69);
co_wait(thd70);
co_wait(thd71);
co_wait(thd72);
co_wait(thd73);
co_wait(thd74);
co_wait(thd75);
co_wait(thd76);
co_wait(thd77);
co_wait(thd78);
co_wait(thd79);
co_wait(thd80);
co_wait(thd81);
co_wait(thd82);
co_wait(thd83);
co_wait(thd84);
co_wait(thd85);
co_wait(thd86);
co_wait(thd87);
co_wait(thd88);
co_wait(thd89);
co_wait(thd90);
co_wait(thd91);
co_wait(thd92);
co_wait(thd93);
co_wait(thd94);
co_wait(thd95);
co_wait(thd96);
co_wait(thd97);
co_wait(thd98);
co_wait(thd99);
co_wait(thd100);
co_wait(thd101);
co_wait(thd102);
co_wait(thd103);
co_wait(thd104);
co_wait(thd105);
co_wait(thd106);
co_wait(thd107);
co_wait(thd108);
co_wait(thd109);
co_wait(thd110);
co_wait(thd111);
co_wait(thd112);
co_wait(thd113);
co_wait(thd114);
co_wait(thd115);
co_wait(thd116);
co_wait(thd117);
co_wait(thd118);
co_wait(thd119);
co_wait(thd120);
co_wait(thd121);
co_wait(thd122);
co_wait(thd123);
co_wait(thd124);
co_wait(thd125);
co_wait(thd126);
co_wait(thd127);
    }
    
    
    
}
int main() {
    
    setbuf(stdout, NULL);

    printf("Test #1. Expect: (X|Y){0, 1, 2, ..., 199}\n");
    test_1();

    printf("\n\nTest #2. Expect: (libco-){200, 201, 202, ..., 399}\n");
    test_2();

    puts("test 3");
    //test_3();

    printf("\n\n");

    return 0;
}

