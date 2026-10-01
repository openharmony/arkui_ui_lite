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

#ifndef GRAPHIC_LITE_SVG_PAINT_SERVERS_H
#define GRAPHIC_LITE_SVG_PAINT_SERVERS_H

#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/list.h"
#include "gfx_utils/rect.h"
#include "svg_element_base.h"

namespace OHOS {

class SvgPaintServerResource : public SvgElementBase {
public:
    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_RESOURCE;
    }

    virtual void ApplyToPaint(Paint& paint,
                              const Rect& localBounds,
                              const TransAffine* localToRecordSpace,
                              bool isFill) const = 0;
};

enum SvgGradientUnits : uint8_t {
    SVG_GRADIENT_OBJECT_BOUNDING_BOX = 0,
    SVG_GRADIENT_USER_SPACE_ON_USE
};

enum SvgSpreadMethod : uint8_t {
    SVG_SPREAD_PAD = 0,
    SVG_SPREAD_REPEAT,
    SVG_SPREAD_REFLECT
};

class SvgStopResource : public SvgElementBase {
public:
    ~SvgStopResource() override;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_RESOURCE;
    }

    bool SetAttribute(const char* name, const char* value) override;

    SvgElementBase* Clone() const override;

    // <stop> does not accept child elements; nested content is ignored.
    void AppendChild(SvgElementBase* child) override;

    float GetOffset() const
    {
        return offset_;
    }

    ColorType GetColor() const
    {
        return color_;
    }

    uint8_t GetOpacity() const
    {
        return opacity_;
    }

private:
    float offset_ = 0.0f;
    ColorType color_ = {};
    uint8_t opacity_ = OPA_OPAQUE;
};

class SvgGradientResourceBase : public SvgPaintServerResource {
public:
    ~SvgGradientResourceBase() override;

    void AppendChild(SvgElementBase* child) override;

    void OnDocumentAttached(SvgDocument* doc) override;

protected:
    SvgGradientResourceBase() = default;

    void AddStopsToPaint(Paint& paint) const;

    void AddSpreadStops(Paint& paint) const;

    void CollectStops(float* offsets, ColorType* colors, uint8_t& count) const;

    void CloneChildren(SvgGradientResourceBase* dest) const;

    SvgSpreadMethod spread_ = SVG_SPREAD_PAD;
    List<SvgElementBase*> children_;
};

class SvgLinearGradientResource : public SvgGradientResourceBase {
public:
    ~SvgLinearGradientResource() override;

    bool SetAttribute(const char* name, const char* value) override;

    SvgElementBase* Clone() const override;

    void ApplyToPaint(Paint& paint,
                      const Rect& localBounds,
                      const TransAffine* localToRecordSpace,
                      bool isFill) const override;

private:
    float x1_ = 0.0f;
    float y1_ = 0.0f;
    float x2_ = 1.0f;
    float y2_ = 0.0f;
    SvgGradientUnits units_ = SVG_GRADIENT_OBJECT_BOUNDING_BOX;
    TransAffine gradientTransform_;
    bool hasGradientTransform_ = false;

    bool ParseCoordAttribute(const char* name, const char* value);

    void ResolveCoordinates(const Rect& bounds, float& x1, float& y1,
                            float& x2, float& y2) const;

    void ApplyTransform(float& x1, float& y1, float& x2, float& y2) const;
};

class SvgRadialGradientResource : public SvgGradientResourceBase {
public:
    ~SvgRadialGradientResource() override;

    bool SetAttribute(const char* name, const char* value) override;

    SvgElementBase* Clone() const override;

    void ApplyToPaint(Paint& paint,
                      const Rect& localBounds,
                      const TransAffine* localToRecordSpace,
                      bool isFill) const override;

private:
    float cx_ = 0.5f;
    float cy_ = 0.5f;
    float r_ = 0.5f;
    float fx_ = 0.0f;
    float fy_ = 0.0f;
    bool hasFx_ = false;
    bool hasFy_ = false;
    SvgGradientUnits units_ = SVG_GRADIENT_OBJECT_BOUNDING_BOX;
    TransAffine gradientTransform_;
    bool hasGradientTransform_ = false;

    bool ParseCoordAttribute(const char* name, const char* value);

    void ResolveCoordinates(const Rect& bounds, float& cx, float& cy, float& r,
                            float& fx, float& fy) const;

    void ApplyTransform(float& cx, float& cy, float& r, float& fx, float& fy) const;
};

class SvgSolidColorResource : public SvgPaintServerResource {
public:
    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    SvgElementBase* Clone() const override;

    void ApplyToPaint(Paint& paint,
                      const Rect& localBounds,
                      const TransAffine* localToRecordSpace,
                      bool isFill) const override;

private:
    ColorType color_ = {};
    uint8_t opacity_ = OPA_OPAQUE;
};

class SvgDefsResource : public SvgElementBase {
public:
    SvgDefsResource() = default;
    ~SvgDefsResource() override;

    SvgDefsResource(const SvgDefsResource&) = delete;
    SvgDefsResource& operator=(const SvgDefsResource&) = delete;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_GENERIC;
    }

    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    void OnDocumentAttached(SvgDocument* doc) override;

    SvgElementBase* Clone() const override;

private:
    List<SvgElementBase*> children_;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_PAINT_SERVERS_H
