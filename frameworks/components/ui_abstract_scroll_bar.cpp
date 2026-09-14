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

#include "components/ui_abstract_scroll_bar.h"

#include <cstdint>
#include <new>

#include "gfx_utils/graphic_log.h"

namespace OHOS {
UIAbstractScrollBar::UIAbstractScrollBar()
{
#if GRAPHIC_ENABLE_SCROLL_FLAG
    ResetIndicatorStyle();
#else
    backgroundStyle_ = &(StyleDefault::GetScrollBarBackgroundStyle());
    foregroundStyle_ = &(StyleDefault::GetScrollBarForegroundStyle());
#endif
}

#if GRAPHIC_ENABLE_SCROLL_FLAG
UIAbstractScrollBar::~UIAbstractScrollBar()
{
    ReleaseCustomStyles();
}

void UIAbstractScrollBar::ReleaseCustomStyles()
{
    Style* defaultBackgroundStyle = &(StyleDefault::GetScrollBarBackgroundStyle());
    Style* defaultForegroundStyle = &(StyleDefault::GetScrollBarForegroundStyle());
    if ((backgroundStyle_ != nullptr) && (backgroundStyle_ != defaultBackgroundStyle)) {
        delete backgroundStyle_;
    }
    if ((foregroundStyle_ != nullptr) && (foregroundStyle_ != defaultForegroundStyle)) {
        delete foregroundStyle_;
    }
    backgroundStyle_ = nullptr;
    foregroundStyle_ = nullptr;
}

void UIAbstractScrollBar::ResetIndicatorStyle()
{
    Style* defaultBackgroundStyle = &(StyleDefault::GetScrollBarBackgroundStyle());
    Style* defaultForegroundStyle = &(StyleDefault::GetScrollBarForegroundStyle());
    ReleaseCustomStyles();
    backgroundStyle_ = defaultBackgroundStyle;
    foregroundStyle_ = defaultForegroundStyle;
    indicatorOpacity_ = OPA_OPAQUE;
    width_ = DEFAULT_SCROLL_BAR_WIDTH;
    minLength_ = DEFAULT_SCROLL_BAR_MIN_LEN;
}

bool UIAbstractScrollBar::EnsureBackgroundStyle()
{
    Style* defaultStyle = &(StyleDefault::GetScrollBarBackgroundStyle());
    if (backgroundStyle_ != defaultStyle) {
        return (backgroundStyle_ != nullptr);
    }
    Style* style = new (std::nothrow) Style(*backgroundStyle_);
    if (style == nullptr) {
        GRAPHIC_LOGE("UIAbstractScrollBar::EnsureBackgroundStyle new Style failed");
        return false;
    }
    backgroundStyle_ = style;
    return true;
}

bool UIAbstractScrollBar::EnsureForegroundStyle()
{
    Style* defaultStyle = &(StyleDefault::GetScrollBarForegroundStyle());
    if (foregroundStyle_ != defaultStyle) {
        return (foregroundStyle_ != nullptr);
    }
    Style* style = new (std::nothrow) Style(*foregroundStyle_);
    if (style == nullptr) {
        GRAPHIC_LOGE("UIAbstractScrollBar::EnsureForegroundStyle new Style failed");
        return false;
    }
    foregroundStyle_ = style;
    return true;
}

void UIAbstractScrollBar::SetIndicatorWidth(uint16_t width)
{
    if (!EnsureBackgroundStyle() || !EnsureForegroundStyle()) {
        return;
    }
    // width_ is later converted to int16_t, reset to default to avoid truncation
    if ((width == 0) || (width > INT16_MAX)) {
        width_ = DEFAULT_SCROLL_BAR_WIDTH;
    } else {
        width_ = width;
    }
    backgroundStyle_->lineWidth_ = width_;
    foregroundStyle_->lineWidth_ = width_;
}

void UIAbstractScrollBar::SetIndicatorColor(ColorType color)
{
    if (!EnsureForegroundStyle()) {
        return;
    }
    foregroundStyle_->bgColor_ = color;
    foregroundStyle_->lineColor_ = color;
}

void UIAbstractScrollBar::SetIndicatorBorderRadius(uint16_t borderRadius)
{
    if (!EnsureForegroundStyle()) {
        return;
    }
    foregroundStyle_->borderRadius_ = borderRadius;
    foregroundStyle_->lineCap_ = (borderRadius == 0) ? CapType::CAP_NONE : CapType::CAP_ROUND;
}

void UIAbstractScrollBar::SetIndicatorMinLength(uint16_t minLength)
{
    // minLength_ is later compared with and assigned to int16_t, reset to default to avoid truncation
    if ((minLength == 0) || (minLength > INT16_MAX)) {
        minLength_ = DEFAULT_SCROLL_BAR_MIN_LEN;
    } else {
        minLength_ = minLength;
    }
}

void UIAbstractScrollBar::SetIndicatorOpacity(uint8_t opacity)
{
    if (!EnsureForegroundStyle()) {
        return;
    }
    indicatorOpacity_ = opacity;
    // reset the style's own bg opacity, so indicatorOpacity_ is the only factor of the slider opacity
    foregroundStyle_->bgOpa_ = OPA_OPAQUE;
}

uint8_t UIAbstractScrollBar::GetBackgroundOpacity(uint8_t backgroundOpa) const
{
    // 8: Shift right 8 bits
    return (backgroundOpa == OPA_OPAQUE) ? opacity_ : (static_cast<uint16_t>(backgroundOpa) * opacity_) >> 8;
}

uint8_t UIAbstractScrollBar::GetSliderOpacity(uint8_t backgroundOpa) const
{
    uint8_t barOpa = GetBackgroundOpacity(backgroundOpa);
    if (barOpa == OPA_OPAQUE) {
        return indicatorOpacity_;
    }
    if (indicatorOpacity_ == OPA_OPAQUE) {
        return barOpa;
    }
    // 8: Shift right 8 bits
    return (static_cast<uint16_t>(barOpa) * indicatorOpacity_) >> 8;
}
#endif // GRAPHIC_ENABLE_SCROLL_FLAG
} // namespace OHOS
