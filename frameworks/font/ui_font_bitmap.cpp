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

#include "font/ui_font_bitmap.h"

#include "draw/draw_utils.h"
#include "font/font_ram_allocator.h"
#include "font/ui_font.h"
#include "font/ui_font_adaptor.h"
#include "font/ui_font_builder.h"
#include "gfx_utils/file.h"
#include "gfx_utils/graphic_log.h"
#include "graphic_config.h"
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
#include "font/ui_multi_font_manager.h"
#endif
#if defined(ENABLE_SHAPING) && ENABLE_SHAPING
#include "font/ui_text_shaping.h"
#endif
#include "font/ui_font_cache_manager.h"

namespace OHOS {
UIFontBitmap::UIFontBitmap() : offset_(0), dynamicFont_() {}

UIFontBitmap::~UIFontBitmap()
{
    CloseFontFd();
}

void UIFontBitmap::CloseFontFd()
{
    for (uint16_t i = 0; i < dynamicFontFd_.Size(); i++) {
        if (dynamicFontFd_[i] >= 0) {
            close(dynamicFontFd_[i]);
            dynamicFontFd_[i] = -1;
        }
    }
}

bool UIFontBitmap::IsVectorFont() const
{
    return false;
}

uint16_t
    UIFontBitmap::GetShapingFontId(char* text, uint8_t& ttfId, uint32_t& script, uint16_t fontId, uint8_t size) const
{
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
    return UIMultiFontManager::GetInstance()->GetShapingFontId(text, fontId, ttfId, script);
#else
    UITextLanguageFontParam* fontParam = UIFontBuilder::GetInstance()->GetTextLangFontsTable(fontId);
    if (fontParam == nullptr) {
        return 0;
    }
    ttfId = fontParam->ttfId;
    return fontParam->shaping;
#endif
}

uint8_t UIFontBitmap::GetFontWeight(uint16_t fontId)
{
    UITextLanguageFontParam* fontParam = UIFontBuilder::GetInstance()->GetTextLangFontsTable(fontId);
    if (fontParam == nullptr) {
        GRAPHIC_LOGE("UIFontBitmap::GetFontWeigh invalid fontId");
        return 0;
    }
    return fontParam->fontWeight;
}

int8_t UIFontBitmap::SetFontPath(const char* path, FontType type)
{
    if ((path == nullptr) || (type != DYNAMIC_FONT)) {
        GRAPHIC_LOGE("UIFontBitmap::SetFontPath invalid parameter");
        return INVALID_RET_VALUE;
    }
#ifdef _WIN32
    int32_t fontFd = open(path, O_RDONLY | O_BINARY);
#else
    int32_t fontFd = open(path, O_RDONLY);
#endif
    if (fontFd < 0) {
        GRAPHIC_LOGE("UIFontBitmap::SetFontPath file Open failed");
        return INVALID_RET_VALUE;
    }

    int32_t ret = dynamicFont_.SetFile(path, fontFd, offset_, GlyphCacheType::CACHE_TYPE_DYNAMIC);
    if (ret == INVALID_RET_VALUE) {
        GRAPHIC_LOGE("GlyphsManager::SetFile failed");
        close(fontFd);
        return ret;
    }
    dynamicFontFd_.PushBack(fontFd);
    return RET_VALUE_OK;
}

uint16_t UIFontBitmap::GetHeight(uint16_t fontId, uint8_t fontSize)
{
    return dynamicFont_.GetFontHeight(fontId);
}

uint16_t UIFontBitmap::GetFontId(const char* ttfName, uint8_t size) const
{
    UIFontBuilder* fontBuilder = UIFontBuilder::GetInstance();
    if (ttfName == nullptr) {
        return fontBuilder->GetBitmapFontIdMax();
    }
    uint16_t id;
    for (id = 0; id < fontBuilder->GetBitmapFontIdMax(); ++id) {
        UITextLanguageFontParam* fontParam = fontBuilder->GetTextLangFontsTable(id);
        if (fontParam != nullptr) {
            if ((fontParam->size == size) && (strncmp(fontParam->ttfName, ttfName, TTF_NAME_LEN_MAX) == 0)) {
                break;
            }
        }
    }
    return id;
}

int16_t UIFontBitmap::GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize)
{
    return GetWidthInFontId(unicode, fontId);
}

uint8_t* UIFontBitmap::GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
{
    return SearchInFont(unicode, glyphNode, fontId);
}

bool UIFontBitmap::IsEmojiFont(uint16_t fontId)
{
    return false;
}

int8_t UIFontBitmap::GetFontHeader(FontHeader& fontHeader, uint16_t fontId, uint8_t fontSize)
{
    const FontHeader* header = dynamicFont_.GetFontHeader(fontId);
    if (header != nullptr) {
        fontHeader = *header;
        return RET_VALUE_OK;
    }
    return INVALID_RET_VALUE;
}

#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
int8_t UIFontBitmap::GetMultiGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId)
{
    int8_t ret = GetGlyphNode(unicode, glyphNode, fontId);
    if (ret == RET_VALUE_OK) {
        return ret;
    }

    uint16_t* searchLists = nullptr;
    int8_t listSize = UIMultiFontManager::GetInstance()->GetSearchFontList(fontId, &searchLists);
    int8_t currentIndex = 0;
    if ((searchLists == nullptr) || (listSize == 0)) {
        return INVALID_RET_VALUE;
    }
    do {
        ret = GetGlyphNode(unicode, glyphNode, searchLists[currentIndex]);
        if (ret != INVALID_RET_VALUE) {
            return ret;
        }
        // switch to next search List
        currentIndex++;
    } while ((currentIndex < listSize) && (searchLists != nullptr));
    return INVALID_RET_VALUE;
}
#endif

int8_t UIFontBitmap::GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
{
    const GlyphNode* node = dynamicFont_.GetGlyphNode(unicode, fontId);
    if (node != nullptr) {
        glyphNode = *node;
        return RET_VALUE_OK;
    }
    GRAPHIC_LOGE("UIFontBitmap::GetGlyphNode get glyphNode failed");
    return INVALID_RET_VALUE;
}

int8_t UIFontBitmap::GetGlyphNodeFromFile(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId)
{
    const GlyphNode* node = dynamicFont_.GetGlyphNodeFromFiles(unicode, fontId);
    if (node != nullptr) {
        glyphNode = *node;
        return RET_VALUE_OK;
    }
    return INVALID_RET_VALUE;
}

int8_t UIFontBitmap::GetFontVersion(FontType type, const char* path, char* version, uint8_t len)
{
    if (type != DYNAMIC_FONT) {
        return INVALID_RET_VALUE;
    }
    return dynamicFont_.GetFontVersion(path, version, len);
}

int8_t UIFontBitmap::SetCurrentLangId(uint8_t langId)
{
    GRAPHIC_LOGE("UIFontBitmap::SetCurrentLangId start");
    UIFontCacheManager* fontCacheManager = UIFontCacheManager::GetInstance();

    if (fontCacheManager->GlyphsCacheInit() != RET_VALUE_OK) {
        GRAPHIC_LOGE("UIFontCacheManager::GlyphsCacheInit init failed");
        return INVALID_RET_VALUE;
    }

    fontCacheManager->BitmapCacheInit();
    return RET_VALUE_OK;
}

UITextLanguageFontParam* UIFontBitmap::GetFontInfo(uint16_t fontId) const
{
    return UIFontBuilder::GetInstance()->GetTextLangFontsTable(fontId);
}

int8_t UIFontBitmap::GetDynamicFontBitmap(uint32_t unicode, BufferInfo& bufInfo, uint16_t fontId)
{
    return dynamicFont_.GetBitmap(unicode, bufInfo, fontId);
}

uint8_t* UIFontBitmap::GetCacheBitmap(uint16_t fontId, uint32_t unicode)
{
    TextStyle textStyle = TEXT_STYLE_NORMAL;
    return UIFontCacheManager::GetInstance()->GetBitmap(fontId, unicode, textStyle);
}

void UIFontBitmap::PutCacheSpace(uint8_t* addr)
{
    UIFontCacheManager::GetInstance()->PutSpace(addr);
}

int16_t UIFontBitmap::GetDynamicFontWidth(uint32_t unicode, uint16_t fontId)
{
    return dynamicFont_.GetFontWidth(unicode, fontId);
}

uint8_t* UIFontBitmap::SearchInFont(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId)
{
    GraphicLockGuard guard(lock_);
    UIFontCacheManager* fontCacheManager =  UIFontCacheManager::GetInstance();
    if (fontCacheManager->GetBitmapCache() == nullptr) {
        return nullptr;
    }
    if (!UIFontAdaptor::IsSameTTFId(fontId, unicode)) {
        GRAPHIC_LOGE("UIFontBitmap::SearchInFont fontId and unicode not match");
        return nullptr;
    }
    int8_t ret = GetGlyphNode(unicode, glyphNode, fontId);
    if (ret != RET_VALUE_OK) {
        return nullptr;
    }
    TextStyle textStyle = TEXT_STYLE_NORMAL;
    uint8_t* bitmap = fontCacheManager->GetBitmap(fontId, unicode, textStyle);
    if (bitmap != nullptr) {
        if (glyphNode.dataFlag == glyphNode.fontId && fontId == glyphNode.fontId) {
            return bitmap;
        } else {
            GRAPHIC_LOGE("DataFlag of bitmap node not equal to fontId.");
        }
    }
    if (glyphNode.kernOff <= glyphNode.dataOff) {
        return nullptr;
    }
    ColorMode mode = UIFont::GetInstance()->GetColorType(fontId);
    BufferInfo bufInfo = UIFontAllocator::GetCacheBuffer(fontId, unicode, mode, glyphNode, false, textStyle);
    ret = dynamicFont_.GetBitmap(unicode, bufInfo, fontId);
    if (ret == RET_VALUE_OK) {
        return reinterpret_cast<uint8_t*>(bufInfo.virAddr);
    }
    PutCacheSpace(reinterpret_cast<uint8_t*>(bufInfo.virAddr));
    return nullptr;
}

int16_t UIFontBitmap::GetWidthInFontId(uint32_t unicode, uint16_t fontId)
{
    if (!UIFontAdaptor::IsSameTTFId(fontId, unicode)) {
        GRAPHIC_LOGE("UIFontBitmap::GetWidthInFontId fontId and unicode not match");
        return INVALID_RET_VALUE;
    }
    return GetDynamicFontWidth(unicode, fontId);
}

void UIFontBitmap::SetFontFileOffset(uint32_t offset)
{
    offset_ = offset;
}

uint16_t UIFontBitmap::GetOffsetPosY(const char* text,
                                     uint16_t lineLength,
                                     bool& isEmoijLarge,
                                     uint16_t fontId,
                                     uint8_t fontSize)
{
    uint32_t i = 0;
    uint16_t textNum = 0;
    uint16_t emojiNum = 0;

    uint16_t loopNum = 0;
    GlyphNode glyphNode;
    GlyphNode emoijMaxNode = {};
    uint8_t maxFontSie = fontSize;
    while (i < lineLength) {
        uint32_t unicode = TypedText::GetUTF8Next(text, i, i);
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
        uint8_t ret = GetMultiGlyphNode(unicode, glyphNode, fontId);
#else
        uint8_t ret = GetGlyphNode(unicode, glyphNode, fontId, fontSize);
#endif
        if (ret == RET_VALUE_OK) {
            uint8_t weight = GetFontWeight(glyphNode.fontId);
            if (weight >= 16) { // 16: bit rgb565 rgba8888
                emoijMaxNode = glyphNode.rows > emoijMaxNode.rows ? glyphNode : emoijMaxNode;
                emojiNum++;
            } else {
                textNum++;
            }
            loopNum++;
        }
    }
    // The number of emoji is the same as the number of cycles, indicating that this line is all emoji
    // The number of words is the same as the number of cycles, which means that this line is all words
    if ((emojiNum == loopNum) || (textNum == loopNum)) {
        isEmoijLarge = true;
        return 0;
    }
    isEmoijLarge = emoijMaxNode.rows > maxFontSie;
    uint16_t offset = 0;
    if (isEmoijLarge) {
        // If the emoji is higher than the text
        if (emoijMaxNode.top >= maxFontSie) {
            offset = emoijMaxNode.top - maxFontSie;
        }
    } else {
        // If text are higher than emoji
        if (maxFontSie >= emoijMaxNode.rows) {
            // should top - ros.
            offset = maxFontSie - emoijMaxNode.rows;
        }
    }
    return offset;
}

uint16_t UIFontBitmap::GetLineMaxHeight(const char* text,
                                        uint16_t lineLength,
                                        uint16_t fontId,
                                        uint8_t fontSize,
                                        uint16_t& letterIndex,
                                        SpannableString* spannableString)
{
    uint16_t maxHeight = GetHeight(fontId, fontSize);
    if (spannableString == nullptr) {
        return maxHeight;
    }

    uint32_t i = 0;
    while (i < lineLength) {
        TypedText::GetUTF8Next(text, i, i);
        if (spannableString != nullptr && spannableString->GetSpannable(letterIndex)) {
            int16_t spannableHeight = 0;
            spannableString->GetFontHeight(letterIndex, spannableHeight, fontId, fontSize);
            uint16_t tempHeight = static_cast<uint16_t>(spannableHeight);
            maxHeight = tempHeight > maxHeight ? tempHeight : maxHeight;
        }
        letterIndex++;
        if (i > 0 && ((text[i - 1] == '\r') || (text[i - 1] == '\n'))) {
            break;
        }
    }

    return maxHeight;
}

void UIFontBitmap::SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen)
{
    BaseFont::SetPsramMemory(psramAddr, psramLen);
    FontRamAllocator::GetInstance().SetRamAddr(psramAddr, psramLen);
}
} // namespace OHOS
