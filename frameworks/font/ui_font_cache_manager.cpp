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

#include "font/ui_font_cache_manager.h"
#include "font/font_ram_allocator.h"
#include "gfx_utils/graphic_log.h"
#include "securec.h"

namespace OHOS {
UIFontCacheManager::UIFontCacheManager() : bitmapCache_(nullptr) {}

UIFontCacheManager::~UIFontCacheManager()
{
    if (bitmapCache_ != nullptr) {
        delete bitmapCache_;
        bitmapCache_ = nullptr;
    }
}

UIFontCacheManager* UIFontCacheManager::GetInstance()
{
    static UIFontCacheManager instance_;
    return &instance_;
}

int8_t UIFontCacheManager::GlyphsCacheInit()
{
    return glyphsCache_.CacheInit();
}

void UIFontCacheManager::ClearCacheFlag()
{
    return glyphsCache_.ClearCacheFlag();
}

GlyphCacheNode* UIFontCacheManager::GetNodeFromCache(uint32_t unicode, uint16_t fontKey, uint16_t cacheType)
{
    return glyphsCache_.GetNodeFromCache(unicode, fontKey, cacheType);
}

GlyphCacheNode* UIFontCacheManager::GetNodeCacheSpace(uint32_t unicode, uint16_t fontKey)
{
    return glyphsCache_.GetNodeCacheSpace(unicode, fontKey);
}

void UIFontCacheManager::SetBitmapCacheSize(uint32_t bitmapCacheSize)
{
    bitmapCacheSize_ = bitmapCacheSize;
}

void UIFontCacheManager::BitmapCacheInit()
{
    if (bitmapCache_ != nullptr) {
        return;
    }

    uint8_t* bitmapCacheAddr =
        reinterpret_cast<uint8_t*>(FontRamAllocator::GetInstance().DynamicAllocate(bitmapCacheSize_));
    if (bitmapCacheAddr == nullptr) {
        GRAPHIC_LOGE("UIFontCacheManager::BitmapCacheInit allocate failed");
        return;
    }

    if (memset_s(bitmapCacheAddr, bitmapCacheSize_, 0, bitmapCacheSize_)!= EOK) {
        GRAPHIC_LOGE("UIFontCacheManager::BitmapCacheInit memset failed");
        return;
    }
    bitmapCache_ = new UIFontCache(bitmapCacheAddr, bitmapCacheSize_);
}

void UIFontCacheManager::BitmapCacheClear()
{
    delete bitmapCache_;
    bitmapCache_ = nullptr;
}

uint8_t* UIFontCacheManager::GetSpace(uint16_t fontKey, uint32_t unicode, uint32_t size, TextStyle textStyle)
{
    if (bitmapCache_ != nullptr) {
        return bitmapCache_->GetSpace(fontKey, unicode, size, textStyle);
    }
    GRAPHIC_LOGE("UIFontCacheManager::GetSpace invalid bitmapCache");
    return nullptr;
}

void UIFontCacheManager::PutSpace(uint8_t* addr)
{
    if (bitmapCache_ != nullptr) {
        bitmapCache_->PutSpace(addr);
    }
    GRAPHIC_LOGE("UIFontCacheManager::PutSpace invalid bitmapCache");
}

uint8_t* UIFontCacheManager::GetBitmap(uint16_t fontKey, uint32_t unicode, TextStyle textStyle)
{
    if (bitmapCache_ != nullptr) {
        return bitmapCache_->GetBitmap(fontKey, unicode, textStyle);
    }
    GRAPHIC_LOGE("UIFontCacheManager::GetBitmap invalid bitmapCache");
    return nullptr;
}
} // namespace OHOS
