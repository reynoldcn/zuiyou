//
//  public.c
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/9.
//  Copyright (c) 2017 WangZhuoqun. All rights reserved.
//


#include "public.h"
//#include <unistd.h>
#define BUF_SIZE 128
#define IS_NUM(x) ((x)>='0'&&(x)<='9')

#if 0
static inline UINT Sz2Int(char szBuf[BUF_SIZE], UINT len)
{
    UINT uiRet = 0;
    int i = len - 1;
    int digit = 1;
    for(; i >= 0; --i, digit *= 10)
    {
        uiRet += digit * (szBuf[i] - '0');
        //digit *= 10;
    }
    return uiRet;
}
#endif

static inline float Sz2Float(char szBuf[BUF_SIZE], UINT uiLen)
{
    float fRet = 0;
    int i = uiLen - 1;
    float digit = 1;
    //DBG("szBuf = %s\n", szBuf);
    for(; i >= 0; --i)
    {
        //DBG("c = %c ", szBuf[i]);
        if(szBuf[i] == '.')
        {
            fRet = fRet / digit;
            digit = 1;
            //DBG(" f = %f\n", fRet);
            continue;
        }
        //if (szBuf[i] >= '0' && szBuf[i] <= '9')
        if(IS_NUM(szBuf[i]))
        {
            fRet += digit * (szBuf[i] - '0');
            digit *= 10;
        }

        //DBG(" f = %f\n", fRet);
    }
    return fRet;
}

#ifdef DETAIL_DATE
UINT Data2Date(IN char szData[BUF_SIZE], OUT Date *pstDate)
{
    char szBuf[BUF_SIZE] = {0};
    char *pData = NULL;
    UINT uiBufIdx = 0;
    UINT uiSepCount = 0;//A Counter for septarators.
    pData = szData;
    
    while(pData++)
    {
        if(*pData == '/' || *pData == ':')
        {
            uiSepCount++;
            switch (uiSepCount)
            {
                case 1:
                    pstDate->uiDay = Sz2Int(szBuf, uiBufIdx);
                    uiBufIdx = 0;
                    break;
                    
                case 2:
                    //pstDate->uiMonth = Sz2Int(szBuf, uiBufIdx);
                    uiBufIdx = 0;
                    break;
                    
                case 3:
                    pstDate->uiYear = Sz2Int(szBuf, uiBufIdx);
                    uiBufIdx = 0;
                    break;
                    
                case 4:
                    pstDate->uiHour = Sz2Int(szBuf, uiBufIdx);
                    uiBufIdx = 0;
                    break;
                    
                case 5:
                    pstDate->uiMin = Sz2Int(szBuf, uiBufIdx);
                    uiBufIdx = 0;
                    break;
                    
                default:
                    //DBG("ERROR: Why the num of septarators over");
                    break;
            }
        }
        else
        {
            szBuf[uiBufIdx++] = *pData;
        }
        
    }
    szBuf[uiBufIdx] = '\0';
    pstDate->uiSec = Sz2Int(szBuf, uiBufIdx);
    
    return ERROR_SUCCESS;
}
#endif

UINT Data2LogItem(IN char *szData, OUT LogItem *pstLogItem)
{
    char szBuf[BUF_SIZE] = {0};
    //char *pData = NULL;
    UINT uiBufIdx = 0;
    UINT uiSepCount = 0;//A Counter for septarators.
    //pData = szData;
    UINT i = 0;
    while(szData[i])
    {
        if(BUF_SIZE == i + 1)
        {
            return ERROR_FAILED;
        }
        //DBG("%c\n", szData[i]);
        if(szData[i] == ' ')
        {
            uiSepCount++;
            switch (uiSepCount)
            {
                case 1:
#ifdef DETAIL_DATE
                    Data2Date(szBuf, &pstLogItem->stDate);
#else
                    szBuf[uiBufIdx++] = '\0';
                    strcpy(pstLogItem->szDate, szBuf);
#endif
                    uiBufIdx = 0;
                    break;
                    
                case 2:
                    szBuf[uiBufIdx++] = '\0';
                    strcpy(pstLogItem->szIntfName, szBuf);
                    uiBufIdx = 0;
                default:
                    //DBG("ERROR: Why the num of septarators over\n");
                    break;
            }
        }
        else
        {
            szBuf[uiBufIdx++] = szData[i];
        }
        i++;
    }
    szBuf[uiBufIdx] = '\0';
    pstLogItem->fResponseTime = Sz2Float(szBuf, uiBufIdx);
    return ERROR_SUCCESS;
}

UINT writeList(IN LogItem *pstLogItem, INOUT LogList *pstList)
{
    UINT uiRead = pstList->uiReadIdx;
    UINT uiWrite = pstList->uiWriteIdx;
    UINT uiMax = pstList->uiMax;
    LogItem *pstItem2Write = NULL;
    
    if((uiRead == 0 && uiWrite == uiMax - 1)
       ||(uiRead == uiWrite + 1))
    {
        return ERROR_FAILED;
    }
    
    pstItem2Write = &pstList->astData[uiWrite];
    memcpy(pstItem2Write, pstLogItem, sizeof(LogItem));
    pstList->uiWriteIdx = (++uiWrite == uiMax)? 0 : uiWrite;
    return ERROR_SUCCESS;
}
UINT readList(OUT LogItem *pstLogItem, INOUT LogList *pstList)
{
    UINT uiRead = pstList->uiReadIdx;
    UINT uiWrite = pstList->uiWriteIdx;
    UINT uiMax = pstList->uiMax;
    LogItem *pstItem2Read = NULL;
    
    if (uiWrite == uiRead)
    {
        DBG("No More Stuff to Read\n");
        return ERROR_FAILED;
    }
    /*
    if((uiWrite == 0 && uiRead == uiMax - 1)
       ||(uiWrite == uiRead + 1))
    {
        return ERROR_FAILED;
    }
    */
 
    pstItem2Read = &pstList->astData[uiRead];    
    memcpy(pstLogItem, pstItem2Read, sizeof(LogItem));
    pstList->uiReadIdx = (++uiRead == uiMax)? 0 : uiRead;
    
    return 0;
}
#if 1
#define MAX_LIST 8
extern LogList g_astList[MAX_LIST];
void TaskMapProc(void *pArg)
{
    Message *pstMsg = (Message*)pArg;
    UINT uiListID = 0;
    LogList *pstList = NULL;
    LogItem *pstItem = (LogItem*)malloc(sizeof(LogItem));
    
    assert(pstMsg != NULL);
    
    uiListID = pstMsg->uiListID;
    pstList = &g_astList[uiListID];
    
    assert(pstList != NULL);
    
    for (; ; ) {
        if (ERROR_SUCCESS == readList(pstItem, pstList))
        {
            PRINT_LOGITEM(pstItem, "Thread Get");
        }
        else
        {
            sleep(1);
        }
    }
}
#endif
