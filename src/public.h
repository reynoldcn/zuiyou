//
//  public.h
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/9.
//  Copyright (c) 2017 WangZhuoqun. All rights reserved.
//
//  Since I am very lazy, I am having all structures and functions integrated
//  right here. Do not be distrubed, I promise I will not mess anything up.
//  GOOD LUCK~

#ifndef zuiyou_public_h
#define zuiyou_public_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define DATA_LEN 128
#define KFIFO_LEN 128

#define IN
#define OUT
#define INOUT

#define ERROR_SUCCESS 0
#define ERROR_FAILED 1
#define ERROR_NULL 2

#define DBG_PRINT
#ifdef DBG_PRINT
#define DBG(format, args...) printf("DBG: "format, ##args)
#define PRINT_LOGITEM(plogitem, str) \
    printf(str" LOG: %s\t%s\t%f\n",\
    (plogitem)->szDate, (plogitem)->szIntfName, (plogitem)->fResponseTime)
#else
#define DBG(format, args...)
#define PRINT_LOGITEM(logitem, str);
#endif

typedef unsigned int UINT;

typedef struct tagDate{
    UINT uiYear;
    UINT uiMonth;
    UINT uiDay;
    UINT uiHour;
    UINT uiMin;
    UINT uiSec;
}Date;

typedef struct tagLogItem{
#ifdef DETAIL_DATE
    Date stDate;
#else
    char szDate[DATA_LEN];
#endif
    char szIntfName[DATA_LEN];
    float fResponseTime;
}LogItem;

UINT Data2Date(IN char *szData, OUT Date *pstDate);
UINT Data2LogItem(IN char *szData, OUT LogItem *pstLogItem);

typedef struct tagLogList{
    UINT uiReadIdx;
    UINT uiWriteIdx;
    //UINT uiFreeNum;
    UINT uiMax;
    LogItem astData[KFIFO_LEN];
}LogList;



UINT writeList(IN LogItem *pstLogItem, INOUT LogList *pstList);
UINT readList(OUT LogItem *pstLogItem, INOUT LogList *pstList);

typedef enum tagMsgType{
    MSG_DONE,
    MSG_100,
}MsgType;

typedef struct tagMessage{
    UINT uiListID;
    MsgType enMsgType;
}Message;

void TaskMapProc(void *pArg);
void TaskReduceProc(void *pArg);
#endif
