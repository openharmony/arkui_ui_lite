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

#ifndef GRAPHIC_LITE_SVG_SHAPE_NODES_H
#define GRAPHIC_LITE_SVG_SHAPE_NODES_H

#include "gfx_utils/list.h"
#include "svg/svg_leaf_node.h"

namespace OHOS {

class SvgRectNode : public SvgLeafNode {
public:
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
    int16_t rx_ = 0;
    int16_t ry_ = 0;
};

class SvgCircleNode : public SvgLeafNode {
public:
    enum class Mode { CIRCLE, ELLIPSE };
    explicit SvgCircleNode(Mode mode = Mode::CIRCLE);

    Rect GetLocalBounds() const override;

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    TransAffine GetGradientCoordinateTransform() const override;

    SvgElementBase* Clone() const override;

private:
    Mode mode_;
    int16_t cx_ = 0;
    int16_t cy_ = 0;
    int16_t r_ = 0;
    int16_t rx_ = 0;
    int16_t ry_ = 0;
};

class SvgLineNode : public SvgLeafNode {
public:
    enum Mode { LINE, POLYLINE, POLYGON };

    explicit SvgLineNode(Mode mode);

    Rect GetLocalBounds() const override;

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    SvgElementBase* Clone() const override;

private:
    Mode mode_;
    int16_t x1_ = 0;
    int16_t y1_ = 0;
    int16_t x2_ = 0;
    int16_t y2_ = 0;
    List<Point> points_;
};

class SvgPathNode : public SvgLeafNode {
public:
    ~SvgPathNode() override;

    Rect GetLocalBounds() const override;

    const char* GetPathData() const { return d_; }

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    SvgElementBase* Clone() const override;

private:
    char* d_ = nullptr;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_SHAPE_NODES_H
