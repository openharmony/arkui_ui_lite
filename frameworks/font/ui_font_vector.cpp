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

#include "font/ui_font_vector.h"
#include <freetype/ftbitmap.h>
#include <freetype/ftoutln.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftstream.h>
#include <freetype/tttags.h>

#include "common/typed_text.h"
#include "draw/draw_utils.h"
#include "font/font_ram_allocator.h"
#include "font/ui_font_cache_manager.h"
#include "gfx_utils/file.h"
#include "gfx_utils/graphic_log.h"
#include "graphic_config.h"
#include "securec.h"
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
#include "font/ui_multi_font_manager.h"
#endif
#if defined(ENABLE_SHAPING) && ENABLE_SHAPING
#include "font/ui_text_shaping.h"
#endif


namespace OHOS {
UIFontVector::UIFontVector()
{
#ifdef _WIN32
    ttfDir_ = _pgmptr;
    size_t len = ttfDir_.size();
    size_t pos = ttfDir_.find_last_of('\\');
    if (pos != std::string::npos) {
        ttfDir_.replace((pos + 1), (len - pos), VECTOR_FONT_DIR);
    }
#else
    ttfDir_ = VECTOR_FONT_DIR;
#endif // _WIN32
    ftLibrary_ = nullptr;
    freeTypeInited_ = ((FT_Init_FreeType(&ftLibrary_) == 0) ? true : false);
}

UIFontVector::~UIFontVector()
{
    if (freeTypeInited_) {
        FT_Done_FreeType(ftLibrary_);
        freeTypeInited_ = false;
        UnregisterFontInfo(DEFAULT_VECTOR_FONT_FILENAME);
    }
}

bool UIFontVector::IsColorEmojiFont(FT_Face& face)
{
    static const uint32_t tag = FT_MAKE_TAG('C', 'B', 'D', 'T');
    FT_ULong length = 0;
    FT_Load_Sfnt_Table(face, tag, 0, nullptr, &length);
    if (length) {
        return true;
    }
    return false;
}

int8_t SetupColorFont(FT_Face face, uint8_t fontSize)
{
    if (face->num_fixed_sizes == 0) {
        return INVALID_RET_VALUE;
    }
    FT_Int bestMatch = 0;
    int32_t diff = MATH_ABS(fontSize - face->available_sizes[0].width);
    for (int32_t i = 1; i < face->num_fixed_sizes; ++i) {
        int32_t ndiff = MATH_ABS(fontSize - face->available_sizes[i].width);
        if (ndiff < diff) {
            bestMatch = i;
            diff = ndiff;
        }
    }
    return FT_Select_Size(face, bestMatch); // FT_Match_Size
}

uint8_t UIFontVector::RegisterFontInfo(const char* ttfName, uint8_t shaping)
{
    if ((ttfName == nullptr) || !freeTypeInited_) {
        return FONT_INVALID_TTF_ID;
    }

    int32_t j = 0;
    while (j < FONT_ID_MAX) {
        if ((fontInfo_[j].ttfName != nullptr) && !strncmp(fontInfo_[j].ttfName, ttfName, TTF_NAME_LEN_MAX)) {
            return j;
        } else if (fontInfo_[j].ttfName == nullptr) {
            std::string ttfPath = ttfDir_;
            ttfPath.append(ttfName);
            int32_t error = FT_New_Face(ftLibrary_, ttfPath.c_str(), 0, &ftFaces_[j]);
            if (error != 0) {
                return FONT_INVALID_TTF_ID;
            }
            fontInfo_[j].ttfName = ttfName;
            fontInfo_[j].shaping = shaping;
            fontInfo_[j].ttfId = j;
            fontInfo_[j].ttcIndex = FONT_TTC_MAX;
            currentFontInfoNum_ = j + 1;
            if (IsColorEmojiFont(ftFaces_[j])) {
                fontInfo_[j].fontWeight = BPP_BIT_32;
            } else {
                fontInfo_[j].fontWeight = BPP_BIT_8;
            }
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
            UIMultiFontManager::GetInstance()->UpdateScript(fontInfo_[j]);
#endif
            return j;
        }
        j++;
    }
    return FONT_INVALID_TTF_ID;
}

uint8_t UIFontVector::RegisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
{
    if (fontsTable == nullptr) {
        return FONT_INVALID_TTF_ID;
    }
    uint8_t count = 0;
    for (uint8_t i = 0; i < num; i++) {
        uint8_t result = RegisterFontInfo(fontsTable[i].ttfName, fontsTable[i].shaping);
        if (result == FONT_INVALID_TTF_ID) {
            continue;
        }
        count++;
    }
    return count;
}

// Note: when use ttc font file, freetype should export FT_Stream_New/FT_Stream_Free function
uint8_t UIFontVector::RegisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
{
    if ((ttcName == nullptr) || !freeTypeInited_) {
        return FONT_INVALID_TTF_ID;
    }

    int32_t i = 0;
    int32_t error = 0;
    int32_t ttfIdx = 0;
    while (i < FONT_TTC_MAX) {
        if ((ttcInfos_[i].ttcName != nullptr) && !strncmp(ttcInfos_[i].ttcName, ttcName, TTF_NAME_LEN_MAX)) {
            return i;
        } else if (ttcInfos_[i].ttcName == nullptr) {
            std::string ttcPath = ttfDir_;
            ttcPath.append(ttcName);
            FT_Open_Args args = {FT_OPEN_PATHNAME, nullptr, 0, const_cast<char*>(ttcPath.c_str()),
                                 nullptr, nullptr, 0, nullptr};
            error = FT_Stream_New(ftLibrary_, &args, &ttcInfos_[i].stream);
            if (error != 0) {
                return FONT_INVALID_TTF_ID;
            }
            ttcInfos_[i].ttcName = ttcName;
            args = {FT_OPEN_STREAM, nullptr, 0, nullptr, ttcInfos_[i].stream, nullptr, 0, nullptr};
            for (uint8_t j = 0; j < count; j++) {
                while ((ttfIdx < FONT_ID_MAX) && fontInfo_[ttfIdx].ttfName != nullptr) {
                    ttfIdx++;
                }

                if (ttfIdx >= FONT_ID_MAX) {
                    return FONT_INVALID_TTF_ID;
                }
                error = FT_Open_Face(ftLibrary_, &args, j, &ftFaces_[ttfIdx]);
                if (error != 0) {
                    continue;
                }
                fontInfo_[ttfIdx].ttfName = ttfInfo[j].ttfName;
                fontInfo_[ttfIdx].shaping = ttfInfo[j].shaping;
                fontInfo_[ttfIdx].ttfId = ttfIdx;
                fontInfo_[ttfIdx].ttfIndex = j;
                fontInfo_[ttfIdx].ttcIndex = i;
                if (IsColorEmojiFont(ftFaces_[ttfIdx])) {
                    fontInfo_[ttfIdx].fontWeight = BPP_BIT_32;
                } else {
                    fontInfo_[ttfIdx].fontWeight = BPP_BIT_8;
                }
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
                UIMultiFontManager::GetInstance()->UpdateScript(fontInfo_[ttfIdx]);
#endif
            }
            return i;
        }
        i++;
    }
    return FONT_INVALID_TTF_ID;
}

uint8_t UIFontVector::UnregisterTtcFontInfo(const char* ttcName, const TtfInfo* ttfInfo, uint8_t count)
{
    if (ttcName == nullptr || ttfInfo == nullptr) {
        return FONT_INVALID_TTF_ID;
    }

    uint8_t i = 0;
    while (i < FONT_TTC_MAX) {
        if ((ttcInfos_[i].ttcName != nullptr) && !strncmp(ttcInfos_[i].ttcName, ttcName, TTF_NAME_LEN_MAX)) {
            for (uint8_t j = 0; j < count; j++) {
                UnregisterFontInfo(ttfInfo[j].ttfName);
            }
            FT_Stream_Free(ttcInfos_[i].stream, 1);
            ttcInfos_[i].ttcName = nullptr;
            ttcInfos_[i].stream = nullptr;
            return i;
        }
        i++;
    }
    return FONT_INVALID_TTF_ID;
}

uint8_t UIFontVector::UnregisterFontInfo(const UITextLanguageFontParam* fontsTable, uint8_t num)
{
    if (fontsTable == nullptr) {
        return 0;
    }
    uint8_t count = 0;
    for (uint8_t i = 0; i < num; i++) {
        uint8_t result = UnregisterFontInfo(fontsTable[i].ttfName);
        if (result == FONT_INVALID_TTF_ID) {
            return FONT_INVALID_TTF_ID;
        }
        count++;
    }
    return count;
}
uint8_t UIFontVector::UnregisterFontInfo(const char* ttfName)
{
    if (ttfName != nullptr) {
        int32_t i = 0;
        while (i < FONT_ID_MAX) {
            if ((fontInfo_[i].ttfName != nullptr) && !strncmp(fontInfo_[i].ttfName, ttfName, TTF_NAME_LEN_MAX)) {
                fontInfo_[i].ttfName = nullptr;
                FT_Done_Face(ftFaces_[i]);
                ftFaces_[i] = nullptr;
                fontSize_[i] = 0;
                return static_cast<uint8_t>(i);
            }
            i++;
        }
    }
    return FONT_INVALID_TTF_ID;
}

const UITextLanguageFontParam* UIFontVector::GetFontInfo(uint16_t fontId) const
{
    if (fontId < FONT_ID_MAX) {
        return static_cast<const UITextLanguageFontParam*>(&fontInfo_[fontId]);
    }
    return nullptr;
}

int32_t UIFontVector::OpenVectorFont(uint8_t ttfId)
{
    int32_t i = 0;
    int32_t fp = 0;
    while (i < FONT_ID_MAX) {
        if (fontInfo_[i].ttfName == nullptr) {
            i++;
            continue;
        }
        if (fontInfo_[i].ttfId == ttfId) {
            std::string ttfPath = ttfDir_;
            ttfPath.append(fontInfo_[i].ttfName);
#ifdef _WIN32
            fp = open(ttfPath.c_str(), O_RDONLY | O_BINARY);
#else
            fp = open(ttfPath.c_str(), O_RDONLY);
#endif
            return fp;
        }
        i++;
    }
    return -1;
}
bool UIFontVector::GetTtfInfo(uint8_t ttfId, uint8_t* ttfBuffer, uint32_t ttfBufferSize, TtfHeader& ttfHeader)
{
    if ((ttfBuffer == nullptr) || (ttfBufferSize == 0)) {
        return false;
    }
    for (int16_t i = 0; i < FONT_ID_MAX; i++) {
        if (fontInfo_[i].ttfName == nullptr) {
            continue;
        }
        if (fontInfo_[i].ttfId == ttfId) {
            if (fontInfo_[i].ttcIndex != FONT_TTC_MAX) {
                return GetTtfInfoFromTtc(ttfBuffer, ttfBufferSize, ttfHeader, fontInfo_[i]);
            } else {
                return GetTtfInfoFromTtf(ttfBuffer, ttfBufferSize, ttfHeader, fontInfo_[i]);
            }
        }
    }
    return false;
}

bool UIFontVector::GetTtfInfoFromTtf(uint8_t* ttfBuffer,
                                     uint32_t ttfBufferSize,
                                     TtfHeader& ttfHeader,
                                     UITextLanguageFontParam fontInfo)
{
    if ((ttfBuffer == nullptr) || (ttfBufferSize == 0)) {
        return false;
    }
    std::string ttfPath = ttfDir_;
    ttfPath.append(fontInfo.ttfName);
    int32_t fpTtf = 0;
#ifdef _WIN32
    fpTtf = open(ttfPath.c_str(), O_RDONLY | O_BINARY);
#else
    fpTtf = open(ttfPath.c_str(), O_RDONLY);
#endif
    if (fpTtf < 0) {
        return false;
    }
    int32_t headerLength = lseek(fpTtf, 0, SEEK_END);
    if (headerLength < 0) {
        close(fpTtf);
        return false;
    }
    ttfHeader.len = static_cast<uint32_t>(headerLength);
    if (ttfHeader.len > ttfBufferSize) {
        close(fpTtf);
        return false;
    }
    int32_t ret = lseek(fpTtf, 0, SEEK_SET);
    if (ret != 0) {
        close(fpTtf);
        return false;
    }
    ret = read(fpTtf, reinterpret_cast<void*>(ttfBuffer), ttfHeader.len);
    if (ret != headerLength) {
        close(fpTtf);
        return false;
    }
    close(fpTtf);
    return true;
}

struct TtcHeader {
    uint32_t ttcTag;
    uint16_t major;
    uint16_t minor;
    int32_t numFonts;
};
bool UIFontVector::GetTtfInfoFromTtc(uint8_t* ttfBuffer,
                                     uint32_t ttfBufferSize,
                                     TtfHeader& ttfHeader,
                                     UITextLanguageFontParam fontInfo)
{
    if ((ttfBuffer == nullptr) || (ttfBufferSize == 0) || (fontInfo.ttcIndex >= FONT_TTC_MAX)) {
        return false;
    }
    FT_Stream stream = ttcInfos_[fontInfo.ttcIndex].stream;
    if (stream == nullptr) {
        return false;
    }

    FT_Error error = FT_Err_Ok;
    if (FT_STREAM_SEEK(0)) {
        return false;
    }

    // read ttc header
    TtcHeader header = {};
    static const FT_Frame_Field ttcHeaderFields[] = {
#undef FT_STRUCTURE
#define FT_STRUCTURE TtcHeader
        FT_FRAME_START(12), // 12: see ttc header
        FT_FRAME_ULONG(ttcTag), FT_FRAME_LONG(numFonts), FT_FRAME_END};
    if (FT_STREAM_READ_FIELDS(ttcHeaderFields, &header)) {
        return false;
    }

    // check if ttc file
    if (header.ttcTag != TTAG_ttcf) { // 'ttcf' - TTC file
        return false;
    }

    uint8_t ttfIndex = fontInfo.ttfIndex;
    if (ttfIndex >= header.numFonts) {
        // invalid index
        return false;
    }

    // change position to the ttf offset
    if (FT_STREAM_SKIP(4 * ttfIndex)) { // 4: table dictionary offset length
        return false;
    }

    // get the ttf length
    uint32_t ttfOffset;
    if (FT_READ_ULONG(ttfOffset)) {
        return false;
    }
    uint32_t ttfLength = 0;
    FT_ULong ttcLength = stream->size;
    if (ttcLength < ttfOffset) {
        return false;
    }
    if (ttfIndex + 1 == header.numFonts) {
        ttfLength = ttcLength - ttfOffset;
    } else {
        uint32_t nextTtfOffset;
        if (FT_READ_ULONG(nextTtfOffset)) {
            return false;
        }
        ttfLength = nextTtfOffset - ttfOffset;
    }
    if (ttfLength > ttfBufferSize) {
        return false;
    }
    if (FT_STREAM_SEEK(ttfOffset) || FT_STREAM_READ(ttfBuffer, ttfLength)) {
        return false;
    }
    ttfHeader.len = ttfLength;

    // read number of tables
    uint16_t numTables;
    if (FT_STREAM_SEEK(ttfOffset + 4) || FT_READ_USHORT(numTables)) { // 4: sfntVersion length
        return false;
    }

    // change the offset of the ttf tableRecord compare with ttfOffset from ttc header
    uint32_t* p = reinterpret_cast<uint32_t*>(ttfBuffer + 20); // 20: 12(TableDirectory) + 8(tableTag and checksum)
    for (uint16_t i = 0; i < numTables; i++) {
        p[0] = FT_PEEK_ULONG(p) - ttfOffset;
        p[0] = FT_PEEK_ULONG(p);
        p += 4; // 4: Table Record size
    }
    return true;
}
bool UIFontVector::IsVectorFont() const
{
    return true;
}

uint8_t UIFontVector::GetFontWeight(uint16_t fontId)
{
    if (fontId >= FONT_ID_MAX) {
        return BPP_BIT_8;
    }

    return fontInfo_[fontId].fontWeight;
}

int8_t UIFontVector::SetFontPath(const char* path, FontType type)
{
    if (path == nullptr) {
        return INVALID_RET_VALUE;
    }
    ttfDir_ = path;
    return RET_VALUE_OK;
}

int8_t UIFontVector::GetFaceInfo(uint16_t fontId, uint8_t fontSize, FaceInfo& faceInfo)
{
    if ((fontId >= FONT_ID_MAX) || (fontSize == 0)) {
        return INVALID_RET_VALUE;
    }

    faceInfo.key = GetKey(fontId, fontSize);
    faceInfo.face = ftFaces_[fontId];

    if (fontSize_[fontId] == fontSize) {
        return RET_VALUE_OK;
    }
    const UITextLanguageFontParam* fontInfo = GetFontInfo(fontId);
    if ((fontInfo == nullptr) || (fontInfo->ttfName == nullptr)) {
        return INVALID_RET_VALUE;
    }

    if (!freeTypeInited_) {
        return INVALID_RET_VALUE;
    }

    // Set the size
    int8_t ret;
    if (IsEmojiFont(fontId)) {
        ret = SetupColorFont(ftFaces_[fontId], fontSize);
    } else {
        ret = FT_Set_Char_Size(faceInfo.face, fontSize * FONT_PIXEL_IN_POINT, 0, 0, 0);
    }

    if (ret != 0) {
        return INVALID_RET_VALUE;
    }
    fontSize_[fontId] = fontSize;
    return RET_VALUE_OK;
}

uint16_t UIFontVector::GetHeight(uint16_t fontId, uint8_t fontSize)
{
    FaceInfo faceInfo;
    int8_t ret = GetFaceInfo(fontId, fontSize, faceInfo);
    if (ret != RET_VALUE_OK) {
        return INVALID_RET_VALUE;
    }
    if (!freeTypeInited_ || (faceInfo.face == nullptr)) {
        return 0;
    }
    return static_cast<uint16_t>(faceInfo.face->size->metrics.height / FONT_PIXEL_IN_POINT);
}

uint16_t
    UIFontVector::GetShapingFontId(char* text, uint8_t& ttfId, uint32_t& script, uint16_t fontId, uint8_t size) const
{
#if defined(ENABLE_MULTI_FONT) && ENABLE_MULTI_FONT
    const UITextLanguageFontParam* fontParam1 = GetFontInfo(fontId);
    if (fontParam1 == nullptr) {
        return 0;
    }
    if (fontParam1->shaping == 0) {
        UIMultiFontManager* multiFontManager = UIMultiFontManager::GetInstance();
        if (!multiFontManager->IsNeedShaping(text, ttfId, script)) {
            return 0; // 0 means  no need to shape
        }
        uint16_t* searchLists = nullptr;
        int8_t length = multiFontManager->GetSearchFontList(fontId, &searchLists);
        const UITextLanguageFontParam* fontParam2 = nullptr;
        for (uint8_t i = 0; i < length; i++) {
            fontParam2 = GetFontInfo(searchLists[i]);
            if (fontParam2 == nullptr) {
                continue;
            }
            if (fontParam2->ttfId == ttfId) {
                return fontParam2->shaping;
            }
        }
        return 0;
    }
    ttfId = fontParam1->ttfId;

#if defined(ENABLE_SHAPING) && ENABLE_SHAPING
    script = UIMultiFontManager::GetInstance()->GetScriptByTtfId(ttfId);
#endif
    return fontParam1->shaping;
#else
    const UITextLanguageFontParam* fontInfo = GetFontInfo(fontId);
    if (fontInfo == nullptr) {
        return 0;
    }
    ttfId = fontInfo->ttfId;
    return fontInfo->shaping;
#endif
}

uint16_t UIFontVector::GetFontId(const char* ttfName, uint8_t fontSize) const
{
    if (ttfName != nullptr) {
        int32_t i = 0;
        while (i < FONT_ID_MAX) {
            if ((fontInfo_[i].ttfName != nullptr) && (strstr(fontInfo_[i].ttfName, ttfName) != nullptr)) {
                return static_cast<uint16_t>(i);
            }
            i++;
        }
    }

    return static_cast<uint16_t>(FONT_ID_MAX);
}

uint16_t UIFontVector::GetFontId(uint32_t unicode) const
{
    int32_t i = 0;
    uint8_t ttfId = ((unicode >> 24) & 0x1F); // 24: Whether 25 ~29 bit storage is ttfId 0x1F:5bit
    while (i < FONT_ID_MAX) {
        if (fontInfo_[i].ttfName == nullptr) {
            i++;
            continue;
        }
        if (fontInfo_[i].ttfId == ttfId) {
            return i;
        }
        i++;
    }
    return FONT_INVALID_TTF_ID;
}

int16_t UIFontVector::GetWidth(uint32_t unicode, uint16_t fontId, uint8_t fontSize)
{
    if ((fontId >= FONT_ID_MAX) || (fontSize == 0)) {
        return INVALID_RET_VALUE;
    }

    GlyphNode node;
    int8_t ret = GetGlyphNode(unicode, node, fontId, fontSize);
    if (ret != RET_VALUE_OK) {
        return INVALID_RET_VALUE;
    }
    return node.advance;
}

int8_t UIFontVector::GetFontHeader(FontHeader& fontHeader, uint16_t fontId, uint8_t fontSize)
{
    FaceInfo faceInfo;
    int8_t ret = GetFaceInfo(fontId, fontSize, faceInfo);
    if (ret != RET_VALUE_OK) {
        return INVALID_RET_VALUE;
    }
    if (!freeTypeInited_ || (faceInfo.face == nullptr)) {
        return INVALID_RET_VALUE;
    }

    fontHeader.ascender = static_cast<int16_t>(faceInfo.face->size->metrics.ascender / FONT_PIXEL_IN_POINT);
    fontHeader.descender = static_cast<int16_t>(faceInfo.face->size->metrics.descender / FONT_PIXEL_IN_POINT);
    fontHeader.fontHeight = static_cast<uint16_t>(faceInfo.face->size->metrics.height / FONT_PIXEL_IN_POINT);
    return RET_VALUE_OK;
}

void UIFontVector::SaveGlyphNode(uint32_t unicode, uint16_t fontKey, Metric *metric)
{
    GlyphCacheNode* node = UIFontCacheManager::GetInstance()->GetNodeCacheSpace(unicode, fontKey);
    if (node == nullptr) {
        return;
    }
    node->node.left = metric->left;
    node->node.top = metric->top;
    node->node.cols = metric->cols;
    node->node.rows = metric->rows;
    node->node.advance = metric->advance;
    node->node.unicode = unicode;
    node->node.fontId = fontKey;
}

int8_t UIFontVector::GetGlyphNode(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
{
    // get glyph from glyph cache
    uint16_t fontKey = GetKey(fontId, fontSize);
    UIFontCacheManager* fontCacheManager = UIFontCacheManager::GetInstance();
    GlyphCacheNode* cacheNode = fontCacheManager->GetNodeFromCache(unicode, fontKey, GlyphCacheType::CACHE_TYPE_NONE);
    if (cacheNode != nullptr) {
        glyphNode = cacheNode->node;
        return RET_VALUE_OK;
    }

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    uint8_t* bitmap = fontCacheManager->GetBitmap(fontKey, unicode, glyphNode.textStyle);
#else
    uint8_t* bitmap = fontCacheManager->GetBitmap(fontKey, unicode, TEXT_STYLE_NORMAL);
#endif
    if (bitmap != nullptr) {
        Metric* f = reinterpret_cast<Metric*>(bitmap);
        glyphNode.left = f->left;
        glyphNode.top = f->top;
        glyphNode.cols = f->cols;
        glyphNode.rows = f->rows;
        glyphNode.advance = f->advance;
        glyphNode.fontId = fontId;

        SaveGlyphNode(unicode, fontKey, f);
        return RET_VALUE_OK;
    }

    FaceInfo faceInfo;
    int8_t ret = GetFaceInfo(fontId, fontSize, faceInfo);
    if (ret != RET_VALUE_OK) {
        return INVALID_RET_VALUE;
    }
    if (faceInfo.face == nullptr) {
        return INVALID_RET_VALUE;
    }

    int8_t error = LoadGlyphIntoFace(fontId, fontSize, unicode, glyphNode);
    if (error != RET_VALUE_OK) {
        return INVALID_RET_VALUE;
    }

    return RET_VALUE_OK;
}

uint8_t* UIFontVector::GetBitmap(uint32_t unicode, GlyphNode& glyphNode, uint16_t fontId, uint8_t fontSize)
{
    uint16_t fontKey = GetKey(fontId, fontSize);
    UIFontCacheManager* fontCacheManager = UIFontCacheManager::GetInstance();
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    uint8_t* bitmap = fontCacheManager->GetBitmap(fontKey, unicode, glyphNode.textStyle);
#else
    uint8_t* bitmap = fontCacheManager->GetBitmap(fontKey, unicode, TEXT_STYLE_NORMAL);
#endif
    if (bitmap != nullptr) {
        Metric* f = reinterpret_cast<Metric*>(bitmap);
        glyphNode.left = f->left;
        glyphNode.top = f->top;
        glyphNode.cols = f->cols;
        glyphNode.rows = f->rows;
        glyphNode.advance = f->advance;
        glyphNode.fontId = fontId;
        SaveGlyphNode(unicode, fontKey, f);
        return bitmap + sizeof(Metric);
    }
    FaceInfo faceInfo;
    int8_t ret = GetFaceInfo(fontId, fontSize, faceInfo);
    if (ret != RET_VALUE_OK) {
        return nullptr;
    }
    if (faceInfo.face == nullptr) {
        return nullptr;
    }

    int8_t error = LoadGlyphIntoFace(fontId, fontSize, unicode, glyphNode);
    if (error != RET_VALUE_OK) {
        return nullptr;
    }

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    bitmap = fontCacheManager->GetBitmap(fontKey, unicode, glyphNode.textStyle);
#else
    bitmap = fontCacheManager->GetBitmap(fontKey, unicode, TEXT_STYLE_NORMAL);
#endif
    if (bitmap != nullptr) {
        return bitmap + sizeof(Metric);
    } else {
        return nullptr;
    }
}

bool UIFontVector::IsEmojiFont(uint16_t fontId)
{
    if (fontId >= FONT_ID_MAX) {
        return false;
    }
    return (fontInfo_[fontId].fontWeight >= 16); // 16: rgb color font
}

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
void UIFontVector::SetItaly(FT_GlyphSlot slot)
{
    if (slot->format != FT_GLYPH_FORMAT_OUTLINE) {
        GRAPHIC_LOGE("SetItaly error");
        return;
    }
    float lean = 0.2f; // Slope of word
    FT_Matrix matrix;
    matrix.xx = 0x10000L; // Staggered matrix along x-axis
    matrix.xy = lean * 0x10000L;
    matrix.yx = 0;
    matrix.yy = 0x10000L; // Staggered matrix along y-axis
    FT_Outline outline = slot->outline;
    FT_Outline_Transform(&outline, &matrix);
}
#endif

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
void UIFontVector::SetBold(uint16_t fontId)
{
    int32_t error;
    FT_GlyphSlot slot = ftFaces_[fontId]->glyph;
    // some reasonable strength, copied from freeType
    FT_Pos xBold = FT_MulFix(ftFaces_[fontId]->units_per_EM, ftFaces_[fontId]->size->metrics.y_scale) / 24;
    FT_Pos yBold = xBold;
    if (ftFaces_[fontId]->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_BBox oldBox;
        FT_Outline_Get_CBox(&slot->outline, &oldBox);
        error = FT_Outline_Embolden(&slot->outline, xBold);
        if (error != 0) {
            GRAPHIC_LOGE("SetBold error");
            return;
        }
    } else if (ftFaces_[fontId]->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
        FT_Library ftLibrary = slot->library;
        error = FT_Bitmap_Embolden(ftLibrary, &slot->bitmap, xBold, yBold);
        if (error != 0) {
            GRAPHIC_LOGE("SetBold error");
            return;
        }
    }
}
#endif

int8_t UIFontVector::LoadGlyphIntoFace(uint16_t& fontId, uint8_t fontSize, uint32_t unicode, GlyphNode& glyphNode)
{
    int32_t error;
    if (IsGlyphFont(unicode) != 0) {
        if (fontId >= FONT_ID_MAX || fontId != GetFontId(unicode)) {
            return INVALID_RET_VALUE;
        }
        error = FT_Load_Glyph(ftFaces_[fontId], unicode & (0xFFFFFF), FT_LOAD_RENDER);
    } else {
        if (IsEmojiFont(fontId)) {
            error = FT_Load_Char(ftFaces_[fontId], unicode, FT_LOAD_COLOR);
        } else {
            error = FT_Load_Char(ftFaces_[fontId], unicode, FT_LOAD_RENDER);
        }
    }
    if ((error != 0) || (ftFaces_[fontId]->glyph->glyph_index == 0)) {
        return INVALID_RET_VALUE;
    }

    FaceInfo faceInfo;
    faceInfo.key = GetKey(fontId, fontSize);
    faceInfo.face = ftFaces_[fontId];

    glyphNode.left = faceInfo.face->glyph->bitmap_left;
    glyphNode.top = faceInfo.face->glyph->bitmap_top;
    glyphNode.cols = faceInfo.face->glyph->bitmap.width;
    glyphNode.rows = faceInfo.face->glyph->bitmap.rows;
    glyphNode.advance = static_cast<uint16_t>(faceInfo.face->glyph->advance.x / FONT_PIXEL_IN_POINT);
    glyphNode.fontId = fontId;

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    SetFace(faceInfo, unicode, glyphNode.textStyle);
#else
    SetFace(faceInfo, unicode);
#endif
    return RET_VALUE_OK;
}

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
int8_t UIFontVector::LoadGlyphIntoFace(uint16_t& fontId, uint32_t unicode, FT_Face face, TextStyle textStyle)
{
    int32_t error;
    if (IsGlyphFont(unicode) != 0) {
        if (fontId != GetFontId(unicode)) {
            return INVALID_RET_VALUE;
        }
        unicode = unicode & (0xFFFFFF); // Whether 0 ~24 bit storage is unicode
        error = FT_Load_Glyph(ftFaces_[fontId], unicode, FT_LOAD_RENDER);
    } else {
        if (IsEmojiFont(fontId)) {
            error = FT_Load_Char(ftFaces_[fontId], unicode, FT_LOAD_COLOR);
        } else {
            error = FT_Load_Char(ftFaces_[fontId], unicode, FT_LOAD_RENDER);
        }
    }
    if ((error != 0) || (ftFaces_[fontId]->glyph->glyph_index == 0)) {
        return INVALID_RET_VALUE;
    }
    if (textStyle == TEXT_STYLE_ITALIC) {
        SetItaly(ftFaces_[fontId]->glyph);
    } else if (textStyle == TEXT_STYLE_BOLD) {
        SetBold(fontId);
    } else if (textStyle == TEXT_STYLE_BOLD_ITALIC) {
        SetItaly(ftFaces_[fontId]->glyph);
        SetBold(fontId);
    }
    if (ftFaces_[fontId]->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        error = FT_Render_Glyph(ftFaces_[fontId]->glyph, FT_RENDER_MODE_NORMAL);
    }
    if ((error != 0) || (ftFaces_[fontId]->glyph->glyph_index == 0)) {
        return INVALID_RET_VALUE;
    }
    return RET_VALUE_OK;
}
#endif

uint8_t UIFontVector::IsGlyphFont(uint32_t unicode)
{
    uint16_t unicodeFontId = GetFontId(unicode);
    if (unicodeFontId == FONT_INVALID_TTF_ID) {
        return 0;
    } else {
        return fontInfo_[unicodeFontId].shaping;
    }
}

void UIFontVector::SetFace(FaceInfo& faceInfo, uint32_t unicode)
{
    SetFace(faceInfo, unicode, TEXT_STYLE_NORMAL);
}

void UIFontVector::SetFace(FaceInfo& faceInfo, uint32_t unicode, TextStyle textStyle)
{
    Metric* f = reinterpret_cast<Metric*>(UIMalloc(sizeof(Metric)));
    if (f == nullptr) {
        return;
    }
    f->advance = static_cast<uint16_t>(faceInfo.face->glyph->advance.x / FONT_PIXEL_IN_POINT);
    f->left = faceInfo.face->glyph->bitmap_left;
    f->top = faceInfo.face->glyph->bitmap_top;
    f->cols = faceInfo.face->glyph->bitmap.width;
    f->rows = faceInfo.face->glyph->bitmap.rows;

    // cache glyph
    SaveGlyphNode(unicode, faceInfo.key, f);

    int16_t pixSize;
    ColorMode mode;
    if (faceInfo.face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
        pixSize = 0x04; // 4 Byte
        mode = ARGB8888;
    } else {
        pixSize = 1;
        mode = A8;
    }

    GlyphNode glyphNode;
    glyphNode.left = f->left;
    glyphNode.top = f->top;
    glyphNode.cols = f->cols;
    glyphNode.rows = f->rows;
    glyphNode.advance = f->advance;
    glyphNode.unicode = unicode;
    glyphNode.fontId = faceInfo.key;
    BufferInfo bufInfo = UIFontAllocator::GetCacheBuffer(faceInfo.key, unicode, mode, glyphNode, true, textStyle);
    uint32_t bitmapSize = bufInfo.stride * bufInfo.height;
    uint32_t rawSize = glyphNode.cols * glyphNode.rows * pixSize;

    if (bufInfo.virAddr != nullptr) {
        if (memcpy_s(bufInfo.virAddr, sizeof(Metric), f, sizeof(Metric)) != EOK) {
            UIFontCacheManager::GetInstance()->PutSpace(reinterpret_cast<uint8_t*>(bufInfo.virAddr));
            UIFree(f);
            return;
        }
        if ((faceInfo.face->glyph->bitmap.buffer != nullptr) &&
            (memcpy_s(reinterpret_cast<uint8_t*>(bufInfo.virAddr) + sizeof(Metric), bitmapSize,
                      faceInfo.face->glyph->bitmap.buffer, rawSize) != EOK)) {
            UIFontCacheManager::GetInstance()->PutSpace(reinterpret_cast<uint8_t*>(bufInfo.virAddr));
            UIFree(f);
            return;
        }
        UIFontAllocator::RearrangeBitmap(bufInfo, rawSize, true);
        ClearFontGlyph(faceInfo.face);
    }
    UIFree(f);
}

void UIFontVector::ClearFontGlyph(FT_Face face)
{
    if ((face != nullptr) && (face->glyph != nullptr)) {
        // free unicode buffer immediately to save memory in multi font file load
        // if not, it will be freed in next glyph load
        ft_glyphslot_free_bitmap(face->glyph);
        FT_Outline_Done(face->glyph->library, &face->glyph->outline);
        if (face->glyph->internal != nullptr) {
            FT_GlyphLoader_Reset(face->glyph->internal->loader);
        }
    }
}

inline uint16_t UIFontVector::GetKey(uint16_t fontId, uint8_t size)
{
    return ((static_cast<uint16_t>(fontId)) << 8) + size; // fontId store at the (8+1)th bit
}

uint16_t UIFontVector::GetOffsetPosY(const char* text,
                                     uint16_t lineLength,
                                     bool& isEmojiLarge,
                                     uint16_t fontId,
                                     uint8_t fontSize)
{
    if (!freeTypeInited_) {
        return INVALID_RET_VALUE;
    }
    uint32_t i = 0;
    uint16_t textNum = 0;
    uint16_t emojiNum = 0;
    uint16_t loopNum = 0;
    GlyphNode glyphNode;
    GlyphNode emojiMaxNode = {};
    uint8_t maxFontSize = fontSize;
    while (i < lineLength) {
        uint32_t unicode = TypedText::GetUTF8Next(text, i, i);
        uint8_t ret = GetGlyphNode(unicode, glyphNode, fontId, fontSize);
        if (ret == RET_VALUE_OK) {
            uint8_t weight = GetFontWeight(glyphNode.fontId);
            // 16: bit rgb565 rgba8888
            if (weight >= 16) {
                emojiMaxNode = glyphNode.rows > emojiMaxNode.rows ? glyphNode : emojiMaxNode;
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
        isEmojiLarge = true;
        return 0;
    }
    isEmojiLarge = emojiMaxNode.rows > maxFontSize;
    uint16_t offset = 0;
    if (isEmojiLarge) {
        // If the emoji is higher than the text
        if (emojiMaxNode.top >= maxFontSize) {
            offset = emojiMaxNode.top - maxFontSize;
        }
    } else {
        // If text are higher than emoji
        if (maxFontSize >= emojiMaxNode.rows) {
            offset = maxFontSize - emojiMaxNode.rows;
        }
    }
    return offset;
}

uint16_t UIFontVector::GetLineMaxHeight(const char* text,
                                        uint16_t lineLength,
                                        uint16_t fontId,
                                        uint8_t fontSize,
                                        uint16_t& letterIndex,
                                        SpannableString* spannableString)
{
    if (!freeTypeInited_) {
        return INVALID_RET_VALUE;
    }
    uint32_t i = 0;
    uint16_t textNum = 0;
    uint16_t emojiNum = 0;
    uint16_t loopNum = 0;
    uint16_t maxHeight = GetHeight(fontId, fontSize);
    while (i < lineLength) {
        uint32_t unicode = TypedText::GetUTF8Next(text, i, i);
        TypedText::IsColourWord(unicode, fontId, fontSize) ? emojiNum++ : textNum++;
        loopNum++;
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
    return GetMaxSubLineHeight(textNum, loopNum, maxHeight, emojiNum);
}

uint16_t UIFontVector::GetMaxSubLineHeight(uint16_t textNum, uint16_t loopNum, uint16_t maxHeight, uint16_t emojiNum)
{
    // The number of words is the same as the number of cycles, which means that this line is all words
    if (textNum == loopNum) {
        for (uint8_t i = 0; i < currentFontInfoNum_; i++) {
            if (!IsEmojiFont(i)) {
                uint16_t height = static_cast<uint16_t>(ftFaces_[i]->size->metrics.height / FONT_PIXEL_IN_POINT);
                if (height > maxHeight) {
                    maxHeight = height;
                }
                return maxHeight;
            }
        }
    }
    // The number of emoji is the same as the number of cycles, indicating that this line is all emoji
    if (emojiNum == loopNum) {
        for (uint8_t i = 0; i < currentFontInfoNum_; i++) {
            if (IsEmojiFont(i)) {
                return static_cast<uint16_t>(ftFaces_[i]->size->metrics.height / FONT_PIXEL_IN_POINT);
            }
        }
    }
    // A line has both emoji and words
    if ((textNum > 0) && (emojiNum > 0)) {
        for (uint8_t i = 0; i < currentFontInfoNum_; i++) {
            uint16_t tmpHeight = static_cast<uint16_t>(ftFaces_[i]->size->metrics.height / FONT_PIXEL_IN_POINT);
            maxHeight = tmpHeight > maxHeight ? tmpHeight : maxHeight;
        }
    }
    return maxHeight;
}

void UIFontVector::SetPsramMemory(uintptr_t psramAddr, uint32_t psramLen)
{
    BaseFont::SetPsramMemory(psramAddr, psramLen);
    FontRamAllocator::GetInstance().SetRamAddr(psramAddr, psramLen);
}

int8_t UIFontVector::SetCurrentLangId(uint8_t langId)
{
    GRAPHIC_LOGE("UIFontVector::SetCurrentLangId start");
    FontRamAllocator::GetInstance().ClearRam();
#if defined(ENABLE_SHAPING) && ENABLE_SHAPING
    UITextShaping::GetInstance()->ClearTtfHeader();
#endif
    UIFontCacheManager* fontCacheManager = UIFontCacheManager::GetInstance();
    fontCacheManager->ClearCacheFlag();
    fontCacheManager->BitmapCacheClear();

    if (fontCacheManager->GlyphsCacheInit() != RET_VALUE_OK) {
        GRAPHIC_LOGE("UIFontCacheManager::GlyphsCacheInit init failed");
        return INVALID_RET_VALUE;
    }

    fontCacheManager->BitmapCacheInit();
    return RET_VALUE_OK;
}
} // namespace OHOS
