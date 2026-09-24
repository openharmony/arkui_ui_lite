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

#ifndef GRAPHIC_LITE_UI_VIEW_FLEX_PUBLIC_H
#define GRAPHIC_LITE_UI_VIEW_FLEX_PUBLIC_H

void SetOverflow(OverflowMode mode);
OverflowMode GetOverflow() const { return overflow_; }
void SetOriginalWidth(int16_t width)
{
    originalWidth_ = width;
}
int16_t GetOriginalWidth() const
{
    return originalWidth_;
}
void SetOriginalWidthSaved(bool saved)
{
    originalWidthSaved_ = saved;
}
bool IsOriginalWidthSaved() const
{
    return originalWidthSaved_;
}
void SetOriginalHeight(int16_t height)
{
    originalHeight_ = height;
}
int16_t GetOriginalHeight() const
{
    return originalHeight_;
}
void SetOriginalHeightSaved(bool saved)
{
    originalHeightSaved_ = saved;
}
bool IsOriginalHeightSaved() const
{
    return originalHeightSaved_;
}

void SetFlexGrow(uint16_t grow);
uint16_t GetFlexGrow() const;
void BeginFlexLayoutSizing();
void EndFlexLayoutSizing();
int16_t GetFlexAutoBasis(bool horizontal) const;
void SetFlexShrink(uint16_t shrink);
uint16_t GetFlexShrink() const;
bool IsFlexShrinkSet() const;
void SetFlexBasis(int16_t basis);
int16_t GetFlexBasis() const;
void SetMinWidth(int16_t width);
int16_t GetMinWidth() const;
void SetMaxWidth(int16_t width);
int16_t GetMaxWidth() const;
void SetMinHeight(int16_t height);
int16_t GetMinHeight() const;
void SetMaxHeight(int16_t height);
int16_t GetMaxHeight() const;
void SetAspectRatio(uint16_t ratio);
uint16_t GetAspectRatio() const;
void SetPositionType(uint8_t type);
uint8_t GetPositionType() const;
void SetFlexLeft(int16_t value);
void SetFlexLeftPercent(float percent);
int16_t GetFlexLeft() const;
float GetFlexLeftPercent() const;
bool IsFlexLeftPercent() const;
bool HasFlexLeft() const;
void ClearFlexLeft();
void SetFlexRight(int16_t value);
void SetFlexRightPercent(float percent);
int16_t GetFlexRight() const;
float GetFlexRightPercent() const;
bool IsFlexRightPercent() const;
bool HasFlexRight() const;
void ClearFlexRight();
void SetFlexTop(int16_t value);
void SetFlexTopPercent(float percent);
int16_t GetFlexTop() const;
float GetFlexTopPercent() const;
bool IsFlexTopPercent() const;
bool HasFlexTop() const;
void ClearFlexTop();
void SetFlexBottom(int16_t value);
void SetFlexBottomPercent(float percent);
int16_t GetFlexBottom() const;
float GetFlexBottomPercent() const;
bool IsFlexBottomPercent() const;
bool HasFlexBottom() const;
void ClearFlexBottom();
void SetMarginLeftAuto(bool autoFlag);
bool IsMarginLeftAuto() const;

enum class AlignSelf : uint8_t {
    AUTO = 0xFF,
    STRETCH = 0xFE
};
static constexpr uint8_t ALIGN_SELF_AUTO = static_cast<uint8_t>(AlignSelf::AUTO);
static constexpr uint8_t ALIGN_SELF_STRETCH = static_cast<uint8_t>(AlignSelf::STRETCH);

void SetAlignSelf(uint8_t alignSelf);
uint8_t GetAlignSelf() const;
void SetHasExplicitWidth(bool hasExplicitWidth);
bool HasExplicitWidth() const;
void SetHasExplicitHeight(bool hasExplicitHeight);
bool HasExplicitHeight() const;
bool IsPhasedLayoutNeed() const;
bool HasFlexItemProperties() const;

#endif // GRAPHIC_LITE_UI_VIEW_FLEX_PUBLIC_H
