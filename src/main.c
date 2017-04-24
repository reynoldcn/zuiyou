//
//  main.c
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/9.
//  Copyright (c) 2017 WangZhuoqun. All rights reserved.
//

#include "public.h"
#include  <pthread.h>
#include <time.h>
#define DEFAULT_FILE_LOCATION "test.log"
#define MAX_LIST 8
#define HASH_LEN 719        //This length is a prime number
LogList g_astList[MAX_LIST];
LogHashNode g_astHashTable[HASH_LEN];
pthread_t g_astPthread[MAX_LIST];

char g_done[MAX_LIST+1];    //I should've used atomic_t instead of char*, but unfortunately
                            //atomic_t is only defined in Linux Kernel.

/*
初始化全局变量，没啥说的。
*/
void ListInit(void)
{
    UINT i = 0;
    for (; i < MAX_LIST; ++i)
    {
        memset(&g_astList[i], 0, sizeof(LogList));
        g_astList[i].uiMax      = KFIFO_LEN;
        g_astList[i].uiReadIdx  = 0;
        g_astList[i].uiWriteIdx = 0;
        memset(g_astList[i].astData, 0, sizeof(LogItem) * KFIFO_LEN);
    }
}

void ResultInit(void)
{
    UINT i = 0;
    for (; i < HASH_LEN; ++i)
    {
        memset(&g_astHashTable[i], 0, sizeof(LogHashNode));
        g_astHashTable[i].bExists = 0;
        pthread_mutex_init(&g_astHashTable[i].lock, NULL);
    }
}

void MapThreadInit(void)
{
    ULONG i;
    for (i = 0; i < MAX_LIST; ++i)
    {
        //stMsg.uiListID = i;
        pthread_create(&g_astPthread[i], NULL, (void *)TaskMapProc, (void *)i);
    }
}
/*
主线程，负责把raw data从log文件里读出，放入队列里
每一个队列都有一个线程负责处理，处理后的东西统一写入一个
hash表，最后把hash表里的东西拿出来，排好序，就是最终产品
*/
void MainThread_ReadFile(char *filename)
{
    FILE *fp = NULL;
    char szLine[DATA_LEN] = {0};
    LogItem *pstLogItem = (LogItem*)malloc(sizeof(LogItem));
    //LogItem *pstLogItemNew = (LogItem*)malloc(sizeof(LogItem));
    UINT flag = 0;
    if((fp = fopen(filename, "r")) == NULL)
    {
        DBG("File cannot be opened\n");
        return;
    }
    else
        DBG("Reading Log And Writing List\n");
    while(!feof(fp))
    {
        fgets(szLine, DATA_LEN, fp);
        if (feof(fp))
        {
            break;
        }
        //printf("%s\n", szLine);
        Data2LogItem(szLine, pstLogItem);
        while(ERROR_FAILED == writeList(pstLogItem, &g_astList[flag]))
        {
            //usleep(1000);
            //DBG("failed write\n");
            flag++;
            flag %= MAX_LIST;
        }
        
            //PRINT_LOGITEM(pstLogItemNew, "get");
    }
    //当整个log读完之后，没有额外分配一个消息结构体用于通知各线程
    //只是发送一个fResponseTime为负的元素进去（正常不会出现）
    //只要线程读到这个，就知道文件已经读完了，可以跳出了
    pstLogItem->fResponseTime = -1.0f;
    for (int i = 0; i < MAX_LIST; ++i)
    {
        while (ERROR_FAILED == writeList(pstLogItem, &g_astList[i]))
        {
            ;
        }
    }
    //sleep(5);
    g_done[MAX_LIST] = '\0';
    //每个队列读完后，都会把g_done字符串中的第n位置1，
    //当都是1，说明所有的队列都读完了，结束线程。
    //后续如果修改了队列数目MAX_LIST,这里也要改
    while (0 != strcmp(g_done, "11111111")) {
        usleep(1);
    }
    fclose(fp);
    free(pstLogItem);
}
#define TIME_STAT

#if 1
int main(int argc, const char * argv[])
{
    
    char filename[256] = DEFAULT_FILE_LOCATION;
#ifdef TIME_STAT
    time_t t_start, t_end;
    t_start = time(NULL);
#endif
#if 1
    if (argc == 2) {
        strcpy(filename, argv[1]);
    }
    else if (argc > 2)
    {
        printf("Too many arguments\n");
        return -1;
    }
    else if (argc < 2)
    {
        printf("No Parameters Set, Use Default\n");
    }
#endif

    ListInit();
    ResultInit();
    MapThreadInit();
    MainThread_ReadFile(filename);
#ifdef SHOW_RESULT
    int i = 0;
    for (; i < HASH_LEN; ++i) {
        LogHashNode a = g_astHashTable[i];
        LogResult *res = NULL;
        if (a.bExists) {
            res = &a.stValue;
            printf("%s, %d, %f\n", res->szIntfName, res->uiCountTotal, res->fResponseTimeTotal);
        }
    }
#endif
    TaskReduceProc(NULL);
#ifdef TIME_STAT
    t_end = time(NULL);
    printf("time: %.0f s\n", difftime(t_end,t_start));
#endif
    return 0;
}

#endif
