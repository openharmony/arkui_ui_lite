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

#ifndef UI_FONT_VECTOR_H
#define UI_FONT_VECTOR_H
#include <memory>
#include "font/base_font.h"
#include "graphic_config.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/tttables.h"
#include "font/glyphs_cache.h"
#include "font/ui_font_cache.h"
namespace OHOS {
class UIFontVector : public BaseFont {
public:
    UIFontVector();

    ~UIFontVector();
    UIFontVector(const UIFontVector&) = delete;
    UIFontVector& operator=(const UIFontVector&) noexcept = delete;
    bool IsVectorFont() const override;
    int8_t SetFontPath(const char* path, FontType type) override;
    uint16_t GetHeight(uint16_t fontId, uint8_t fontSize) override;
    uint16_t GetFontId(const char* ttfName, uint8_t fontSize = 0) const override;
    int16_t GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize) override;
    uint8_t* GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize) override;
    int8_t GetFontHeader(FontHeader& fontHeader, uint16_t fontId, uint8_t fontSize) override;
    int8_t GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize) override;
    uint8_t GetFontWeight(uint16_t fontId) override;
    uint16_t
        GetShapingFontId(char* text, uint8_t& ttfId, uint32_t& script, uint16_t fontId, uint8_t size) const override;
    uint8_t RegisterFontInfo(const char* ttfName, uint8_t shaping = 0) override;
    uint8_t RegisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num) override;
    uint8_t UnregisterFontInfo(const char* ttfName) override;
    uint8_t UnregisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num) override;
    uint8_t RegisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count) override;
    uint8_t UnregisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count) override;
    const UITextLanguageFontParam* GetFontInfo(uint16_t fontId) const override;
    int32_t OpenVectorFont(uint8_t ttfId) override;
    bool IsColorEmojiFont(FT_Face &face);
    uint16_t GetOffsetPosY(const char* text, uint16_t lineLength,
                           bool& isEmojiLarge, uint16_t fontId, uint8_t fontSize) override;
    uint16_t GetLineMaxHeight(const char* text, uint16_t lineLength, uint16_t fontId, uint8_t fontSize,
                              uint16_t& letterIndex, SpannableString* spannableString)  override;
    bool GetTtfInfo(uint8_t ttfId, uint8_t* ttfBuffer, uint32_t ttfBufferSize, TtfHeader& ttfHeader) override;
    void SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen) override;
    int8_t SetCurrentLangId(uint8_t langId) override;
    bool IsEmojiFont(uint16_t fontId) override;

private:
    static constexpr uint8_t FONT_ID_MAX = 0xFF;
    static constexpr uint8_t FONT_TTC_MAX = 0x20;
    static constexpr uint8_t FONT_INVALID_TTF_ID = 0xFF;
    static constexpr uint8_t TTF_NAME_LEN_MAX = 128;
    UITextLanguageFontParam fontInfo_[FONT_ID_MAX] = {{}};
    std::string ttfDir_;
    FT_Library ftLibrary_;
    FT_Face ftFaces_[FONT_ID_MAX] = {0};
    uint8_t fontSize_[FONT_ID_MAX] = {0};
    uint8_t currentFontInfoNum_ = 0;
    bool freeTypeInited_;
    struct FaceInfo {
        FT_Face face;
        uint16_t key;
    };
    struct TtcInfo {
        const char* ttcName;
        FT_Stream stream;
    };
    TtcInfo ttcInfos_[FONT_TTC_MAX] = {};
    void SetFace(FaceInfo& faceInfo, uint32_t unicode);
    void SetFace(FaceInfo& faceInfo, uint32_t unicode, TextStyle textStyle);
    uint16_t GetFontId(uint32_t unicode) const;
    uint16_t GetKey(uint16_t fontId, uint8_t size);
    int8_t LoadGlyphIntoFace(uint16_t& fontId, uint8_t fontSize, uint32_t unicode, GlyphNode& glyphNode);
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    int8_t LoadGlyphIntoFace(uint16_t& fontId, uint32_t unicode, FT_Face face, TextStyle textStyle);
#endif
    void SaveGlyphNode(uint32_t unicode, uint16_t fontKey, Metric* metric);
    uint8_t IsGlyphFont(uint32_t unicode);
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    void SetItaly(FT_GlyphSlot slot);
    void SetBold(uint16_t fontId);
#endif
    int8_t GetFaceInfo(uint16_t fontId, uint8_t fontSize, FaceInfo& faceInfo);
    uint16_t GetMaxSubLineHeight(uint16_t textNum, uint16_t loopNum, uint16_t maxHeight, uint16_t emojiNum);
    bool GetTtfInfoFromTtf(uint8_t* ttfBuffer,
                           uint32_t ttfBufferSize,
                           TtfHeader& ttfHeader,
                           UITextLanguageFontParam fontInfo);
    bool GetTtfInfoFromTtc(uint8_t* ttfBuffer,
                           uint32_t ttfBufferSize,
                           TtfHeader& ttfHeader,
                           UITextLanguageFontParam fontInfo);
    void ClearFontGlyph(FT_Face face);
};
} // namespace OHOS
#endif
