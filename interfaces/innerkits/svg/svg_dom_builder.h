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

#ifndef GRAPHIC_LITE_SVG_DOM_BUILDER_H
#define GRAPHIC_LITE_SVG_DOM_BUILDER_H

#include "svg/svg_document.h"
#include "svg/svg_xml_parser.h"

namespace OHOS {

class SvgDomBuilder : public SvgXmlParserListener {
public:
    SvgDomBuilder();

    ~SvgDomBuilder() override;

    SvgResult Build(const char* xml, SvgDocument& dom);

    void OnStartElement(const char* tag) override;

    void OnEndElement(const char* tag) override;

    void OnAttribute(const char* name, const char* value) override;

    void OnText(const char* text) override;

private:
    SvgElementBase* CreateNode(const char* tag) const;

    void PushNode(SvgElementBase* node);

    SvgElementBase* PopNode();

    SvgElementBase* CurrentNode() const;

    void ParseStyleAttribute(const char* value);

    SvgDocument* dom_;
    SvgElementBase* stack_[SVG_MAX_STACK_DEPTH];
    uint16_t depth_;
    bool rootSet_;
    uint16_t skipDepth_;
    uint16_t overflowDepth_;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_DOM_BUILDER_H
