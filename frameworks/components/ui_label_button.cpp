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

#include "components/ui_label_button.h"
#include "common/typed_text.h"
#include "draw/draw_label.h"
#include "font/ui_font.h"

namespace OHOS {
UILabelButton::UILabelButton() : labelButtonText_(nullptr), offset_({ 0, 0 })
{
    labelStyle_ = StyleDefault::GetDefaultStyle();
#if defined(CONFIG_SCALE_FONT_SIZE) && (CONFIG_SCALE_FONT_SIZE == 1)
    scaleRatio_ = Text::GetDefaultScale();
    maxLines_ = 2; // 2: label button default max line count
    needRefresh_ = true;
    limitTextHeight_ = 0;
    ellipsisIndex_ = Text::TEXT_ELLIPSIS_END_INV;
    textRectWidth_ = 0;
    forceRemeasure_ = false;
#endif
}

void UILabelButton::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    UIButton::OnDraw(gfxDstBuffer, invalidatedArea);

    Rect textRect = GetContentRect();
    textRect.SetLeft(textRect.GetLeft() + offset_.x);
    textRect.SetTop(textRect.GetTop() + offset_.y);
    InitLabelButtonText();
#if defined(CONFIG_SCALE_FONT_SIZE) && (CONFIG_SCALE_FONT_SIZE == 1)
    if (forceRemeasure_ || HasFontSizeScale()) {
        textRect.SetWidth(textRectWidth_ != 0 ? textRectWidth_ : (textRect.GetWidth() - offset_.x));
        textRect.SetHeight(textRect.GetHeight() - offset_.y);
        labelButtonText_->OnDraw(gfxDstBuffer, invalidatedArea, GetOrigRect(), textRect, 0,
                                 labelStyle_, ellipsisIndex_, GetMixOpaScale());
        return;
    }
#endif
    labelButtonText_->ReMeasureTextSize(textRect, labelStyle_);
    OpacityType opa = GetMixOpaScale();
    uint16_t ellipsisIndex = labelButtonText_->GetEllipsisIndex(textRect, labelStyle_);
    labelButtonText_->OnDraw(gfxDstBuffer, invalidatedArea, GetOrigRect(), textRect, 0,
                             labelStyle_, ellipsisIndex, opa);
}

UILabelButton::~UILabelButton()
{
    if (labelButtonText_ != nullptr) {
        delete labelButtonText_;
        labelButtonText_ = nullptr;
    }
}

#if defined(CONFIG_SCALE_FONT_SIZE) && (CONFIG_SCALE_FONT_SIZE == 1)
bool UILabelButton::SetFontSizeScale(float ratio)
{
    float realRatio = ratio;
    if (realRatio > MAX_SCALE_RATIO) {
        GRAPHIC_LOGW("UILabelButton::SetFontSizeScale ratio over max: %f", ratio);
        realRatio = MAX_SCALE_RATIO;
    }

    if (fabs(realRatio - scaleRatio_) < 1e-6f) {
        return true;
    }
    InitLabelButtonText();
    if (!labelButtonText_->SetFontSizeScale(realRatio)) {
        return false;
    }

    scaleRatio_ = realRatio;
    needRefresh_ = true;

    return true;
}

float UILabelButton::GetFontSizeScale() const
{
    return scaleRatio_;
}

bool UILabelButton::HasFontSizeScale() const
{
    return fabs(scaleRatio_ - DEFAULT_SCALE_RATIO) > 1e-6f;
}

int16_t UILabelButton::GetTextHeight()
{
    InitLabelButtonText();
    if (labelButtonText_->IsNeedRefresh()) {
        ReMeasure();
    }

    return limitTextHeight_;
}

bool UILabelButton::SetMaxLines(int32_t count)
{
    if (count > MAX_LINE_COUNT) {
        return false;
    }

    if (maxLines_ != count) {
        needRefresh_ = true;
    }
    maxLines_ = count;

    return true;
}

void UILabelButton::ReMeasure()
{
#if defined(CONFIG_DYNAMIC_LAYOUT) && (CONFIG_DYNAMIC_LAYOUT == 1)
    UIView::ReMeasure();
#endif

    if (!forceRemeasure_ && !HasFontSizeScale()) {
        return;
    }

    if (!needRefresh_) {
        return;
    }

    needRefresh_ = false;
    InitLabelButtonText();
    Rect rect = GetContentRect();
    rect.SetWidth(textRectWidth_ != 0 ? textRectWidth_ : (rect.GetWidth() - offset_.x * 2)); // 2: left and right
    rect.SetHeight(rect.GetHeight() - offset_.y * 2); // 2: top and bottom
    TextSizeLimitArg limit = { rect, maxLines_ };

    labelButtonText_->ReMeasureTextSize(limit.rect, labelStyle_, limit.maxLines);
    limitTextHeight_ = labelButtonText_->GetTextSize().y;

    labelButtonText_->ReMeasureTextSize(limit.rect, labelStyle_);
    limit.rect.SetHeight(limitTextHeight_);
    ellipsisIndex_ = labelButtonText_->GetEllipsisIndex(limit.rect, labelStyle_);
    labelButtonText_->ReMeasureTextWidthInEllipsisMode(limit.rect, labelStyle_, ellipsisIndex_);

    if (maxLines_ != MAX_LINE_COUNT) {
        SetHeight(limitTextHeight_ + offset_.y * 2); // 2: top and bottom
    }
}

void UILabelButton::SetTextRectWidth(uint16_t width)
{
    textRectWidth_ = width;
}

void UILabelButton::SetForceRemeasure(bool flag)
{
    forceRemeasure_ = flag;
}
#endif
}  // namespace OHOS
