//
//  public.c
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/9.
//  Copyright (c) 2017 WangZhuoqun. All rights reserved.
//


#include "public.h"
#include "hash.h"
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
    for(; i >= 0; --i)
    {
        if(szBuf[i] == '.')
        {
            fRet = fRet / digit;
            digit = 1;
            continue;
        }
        if(IS_NUM(szBuf[i]))
        {
            fRet += digit * (szBuf[i] - '0');
            digit *= 10;
        }
    }
    return fRet;
}
/*
最初设计的时候把日期当成一个很重要的参数来看待的，
很快我发现对本题来说日期根本没啥用
so 保留一个DETAIL_DATE模式，不过并没有实现好，
Please Ignore This Func
*/
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
/*
write和read部分我参照无锁结构体，循环缓冲区的思想，进行设计的。
因为在这个proj里同一时刻对于一个队列来说，只有1个读者，1个写者。
那么只要保证读者操作到的索引，永远在写索引后面，就ok了
*/
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
    {//此时说明当前队列里没有可读的元素了
        //DBG("No More Stuff to Read\n");
        return ERROR_FAILED;
    }
 
    pstItem2Read = &pstList->astData[uiRead];

    memcpy(pstLogItem, pstItem2Read, sizeof(LogItem));
    //为了确保不会重复读，把每次读完的元素都清零
    memset(pstItem2Read, 0, sizeof(LogItem));
    pstList->uiReadIdx = (++uiRead == uiMax)? 0 : uiRead;
    
    return 0;
}

#define MAX_LIST 8
extern LogList g_astList[MAX_LIST];
extern LogHashNode g_astHashTable[HASH_LEN];
extern char g_done[MAX_LIST+1]; 
void TaskMapProc(void *pArg)
{
    UINT uiListID = 0;
    LogList *pstList = NULL;
    LogItem *pstItem = (LogItem*)malloc(sizeof(LogItem));

    uiListID = (UINT)(ULONG)pArg;   //参数就是该线程要处理的队列号
    pstList = &g_astList[uiListID];
    
    assert(pstList != NULL);
    DBG("Starting Thread%d\n", uiListID);
    for (; ; )
    {
        if (ERROR_SUCCESS == readList(pstItem, pstList))
        {
            if(pstItem->fResponseTime < 0)
            {
                DBG("Stop Thread%d\n", uiListID);
                break;
            }
            HashTable_Update(g_astHashTable, pstItem, HASH_LEN);
        }
        else
        {
            usleep(1000);
        }
    }
    g_done[uiListID] = '1';
}

typedef struct tagListNode{
    LogResult *val;
    struct tagListNode *next;
}ListNode;

//用于处理结果值的，此时传入一个已经排好序的链表
//对于本题，我的做法是把结果写入一个output.txt
//文件里，其格式为：
//接口名 访问次数 平均相应时间 超过0.1s的次数
static inline void ProcessResult(ListNode *pstHead)
{
    FILE *fp = NULL;
    LogResult *pstRes = NULL;
#define DATA_LEN 128
    char szLine[DATA_LEN] = {0};
    
    if((fp = fopen("Output.txt", "w")) == NULL)
    {
        DBG("File cannot be opened\n");
        return;
    }
    else
    {
        DBG("Writing Report\n");
    }
    for (ListNode *pstNode = pstHead; pstNode != NULL; pstNode = pstNode->next)
    {
        pstRes = pstNode->val;
        snprintf(szLine, DATA_LEN, "%s %d %f %d\n", pstRes->szIntfName,
                 pstRes->uiCountTotal,
                 pstRes->fResponseTimeTotal/pstRes->uiCountTotal,
                 pstRes->uiCountTimeOut);
        fputs(szLine, fp);
        //DBG("%s, %d, %f\n", pstRes->szIntfName, pstRes->uiCountTotal, pstRes->fResponseTimeTotal);
    }
    fclose(fp);
    //DBG("%d, %d\n", listlen, notinsert);
}
void TaskReduceProc(void *pArg)
{
    ListNode *pstNode = NULL;
    ListNode *pstNext = NULL;
    ListNode *pstHead = NULL;
    LogHashNode *pstHashNode = NULL;
    LogResult *pstRes = NULL;
    UINT i = 0;
    UINT bFirstHit = 1;
    UINT bInsert = 0;
    for (; i < HASH_LEN; ++i)
    {
        pstHashNode = &g_astHashTable[i];
        pstRes = &pstHashNode->stValue;
        if (!pstHashNode->bExists)
        {
            continue;
        }
        if (bFirstHit)
        {
            bFirstHit = 0;
            pstNode = malloc(sizeof(ListNode));
            pstNode->val = &pstHashNode->stValue;
            pstNode->next = NULL;
            pstHead = pstNode;
        }
        else
        {
            pstNode = pstHead;
            bInsert = 0;
            while (pstNode != NULL)             //遍历当前的链表
            {
                if(pstNode->val->uiCountTotal
                    >= pstRes->uiCountTotal)    //如果新节点比当前遍历到的链表节点小，就插入该节点的前面
                {
                    if (pstNode->next == NULL) {//如果此时已经遍历到链表的最后了，直接插到最后位置
                        pstNext = malloc(sizeof(ListNode));
                        pstNext->val = pstRes;
                        pstNext->next = NULL;
                        pstNode->next = pstNext;
                    }
                    else
                    {
                        pstNext = pstNode->next;//先插到该节点后面，然后交换它们的val，就相当于插了
                                                //一个新节点在当前节点前面
                        pstNode->next = malloc(sizeof(ListNode));
                        pstNode->next->next = pstNext;
                        pstNode->next->val = pstRes;
                        pstNext = pstNode->next;
                    }
                    //listlen++;
                    LogResult *pstTemp = pstNode->val;
                    pstNode->val = pstNext->val;
                    pstNext->val = pstTemp;
                    bInsert = 1;
                    break;
                }
                pstNext = pstNode;              //I am still wondering here
                                                //though it must be a simple shit
                pstNode = pstNode->next;
            }
            if (!bInsert) {
#if 0
                pstNode = malloc(sizeof(ListNode));
                pstNode->val = pstRes;
                pstNode->next = NULL;
#else
                //printf("%p, %p\n", pstNext->next, pstNode);
                pstNext->next = malloc(sizeof(ListNode));
                pstNext->next->val = pstRes;
                pstNext->next->next = NULL;
#endif
                //notinsert++;
                //listlen++;
            }
            
        }
    }
    ProcessResult(pstHead);
    
    for (pstNode = pstHead; pstNode != NULL; pstNode = pstNode->next)
    {
        free(pstNode);
    }
}


