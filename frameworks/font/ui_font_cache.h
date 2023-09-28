/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

/*
 * Font cache is used for managing font bitmap.
 * Each bitmap is contained in a chunk block which can be searched quickly in a hash-table.
 * All the struct and memory block are force-aligned to ALIGNMENT_BYTES.
 * To make the most frequently used memroy hot, chunk blocks are managed with an easy lru algorithm.
 * Memory maps of font cache is shown below:
 *
 *  aligned bitmapCache    ─────────►┌────────────────────┐
 *  (FONT_BITMAP_CACHE_SIZE)         │    HashTable       │
 *                                   │    (ListHead*32)   │
 *  UIFontAllocator::free_ ──────────┼────────────────────┼───────────► ┌──────────────┐
 *                                   │    chunk block     │             │ struct Chunk │
 *                                   ├────────────────────┼──────┐      ├──────────────┤
 *                                   │    ...             │      │      │ struct Bitmap│
 *                                   ├────────────────────┤      │      ├──────────────┤
 *                                   │    chunk block     │      │      │  FontCache   │
 *                                   ├────────────────────┤      │      │  (20*20*n)   │
 *                                   │    last_chunk      │      └────► └──────────────┘
 *                                   │    (Head only)     │
 *                                   └────────────────────┘
 */
#ifndef UI_FONT_CACHE_H
#define UI_FONT_CACHE_H

#include "ui_font_allocator.h"
#include "font/ui_font_header.h"

namespace OHOS {
class UIFontCache : public HeapBase {
public:
    static constexpr uint8_t FONT_CACHE_HASH_NR = 32;
    static constexpr uint32_t FONT_CACHE_MIN_SIZE = 20 * 20;
    struct UI_STRUCT_ALIGN ListHead {
        ListHead* prev;
        ListHead* next;
    };
    struct UI_STRUCT_ALIGN Bitmap {
        ListHead hashHead;
        ListHead lruHead;
        uint32_t fontId; // bitmap font: fontId vector font: fontKey ttfId + fontsize
        uint32_t unicode;
        uint32_t reserve1;
        uint32_t reserve2;
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
        TextStyle textStyle;
#endif
        uint8_t data[];
    };

    UIFontCache(uint8_t* ram, uint32_t size);

    ~UIFontCache();

    /* default textStyle is TEXT_STYLE_NORMAL, TextStyle textStyle = TEXT_STYLE_NORMAL */
    uint8_t* GetSpace(uint16_t fontKey, uint32_t unicode, uint32_t size, TextStyle textStyle);
    void PutSpace(uint8_t* addr);
    uint8_t* GetBitmap(uint16_t fontKey, uint32_t unicode, TextStyle textStyle);

private:
    void UpdateLru(Bitmap* bitmap);
    void ListInit(ListHead* head)
    {
        head->prev = head;
        head->next = head;
    }
    void ListAdd(ListHead* node, ListHead* head)
    {
        head->next->prev = node;
        node->next = head->next;
        node->prev = head;
        head->next = node;
    }
    void ListDel(ListHead* node)
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }

    UIFontAllocator allocator_;
    ListHead* hashTable_ = nullptr;
    ListHead lruList_ = {};
};
} // namespace OHOS
#endif /* UI_FONT_CACHE_H */
