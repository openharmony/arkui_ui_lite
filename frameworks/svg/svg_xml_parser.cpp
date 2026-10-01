/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "svg/svg_xml_parser.h"
#include <cstring>

namespace OHOS {

namespace {

constexpr uint16_t MAX_TAG_LEN = 64;
constexpr uint16_t MAX_ATTR_NAME_LEN = 64;
constexpr uint16_t MAX_ATTR_VALUE_LEN = 256;
constexpr uint8_t COMMENT_START_LEN = 3;
constexpr uint8_t COMMENT_END_LEN = 3;
constexpr uint8_t CDATA_START_LEN = 8;
constexpr uint8_t CDATA_END_LEN = 3;
constexpr uint8_t PI_SUFFIX_LEN = 2;

bool IsSpace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

const char* SkipSpaces(const char* p)
{
    while (IsSpace(*p)) {
        p++;
    }
    return p;
}

uint32_t CopyName(char* dst, const char* src, uint32_t maxLen)
{
    uint32_t i = 0;
    while (i < maxLen - 1 && *src != '\0' && !IsSpace(*src) && *src != '>' &&
           *src != '/' && *src != '=') {
        dst[i++] = *src++;
    }
    dst[i] = '\0';
    return i;
}

void EmitAttribute(SvgXmlParserListener* listener, const char* name, const char* value)
{
    if (listener == nullptr || name == nullptr || name[0] == '\0') {
        return;
    }
    listener->OnAttribute(name, value);
}

const char* SkipNameTail(const char* p)
{
    while (*p != '\0' && !IsSpace(*p) && *p != '>' && *p != '/' && *p != '=') {
        p++;
    }
    return p;
}

const char* SkipToQuote(const char* p, char quote)
{
    while (*p != '\0' && *p != quote) {
        p++;
    }
    if (*p == quote) {
        p++;
    }
    return p;
}

const char* ParseAttrValue(const char* p, char* value, uint32_t maxLen)
{
    char quote = *p;
    if (quote != '\"' && quote != '\'') {
        return p;
    }
    p++;
    uint32_t i = 0;
    while (i < maxLen - 1 && *p != '\0' && *p != quote) {
        value[i++] = *p++;
    }
    value[i] = '\0';
    if (*p != quote) {
        // The value was truncated because the buffer is full; advance past the
        // closing quote so subsequent parsing stays aligned with the markup.
        p = SkipToQuote(p, quote);
    } else {
        p++;
    }
    return p;
}

const char* ParseAttributes(const char* p, SvgXmlParserListener* listener, bool selfClose)
{
    char name[MAX_ATTR_NAME_LEN] = { 0 };
    char value[MAX_ATTR_VALUE_LEN] = { 0 };
    while (*p != '\0' && *p != '>' && !(selfClose && *p == '/')) {
        p = SkipSpaces(p);
        if (*p == '>' || *p == '/' || *p == '\0') {
            break;
        }
        value[0] = '\0';
        CopyName(name, p, MAX_ATTR_NAME_LEN);
        p += strlen(name);
        p = SkipNameTail(p);
        p = SkipSpaces(p);
        if (*p == '=') {
            p++;
            p = SkipSpaces(p);
            p = ParseAttrValue(p, value, MAX_ATTR_VALUE_LEN);
        }
        EmitAttribute(listener, name, value);
    }
    return p;
}

const char* ParseStartTag(const char* p, SvgXmlParserListener* listener, bool& selfClose)
{
    char tag[MAX_TAG_LEN] = { 0 };
    p = SkipSpaces(p);
    CopyName(tag, p, MAX_TAG_LEN);
    p += strlen(tag);
    p = SkipNameTail(p);
    if (listener != nullptr) {
        listener->OnStartElement(tag);
    }
    p = ParseAttributes(p, listener, selfClose);
    if (*p == '/') {
        selfClose = true;
        p++;
    }
    if (*p == '>') {
        p++;
    }
    if (selfClose && listener != nullptr) {
        listener->OnEndElement(tag);
    }
    return p;
}

const char* ParseEndTag(const char* p, SvgXmlParserListener* listener)
{
    char tag[MAX_TAG_LEN] = { 0 };
    p = SkipSpaces(p);
    CopyName(tag, p, MAX_TAG_LEN);
    p += strlen(tag);
    while (*p != '>' && *p != '\0') {
        p++;
    }
    if (*p == '>') {
        p++;
    }
    if (listener != nullptr) {
        listener->OnEndElement(tag);
    }
    return p;
}

const char* ParseComment(const char* p)
{
    if (strncmp(p, "!--", COMMENT_START_LEN) == 0) {
        p += COMMENT_START_LEN;
        while (*p != '\0' && strncmp(p, "-->", COMMENT_END_LEN) != 0) {
            p++;
        }
        if (*p != '\0') {
            p += COMMENT_END_LEN;
        }
        return p;
    }
    if (strncmp(p, "![CDATA[", CDATA_START_LEN) == 0) {
        p += CDATA_START_LEN;
        while (*p != '\0' && strncmp(p, "]]>", CDATA_END_LEN) != 0) {
            p++;
        }
        if (*p != '\0') {
            p += CDATA_END_LEN;
        }
        return p;
    }
    while (*p != '>' && *p != '\0') {
        p++;
    }
    if (*p == '>') {
        p++;
    }
    return p;
}

const char* ParseText(const char* p, SvgXmlParserListener* listener)
{
    char text[SVG_MAX_TEXT_LEN] = { 0 };
    uint32_t i = 0;
    // NOTE: text content longer than SVG_MAX_TEXT_LEN is silently truncated.
    // Callers that need full text must ensure the input does not exceed this limit.
    while (*p != '<' && *p != '\0' && i < SVG_MAX_TEXT_LEN - 1) {
        text[i++] = *p++;
    }
    text[i] = '\0';
    if (listener != nullptr && i > 0) {
        listener->OnText(text);
    }
    return p;
}

const char* ParseMarkup(const char* p, SvgXmlParserListener* listener)
{
    if (listener == nullptr) {
        return p;
    }
    p++;
    if (*p == '/') {
        p++;
        p = ParseEndTag(p, listener);
    } else if (*p == '!') {
        p = ParseComment(p);
    } else if (*p == '?') {
        const char* end = strstr(p, "?>");
        if (end != nullptr) {
            p = end + PI_SUFFIX_LEN;
        } else {
            while (*p != '\0') {
                p++;
            }
        }
    } else {
        bool selfClose = false;
        p = ParseStartTag(p, listener, selfClose);
    }
    return p;
}

} // namespace

SvgResult SvgXmlParser::Parse(const char* xml, SvgXmlParserListener* listener)
{
    if (xml == nullptr || listener == nullptr) {
        return SvgResult::SVG_RESULT_INVALID_PARAM;
    }
    listener_ = listener;
    const char* p = xml;
    while (*p != '\0') {
        p = SkipSpaces(p);
        if (*p == '\0') {
            break;
        }
        if (*p == '<') {
            p = ParseMarkup(p, listener_);
        } else {
            p = ParseText(p, listener_);
        }
    }
    return SvgResult::SVG_RESULT_OK;
}

} // namespace OHOS
