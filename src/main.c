//
//  main.c
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/9.
//  Copyright (c) 2017 WangZhuoqun. All rights reserved.
//

#include "public.h"
#include  <pthread.h>

#define DEFAULT_FILE_LOCATION "../../data/server_access.log"
#define MAX_LIST 8
LogList g_astList[MAX_LIST];

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

void MainThread_ReadFile(char *filename)
{
    FILE *fp = NULL;
    char szLine[DATA_LEN] = {0};
    LogItem *pstLogItem = (LogItem*)malloc(sizeof(LogItem));
    //LogItem *pstLogItemNew = (LogItem*)malloc(sizeof(LogItem));

    if((fp = fopen(filename, "r")) == NULL)
    {
        DBG("File cannot be opened\n");
        return;
    }
    else
        DBG("Reading log\n");
    while(!feof(fp))
    {
        fgets(szLine, DATA_LEN, fp);
        Data2LogItem(szLine, pstLogItem);
        if(ERROR_FAILED == writeList(pstLogItem, &g_astList[0]))
        {
            DBG("failed write\n");
        }
            //PRINT_LOGITEM(pstLogItemNew, "get");
    }
    fclose(fp);
    free(pstLogItem);
}


int main(int argc, const char * argv[])
{
    
    char *filename = DEFAULT_FILE_LOCATION;
    pthread_t pth_id = 0;
    Message *pstMsg = (Message*)malloc(sizeof(Message));
    pstMsg->uiListID = 0;
    pstMsg->enMsgType = MSG_DONE;
    ListInit();
    pthread_create(&pth_id, NULL, (void *)TaskMapProc, (void *)pstMsg);
    MainThread_ReadFile(filename);
    
    
    
    return 0;
}
