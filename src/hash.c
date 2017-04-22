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
    pthread_mutex_lock (&astHashTable[abs(iPos)].lock);
    if (iPos < 0)
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
    pthread_mutex_unlock (&astHashTable[iPos].lock);
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