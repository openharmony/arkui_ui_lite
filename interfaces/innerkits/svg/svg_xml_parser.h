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

#ifndef GRAPHIC_LITE_SVG_XML_PARSER_H
#define GRAPHIC_LITE_SVG_XML_PARSER_H

#include "svg/svg_types.h"

namespace OHOS {

class SvgXmlParserListener {
public:
    virtual ~SvgXmlParserListener() = default;

    virtual void OnStartElement(const char* tag) = 0;

    virtual void OnEndElement(const char* tag) = 0;

    virtual void OnAttribute(const char* name, const char* value) = 0;

    virtual void OnText(const char* text) = 0;
};

class SvgXmlParser {
public:
    SvgXmlParser() = default;

    ~SvgXmlParser() = default;

    SvgResult Parse(const char* xml, SvgXmlParserListener* listener);

private:
    SvgXmlParserListener* listener_ = nullptr;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_XML_PARSER_H
