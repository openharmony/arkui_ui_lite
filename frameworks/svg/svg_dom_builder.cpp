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

#include "svg/svg_dom_builder.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_element_factory.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_generic_node.h"
#include "svg/svg_image_node.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_text_nodes.h"
#include <cstring>

namespace OHOS {

namespace {

constexpr uint16_t MAX_STYLE_NAME_LEN = 64;
constexpr uint16_t MAX_STYLE_VALUE_LEN = 256;

bool ParseStyleName(const char*& p, char* name, uint32_t nameSize)
{
    if (p == nullptr || name == nullptr || nameSize == 0) {
        return false;
    }
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    uint32_t i = 0;
    while (*p != '\0' && *p != ':' && *p != ';' && i < nameSize - 1) {
        name[i++] = *p++;
    }
    while (i > 0 && (name[i - 1] == ' ' || name[i - 1] == '\t')) {
        i--;
    }
    name[i] = '\0';
    return i > 0;
}

bool ParseStyleValue(const char*& p, char* val, uint32_t valSize)
{
    if (p == nullptr || val == nullptr || valSize == 0) {
        return false;
    }
    if (*p != ':') {
        return false;
    }
    p++;
    while (*p == ' ') {
        p++;
    }
    uint32_t i = 0;
    while (*p != '\0' && *p != ';' && i < valSize - 1) {
        val[i++] = *p++;
    }
    while (i > 0 && (val[i - 1] == ' ' || val[i - 1] == '\t')) {
        i--;
    }
    val[i] = '\0';
    return true;
}

struct TagEntry {
    const char* tag;
    SvgElementType type;
};

const TagEntry TAG_MAP[] = {
    // Containers
    {"svg", SVG_ROOT},
    {"g", SVG_GROUP},
    {"defs", SVG_DEFS},
    {"use", SVG_USE},
    {"switch", SVG_SWITCH},
    {"a", SVG_A},
    // Shapes
    {"rect", SVG_RECT},
    {"circle", SVG_CIRCLE},
    {"ellipse", SVG_ELLIPSE},
    {"line", SVG_LINE},
    {"polyline", SVG_POLYLINE},
    {"polygon", SVG_POLYGON},
    {"path", SVG_PATH},
    // Text
    {"text", SVG_TEXT},
    {"textArea", SVG_TEXT_AREA},
    {"tspan", SVG_TSPAN},
    {"tbreak", SVG_TBREAK},
    {"tref", SVG_TREF},
    // Paint servers
    {"linearGradient", SVG_LINEAR_GRADIENT},
    {"radialGradient", SVG_RADIAL_GRADIENT},
    {"solidColor", SVG_SOLID_COLOR},
    {"stop", SVG_STOP},
    // Media
    {"image", SVG_IMAGE},
    {"img", SVG_IMAGE},
    {"video", SVG_VIDEO},
    {"audio", SVG_AUDIO},
    {"animation", SVG_ANIMATION},
    {"foreignObject", SVG_FOREIGN_OBJECT},
    {"object", SVG_OBJECT},
    {"iframe", SVG_IFRAME},
    {"applet", SVG_APPLET},
    // Metadata
    {"desc", SVG_DESC},
    {"title", SVG_TITLE},
    {"metadata", SVG_METADATA},
    {"style", SVG_STYLE},
    {"script", SVG_SCRIPT},
    // Fonts
    {"font", SVG_FONT},
    {"font-face", SVG_FONT_FACE},
    {"font-face-src", SVG_FONT_FACE_SRC},
    {"font-face-uri", SVG_FONT_FACE_URI},
    {"glyph", SVG_GLYPH},
    {"hkern", SVG_HKERN},
    {"missing-glyph", SVG_MISSING_GLYPH},
    // Events
    {"handler", SVG_HANDLER},
    {"listener", SVG_LISTENER},
    // Resources
    {"prefetch", SVG_PREFETCH},
    {"discard", SVG_DISCARD},
    // Animations
    {"animate", SVG_ANIMATE},
    {"animateColor", SVG_ANIMATE_COLOR},
    {"animateMotion", SVG_ANIMATE_MOTION},
    {"animateTransform", SVG_ANIMATE_TRANSFORM},
    {"set", SVG_SET},
    {"mpath", SVG_MPATH},
};

SvgElementType MatchSvgElementType(const char* tag)
{
    if (tag == nullptr) {
        return SVG_UNKNOWN;
    }
    const uint32_t entryCount = sizeof(TAG_MAP) / sizeof(TAG_MAP[0]);
    for (uint32_t i = 0; i < entryCount; ++i) {
        if (strcmp(tag, TAG_MAP[i].tag) == 0) {
            return TAG_MAP[i].type;
        }
    }
    return SVG_UNKNOWN;
}

} // namespace

SvgDomBuilder::SvgDomBuilder()
    : dom_(nullptr), depth_(0), rootSet_(false), skipDepth_(0), overflowDepth_(0)
{
    for (uint16_t i = 0; i < SVG_MAX_STACK_DEPTH; i++) {
        stack_[i] = nullptr;
    }
}

SvgDomBuilder::~SvgDomBuilder() {}

SvgResult SvgDomBuilder::Build(const char* xml, SvgDocument& dom)
{
    if (xml == nullptr) {
        return SvgResult::SVG_RESULT_INVALID_PARAM;
    }
    dom_ = &dom;
    depth_ = 0;
    rootSet_ = false;
    skipDepth_ = 0;
    overflowDepth_ = 0;
    SvgXmlParser parser;
    SvgResult result = parser.Parse(xml, this);
    if (depth_ > 0) {
        result = SvgResult::SVG_RESULT_ERROR;
        dom_->ClearIds();
        dom_->SetRoot(nullptr);
        while (depth_ > 0) {
            SvgElementBase* node = PopNode();
            UiDelete(node);
        }
    }
    return result;
}

void SvgDomBuilder::OnStartElement(const char* tag)
{
    if (tag == nullptr) {
        return;
    }
    if (skipDepth_ > 0) {
        skipDepth_++;
        return;
    }
    if (rootSet_ && depth_ == 0) {
        skipDepth_ = 1;
        return;
    }
    if (depth_ >= SVG_MAX_STACK_DEPTH) {
        overflowDepth_++;
        return;
    }
    SvgElementBase* node = CreateNode(tag);
    if (node == nullptr) {
        skipDepth_ = 1;
        return;
    }
    PushNode(node);
}

void SvgDomBuilder::OnEndElement(const char* tag)
{
    (void)tag;
    if (overflowDepth_ > 0) {
        overflowDepth_--;
        return;
    }
    if (skipDepth_ > 0) {
        skipDepth_--;
        return;
    }
    SvgElementBase* node = PopNode();
    if (node == nullptr) {
        return;
    }
    if (depth_ > 0) {
        SvgElementBase* parent = CurrentNode();
        if (parent != nullptr) {
            parent->AppendChild(node);
        } else {
            UiDelete(node);
        }
    } else if (dom_ != nullptr) {
        dom_->SetRoot(node);
        rootSet_ = true;
    } else {
        UiDelete(node);
    }
}

void SvgDomBuilder::OnAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr || skipDepth_ > 0 || overflowDepth_ > 0) {
        return;
    }
    SvgElementBase* node = CurrentNode();
    if (node == nullptr) {
        return;
    }
    if (strcmp(name, "id") == 0) {
        node->SetAttribute(name, value);
        return;
    }
    if (strcmp(name, "style") == 0) {
        ParseStyleAttribute(value);
        return;
    }
    node->SetAttribute(name, value);
}

void SvgDomBuilder::OnText(const char* text)
{
    if (skipDepth_ > 0 || overflowDepth_ > 0) {
        return;
    }
    SvgElementBase* node = CurrentNode();
    if (node == nullptr) {
        return;
    }
    SvgGenericNode* generic = dynamic_cast<SvgGenericNode*>(node);
    if (generic != nullptr) {
        generic->SetTextContent(text);
        return;
    }
    SvgTextNode* textNode = dynamic_cast<SvgTextNode*>(node);
    if (textNode != nullptr) {
        textNode->SetTextContent(text);
        return;
    }
    SvgTSpanNode* tspan = dynamic_cast<SvgTSpanNode*>(node);
    if (tspan != nullptr) {
        tspan->SetTextContent(text);
    }
}

SvgElementBase* SvgDomBuilder::CreateNode(const char* tag) const
{
    return CreateSvgElementByType(MatchSvgElementType(tag));
}

void SvgDomBuilder::PushNode(SvgElementBase* node)
{
    if (node == nullptr || depth_ >= SVG_MAX_STACK_DEPTH) {
        return;
    }
    if (dom_ != nullptr) {
        node->OnDocumentAttached(dom_);
    }
    stack_[depth_++] = node;
}

SvgElementBase* SvgDomBuilder::PopNode()
{
    if (depth_ == 0) {
        return nullptr;
    }
    SvgElementBase* node = stack_[--depth_];
    stack_[depth_] = nullptr;
    return node;
}

SvgElementBase* SvgDomBuilder::CurrentNode() const
{
    if (depth_ == 0) {
        return nullptr;
    }
    return stack_[depth_ - 1];
}

void SvgDomBuilder::ParseStyleAttribute(const char* value)
{
    if (value == nullptr) {
        return;
    }
    const char* p = value;
    char name[MAX_STYLE_NAME_LEN] = { 0 };
    char val[MAX_STYLE_VALUE_LEN] = { 0 };
    while (*p != '\0') {
        if (!ParseStyleName(p, name, sizeof(name))) {
            if (*p == ';') {
                p++;
            }
            continue;
        }
        if (ParseStyleValue(p, val, sizeof(val))) {
            OnAttribute(name, val);
        }
        if (*p == ';') {
            p++;
        }
    }
}

} // namespace OHOS
