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

#ifndef GRAPHIC_LITE_SVG_TEXT_NODES_H
#define GRAPHIC_LITE_SVG_TEXT_NODES_H

#include "svg/svg_element_base.h"
#include "svg/svg_leaf_node.h"
#include "svg/svg_paint_state.h"

namespace OHOS {

constexpr uint8_t SVG_DEFAULT_FONT_SIZE = 16;

struct TextCursor {
    float x;
    float y;
};

class SvgTSpanNode : public SvgElementBase {
public:
    ~SvgTSpanNode() override;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_GENERIC;
    }

    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    bool SetTextContent(const char* text);

    SvgElementBase* Clone() const override;

    const char* GetTextContent() const
    {
        return textContent_;
    }

    int16_t GetX() const
    {
        return x_;
    }

    int16_t GetY() const
    {
        return y_;
    }

    int16_t GetDx() const
    {
        return dx_;
    }

    int16_t GetDy() const
    {
        return dy_;
    }

    uint8_t GetFontSize() const
    {
        return fontSize_;
    }

    const SvgPaintState& GetPaintState() const
    {
        return paintState_;
    }

    bool HasX() const
    {
        return hasX_;
    }

    bool HasY() const
    {
        return hasY_;
    }

private:
    int16_t x_ = 0;
    int16_t y_ = 0;
    int16_t dx_ = 0;
    int16_t dy_ = 0;
    bool hasX_ = false;
    bool hasY_ = false;
    uint8_t fontSize_ = 0;
    char* textContent_ = nullptr;
    SvgPaintState paintState_;
};

class SvgTextNode : public SvgLeafNode {
public:
    ~SvgTextNode() override;

    bool SetAttribute(const char* name, const char* value) override;

    bool SetTextContent(const char* text);

    const char* GetTextContent() const
    {
        return textContent_;
    }

    Rect GetLocalBounds() const override;

    void AppendChild(SvgElementBase* child) override;

    SvgElementBase* Clone() const override;

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    int16_t x_ = 0;
    int16_t y_ = 0;
    int16_t dx_ = 0;
    int16_t dy_ = 0;
    uint8_t fontSize_ = SVG_DEFAULT_FONT_SIZE;
    char* textContent_ = nullptr;

private:
    void DrawTSpanChild(UICanvas& canvas, const TransAffine& record, float scaleY,
                        TextCursor& cursor, SvgTSpanNode* tspan);
    Rect MeasureTSpanBounds(const SvgTSpanNode* tspan, uint8_t size,
                            int16_t& cursorX, int16_t& cursorY) const;
};

class SvgTextAreaNode : public SvgTextNode {
public:
    Rect GetLocalBounds() const override;

    SvgElementBase* Clone() const override;

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override;

    void RecordGeometry(UICanvas& canvas) override;

    int16_t width_ = 0;
    int16_t height_ = 0;

private:
    void DrawTextAreaChild(UICanvas& canvas, const TransAffine& record, float scaleY,
                           TextCursor& cursor, uint16_t wrapWidth,
                           SvgElementBase* child);
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_TEXT_NODES_H
