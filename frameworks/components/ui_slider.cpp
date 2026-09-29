/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "components/ui_slider.h"

#if GRAPHIC_ENABLE_SLIDER_FLAG
#include <cstdio>
#include <cstring>
#endif

#include "common/image.h"
#if GRAPHIC_ENABLE_SLIDER_FLAG
#include "components/ui_label.h"
#include "draw/draw_label.h"
#include "font/ui_font.h"
#endif
#include "dock/focus_manager.h"
#include "dock/vibrator_manager.h"
#include "draw/draw_image.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "gfx_utils/graphic_log.h"
#include "imgdecode/cache_manager.h"
#if GRAPHIC_ENABLE_SLIDER_FLAG
#include "securec.h"
#endif
#include "themes/theme_manager.h"

namespace OHOS {
#if GRAPHIC_ENABLE_SLIDER_FLAG
namespace {
constexpr uint8_t DEFAULT_TICK_LINE_WIDTH = 1;
constexpr uint8_t DEFAULT_TOAST_OPA = 204;
constexpr int16_t TOAST_PADDING = 4;
constexpr int16_t TOAST_OFFSET = 2;
constexpr int16_t MARK_TEXT_OFFSET = 2;
constexpr uint8_t COLOR_RED_SHIFT = 16;
constexpr uint8_t COLOR_GREEN_SHIFT = 8;
constexpr uint16_t COLOR_MIX_MAX = 255;
constexpr uint32_t COLOR_CHANNEL_MASK = 0xFF;
constexpr int16_t HALF_DIVISOR = 2;

bool IsHorizontal(UISlider::Direction direction)
{
    return ((direction == UISlider::Direction::DIR_LEFT_TO_RIGHT) ||
            (direction == UISlider::Direction::DIR_RIGHT_TO_LEFT));
}

bool IsReverse(UISlider::Direction direction)
{
    return ((direction == UISlider::Direction::DIR_RIGHT_TO_LEFT) ||
            (direction == UISlider::Direction::DIR_BOTTOM_TO_TOP));
}

int16_t ClampInt16(int32_t value, int16_t min, int16_t max)
{
    if (max < min) {
        return min;
    }
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return static_cast<int16_t>(value);
}

void SortValues(int32_t* values, uint16_t count)
{
    if (values == nullptr) {
        return;
    }
    for (uint16_t i = 1; i < count; i++) {
        int32_t value = values[i];
        uint16_t j = i;
        while ((j > 0) && (values[j - 1] > value)) {
            values[j] = values[j - 1];
            j--;
        }
        values[j] = value;
    }
}

size_t GetBoundedStringLength(const char* str, size_t maxLen)
{
    if (str == nullptr) {
        return 0;
    }
    size_t len = 0;
    while ((len < maxLen) && (str[len] != '\0')) {
        len++;
    }
    return len;
}
} // namespace
#endif

UISlider::UISlider() : knobWidth_(0), knobStyleAllocFlag_(false), knobImage_(nullptr), listener_(nullptr)
{
#if GRAPHIC_ENABLE_SLIDER_FLAG
    disabled_ = false;
    showDisabledToast_ = false;
    enableToast_ = false;
    toastVisible_ = false;
    toastAutoHide_ = false;
    needEraseToast_ = false;
    enableTicks_ = false;
    showMarkings_ = false;
    showMarkText_ = false;
    expandClickArea_ = false;
#endif
    touchable_ = true;
    draggable_ = true;
    dragParentInstead_ = false;
#if ENABLE_FOCUS_MANAGER
    focusable_ = true;
#endif
#if ENABLE_ROTATE_INPUT
    rotateFactor_ = DEFAULT_SLIDER_ROTATE_FACTOR;
    cachedRotation_ = 0;
#endif

    Theme* theme = ThemeManager::GetInstance().GetCurrent();
    if (theme != nullptr) {
        knobStyle_ = &(theme->GetSliderKnobStyle());
    } else {
        knobStyle_ = &(StyleDefault::GetSliderKnobStyle());
    }
}

UISlider::~UISlider()
{
    if (knobImage_ != nullptr) {
        delete knobImage_;
        knobImage_ = nullptr;
    }

    if (knobStyleAllocFlag_) {
        delete knobStyle_;
        knobStyle_ = nullptr;
        knobStyleAllocFlag_ = false;
    }
}

void UISlider::SetKnobStyle(const Style& style)
{
    if (!knobStyleAllocFlag_) {
        knobStyle_ = new Style;
        if (knobStyle_ == nullptr) {
            GRAPHIC_LOGE("new Style fail");
            return;
        }
        knobStyleAllocFlag_ = true;
    }
    *knobStyle_ = style;
}

void UISlider::SetKnobStyle(uint8_t key, int64_t value)
{
    if (!knobStyleAllocFlag_) {
        knobStyle_ = new Style(*knobStyle_);
        if (knobStyle_ == nullptr) {
            GRAPHIC_LOGE("new Style fail");
            return;
        }
        knobStyleAllocFlag_ = true;
    }
    knobStyle_->SetStyle(key, value);
}

const Style& UISlider::GetKnobStyle() const
{
    return *knobStyle_;
}

int64_t UISlider::GetKnobStyle(uint8_t key) const
{
    return knobStyle_->GetStyle(key);
}

void UISlider::SetKnobImage(const ImageInfo* knobImage)
{
    if (!InitImage()) {
        return;
    }
    knobImage_->SetSrc(knobImage);
}

void UISlider::SetKnobImage(const char* knobImage)
{
    if (!InitImage()) {
        return;
    }
    knobImage_->SetSrc(knobImage);
}

void UISlider::DrawKnob(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Rect& foregroundRect)
{
    int16_t halfKnobWidth = GetKnobWidth() / 2; // 2: half
    int16_t offset;
    Rect knobBar;
    switch (direction_) {
        case Direction::DIR_LEFT_TO_RIGHT: {
            offset = (knobWidth_ - progressHeight_) / 2; // 2: half
            knobBar.SetRect(foregroundRect.GetRight() - halfKnobWidth, foregroundRect.GetTop() - offset,
                            foregroundRect.GetRight() + halfKnobWidth, foregroundRect.GetBottom() + offset);
            break;
        }
        case Direction::DIR_RIGHT_TO_LEFT: {
            offset = (knobWidth_ - progressHeight_) / 2; // 2: half
            knobBar.SetRect(foregroundRect.GetLeft() - halfKnobWidth, foregroundRect.GetTop() - offset,
                            foregroundRect.GetLeft() + halfKnobWidth, foregroundRect.GetBottom() + offset);
            break;
        }
        case Direction::DIR_BOTTOM_TO_TOP: {
            offset = (knobWidth_ - progressWidth_) / 2; // 2: half
            knobBar.SetRect(foregroundRect.GetLeft() - offset, foregroundRect.GetTop() - halfKnobWidth,
                            foregroundRect.GetRight() + offset, foregroundRect.GetTop() + halfKnobWidth);
            break;
        }
        case Direction::DIR_TOP_TO_BOTTOM: {
            offset = (knobWidth_ - progressWidth_) / 2; // 2: half
            knobBar.SetRect(foregroundRect.GetLeft() - offset, foregroundRect.GetBottom() - halfKnobWidth,
                            foregroundRect.GetRight() + offset, foregroundRect.GetBottom() + halfKnobWidth);
            break;
        }
        default: {
            GRAPHIC_LOGW("UISlider::DrawKnob Direction error!\n");
        }
    }
    DrawValidRect(gfxDstBuffer, knobImage_, knobBar, invalidatedArea, *knobStyle_, 0);
}

bool UISlider::InitImage()
{
    if (!UIAbstractProgress::InitImage()) {
        return false;
    }
    if (knobImage_ == nullptr) {
        knobImage_ = new Image();
        if (knobImage_ == nullptr) {
            GRAPHIC_LOGE("new Image fail");
            return false;
        }
    }
    return true;
}

void UISlider::SetImage(const ImageInfo* backgroundImage, const ImageInfo* foregroundImage)
{
    if (!InitImage()) {
        return;
    }
    backgroundImage_->SetSrc(backgroundImage);
    foregroundImage_->SetSrc(foregroundImage);
}

void UISlider::SetImage(const char* backgroundImage, const char* foregroundImage)
{
    if (!InitImage()) {
        return;
    }
    backgroundImage_->SetSrc(backgroundImage);
    foregroundImage_->SetSrc(foregroundImage);
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
void UISlider::SetDisabled(bool disabled)
{
    disabled_ = disabled;
    if (!disabled_) {
        showDisabledToast_ = false;
    }
}

void UISlider::SetDisabledToastMsg(const char* msg)
{
    if ((msg == nullptr) || (msg[0] == '\0')) {
        disabledToastMsg_[0] = '\0';
        showDisabledToast_ = false;
        return;
    }

    size_t len = GetBoundedStringLength(msg, MAX_DISABLED_TOAST_MSG_LEN);
    if (memcpy_s(disabledToastMsg_, sizeof(disabledToastMsg_), msg, len) != EOK) {
        disabledToastMsg_[0] = '\0';
        GRAPHIC_LOGE("UISlider::SetDisabledToastMsg memcpy_s failed");
        return;
    }
    disabledToastMsg_[len] = '\0';
}

void UISlider::SetValues(const int32_t* values, uint16_t count)
{
    valuesCount_ = 0;
    if ((values == nullptr) || (count == 0)) {
        return;
    }

    if (count > MAX_MARK_VALUE_COUNT) {
        GRAPHIC_LOGW("UISlider::SetValues values count %d exceeds max %d, truncate\n", static_cast<int32_t>(count),
                     static_cast<int32_t>(MAX_MARK_VALUE_COUNT));
    }
    uint16_t copyCount = (count > MAX_MARK_VALUE_COUNT) ? MAX_MARK_VALUE_COUNT : count;
    for (uint16_t i = 0; i < copyCount; i++) {
        values_[i] = values[i];
    }
    SortValues(values_, copyCount);

    for (uint16_t i = 0; i < copyCount; i++) {
        if ((valuesCount_ == 0) || (values_[i] != values_[valuesCount_ - 1])) {
            values_[valuesCount_++] = values_[i];
        }
    }

    if (valuesCount_ < copyCount) {
        GRAPHIC_LOGW("UISlider::SetValues duplicate values removed, %d -> %d\n",
                     static_cast<int32_t>(copyCount), static_cast<int32_t>(valuesCount_));
    }
}

void UISlider::SetBgGradientColors(const ColorType* colors, uint16_t count)
{
    bgGradientColorsCount_ = 0;
    if ((colors == nullptr) || (count == 0)) {
        return;
    }
    uint16_t copyCount = (count > MAX_GRADIENT_COLOR_COUNT) ? MAX_GRADIENT_COLOR_COUNT : count;
    for (uint16_t i = 0; i < copyCount; i++) {
        bgGradientColors_[i] = colors[i];
    }
    bgGradientColorsCount_ = copyCount;
}

void UISlider::SetOnTintGradientColors(const ColorType* colors, uint16_t count)
{
    onTintGradientColorsCount_ = 0;
    if ((colors == nullptr) || (count == 0)) {
        return;
    }
    uint16_t copyCount = (count > MAX_GRADIENT_COLOR_COUNT) ? MAX_GRADIENT_COLOR_COUNT : count;
    for (uint16_t i = 0; i < copyCount; i++) {
        onTintGradientColors_[i] = colors[i];
    }
    onTintGradientColorsCount_ = copyCount;
}

void UISlider::SetShowMarkings(bool enable)
{
    showMarkings_ = enable;
}

void UISlider::SetMarkingsSize(int16_t size)
{
    if ((size <= 0) || (size > MAX_MARKINGS_SIZE)) {
        GRAPHIC_LOGW("UISlider::SetMarkingsSize invalid markings size: %d, restore default\n", size);
        markingsSize_ = DEFAULT_MARKINGS_SIZE;
        return;
    }
    markingsSize_ = size;
}

void UISlider::SetShowMarkText(bool enable)
{
    showMarkText_ = enable;
}

void UISlider::SetExpandClickArea(bool enable)
{
    expandClickArea_ = enable;
}

void UISlider::EnableToast(bool enable)
{
    enableToast_ = enable;
    if (!enableToast_) {
        toastVisible_ = false;
        toastAutoHide_ = false;
    }
}

void UISlider::EnableTicks(bool enable)
{
    enableTicks_ = enable;
}

void UISlider::EnableMarkings(bool enable)
{
    SetShowMarkings(enable);
}

void UISlider::EnableMarkText(bool enable)
{
    SetShowMarkText(enable);
}

void UISlider::SetMarkingsType(MarkingsType type)
{
    if ((type == MARKINGS_LINE) || (type == MARKINGS_DOT)) {
        markingsType_ = type;
        return;
    }
    GRAPHIC_LOGW("UISlider::SetMarkingsType invalid markings type: %d, restore default\n",
                 static_cast<int32_t>(type));
    markingsType_ = DEFAULT_MARKINGS_TYPE;
}

void UISlider::DrawBackground(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    if (bgGradientColorsCount_ >= HALF_DIVISOR) {
        Point startPoint;
        int16_t progressWidth;
        int16_t progressHeight;
        uint16_t radius;
        GetBackgroundParam(startPoint, progressWidth, progressHeight, radius, *backgroundStyle_);
        static_cast<void>(radius);
        Rect rect(startPoint.x, startPoint.y, startPoint.x + progressWidth - 1, startPoint.y + progressHeight - 1);
        DrawGradientRect(gfxDstBuffer, invalidatedArea, rect, *backgroundStyle_, bgGradientColors_,
            bgGradientColorsCount_);
        return;
    }
    UIBoxProgress::DrawBackground(gfxDstBuffer, invalidatedArea);
}
#endif

void UISlider::DrawForeground(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, Rect& coords)
{
    Point startPoint;
    int16_t progressWidth;
    int16_t progressHeight;
    uint16_t radius;
    GetBackgroundParam(startPoint, progressWidth, progressHeight, radius, *foregroundStyle_);

#if GRAPHIC_ENABLE_SLIDER_FLAG
    Rect foregroundRect;
    if (!CalculateForegroundCoords(startPoint, progressWidth, progressHeight, radius, coords, foregroundRect)) {
        return;
    }
    if (!coords.Intersect(coords, invalidatedArea)) {
        return;
    }
    if (onTintGradientColorsCount_ >= HALF_DIVISOR) {
        DrawGradientRect(gfxDstBuffer, invalidatedArea, coords, *foregroundStyle_, onTintGradientColors_,
            onTintGradientColorsCount_);
        return;
    }
    DrawValidRect(gfxDstBuffer, foregroundImage_, foregroundRect, coords, *foregroundStyle_, radius);
#else
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    int16_t length;
    Rect foregroundRect;

    switch (direction_) {
        case Direction::DIR_LEFT_TO_RIGHT: {
            length = GetCurrentPos(progressWidth_ + 1);
            foregroundRect.SetRect(startPoint.x, startPoint.y, startPoint.x + progressWidth - 1,
                                   startPoint.y + progressHeight_ - 1);

            left = startPoint.x - radius - 1;
            right = left + length;
            coords.SetRect(left, startPoint.y, right, startPoint.y + progressHeight_ - 1);
            break;
        }
        case Direction::DIR_RIGHT_TO_LEFT: {
            length = GetCurrentPos(progressWidth_ + 1);
            foregroundRect.SetRect(startPoint.x, startPoint.y, startPoint.x + progressWidth - 1,
                                   startPoint.y + progressHeight_ - 1);

            right = startPoint.x + progressWidth + radius + 1;
            left = right - length;
            coords.SetRect(left, startPoint.y, right, startPoint.y + progressHeight_ - 1);
            break;
        }
        case Direction::DIR_TOP_TO_BOTTOM: {
            length = GetCurrentPos(progressHeight_ + 1);
            foregroundRect.SetRect(startPoint.x, startPoint.y, startPoint.x + progressWidth_ - 1,
                                   startPoint.y + progressHeight - 1);

            top = startPoint.y - radius - 1;
            bottom = top + length;
            coords.SetRect(startPoint.x, top, startPoint.x + progressWidth_ - 1, bottom);
            break;
        }
        case Direction::DIR_BOTTOM_TO_TOP: {
            length = GetCurrentPos(progressHeight_ + 1);
            foregroundRect.SetRect(startPoint.x, startPoint.y, startPoint.x + progressWidth_ - 1,
                                   startPoint.y + progressHeight - 1);

            bottom = startPoint.y + progressHeight + radius + 1;
            top = bottom - length;
            coords.SetRect(startPoint.x, top, startPoint.x + progressWidth_ - 1, bottom);
            break;
        }
        default: {
            GRAPHIC_LOGE("UISlider: DrawForeground direction Err!\n");
            return;
        }
    }
    if (coords.Intersect(coords, invalidatedArea)) {
        DrawValidRect(gfxDstBuffer, foregroundImage_, foregroundRect, coords, *foregroundStyle_, radius);
    }
#endif
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
bool UISlider::CalculateForegroundCoords(const Point& startPoint, int16_t progressWidth, int16_t progressHeight,
                                         uint16_t radius, Rect& coords, Rect& foregroundRect)
{
    int16_t length = GetCurrentPos(IsHorizontal(direction_) ? progressWidth_ + 1 : progressHeight_ + 1);
    if (IsHorizontal(direction_)) {
        foregroundRect.SetRect(startPoint.x, startPoint.y, static_cast<int16_t>(startPoint.x + progressWidth - 1),
                               static_cast<int16_t>(startPoint.y + progressHeight_ - 1));
    } else {
        foregroundRect.SetRect(startPoint.x, startPoint.y, static_cast<int16_t>(startPoint.x + progressWidth_ - 1),
                               static_cast<int16_t>(startPoint.y + progressHeight - 1));
    }

    switch (direction_) {
        case Direction::DIR_LEFT_TO_RIGHT:
            coords.SetRect(static_cast<int16_t>(startPoint.x - radius - 1), startPoint.y,
                           static_cast<int16_t>(startPoint.x - radius - 1 + length),
                           static_cast<int16_t>(startPoint.y + progressHeight_ - 1));
            return true;
        case Direction::DIR_RIGHT_TO_LEFT:
            coords.SetRect(static_cast<int16_t>(startPoint.x + progressWidth + radius + 1 - length), startPoint.y,
                           static_cast<int16_t>(startPoint.x + progressWidth + radius + 1),
                           static_cast<int16_t>(startPoint.y + progressHeight_ - 1));
            return true;
        case Direction::DIR_TOP_TO_BOTTOM:
            coords.SetRect(startPoint.x, static_cast<int16_t>(startPoint.y - radius - 1),
                           static_cast<int16_t>(startPoint.x + progressWidth_ - 1),
                           static_cast<int16_t>(startPoint.y - radius - 1 + length));
            return true;
        case Direction::DIR_BOTTOM_TO_TOP:
            coords.SetRect(startPoint.x, static_cast<int16_t>(startPoint.y + progressHeight + radius + 1 - length),
                           static_cast<int16_t>(startPoint.x + progressWidth_ - 1),
                           static_cast<int16_t>(startPoint.y + progressHeight + radius + 1));
            return true;
        default:
            GRAPHIC_LOGE("UISlider: DrawForeground direction Err!\n");
            return false;
    }
}
#endif

void UISlider::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, GetOrigRect(), invalidatedArea, *style_, opaScale_);

    Rect trunc(invalidatedArea);
    if (trunc.Intersect(trunc, GetOrigRect())) {
        DrawBackground(gfxDstBuffer, trunc);
        Rect foregroundRect;
        DrawForeground(gfxDstBuffer, trunc, foregroundRect);
#if GRAPHIC_ENABLE_SLIDER_FLAG
        if (showMarkings_) {
            DrawMarkings(gfxDstBuffer, trunc);
        }
        if (showMarkText_) {
            DrawMarkText(gfxDstBuffer, trunc);
        }
#endif
        DrawKnob(gfxDstBuffer, trunc, foregroundRect);
#if GRAPHIC_ENABLE_SLIDER_FLAG
        if (enableToast_ && toastVisible_) {
            char textBuffer[TEXT_BUFFER_SIZE] = {0};
            if (sprintf_s(textBuffer, sizeof(textBuffer), "%d", toastValue_) >= 0) {
                DrawToast(gfxDstBuffer, trunc, textBuffer);
            }
            if (toastAutoHide_) {
                toastVisible_ = false;
                toastAutoHide_ = false;
                needEraseToast_ = true;
            }
        }
        if (showDisabledToast_) {
            DrawDisabledToast(gfxDstBuffer, trunc);
            showDisabledToast_ = false;
            needEraseToast_ = true;
        }
#endif
    }
#if defined(GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG) && GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG
    DrawGradientBackground(gfxDstBuffer, invalidatedArea);
#endif // GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
void UISlider::ReMeasure()
{
#if defined(CONFIG_DYNAMIC_LAYOUT) && (CONFIG_DYNAMIC_LAYOUT == 1)
    UIView::ReMeasure();
#endif
    if (needEraseToast_) {
        needEraseToast_ = false;
        Invalidate();
    }
}

uint16_t UISlider::BuildMarkValues(int32_t* values, uint16_t maxCount) const
{
    if ((values == nullptr) || (maxCount == 0) || (rangeMax_ < rangeMin_)) {
        return 0;
    }

    uint16_t count = 0;
    if (valuesCount_ > 0) {
        for (uint16_t i = 0; (i < valuesCount_) && (count < maxCount); i++) {
            if ((values_[i] >= rangeMin_) && (values_[i] <= rangeMax_)) {
                values[count++] = values_[i];
            }
        }
        return count;
    }

    uint32_t step = GetStep();
    if (step == 0) {
        step = 1;
    }
    int32_t value = rangeMin_;
    while ((value <= rangeMax_) && (count < maxCount)) {
        values[count++] = value;
        if (value == rangeMax_) {
            break;
        }
        if (rangeMax_ - value <= static_cast<int32_t>(step)) {
            value = rangeMax_;
        } else {
            value += static_cast<int32_t>(step);
        }
    }
    EnsureLastMarkValue(values, count, maxCount);
    return count;
}

void UISlider::EnsureLastMarkValue(int32_t* values, uint16_t count, uint16_t maxCount) const
{
    if ((count == maxCount) && (count > 0) && (values[count - 1] != rangeMax_)) {
        values[count - 1] = rangeMax_;
    }
}

void UISlider::DrawMarkingItem(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Point& startPoint,
                               int16_t progressWidth, int16_t progressHeight, int32_t value, int32_t range)
{
    int32_t ratioLength = IsHorizontal(direction_) ? progressWidth : progressHeight;
    int32_t pos = static_cast<int32_t>((static_cast<int64_t>(value - rangeMin_) * ratioLength) / range);
    if (IsReverse(direction_)) {
        pos = ratioLength - pos;
    }

    Point start;
    Point end;
    Point center;
    if (IsHorizontal(direction_)) {
        int16_t x = static_cast<int16_t>(startPoint.x + pos);
        int16_t centerY = static_cast<int16_t>(startPoint.y + progressHeight / 2);
        center = {x, centerY};
        start = {x, static_cast<int16_t>(centerY - markingsSize_ / 2)};
        end = {x, static_cast<int16_t>(start.y + markingsSize_)};
    } else {
        int16_t y = static_cast<int16_t>(startPoint.y + pos);
        int16_t centerX = static_cast<int16_t>(startPoint.x + progressWidth / 2);
        center = {centerX, y};
        start = {static_cast<int16_t>(centerX - markingsSize_ / 2), y};
        end = {static_cast<int16_t>(start.x + markingsSize_), y};
    }
    if (markingsType_ == MARKINGS_DOT) {
        Style dotStyle = StyleDefault::GetBackgroundTransparentStyle();
        dotStyle.bgColor_ = markingsColor_;
        dotStyle.bgOpa_ = OPA_OPAQUE;
        dotStyle.borderRadius_ = markingsSize_ / HALF_DIVISOR;
        Rect dotRect;
        dotRect.SetRect(static_cast<int16_t>(center.x - markingsSize_ / HALF_DIVISOR),
                        static_cast<int16_t>(center.y - markingsSize_ / HALF_DIVISOR),
                        static_cast<int16_t>(center.x + markingsSize_ / HALF_DIVISOR),
                        static_cast<int16_t>(center.y + markingsSize_ / HALF_DIVISOR));
        BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, dotRect, invalidatedArea, dotStyle, opaScale_);
    } else {
        BaseGfxEngine::GetInstance()->DrawLine(gfxDstBuffer, start, end, invalidatedArea, DEFAULT_TICK_LINE_WIDTH,
                                               markingsColor_, opaScale_);
    }
}

void UISlider::DrawMarkings(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    if (rangeMax_ <= rangeMin_) {
        return;
    }
    int32_t values[MAX_MARK_VALUE_COUNT] = {0};
    uint16_t count = BuildMarkValues(values, MAX_MARK_VALUE_COUNT);
    if (count == 0) {
        return;
    }

    Point startPoint;
    int16_t progressWidth;
    int16_t progressHeight;
    uint16_t radius;
    GetBackgroundParam(startPoint, progressWidth, progressHeight, radius, *backgroundStyle_);
    static_cast<void>(radius);

    int32_t range = rangeMax_ - rangeMin_;
    for (uint16_t i = 0; i < count; i++) {
        DrawMarkingItem(gfxDstBuffer, invalidatedArea, startPoint, progressWidth, progressHeight, values[i], range);
    }
}

void UISlider::DrawLabelText(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Point& pos,
                             int16_t textWidth, int16_t textHeight, uint16_t fontId, const Style& textStyle,
                             const char* text, uint16_t textLength)
{
    Point textPos = pos;
    Point offset = {0, 0};
    Rect textRect(textPos.x, textPos.y, static_cast<int16_t>(textPos.x + textWidth - 1),
                  static_cast<int16_t>(textPos.y + textHeight - 1));
    Rect mask;
    if (!mask.Intersect(invalidatedArea, textRect)) {
        return;
    }
    uint16_t letterIndex = 0;
    LabelLineInfo labelLine {textPos, offset, mask, textHeight, textLength,
                             0, OPA_OPAQUE, textStyle, text, textLength,
                             0, fontId, TOAST_FONT_SIZE, 0, TEXT_DIRECT_LTR,
                             nullptr, true,
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
                             nullptr,
#endif
                             nullptr, nullptr, nullptr, nullptr, 0};
    DrawLabel::DrawTextOneLine(gfxDstBuffer, labelLine, letterIndex);
}

void UISlider::DrawMarkTextItem(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const MarkTextParam& param,
                                int32_t value)
{
    int32_t ratioLength = IsHorizontal(direction_) ? param.progressWidth : param.progressHeight;
    int32_t pos = static_cast<int32_t>((static_cast<int64_t>(value - rangeMin_) * ratioLength) / param.range);
    if (IsReverse(direction_)) {
        pos = ratioLength - pos;
    }

    char text[TEXT_BUFFER_SIZE] = {0};
    if (sprintf_s(text, sizeof(text), "%d", value) < 0) {
        return;
    }
    uint16_t textLength = static_cast<uint16_t>(std::strlen(text));
    int16_t textWidth = TypedText::GetTextWidth(text, param.fontId, TOAST_FONT_SIZE, textLength,
                                                param.textStyle.letterSpace_);
    if (textWidth <= 0) {
        return;
    }

    Point posPoint {};
    if (IsHorizontal(direction_)) {
        posPoint.x = static_cast<int16_t>(param.startPoint.x + pos - textWidth / HALF_DIVISOR);
        posPoint.y = static_cast<int16_t>(param.startPoint.y + param.progressHeight + markingsSize_ +
                                          MARK_TEXT_OFFSET);
    } else {
        posPoint.x = static_cast<int16_t>(param.startPoint.x + param.progressWidth + markingsSize_ +
                                          MARK_TEXT_OFFSET);
        posPoint.y = static_cast<int16_t>(param.startPoint.y + pos - param.lineHeight / HALF_DIVISOR);
    }
    posPoint.x = ClampInt16(posPoint.x, param.viewRect.GetLeft(),
                            static_cast<int16_t>(param.viewRect.GetRight() - textWidth + 1));
    posPoint.y = ClampInt16(posPoint.y, param.viewRect.GetTop(),
                            static_cast<int16_t>(param.viewRect.GetBottom() - param.lineHeight + 1));
    DrawLabelText(gfxDstBuffer, invalidatedArea, posPoint, textWidth, param.lineHeight, param.fontId,
                  param.textStyle, text, textLength);
}

void UISlider::DrawMarkText(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    if (rangeMax_ <= rangeMin_) {
        return;
    }
    int32_t values[MAX_MARK_VALUE_COUNT] = {0};
    uint16_t count = BuildMarkValues(values, MAX_MARK_VALUE_COUNT);
    if (count == 0) {
        return;
    }

    MarkTextParam param;
    uint16_t radius;
    GetBackgroundParam(param.startPoint, param.progressWidth, param.progressHeight, radius, *backgroundStyle_);
    static_cast<void>(radius);
    param.range = rangeMax_ - rangeMin_;
    param.fontId = UIFont::GetInstance()->GetFontId(DEFAULT_VECTOR_FONT_FILENAME);
    param.lineHeight = UIFont::GetInstance()->GetHeight(param.fontId, TOAST_FONT_SIZE);
    param.lineHeight = (param.lineHeight == 0) ? TOAST_FONT_SIZE : param.lineHeight;
    param.textStyle = StyleDefault::GetLabelStyle();
    param.textStyle.textColor_ = markTextColor_;
    param.textStyle.textOpa_ = OPA_OPAQUE;
    param.viewRect = GetOrigRect();

    uint16_t step = (count > MAX_TICK_VALUE_COUNT) ?
        static_cast<uint16_t>((count + MAX_TICK_VALUE_COUNT - 1) / MAX_TICK_VALUE_COUNT) : 1;
    for (uint16_t i = 0; i < count; i++) {
        if ((i % step == 0) || (i == count - 1)) {
            DrawMarkTextItem(gfxDstBuffer, invalidatedArea, param, values[i]);
        }
    }
}

void UISlider::DrawGradientRect(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Rect& rect,
                                const Style& style, const ColorType* colors, uint16_t count)
{
    if ((colors == nullptr) || (count < HALF_DIVISOR)) {
        DrawValidRect(gfxDstBuffer, nullptr, rect, invalidatedArea, style, 0);
        return;
    }

    Rect drawRect(rect);
    if (!drawRect.Intersect(drawRect, invalidatedArea)) {
        return;
    }

    bool horizontal = drawRect.GetWidth() >= drawRect.GetHeight();
    int16_t span = horizontal ? drawRect.GetWidth() : drawRect.GetHeight();
    if (span <= 0) {
        return;
    }

    BaseGfxEngine* gfxEngine = BaseGfxEngine::GetInstance();
    Style sliceStyle = style;
    sliceStyle.bgOpa_ = OPA_OPAQUE;
    // Quantize the gradient into fixed-step slices: each slice samples the interpolated color
    // at its midpoint, reducing DrawRect calls from one per pixel to one per QUANTIZATION_STEP
    // pixels. Per-pixel merging does not work for real gradients where adjacent pixels almost
    // never share the same color.
    constexpr int16_t QUANTIZATION_STEP = 8;
    for (int16_t i = 0; i < span; i += QUANTIZATION_STEP) {
        int16_t end = static_cast<int16_t>(i + QUANTIZATION_STEP);
        if (end > span) {
            end = span;
        }
        int16_t mid = static_cast<int16_t>(i + (end - i) / HALF_DIVISOR);
        int32_t scaled = static_cast<int32_t>(count - 1) * mid;
        uint16_t segment = static_cast<uint16_t>(scaled / span);
        if (segment >= count - 1) {
            segment = count - HALF_DIVISOR;
        }
        int16_t segmentStart = static_cast<int16_t>((static_cast<int32_t>(segment) * span) / (count - 1));
        int16_t segmentEnd = static_cast<int16_t>((static_cast<int32_t>(segment + 1) * span) / (count - 1));
        int16_t segmentSpan = static_cast<int16_t>((segmentEnd > segmentStart) ? (segmentEnd - segmentStart) : 1);
        int16_t localOffset = static_cast<int16_t>(mid - segmentStart);
        uint8_t mix = static_cast<uint8_t>((localOffset * COLOR_MIX_MAX) / segmentSpan);
        sliceStyle.bgColor_ = InterpolateColor(colors[segment], colors[segment + 1], mix);
        Rect sliceRect(drawRect);
        if (horizontal) {
            sliceRect.SetRect(static_cast<int16_t>(drawRect.GetLeft() + i), drawRect.GetTop(),
                              static_cast<int16_t>(drawRect.GetLeft() + end - 1), drawRect.GetBottom());
        } else {
            sliceRect.SetRect(drawRect.GetLeft(), static_cast<int16_t>(drawRect.GetTop() + i),
                              drawRect.GetRight(), static_cast<int16_t>(drawRect.GetTop() + end - 1));
        }
        gfxEngine->DrawRect(gfxDstBuffer, sliceRect, invalidatedArea, sliceStyle, opaScale_);
    }
}

bool UISlider::ShouldAcceptClick(const Point& pos)
{
    if (expandClickArea_) {
        return true;
    }

    Point startPoint;
    int16_t progressWidth;
    int16_t progressHeight;
    uint16_t radius;
    GetBackgroundParam(startPoint, progressWidth, progressHeight, radius, *backgroundStyle_);
    static_cast<void>(radius);
    Rect trackRect;
    trackRect.SetRect(startPoint.x, startPoint.y, startPoint.x + progressWidth - 1,
                      startPoint.y + progressHeight - 1);
    return ((pos.x >= trackRect.GetLeft()) && (pos.x <= trackRect.GetRight()) &&
            (pos.y >= trackRect.GetTop()) && (pos.y <= trackRect.GetBottom()));
}

ColorType UISlider::InterpolateColor(const ColorType& start, const ColorType& end, uint8_t mix) const
{
    uint32_t start32 = Color::ColorTo32(start);
    uint32_t end32 = Color::ColorTo32(end);
    uint8_t sr = static_cast<uint8_t>((start32 >> COLOR_RED_SHIFT) & COLOR_CHANNEL_MASK);
    uint8_t sg = static_cast<uint8_t>((start32 >> COLOR_GREEN_SHIFT) & COLOR_CHANNEL_MASK);
    uint8_t sb = static_cast<uint8_t>(start32 & COLOR_CHANNEL_MASK);
    uint8_t er = static_cast<uint8_t>((end32 >> COLOR_RED_SHIFT) & COLOR_CHANNEL_MASK);
    uint8_t eg = static_cast<uint8_t>((end32 >> COLOR_GREEN_SHIFT) & COLOR_CHANNEL_MASK);
    uint8_t eb = static_cast<uint8_t>(end32 & COLOR_CHANNEL_MASK);
    uint8_t r = static_cast<uint8_t>((static_cast<uint16_t>(sr) * (COLOR_MIX_MAX - mix) +
                                      static_cast<uint16_t>(er) * mix) / COLOR_MIX_MAX);
    uint8_t g = static_cast<uint8_t>((static_cast<uint16_t>(sg) * (COLOR_MIX_MAX - mix) +
                                      static_cast<uint16_t>(eg) * mix) / COLOR_MIX_MAX);
    uint8_t b = static_cast<uint8_t>((static_cast<uint16_t>(sb) * (COLOR_MIX_MAX - mix) +
                                      static_cast<uint16_t>(eb) * mix) / COLOR_MIX_MAX);
    return Color::GetColorFromRGBA(r, g, b, COLOR_CHANNEL_MASK);
}

bool UISlider::InitToastParam(const char* text, ToastParam& param)
{
    if ((text == nullptr) || (text[0] == '\0')) {
        return false;
    }

    param.viewRect = GetOrigRect();
    param.fontId = UIFont::GetInstance()->GetFontId(DEFAULT_VECTOR_FONT_FILENAME);
    param.textStyle = StyleDefault::GetLabelStyle();
    param.textStyle.textColor_ = Color::White();
    param.textStyle.textOpa_ = OPA_OPAQUE;
    param.textLength = static_cast<uint16_t>(std::strlen(text));
    param.textWidth = TypedText::GetTextWidth(text, param.fontId, TOAST_FONT_SIZE, param.textLength,
                                              param.textStyle.letterSpace_);
    param.textHeight = static_cast<int16_t>(UIFont::GetInstance()->GetHeight(param.fontId, TOAST_FONT_SIZE));
    if (param.textHeight <= 0) {
        param.textHeight = TOAST_FONT_SIZE;
    }
    if ((param.textWidth <= 0) || (param.textHeight <= 0)) {
        return false;
    }

    uint16_t radius;
    param.toastWidth = static_cast<int16_t>(param.textWidth + TOAST_PADDING * HALF_DIVISOR);
    param.toastHeight = static_cast<int16_t>(param.textHeight + TOAST_PADDING * HALF_DIVISOR);
    GetBackgroundParam(param.startPoint, param.progressWidth, param.progressHeight, radius, *backgroundStyle_);
    static_cast<void>(radius);
    return true;
}

void UISlider::LocateToast(const ToastParam& param, int16_t& left, int16_t& top) const
{
    left = param.viewRect.GetLeft();
    top = param.viewRect.GetTop();
    if (rangeMax_ <= rangeMin_) {
        return;
    }

    int32_t ratioLength = IsHorizontal(direction_) ? param.progressWidth : param.progressHeight;
    int32_t pos = static_cast<int32_t>((static_cast<int64_t>(curValue_ - rangeMin_) * ratioLength) /
                                       (rangeMax_ - rangeMin_));
    if (IsReverse(direction_)) {
        pos = ratioLength - pos;
    }
    if (IsHorizontal(direction_)) {
        int16_t centerX = static_cast<int16_t>(param.startPoint.x + pos);
        left = static_cast<int16_t>(centerX - param.toastWidth / HALF_DIVISOR);
        top = static_cast<int16_t>(param.startPoint.y - param.toastHeight - TOAST_OFFSET);
    } else {
        int16_t centerY = static_cast<int16_t>(param.startPoint.y + pos);
        left = static_cast<int16_t>(param.startPoint.x + param.progressWidth + TOAST_OFFSET);
        top = static_cast<int16_t>(centerY - param.toastHeight / HALF_DIVISOR);
    }
}

void UISlider::DrawToastRect(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, int16_t left, int16_t top,
                             const ToastParam& param)
{
    Style toastStyle = StyleDefault::GetBackgroundTransparentStyle();
    toastStyle.bgColor_ = Color::Black();
    toastStyle.bgOpa_ = DEFAULT_TOAST_OPA;
    toastStyle.borderRadius_ = TOAST_PADDING;
    Rect toastRect;
    toastRect.SetRect(left, top, static_cast<int16_t>(left + param.toastWidth - 1),
                      static_cast<int16_t>(top + param.toastHeight - 1));
    BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, toastRect, invalidatedArea, toastStyle, opaScale_);
}

void UISlider::DrawToast(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const char* text)
{
    ToastParam param {};
    if (!InitToastParam(text, param)) {
        return;
    }

    int16_t left;
    int16_t top;
    LocateToast(param, left, top);
    left = ClampInt16(left, param.viewRect.GetLeft(),
                      static_cast<int16_t>(param.viewRect.GetRight() - param.toastWidth + 1));
    top = ClampInt16(top, param.viewRect.GetTop(),
                     static_cast<int16_t>(param.viewRect.GetBottom() - param.toastHeight + 1));
    DrawToastRect(gfxDstBuffer, invalidatedArea, left, top, param);
    DrawLabelText(gfxDstBuffer, invalidatedArea,
                  {static_cast<int16_t>(left + TOAST_PADDING), static_cast<int16_t>(top + TOAST_PADDING)},
                  param.textWidth, param.textHeight, param.fontId, param.textStyle, text, param.textLength);
}

void UISlider::DrawDisabledToast(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    DrawToast(gfxDstBuffer, invalidatedArea, disabledToastMsg_);
}
#endif

int32_t UISlider::CalculateCurrentValue(int16_t length, int16_t totalLength)
{
    if (totalLength != 0) {
        return static_cast<int32_t>(rangeMin_ + (static_cast<int64_t>(rangeMax_) - rangeMin_) * length / totalLength);
    }
    return 0;
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
int32_t UISlider::CalculateHorizontalValueByPosition(const Point& knobPosition, const Point& startPoint)
{
    if (knobPosition.x <= startPoint.x) {
        return (direction_ == Direction::DIR_LEFT_TO_RIGHT) ? rangeMin_ : rangeMax_;
    }
    if (knobPosition.x >= startPoint.x + progressWidth_ - 1) {
        return (direction_ == Direction::DIR_LEFT_TO_RIGHT) ? rangeMax_ : rangeMin_;
    }
    if (direction_ == Direction::DIR_LEFT_TO_RIGHT) {
        return CalculateCurrentValue(knobPosition.x - startPoint.x, progressWidth_);
    }
    return CalculateCurrentValue(startPoint.x + progressWidth_ - knobPosition.x, progressWidth_);
}

int32_t UISlider::CalculateVerticalValueByPosition(const Point& knobPosition, const Point& startPoint)
{
    if (knobPosition.y <= startPoint.y) {
        return (direction_ == Direction::DIR_TOP_TO_BOTTOM) ? rangeMin_ : rangeMax_;
    }
    if (knobPosition.y >= startPoint.y + progressHeight_ - 1) {
        return (direction_ == Direction::DIR_TOP_TO_BOTTOM) ? rangeMax_ : rangeMin_;
    }
    if (direction_ == Direction::DIR_TOP_TO_BOTTOM) {
        return CalculateCurrentValue(knobPosition.y - startPoint.y, progressHeight_);
    }
    return CalculateCurrentValue(startPoint.y + progressHeight_ - knobPosition.y, progressHeight_);
}

int32_t UISlider::CalculateValueByPosition(const Point& knobPosition)
{
    Point startPoint;
    Rect rect = GetOrigRect();
    // 2: Half of the gap
    startPoint.x = rect.GetLeft() + style_->borderWidth_ + style_->paddingLeft_ + (GetWidth() - progressWidth_) / 2;
    // 2: Half of the gap
    startPoint.y = rect.GetTop() + style_->borderWidth_ + style_->paddingTop_ + (GetHeight() - progressHeight_) / 2;

    switch (direction_) {
        case Direction::DIR_LEFT_TO_RIGHT:
        case Direction::DIR_RIGHT_TO_LEFT:
            return CalculateHorizontalValueByPosition(knobPosition, startPoint);
        case Direction::DIR_BOTTOM_TO_TOP:
        case Direction::DIR_TOP_TO_BOTTOM:
            return CalculateVerticalValueByPosition(knobPosition, startPoint);
        default:
            GRAPHIC_LOGW("UISlider::UpdateCurrentValue Direction error!\n");
            return curValue_;
    }
}

int32_t UISlider::UpdateCurrentValue(const Point& knobPosition)
{
    int32_t value = CalculateValueByPosition(knobPosition);
    if (enableTicks_ || (valuesCount_ > 0)) {
        value = SnapToMarkValue(value);
    }
    SetValue(value);
    lastValue_ = curValue_;
    return curValue_;
}
#else
int32_t UISlider::UpdateCurrentValue(const Point& knobPosition)
{
    Point startPoint;
    Rect rect = GetOrigRect();
    // 2: Half of the gap
    startPoint.x = rect.GetLeft() + style_->borderWidth_ + style_->paddingLeft_ + (GetWidth() - progressWidth_) / 2;
    // 2: Half of the gap
    startPoint.y = rect.GetTop() + style_->borderWidth_ + style_->paddingTop_ + (GetHeight() - progressHeight_) / 2;

    int32_t value = curValue_;
    switch (direction_) {
        case Direction::DIR_LEFT_TO_RIGHT:
            if (knobPosition.x <= startPoint.x) {
                value = rangeMin_;
            } else if (knobPosition.x >= startPoint.x + progressWidth_) {
                value = rangeMax_;
            } else {
                value = CalculateCurrentValue(knobPosition.x - startPoint.x, progressWidth_);
            }
            break;
        case Direction::DIR_RIGHT_TO_LEFT:
            if (knobPosition.x <= startPoint.x) {
                value = rangeMax_;
            } else if (knobPosition.x >= startPoint.x + progressWidth_) {
                value = rangeMin_;
            } else {
                value = CalculateCurrentValue(startPoint.x + progressWidth_ - knobPosition.x, progressWidth_);
            }
            break;
        case Direction::DIR_BOTTOM_TO_TOP:
            if (knobPosition.y <= startPoint.y) {
                value = rangeMax_;
            } else if (knobPosition.y >= startPoint.y + progressHeight_) {
                value = rangeMin_;
            } else {
                value = CalculateCurrentValue(startPoint.y + progressHeight_ - knobPosition.y, progressHeight_);
            }
            break;
        case Direction::DIR_TOP_TO_BOTTOM:
            if (knobPosition.y <= startPoint.y) {
                value = rangeMin_;
            } else if (knobPosition.y >= startPoint.y + progressHeight_) {
                value = rangeMax_;
            } else {
                value = CalculateCurrentValue(knobPosition.y - startPoint.y, progressHeight_);
            }
            break;
        default:
            GRAPHIC_LOGW("UISlider::UpdateCurrentValue Direction error!\n");
    }
    SetValue(value);
    return value;
}
#endif

#if GRAPHIC_ENABLE_SLIDER_FLAG
int32_t UISlider::SnapToMarkValue(int32_t value) const
{
    if (rangeMax_ < rangeMin_) {
        return value;
    }
    if (value <= rangeMin_) {
        return rangeMin_;
    }
    if (value >= rangeMax_) {
        return rangeMax_;
    }

    if (valuesCount_ > 0) {
        int32_t snappedValue = value;
        bool hasValidValue = false;
        int32_t minDistance = 0;
        for (uint16_t i = 0; i < valuesCount_; i++) {
            if ((values_[i] < rangeMin_) || (values_[i] > rangeMax_)) {
                continue;
            }
            int32_t distance = MATH_ABS(value - values_[i]);
            if (!hasValidValue || (distance < minDistance)) {
                hasValidValue = true;
                minDistance = distance;
                snappedValue = values_[i];
            }
        }
        return hasValidValue ? snappedValue : value;
    }

    uint32_t step = GetStep();
    if (step == 0) {
        step = 1;
    }
    int32_t stepValue = static_cast<int32_t>(step);
    int32_t offset = value - rangeMin_;
    int32_t snappedValue = rangeMin_ + ((offset + stepValue / 2) / stepValue) * stepValue; // 2: half step
    if (snappedValue < rangeMin_) {
        return rangeMin_;
    }
    if (snappedValue > rangeMax_) {
        return rangeMax_;
    }
    return snappedValue;
}

bool UISlider::HandleDisabledEvent()
{
    if (!disabled_) {
        return false;
    }
    if (disabledToastMsg_[0] != '\0') {
        showDisabledToast_ = true;
        Invalidate();
    }
    return true;
}

void UISlider::ShowValueToast(int32_t value, bool autoHide)
{
    if (!enableToast_) {
        return;
    }
    toastValue_ = value;
    toastVisible_ = true;
    toastAutoHide_ = autoHide;
}

void UISlider::HideValueToast()
{
    toastVisible_ = false;
    toastAutoHide_ = false;
}
#endif

bool UISlider::OnClickEvent(const ClickEvent& event)
{
    Point knobPosition = event.GetCurrentPos();
#if GRAPHIC_ENABLE_SLIDER_FLAG
    if (!ShouldAcceptClick(knobPosition)) {
        return UIView::OnClickEvent(event);
    }
    if (disabled_) {
        return UIView::OnClickEvent(event);
    }
#endif
    int32_t value = UpdateCurrentValue(knobPosition);
    if (listener_ != nullptr) {
        listener_->OnChange(value);
    }
    bool ret = UIView::OnClickEvent(event);
    Invalidate();
    return ret;
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
bool UISlider::OnPressEvent(const PressEvent& event)
{
    Point knobPosition = event.GetCurrentPos();
    if (!ShouldAcceptClick(knobPosition)) {
        return UIView::OnPressEvent(event);
    }
    if (HandleDisabledEvent()) {
        return UIView::OnPressEvent(event);
    }
    if (enableToast_) {
        int32_t value = UpdateCurrentValue(knobPosition);
        ShowValueToast(value, false);
        Invalidate();
    }
    return UIView::OnPressEvent(event);
}

bool UISlider::OnReleaseEvent(const ReleaseEvent& event)
{
    showDisabledToast_ = false;
    HideValueToast();
    needEraseToast_ = false;
    Invalidate();
    return UIView::OnReleaseEvent(event);
}
#endif

bool UISlider::OnDragEvent(const DragEvent& event)
{
    Point knobPosition = event.GetCurrentPos();
#if GRAPHIC_ENABLE_SLIDER_FLAG
    if (!ShouldAcceptClick(knobPosition)) {
        return UIView::OnDragEvent(event);
    }
    if (HandleDisabledEvent()) {
        return UIView::OnDragEvent(event);
    }
#endif
    int32_t value = UpdateCurrentValue(knobPosition);
#if GRAPHIC_ENABLE_SLIDER_FLAG
    ShowValueToast(value, false);
#endif
    if (listener_ != nullptr) {
        listener_->OnChange(value);
    }
    Invalidate();
    return UIView::OnDragEvent(event);
}

bool UISlider::OnDragEndEvent(const DragEvent& event)
{
    Point knobPosition = event.GetCurrentPos();
#if GRAPHIC_ENABLE_SLIDER_FLAG
    HideValueToast();
    if (disabled_) {
        showDisabledToast_ = false;
        Invalidate();
        return UIView::OnDragEndEvent(event);
    }
    if (!ShouldAcceptClick(knobPosition)) {
        Invalidate();
        return UIView::OnDragEndEvent(event);
    }
#endif
    int32_t value = UpdateCurrentValue(knobPosition);
    if (listener_ != nullptr) {
        listener_->OnChange(value);
        listener_->OnRelease(value);
    }
    Invalidate();
    return UIView::OnDragEndEvent(event);
}

#if ENABLE_ROTATE_INPUT
bool UISlider::OnRotateEvent(const RotateEvent& event)
{
#if GRAPHIC_ENABLE_SLIDER_FLAG
    if (HandleDisabledEvent()) {
        return UIView::OnRotateEvent(event);
    }
#endif
    int32_t realRotation = 0;
    cachedRotation_ += event.GetRotate() * rotateFactor_;
    realRotation = static_cast<int32_t>(cachedRotation_);
    if (realRotation == 0) {
        return UIView::OnRotateEvent(event);
    }
    cachedRotation_ = 0;
#if ENABLE_VIBRATOR
    int32_t lastValue = curValue_;
#endif
#if GRAPHIC_ENABLE_SLIDER_FLAG
    int32_t value = curValue_ + realRotation;
    if (enableTicks_ || (valuesCount_ > 0)) {
        value = SnapToMarkValue(value);
    }
    SetValue(value);
    ShowValueToast(curValue_, true);
#else
    SetValue(curValue_ + realRotation);
#endif
    if (listener_ != nullptr) {
        listener_->OnChange(curValue_);
    }
#if ENABLE_VIBRATOR
    VibratorFunc vibratorFunc = VibratorManager::GetInstance()->GetVibratorFunc();
    if (vibratorFunc != nullptr && lastValue != curValue_) {
        if (curValue_ == rangeMin_ || curValue_ == rangeMax_) {
            GRAPHIC_LOGI("UISlider::OnRotateEvent calls TYPE_THREE vibrator");
            vibratorFunc(VibratorType::VIBRATOR_TYPE_THREE);
        } else {
            int32_t changedValue = MATH_ABS(curValue_ - lastValue);
            for (int32_t i = 0; i < changedValue; i++) {
                GRAPHIC_LOGI("UISlider::OnRotateEvent calls TYPE_TWO vibrator");
                vibratorFunc(VibratorType::VIBRATOR_TYPE_TWO);
            }
        }
    }
#endif
    return UIView::OnRotateEvent(event);
}

bool UISlider::OnRotateEndEvent(const RotateEvent& event)
{
    cachedRotation_ = 0;
    return UIView::OnRotateEndEvent(event);
}
#endif
} // namespace OHOS
