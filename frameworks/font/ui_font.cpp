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

#include "font/ui_font.h"
#include "common/text.h"
#include "font/ui_font_cache.h"
#include "font/font_ram_allocator.h"
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
#include "font/ui_font_vector.h"
#endif
#if defined(ENABLE_BITMAP_FONT) && ENABLE_BITMAP_FONT
#include "font/ui_font_bitmap.h"
#endif
#include "graphic_config.h"
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
#include "font/ui_multi_font_manager.h"
#endif

namespace OHOS {
bool UIFont::setFontAllocFlag_ = false;
UIFont::UIFont() : instance_(nullptr), defaultInstance_(nullptr) {}

UIFont::~UIFont() {}

UIFont* UIFont::GetInstance()
{
    static UIFont instance;
#if defined(ENABLE_BITMAP_FONT) && ENABLE_BITMAP_FONT
    if (instance.instance_ == nullptr) {
        instance.defaultInstance_ = new UIFontBitmap();
        instance.instance_ = instance.defaultInstance_;
        setFontAllocFlag_ = true;
    }
#endif
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    if (instance.instance_ == nullptr) {
        instance.defaultInstance_ = new UIFontVector();
        instance.instance_ = instance.defaultInstance_;
        setFontAllocFlag_ = true;
    }
#endif
    return &instance;
}

void UIFont::SetFont(BaseFont* font)
{
    if (font != nullptr) {
        if (defaultInstance_ != nullptr && setFontAllocFlag_) {
            delete defaultInstance_;
            defaultInstance_ = nullptr;
            setFontAllocFlag_ = false;
        }
        defaultInstance_ = font;
        instance_ = font;
    }
}

BaseFont* UIFont::GetFont()
{
    return instance_;
}

#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
void UIFont::SetBitmapFont(BaseFont* font)
{
    if (font == nullptr) {
        return;
    }
    GetBitmapInstance()->SetFont(font);
}

UIFont* UIFont::GetBitmapInstance()
{
    static UIFont instance;
    if (instance.instance_ == nullptr) {
        instance.defaultInstance_ = new UIFontBitmap();
        instance.instance_ = instance.defaultInstance_;
        setFontAllocFlag_ = true;
    }
    return &instance;
}
#endif

void UIFont::SetFontFileOffset(uint32_t offset)
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    GetBitmapInstance()->GetFont()->SetFontFileOffset(offset);
#else
    instance_->SetFontFileOffset(offset);
#endif
}

int8_t UIFont::SetCurrentLangId(uint8_t langId)
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->SetCurrentLangId(langId);
#else
    return instance_->SetCurrentLangId(langId);
#endif
}

uint8_t UIFont::GetCurrentLangId() const
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->GetCurrentLangId();
#else
    return instance_->GetCurrentLangId();
#endif
}

int8_t  UIFont::SetFontPath(const char* path, BaseFont::FontType type)
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    if (type == BaseFont::FontType::VECTOR_FONT) {
        return instance_->SetFontPath(path, type);
    }
    return GetBitmapInstance()->GetFont()->SetFontPath(path, type);
#else
    return instance_->SetFontPath(path, type);
#endif
}

int8_t UIFont::GetTextUtf8(uint16_t textId, uint8_t** utf8Addr, uint16_t& utf8Len) const
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->GetTextUtf8(textId, utf8Addr, utf8Len);
#else
    return instance_->GetTextUtf8(textId, utf8Addr, utf8Len);
#endif
}

int8_t UIFont::GetTextParam(uint16_t textId, UITextLanguageTextParam& param) const
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->GetTextParam(textId, param);
#else
    return instance_->GetTextParam(textId, param);
#endif
}

int8_t UIFont::GetWildCardStaticStr(uint16_t textId,
                                    UITextWildcardStaticType type,
                                    uint8_t** strAddr,
                                    uint16_t& strLen) const
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->GetWildCardStaticStr(textId, type, strAddr, strLen);
#else
    return instance_->GetWildCardStaticStr(textId, type, strAddr, strLen);
#endif
}

int8_t UIFont::GetCodePoints(uint16_t textId, uint32_t** codePoints, uint16_t& codePointsNum) const
{
#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    return GetBitmapInstance()->GetFont()->GetCodePoints(textId, codePoints, codePointsNum);
#else
    return instance_->GetCodePoints(textId, codePoints, codePointsNum);
#endif
}

uint8_t* UIFont::GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize,
                           uint8_t shapingFont)
{
    uint8_t* bitmap = nullptr;
#if ENABLE_MULTI_FONT
    // shaping font is in search list, search shaping font first
    if (shapingFont > 1) {
        bitmap = instance_->GetBitmap(unicode, glyphNode, shapingFont, fontSize);
        if (bitmap != nullptr) {
            return bitmap;
        }
    }
#endif
    bitmap = instance_->GetBitmap(unicode, glyphNode, fontId, fontSize);
    if (bitmap != nullptr) {
        return bitmap;
    }
#if ENABLE_MULTI_FONT
    uint16_t* searchLists = nullptr;
    int32_t listSize = UIMultiFontManager::GetInstance()->GetSearchFontList(fontId, &searchLists);
    int32_t currentIndex = 0;
    if ((searchLists == nullptr) || (listSize == 0)) {
        return nullptr;
    }
    do {
        bitmap = instance_->GetBitmap(unicode, glyphNode, searchLists[currentIndex], fontSize);
        if (bitmap != nullptr) {
            return bitmap;
        }
        // switch to next search List
        currentIndex++;
    } while ((currentIndex < listSize) && (searchLists != nullptr));
#endif
    return nullptr;
}

int8_t UIFont::GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
{
    int8_t result = instance_->GetGlyphNode(unicode, glyphNode, fontId, fontSize);
    if (result == RET_VALUE_OK) {
        return result;
    }

#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
    uint16_t* searchLists = nullptr;
    int32_t listSize = UIMultiFontManager::GetInstance()->GetSearchFontList(fontId, &searchLists);
    if ((searchLists == nullptr) || (listSize == 0)) {
        return INVALID_RET_VALUE;
    }
    int32_t currentIndex = 0;
    do {
        result = instance_->GetGlyphNode(unicode, glyphNode, searchLists[currentIndex], fontSize);
        if (result == RET_VALUE_OK) {
            return result;
        }
        currentIndex++;
    } while ((currentIndex < listSize) && (searchLists != nullptr));
#endif
    return INVALID_RET_VALUE;
}

uint16_t UIFont::GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize, uint8_t shapingId)
{
    int16_t result;
#if ENABLE_MULTI_FONT
    if (shapingId > 1) {
        result = instance_->GetWidth(unicode, shapingId, fontSize);
        if (result >= 0) {
            return result;
        }
    }
#endif
    result = instance_->GetWidth(unicode, fontId, fontSize);
    if (result >= 0) {
        return result;
    }

#if ENABLE_MULTI_FONT
    uint16_t* searchLists = nullptr;
    int32_t listSize = UIMultiFontManager::GetInstance()->GetSearchFontList(fontId, &searchLists);
    if ((searchLists == nullptr) || (listSize == 0)) {
        return 0;
    }
    int32_t currentIndex = 0;
    do {
        result = instance_->GetWidth(unicode, searchLists[currentIndex], fontSize);
        if (result >= 0) {
            return result;
        }
        currentIndex++;
    } while ((currentIndex < listSize) && (searchLists != nullptr));
#endif
    return 0;
}

uint16_t UIFont::GetLineMaxHeight(const char* text, uint16_t lineLength, uint16_t fontId, uint8_t fontSize,
                                  uint16_t letterIndex, SpannableString* spannableString)
{
    return instance_->GetLineMaxHeight(text, lineLength, fontId, fontSize, letterIndex, spannableString);
}
} // namespace OHOS
