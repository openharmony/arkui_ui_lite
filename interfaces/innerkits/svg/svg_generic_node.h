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

#ifndef GRAPHIC_LITE_SVG_GENERIC_NODE_H
#define GRAPHIC_LITE_SVG_GENERIC_NODE_H

#include "gfx_utils/list.h"
#include "svg_element_base.h"
#include "svg_types.h"

namespace OHOS {

class SvgGenericNode : public SvgElementBase {
public:
    explicit SvgGenericNode(SvgElementType tag);

    ~SvgGenericNode() override;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_GENERIC;
    }

    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    void SetTextContent(const char* text);

    SvgElementType GetTag() const
    {
        return tag_;
    }

    const char* GetAttribute(const char* name) const;

    void OnDocumentAttached(SvgDocument* doc) override;

private:
    struct AttrEntry {
        char name[SVG_MAX_ATTR_LEN];
        char value[SVG_MAX_ATTR_LEN];
    };

    SvgElementType tag_;
    List<AttrEntry> attrs_;
    List<SvgElementBase*> children_;
    char* textContent_;

    bool FindAndUpdateAttr(const char* name, const char* value);
    void AppendAttr(const char* name, const char* value);
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_GENERIC_NODE_H
