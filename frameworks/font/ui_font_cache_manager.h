/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef UI_FONT_CACHE_MANAGER_H
#define UI_FONT_CACHE_MANAGER_H

#include "font/glyphs_cache.h"
#include "font/ui_font_cache.h"

namespace OHOS {
class UIFontCacheManager : public HeapBase {
public:
    UIFontCacheManager(const UIFontCacheManager&) = delete;
    UIFontCacheManager& operator=(const UIFontCacheManager&) = delete;
    static UIFontCacheManager* GetInstance();
    int8_t GlyphsCacheInit();
    void ClearCacheFlag();
    GlyphCacheNode* GetNodeFromCache(uint32_t unicode, uint16_t fontKey, uint16_t cacheType);
    GlyphCacheNode* GetNodeCacheSpace(uint32_t unicode, uint16_t fontKey);
    void SetBitmapCacheSize(uint32_t bitmapCacheSize);
    void BitmapCacheInit();
    void BitmapCacheClear();
    /* default TextStyle is TEXT_STYLE_NORMAL. */
    uint8_t* GetSpace(uint16_t fontKey, uint32_t unicode, uint32_t size, TextStyle textStyle);
    void PutSpace(uint8_t* addr);
    uint8_t* GetBitmap(uint16_t fontKey, uint32_t unicode, TextStyle textStyle);

    UIFontCache* GetBitmapCache()
    {
        return bitmapCache_;
    }

private:
    static constexpr uint32_t FONT_BITMAP_CACHE_SIZE = 0x64000;
    UIFontCacheManager();
    ~UIFontCacheManager();
    UIFontCache* bitmapCache_;
    GlyphsCache glyphsCache_;
    uint32_t bitmapCacheSize_ = FONT_BITMAP_CACHE_SIZE;
};
} // namespace OHOS
#endif /* UI_FONT_CACHE_MANAGER_H */