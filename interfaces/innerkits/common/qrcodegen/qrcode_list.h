/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __QRCODE_LIST_H__
#define __QRCODE_LIST_H__

#include <stddef.h>
#include <cstddef>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct QRCODE_DL_LIST {
    struct QRCODE_DL_LIST *pstPrev;
    struct QRCODE_DL_LIST *pstNext;
} QRCODE_DL_LIST;

static inline void QrcodeListInit(QRCODE_DL_LIST *list)
{
    list->pstNext = list;
    list->pstPrev = list;
}

static inline void QrCodeListAdd(QRCODE_DL_LIST *list, QRCODE_DL_LIST *node)
{
    node->pstNext = list->pstNext;
    node->pstPrev = list;
    list->pstNext->pstPrev = node;
    list->pstNext = node;
}

static inline void QrCodeListTailInsert(QRCODE_DL_LIST *list, QRCODE_DL_LIST *node)
{
    QrCodeListAdd(list->pstPrev, node);
}

static inline bool QrCodeListEmpty(QRCODE_DL_LIST *node)
{
    return (node->pstNext == node);
}

typedef struct {
    QRCODE_DL_LIST *cur;
    QRCODE_DL_LIST *next;
    QRCODE_DL_LIST *head;
    size_t memberOffset;
} QrcodeDlListIter;

static inline void QrcodeDlListIterInitTyped(QrcodeDlListIter *iter, QRCODE_DL_LIST *list,
    const QRCODE_DL_LIST *memberPtr)
{
    iter->cur = list->pstNext;
    iter->next = list->pstNext->pstNext;
    iter->head = list;
    iter->memberOffset = (size_t)((const char *)memberPtr - (const char *)0);
}

static inline bool QrcodeDlListIterHasNext(QrcodeDlListIter *iter)
{
    return iter->cur != iter->head;
}

static inline void *QrcodeDlListIterNext(QrcodeDlListIter *iter)
{
    void *item = (char *)iter->cur - iter->memberOffset;
    iter->cur = iter->next;
    iter->next = iter->next->pstNext;
    return item;
}

static inline void *QrcodeDlListIterNextSafe(QrcodeDlListIter *iter)
{
    void *item = (char *)iter->cur - iter->memberOffset;
    iter->cur = iter->next;
    iter->next = iter->next->pstNext;
    return item;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
