//
//  hash.c
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/13.
//  Copyright (c) 2017年 WangZhuoqun. All rights reserved.
//

#include "hash.h"
#include "public.h"
#include <ctype.h>

static inline UINT HashString(char szInftName[DATA_LEN], UINT dwHashType)
{
    UINT i = 0;
    UINT seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
    UINT ch;
    while(szInftName[i] != 0)
    {
        ch = toupper(szInftName[i++]);
        seed1 = ((dwHashType << 8) + ch) ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }
    return seed1;
}
//extern LogHashNode g_astHashTable[HASH_LEN];
#define HASH_A 0
#define HASH_B 1
#define HASH_C 2
static inline int GetHashTablePos(IN LogHashNode astHashTable[],
                                  IN char szInftName[DATA_LEN],
                                  IN UINT uiSize)
{
    UINT uiHashA = HashString(szInftName, HASH_A);
    UINT uiHashB = HashString(szInftName, HASH_B);
    UINT uiHashC = HashString(szInftName, HASH_C);
    UINT uiHashStart = uiHashA % uiSize, uiHashPos = uiHashStart;
    while (astHashTable[uiHashPos].bExists)
    {
        if (astHashTable[uiHashPos].uiHashB == uiHashB
             && astHashTable[uiHashPos].uiHashC == uiHashC)
            return uiHashPos;
        else
            uiHashPos = (uiHashPos + 1) % uiSize;
        if (uiHashPos == uiHashStart)
            break;
    }
    return -uiHashPos; //Error value
}
/*
以上hash算法是百度到的，据说是Blizzard用的hash算法。
其思路很简单，对每个字符串求三个hash值，如果两个字符串三个hash值都相等，
那么这两个字符串不相等的概率小于10的负20次方（？大概这个数）
ps：其实我们题目里只有20个不同的接口，这样算法只针对这个题目来说的话有点过分了。
ps：But my program is extremly rigorous XD.
*/
int HashTable_Update(INOUT LogHashNode astHashTable[], IN LogItem *pstKey, IN UINT uiSize)
{
    int iPos = 0;
    /*
    UINT *puiHashA;
    UINT *puiHashB;
    UINT *puiHashC;
     */
    UINT *puiCountTotal;
    UINT *puiCountTimeOut;
    float *pfResponseTimeTotal;
    iPos = GetHashTablePos(astHashTable, pstKey->szIntfName, uiSize);
    pthread_mutex_lock (&astHashTable[abs(iPos)].lock); //不知道iPos是正数还是负数，先取绝对值再说。
    if (iPos < 0)                                       //如果是新插入hash表的字符串，会返回其位置的相反数
    {
        //DBG("New LogItem\n");
        iPos = 0 - iPos;

        strcpy(astHashTable[iPos].stValue.szIntfName, pstKey->szIntfName);
        /*puiHashA = &astHashTable[iPos].uiHashA;
        puiHashB = &astHashTable[iPos].uiHashB;
        puiHashC = &astHashTable[iPos].uiHashC;*/
        astHashTable[iPos].uiHashA = HashString(pstKey->szIntfName, HASH_A);
        astHashTable[iPos].uiHashB = HashString(pstKey->szIntfName, HASH_B);
        astHashTable[iPos].uiHashC = HashString(pstKey->szIntfName, HASH_C);
        astHashTable[iPos].bExists = 1;
        //pthread_mutex_unlock (&astHashTable[iPos].lock);
    }
    //pthread_mutex_lock (&astHashTable[iPos].lock);
    pfResponseTimeTotal = &(astHashTable[iPos].stValue.fResponseTimeTotal);
    puiCountTimeOut     = &(astHashTable[iPos].stValue.uiCountTimeOut);
    puiCountTotal       = &(astHashTable[iPos].stValue.uiCountTotal);
    
    *pfResponseTimeTotal += pstKey->fResponseTime;
    *puiCountTotal       += 1;
    
    if (pstKey->fResponseTime >= TIMEOUT) {
        *puiCountTimeOut += 1;
    }
    pthread_mutex_unlock (&astHashTable[iPos].lock);    //此时iPos肯定是正数了。
    return iPos;
    
}
//LogHashNode astHashTable[HASH_LEN];
#if 0
int main(void)
{
    LogHashNode astHashTable[HASH_LEN];
    LogResult *pstRes = NULL;
    memset(astHashTable, 0, HASH_LEN * sizeof(LogHashNode));
    
    LogItem key1;
    LogItem key2;
    LogItem key3;
    
    int iPos = 0;
    
    key1.fResponseTime = 0.05f;
    strcpy(key1.szIntfName, "wangzhuoqun");
    
    key2.fResponseTime = 0.1f;
    strcpy(key2.szIntfName, "chenpeiran");
    
    key3.fResponseTime = 0.2f;
    strcpy(key3.szIntfName, "guochen");
    
    iPos = HashTable_Update(astHashTable, &key1, HASH_LEN);
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    iPos = HashTable_Update(astHashTable, &key2, HASH_LEN);
    
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    
    iPos = HashTable_Update(astHashTable, &key3, HASH_LEN);
    
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    
    iPos = HashTable_Update(astHashTable, &key1, HASH_LEN);
    
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    
    iPos = HashTable_Update(astHashTable, &key2, HASH_LEN);
    
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    
    iPos = HashTable_Update(astHashTable, &key3, HASH_LEN);
    
    pstRes = &astHashTable[iPos].stValue;
    printf("%d\n", iPos);
    printf("%s, count = %d\nmore than 0.1 = %d\ntotal time = %f\n",
           pstRes->szIntfName, pstRes->uiCountTotal, pstRes->uiCountTimeOut, pstRes->fResponseTimeTotal);
    
    return 0;
}
#endif
