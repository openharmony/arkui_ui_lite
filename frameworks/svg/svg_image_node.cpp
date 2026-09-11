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

#include "svg/svg_image_node.h"
#include "svg/svg_attribute_parser.h"
#include <cmath>
#include <cstring>

namespace OHOS {

using SvgAttributeParser::MATRIX_INDEX_A;
using SvgAttributeParser::MATRIX_INDEX_D;

namespace {

constexpr float PI_F = 3.14159265358979323846f;
constexpr float IMG_RAD_TO_DEG = 180.0f / PI_F;
constexpr float IMG_ROTATION_EPSILON_DEG = 0.01f;

char* CopyString(const char* src)
{
    if (src == nullptr) {
        return nullptr;
    }
    uint32_t len = strlen(src) + 1;
    char* dst = new char[len];
    if (dst == nullptr || memcpy_s(dst, len, src, len) != EOK) {
        delete[] dst;
        return nullptr;
    }
    return dst;
}

// Extract the rotation angle (degrees) carried by an accumulated transform.
// See GetRecordRotationDeg() in svg_text_nodes.cpp for the derivation; the
// affine is [sx shx tx; shy sy ty] and atan2(shy, sx) recovers the SVG
// y-down / clockwise-positive rotation angle.
float GetRecordRotationDeg(const TransAffine& record)
{
    const float* m = record.GetData();
    return std::atan2(m[MATRIX_INDEX_D], m[MATRIX_INDEX_A]) * IMG_RAD_TO_DEG;
}

} // namespace

SvgImageNode::~SvgImageNode()
{
    delete[] href_;
    href_ = nullptr;
}

Rect SvgImageNode::GetLocalBounds() const
{
    if (width_ <= 0 || height_ <= 0) {
        // Return an empty rectangle whose inclusive-boundary width/height are 0.
        return {x_, y_, static_cast<int16_t>(x_ - 1), static_cast<int16_t>(y_ - 1)};
    }
    int16_t right = x_ + width_ - 1;
    int16_t bottom = y_ + height_ - 1;
    return {x_, y_, right, bottom};
}

bool SvgImageNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "x") == 0) {
        x_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "y") == 0) {
        y_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "width") == 0) {
        width_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "height") == 0) {
        height_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "xlink:href") == 0 || strcmp(name, "href") == 0) {
        delete[] href_;
        href_ = CopyString(value);
    } else {
        return false;
    }
    return true;
}

void SvgImageNode::RecordGeometry(UICanvas& canvas)
{
#if defined(GRAPHIC_ENABLE_DRAW_IMAGE_FLAG) && GRAPHIC_ENABLE_DRAW_IMAGE_FLAG
    if (href_ == nullptr || width_ <= 0 || height_ <= 0) {
        return;
    }
    TransAffine record = GetRecordTransform();
    float px = static_cast<float>(x_);
    float py = static_cast<float>(y_);
    record.Transform(&px, &py);
    Point start = {SvgAttributeParser::ClampToInt16(px), SvgAttributeParser::ClampToInt16(py)};
    // The CTM is baked into vector geometry, but a rasterised <image> only rotates
    // in UICanvas::DoDrawImage when its paint carries a non-identity transform. Replay
    // the CTM's rotation onto a paint copy so the image rotates together with its
    // (already-transformed) anchor. The rotation angle is recovered from the CTM as
    // atan2(shy, sx); feeding that same angle to Paint::Rotate makes the <image> glyph
    // follow the group's orbit (confirmed correct against a reference browser).
    Paint drawPaint = inheritedPaint_;
    float angle = GetRecordRotationDeg(record);
    if (std::fabs(angle) > IMG_ROTATION_EPSILON_DEG) {
        drawPaint.Rotate(angle);
    }
    canvas.DrawImage(start, href_, drawPaint, width_, height_);
#else
    (void)canvas;
#endif
}

SvgElementBase* SvgImageNode::Clone() const
{
    SvgImageNode* clone = new SvgImageNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x_ = x_;
    clone->y_ = y_;
    clone->width_ = width_;
    clone->height_ = height_;
    clone->href_ = CopyString(href_);
    clone->CopyStateFrom(*this);
    return clone;
}

} // namespace OHOS
