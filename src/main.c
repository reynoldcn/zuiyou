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
        //printf("%s\n", szLine);
        Data2LogItem(szLine, pstLogItem);
        while(ERROR_FAILED == writeList(pstLogItem, &g_astList[flag]))
        {
            //usleep(1000);
            //DBG("failed write\n");
            flag++;
            flag %= 4;
        }
        
            //PRINT_LOGITEM(pstLogItemNew, "get");
    }
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
    }
    printf("%d, %s, %s\n", argc, argv[0], filename);
#endif

    ListInit();
    ResultInit();
    MapThreadInit();

    MainThread_ReadFile(filename);
    sleep(5);
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
