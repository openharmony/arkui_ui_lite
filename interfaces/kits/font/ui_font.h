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
#ifndef UI_FONT_H
#define UI_FONT_H
#include "font/base_font.h"

namespace OHOS {
class UIFont : public HeapBase {
public:
    UIFont(const UIFont &) = delete;
    UIFont& operator=(const UIFont &) = delete;

    /**
     * @brief judge whether you need shaping for text
     *
     * @param text means input text
     * @param ttfId font index
     * @param script shaping script
     * @param fontId font index
     * @param size font size
     * @return int8_t Shaping mode
     */
    uint16_t GetShapingFontId(char* text, uint8_t& ttfId, uint32_t& script, uint16_t fontId, uint8_t size) const
    {
        return instance_->GetShapingFontId(text, ttfId, script, fontId, size);
    }

    /**
     * @brief Get width of the letter
     *
     * @param unicode: [in] unicode or glyph index according to isGlyph param
     * @param fontId: [in] font id
     * @param fontSize: [in] font size
     * @param shapingId: [in] font shaping id
     * @return uint16_t: the letter width
     */
    uint16_t GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize, uint8_t shapingId);

    /**
     * @brief Get height for specific font
     *
     * @param fontId: font id
     * @param fontSize: font size
     *
     * @return uint16_t
     */
    uint16_t GetHeight(uint16_t fontId, uint8_t fontSize)
    {
        return instance_->GetHeight(fontId, fontSize);
    }

    /**
     * @brief Get the font weight
     * @param fontId
     * @return uint8_t: fontWeight
     */
    uint8_t GetFontWeight(uint16_t fontId)
    {
        return instance_->GetFontWeight(fontId);
    }

    /**
     * @brief Get font header
     *
     * @param fontHeader
     * @return int8_t
     */
    int8_t GetFontHeader(FontHeader& fontHeader, uint16_t fontId, uint8_t fontSize)
    {
        return instance_->GetFontHeader(fontHeader, fontId, fontSize);
    }

    /**
     * @brief Get font id
     *
     * @param name
     * @param size
     * @return uint16_t
     */
    uint16_t GetFontId(const char* name, uint8_t size = 0)
    {
        return instance_->GetFontId(name, size);
    }

    /**
     * @brief Set the cache start address
     *
     * @param psramAddr
     * @param psramLen
     */
    void SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen)
    {
        instance_->SetPsramMemory(psramAddr, psramLen);
    }

    /**
     * @brief Get bitmap for specific unicode
     *
     * @param unicode
     * @return uint8_t*
     */
    uint8_t* GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize, uint8_t shapingFont);

    int8_t GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize);

    /**
     * @brief Indicates whether the current font library is a vector font library.
     * @return uint8_t: 0 BitmapFont  1 VectorFont
     */
    bool IsVectorFont()
    {
        return instance_->IsVectorFont();
    }

    int8_t SetCurrentLangId(uint8_t langId);

    uint8_t GetCurrentLangId() const;

    int8_t GetDefaultParamByLangId(uint8_t langId, LangTextParam** pParam) const
    {
        return instance_->GetDefaultParamByLangId(langId, pParam);
    }

    int8_t GetTextUtf8(uint16_t textId, uint8_t** utf8Addr, uint16_t& utf8Len) const;

    uint8_t GetFontTtfId(uint16_t fontId, uint8_t size) const
    {
        return instance_->GetFontTtfId(fontId, size);
    }

    int32_t OpenVectorFont(uint8_t ttfId)
    {
        return instance_->OpenVectorFont(ttfId);
    }

    bool GetTtfInfo(uint8_t ttfId, uint8_t* ttfBuffer, uint32_t bufferSize, TtfHeader& ttfHeader)
    {
        return instance_->GetTtfInfo(ttfId, ttfBuffer, bufferSize, ttfHeader);
    }

    const UITextLanguageFontParam* GetFontInfo(uint16_t fontId) const
    {
        return instance_->GetFontInfo(fontId);
    }

    int8_t SetFontPath(const char* path, BaseFont::FontType type);

    int8_t GetFontVersion(BaseFont::FontType type, const char* path, char* version, uint8_t len) const
    {
        return instance_->GetFontVersion(type, path, version, len);
    }

    uint8_t RegisterFontInfo(const char* ttfName, uint8_t shaping = 0)
    {
        return instance_->RegisterFontInfo(ttfName, shaping);
    }

    uint8_t RegisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
    {
        return instance_->RegisterFontInfo(fontsTable, num);
    }

    uint8_t RegisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
    {
        return instance_->RegisterTtcFontInfo(ttcName, ttfInfo, count);
    }

    uint8_t UnregisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
    {
        return instance_->UnregisterTtcFontInfo(ttcName, ttfInfo, count);
    }

    uint8_t UnregisterFontInfo(const char* ttfName)
    {
        return instance_->UnregisterFontInfo(ttfName);
    }

    uint8_t UnregisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
    {
        return instance_->UnregisterFontInfo(fontsTable, num);
    }

    int8_t GetTextParam(uint16_t textId, UITextLanguageTextParam& param) const;

    int8_t GetWildCardStaticStr(uint16_t textId, UITextWildcardStaticType type,
        uint8_t** strAddr, uint16_t& strLen) const;

    int8_t GetCodePoints(uint16_t textId, uint32_t** codePoints, uint16_t& codePointsNum) const;

    ColorMode GetColorType(uint16_t fontId)
    {
        switch (instance_->GetFontWeight(fontId)) {
            case FONT_WEIGHT_1:
                return A1;
            case FONT_WEIGHT_2:
                return A2;
            case FONT_WEIGHT_4:
                return A4;
            case FONT_WEIGHT_8:
                return A8;
            case FONT_WEIGHT_32:
                return ARGB8888;
            default:
                return UNKNOWN;
        }
    }

    static UIFont* GetInstance();
    void SetFont(BaseFont* font);
    BaseFont* GetFont();

    void SetFontFileOffset(uint32_t offset);

    virtual uint16_t
        GetOffsetPosY(const char* text, uint16_t lineLength, bool& isAllEmoji, uint16_t fontId, uint8_t fontSize)
    {
        return instance_->GetOffsetPosY(text, lineLength, isAllEmoji, fontId, fontSize);
    }

    virtual uint16_t GetLineMaxHeight(const char* text, uint16_t lineLength, uint16_t fontId, uint8_t fontSize,
                                      uint16_t letterIndex, SpannableString* spannableString);

    bool IsEmojiFont(uint16_t fontid)
    {
        return instance_->IsEmojiFont(fontid);
    }

#if (defined(ENABLE_MIX_FONT) && (ENABLE_MIX_FONT == 1))
    /**
     * @brief Set bitmap font, only needed when using both vector font and bitmap font
     *
     * @param font bitmap font
     */
    void SetBitmapFont(BaseFont* font);

    /**
     * @brief Get bitmap font, only needed when using both vector font and bitmap font
     *
     * @return UIFont bitmap font instance
     */
    static UIFont* GetBitmapInstance();
#endif

private:
    UIFont();
    /**
     * @brief Destroy the UIFontFactor object
     *
     */
    ~UIFont();

    BaseFont* instance_;
    BaseFont* defaultInstance_;
    static bool setFontAllocFlag_;
};
} // namespace OHOS
#endif
