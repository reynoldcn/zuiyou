//
//  hash.h
//  zuiyou
//
//  Created by WangZhuoqun on 17/4/13.
//  Copyright (c) 2017年 WangZhuoqun. All rights reserved.
//

#ifndef __zuiyou__hash__
#define __zuiyou__hash__

#include <stdio.h>
#include "public.h"

#define HASH_LEN 719    //Select a prime number

int HashTable_Update(INOUT LogHashNode astHashTable[], IN LogItem *pstKey, IN UINT uiSize);


#endif /* defined(__zuiyou__hash__) */
