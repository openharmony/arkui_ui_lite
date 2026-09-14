/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_UI_ABSTRACT_SCROLL_BAR_H
#define GRAPHIC_LITE_UI_ABSTRACT_SCROLL_BAR_H

#include "gfx_utils/rect.h"
#include "gfx_utils/style.h"
#include "gfx_utils/graphic_buffer.h"

namespace OHOS {
class UIAbstractScrollBar : public HeapBase {
public:
    UIAbstractScrollBar();

#if GRAPHIC_ENABLE_SCROLL_FLAG
    UIAbstractScrollBar(const UIAbstractScrollBar&) = delete;
    UIAbstractScrollBar& operator=(const UIAbstractScrollBar&) = delete;

    virtual ~UIAbstractScrollBar();
#else
    virtual ~UIAbstractScrollBar() {}
#endif

#if GRAPHIC_ENABLE_SCROLL_FLAG
    // default scroll bar width in pixels, also used as the fallback of SetIndicatorWidth(0)
    static constexpr uint16_t DEFAULT_SCROLL_BAR_WIDTH = 4;
    // default minimum slider length in pixels, also used as the fallback of SetIndicatorMinLength(0)
    static constexpr uint16_t DEFAULT_SCROLL_BAR_MIN_LEN = 10;
#endif

    virtual void SetPosition(int16_t x, int16_t y, int16_t width, int16_t height) {}

    virtual void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, uint8_t backgroundOpa) {}

    virtual void SetScrollBarSide(uint8_t side) {}

#if GRAPHIC_ENABLE_SCROLL_FLAG
    void ResetIndicatorStyle();

    void SetIndicatorWidth(uint16_t width);

    uint16_t GetIndicatorWidth() const
    {
        return width_;
    }

    void SetIndicatorColor(ColorType color);

    ColorType GetIndicatorColor() const
    {
        return foregroundStyle_->bgColor_;
    }

    void SetIndicatorBorderRadius(uint16_t borderRadius);

    uint16_t GetIndicatorBorderRadius() const
    {
        return foregroundStyle_->borderRadius_;
    }

    void SetIndicatorMinLength(uint16_t minLength);

    uint16_t GetIndicatorMinLength() const
    {
        return minLength_;
    }

    void SetIndicatorOpacity(uint8_t opacity);

    uint8_t GetIndicatorOpacity() const
    {
        return indicatorOpacity_;
    }
#endif

    void SetScrollProgress(float scrollProgress)
    {
        scrollProgress_ = scrollProgress;
    }

    void SetForegroundProportion(float foregroundPropotion)
    {
        if ((foregroundPropotion < 0) || (foregroundPropotion > 1.0f)) {
            return;
        }
        foregroundProportion_ = foregroundPropotion;
    }

    void SetOpacity(uint8_t opacity)
    {
        opacity_ = opacity;
    }

    uint8_t GetOpacity()
    {
        return opacity_;
    }

protected:
#if GRAPHIC_ENABLE_SCROLL_FLAG
    // track background opacity: only combines the view opacity and the fade-in/out animation
    // opacity, not affected by the slider opacity
    uint8_t GetBackgroundOpacity(uint8_t backgroundOpa) const;

    // slider opacity: combines the slider opacity on top of the track background opacity
    uint8_t GetSliderOpacity(uint8_t backgroundOpa) const;

    bool EnsureBackgroundStyle();

    bool EnsureForegroundStyle();

    void ReleaseCustomStyles();
#endif
    uint8_t opacity_ = OPA_OPAQUE;
#if GRAPHIC_ENABLE_SCROLL_FLAG
    uint8_t indicatorOpacity_ = OPA_OPAQUE;
    uint16_t width_ = DEFAULT_SCROLL_BAR_WIDTH;
    uint16_t minLength_ = DEFAULT_SCROLL_BAR_MIN_LEN;
#endif
    float scrollProgress_ = 0;
    float foregroundProportion_ = 0;
    Style* backgroundStyle_ = nullptr;
    Style* foregroundStyle_ = nullptr;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_UI_ABSTRACT_SCROLL_BAR_H
