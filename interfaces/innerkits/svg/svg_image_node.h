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

#ifndef GRAPHIC_LITE_SVG_IMAGE_NODE_H
#define GRAPHIC_LITE_SVG_IMAGE_NODE_H

#include "svg/svg_leaf_node.h"

namespace OHOS {

class SvgImageNode : public SvgLeafNode {
public:
    ~SvgImageNode() override;

    Rect GetLocalBounds() const override;

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    SvgElementBase* Clone() const override;

private:
    int16_t x_ = 0;
    int16_t y_ = 0;
    int16_t width_ = 0;
    int16_t height_ = 0;
    char* href_ = nullptr;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_IMAGE_NODE_H
