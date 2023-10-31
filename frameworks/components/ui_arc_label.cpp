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

#include "components/ui_arc_label.h"

#include "common/typed_text.h"
#include "draw/draw_label.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "font/ui_font.h"
#include "themes/theme_manager.h"

namespace OHOS {
static constexpr uint16_t DEFAULT_ARC_LABEL_ROLL_COUNT = 1;
static constexpr uint16_t DEFAULT_ARC_LABEL_ANIMATOR_SPEED = 10;

class ArcLabelAnimator : public Animator, public AnimatorCallback {
public:
    ArcLabelAnimator(uint16_t rollCount, UIView* view)
        : Animator(this, view, 0, true),
          waitCount_(ANIM_WAIT_COUNT),
          speed_(0),
          preRunTime_(0),
          decimal_(0),
          rollCount_(rollCount)
    {
    }

    virtual ~ArcLabelAnimator() {}

    void Callback(UIView* view) override
    {
        if (view == nullptr || rollCount_ == 0) {
            return;
        }

        uint32_t curTime = GetRunTime();
        if (waitCount_ > 0) {
            waitCount_--;
            preRunTime_ = curTime;
            return;
        }
        if (curTime == preRunTime_) {
            return;
        }
        uint32_t time = (curTime > preRunTime_) ? (curTime - preRunTime_) : (UINT32_MAX - preRunTime_ + curTime);
        // 1000: 1000 milliseconds is 1 second
        float floatStep = (static_cast<float>(time * speed_) / 1000) + decimal_;
        uint16_t integerStep = static_cast<uint16_t>(floatStep);
        decimal_ = floatStep - integerStep;
        preRunTime_ = curTime;

        if (integerStep == 0) {
            return;
        }

        CalculatedOffsetAngle(view);
    }

    void SetRollSpeed(uint16_t speed)
    {
        speed_ = speed;
    }

    int16_t GetRollSpeed() const
    {
        return speed_;
    }

    void SetRollCount(uint16_t rollCount)
    {
        rollCount_ = rollCount;
    }

    void RegisterScrollListener(ArcLabelScrollListener* scrollListener)
    {
        scrollListener_ = scrollListener;
    }

private:
    void CalculatedOffsetAngle(UIView* view)
    {
        if (view == nullptr) {
            return;
        }
        UIArcLabel* arcLabel = static_cast<UIArcLabel*>(view);
        if (arcLabel == nullptr) {
            return;
        }

        int16_t startAngle = arcLabel->GetArcTextStartAngle();
        int16_t endAngle = arcLabel->GetArcTextEndAngle();
        uint16_t arcAngle = (startAngle < endAngle) ? (endAngle - startAngle) :
            (startAngle - endAngle);

        if (arcLabel->offsetAngle_ < arcAngle) {
            arcLabel->offsetAngle_ += DEFAULT_CHANGE_ANGLE;
        } else {
            rollCount_--;
            if (rollCount_ > 0) {
                arcLabel->offsetAngle_ = arcLabel->animator_.secondLapOffsetAngle_;
            }
        }

        if (rollCount_ == 0) {
            if (scrollListener_) {
                scrollListener_->Finish();
            }
            Stop();
        }
        view->Invalidate();
    }

private:
    static constexpr uint8_t ANIM_WAIT_COUNT = 50;
    static constexpr float DEFAULT_CHANGE_ANGLE = 1.0f;
    uint16_t waitCount_;
    uint16_t speed_;
    uint32_t preRunTime_;
    float decimal_;

    uint16_t rollCount_;
    ArcLabelScrollListener* scrollListener_;
};

UIArcLabel::UIArcLabel()
    : arcLabelText_(nullptr),
      compatibilityMode_(true),
      offsetAngle_(0.0f),
      arcTextInfo_{0},
      needRefresh_(false),
      hasAnimator_(false),
      textSize_({0, 0}),
      radius_(0),
      startAngle_(0),
      endAngle_(0),
      arcCenter_({0, 0}),
      orientation_(TextOrientation::INSIDE)
{
    Theme* theme = ThemeManager::GetInstance().GetCurrent();
    style_ = (theme != nullptr) ? &(theme->GetLabelStyle()) : &(StyleDefault::GetLabelStyle());

    animator_.animator = nullptr;
    animator_.scrollListener = nullptr;
    animator_.speed = DEFAULT_ARC_LABEL_ANIMATOR_SPEED;
    animator_.rollCount = DEFAULT_ARC_LABEL_ROLL_COUNT;
    animator_.secondLapOffsetAngle_ = 0.0f;
}

UIArcLabel::~UIArcLabel()
{
    if (arcLabelText_ != nullptr) {
        delete arcLabelText_;
        arcLabelText_ = nullptr;
    }

    if (hasAnimator_) {
        delete animator_.animator;
        animator_.animator = nullptr;
        hasAnimator_ = false;
    }
}

void UIArcLabel::SetStyle(uint8_t key, int64_t value)
{
    UIView::SetStyle(key, value);
    RefreshArcLabel();
}

void UIArcLabel::SetText(const char* text)
{
    if (text == nullptr) {
        return;
    }
    InitArcLabelText();
    arcLabelText_->SetText(text);
    if (arcLabelText_->IsNeedRefresh()) {
        RefreshArcLabel();
    }
}

const char* UIArcLabel::GetText() const
{
    return (arcLabelText_ == nullptr) ? nullptr : arcLabelText_->GetText();
}

void UIArcLabel::SetAlign(UITextLanguageAlignment horizontalAlign)
{
    InitArcLabelText();
    arcLabelText_->SetAlign(horizontalAlign, TEXT_ALIGNMENT_TOP);
    if (arcLabelText_->IsNeedRefresh()) {
        RefreshArcLabel();
    }
}

UITextLanguageAlignment UIArcLabel::GetHorAlign()
{
    InitArcLabelText();
    return arcLabelText_->GetHorAlign();
}

UITextLanguageDirect UIArcLabel::GetDirect()
{
    InitArcLabelText();
    return arcLabelText_->GetDirect();
}

void UIArcLabel::SetFontId(uint16_t fontId)
{
    InitArcLabelText();
    arcLabelText_->SetFontId(fontId);
    if (arcLabelText_->IsNeedRefresh()) {
        RefreshArcLabel();
    }
}

uint16_t UIArcLabel::GetFontId()
{
    InitArcLabelText();
    return arcLabelText_->GetFontId();
}

void UIArcLabel::SetFont(const char* name, uint8_t size)
{
    if (name == nullptr) {
        return;
    }
    InitArcLabelText();
    arcLabelText_->SetFont(name, size);
    if (arcLabelText_->IsNeedRefresh()) {
        RefreshArcLabel();
    }
}

void UIArcLabel::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    InitArcLabelText();
    const char* text = arcLabelText_->GetText();
    if ((text == nullptr) || (radius_ == 0)) {
        return;
    }
    OpacityType opa = GetMixOpaScale();
    UIView::OnDraw(gfxDstBuffer, invalidatedArea);
    DrawArcText(gfxDstBuffer, invalidatedArea, opa, arcTextInfo_, orientation_);
}

void UIArcLabel::DrawArcText(BufferInfo& gfxDstBuffer,
                             const Rect& mask,
                             OpacityType opaScale,
                             ArcTextInfo arcTextInfo,
                             TextOrientation orientation)
{
    Point center;
    center.x = arcTextInfo_.arcCenter.x + GetRect().GetX();
    center.y = arcTextInfo_.arcCenter.y + GetRect().GetY();
    Rect temp = mask;
    if (compatibilityMode_ && hasAnimator_) {
        temp.SetLeft(center.x - radius_);
        temp.SetTop(center.y - radius_);
        temp.SetWidth(radius_ * 2); // 2 mean diameter
        temp.SetHeight(radius_ * 2);
    }
    arcTextInfo.hasAnimator = hasAnimator_;

    DrawLabel::DrawArcText(gfxDstBuffer, temp, arcLabelText_->GetText(), center, arcLabelText_->GetFontId(),
                           arcLabelText_->GetFontSize(), arcTextInfo, offsetAngle_,
                           orientation, *style_, opaScale, compatibilityMode_);
}

Rect UIArcLabel::GetArcTextRect(const char* text, uint16_t fontId, uint8_t fontSize, const Point& arcCenter,
                                int16_t letterSpace, TextOrientation orientation, const ArcTextInfo& arcTextInfo)
{
    return TypedText::GetArcTextRect(text, fontId, fontSize, arcCenter, letterSpace, orientation, arcTextInfo);
}

void UIArcLabel::RefreshArcLabel()
{
    Invalidate();
    if (!needRefresh_) {
        needRefresh_ = true;
    }
}

void UIArcLabel::ReMeasure()
{
    if (!needRefresh_) {
        return;
    }
    needRefresh_ = false;
    InitArcLabelText();

    MeasureArcTextInfo();
    arcTextInfo_.shapingFontId = arcLabelText_->GetShapingFontId();
    arcTextInfo_.codePoints = arcLabelText_->GetCodePoints();
    arcTextInfo_.codePointsNum = arcLabelText_->GetCodePointNum();
    Rect textRect =
        GetArcTextRect(arcLabelText_->GetText(), arcLabelText_->GetFontId(), arcLabelText_->GetFontSize(),
                       arcCenter_, style_->letterSpace_, orientation_, arcTextInfo_);
    int16_t arcTextWidth = textRect.GetWidth();
    int16_t arcTextHeight = textRect.GetHeight();

    if (compatibilityMode_) {
        SetPosition(textRect.GetX(), textRect.GetY());
        Resize(arcTextWidth, arcTextHeight);
    }

    arcTextInfo_.arcCenter.x = arcCenter_.x - GetX() + style_->borderWidth_ + style_->paddingLeft_;
    arcTextInfo_.arcCenter.y = arcCenter_.y - GetY() + style_->borderWidth_ + style_->paddingTop_;
    textSize_.x = arcTextWidth;
    textSize_.y = arcTextHeight;
    Invalidate();
}

uint32_t UIArcLabel::GetLineEnd(int16_t maxLength)
{
    const char* text = arcLabelText_->GetText();
    if (text == nullptr) {
        return 0;
    }

    return TypedText::GetNextLine(&text[arcTextInfo_.lineStart], arcLabelText_->GetFontId(),
        arcLabelText_->GetFontSize(), style_->letterSpace_, maxLength);
}

void UIArcLabel::MeasureArcTextInfo()
{
    const char* text = arcLabelText_->GetText();
    if (text == nullptr) {
        return;
    }
    uint16_t letterHeight = UIFont::GetInstance()->GetHeight(arcLabelText_->GetFontId(), arcLabelText_->GetFontSize());
    if (compatibilityMode_) {
        arcTextInfo_.radius = ((orientation_ == TextOrientation::INSIDE) ? radius_ : (radius_ - letterHeight));
    } else {
        arcTextInfo_.radius = radius_;
    }
    if (arcTextInfo_.radius == 0) {
        return;
    }

    uint16_t arcAngle;
    if (startAngle_ < endAngle_) {
        arcAngle = endAngle_ - startAngle_;
        arcTextInfo_.direct = TEXT_DIRECT_LTR; // Clockwise
        arcLabelText_->SetDirect(TEXT_DIRECT_LTR);
    } else {
        arcAngle = startAngle_ - endAngle_;
        arcTextInfo_.direct = TEXT_DIRECT_RTL; // Counterclockwise
        arcLabelText_->SetDirect(TEXT_DIRECT_RTL);
    }

    OnMeasureArcTextInfo(arcAngle, letterHeight);
}

void UIArcLabel::OnMeasureArcTextInfo(const uint16_t arcAngle, const uint16_t letterHeight)
{
    const char* text = arcLabelText_->GetText();
    if (text == nullptr) {
        return;
    }

    // calculate max arc length
    float maxLength = static_cast<float>((UI_PI * radius_ * arcAngle) / SEMICIRCLE_IN_DEGREE);
    arcTextInfo_.lineStart = 0;

    Rect rect;
    rect.SetWidth(static_cast<int16_t>(maxLength));
    arcLabelText_->ReMeasureTextSize(rect, *style_);

    arcTextInfo_.lineEnd = GetLineEnd(static_cast<int16_t>(maxLength));
    arcTextInfo_.startAngle = startAngle_ > CIRCLE_IN_DEGREE ? startAngle_ % CIRCLE_IN_DEGREE : startAngle_;
    arcTextInfo_.endAngle = endAngle_ > CIRCLE_IN_DEGREE ? endAngle_ % CIRCLE_IN_DEGREE : endAngle_;

    int16_t actLength = GetArcLength();
    if ((arcLabelText_->GetHorAlign() != TEXT_ALIGNMENT_LEFT) && (actLength < maxLength)) {
        float gapLength = maxLength - actLength;
        if (arcLabelText_->GetHorAlign() == TEXT_ALIGNMENT_CENTER) {
            gapLength = gapLength / 2; // 2: half
        }
        arcTextInfo_.startAngle += TypedText::GetAngleForArcLen(gapLength, letterHeight, arcTextInfo_.radius,
                                                                arcTextInfo_.direct, orientation_);
    }

    int16_t maxTextLength = arcLabelText_->GetMetaTextWidth(*style_);

    float maxTextAngle = 0.0f;
    if (compatibilityMode_) {
        maxTextAngle = TypedText::GetAngleForArcLen(maxTextLength, letterHeight, arcTextInfo_.radius,
            arcTextInfo_.direct, orientation_);
    } else {
        maxTextAngle = TypedText::GetAngleForArcLen(maxTextLength, style_->letterSpace_, arcTextInfo_.radius);
        maxTextAngle = arcLabelText_->GetDirect() == TEXT_DIRECT_RTL ? -maxTextAngle : maxTextAngle;
    }

    if (arcLabelText_->GetDirect() == TEXT_DIRECT_LTR) {
        animator_.secondLapOffsetAngle_ = -maxTextAngle;
    } else if (arcLabelText_->GetDirect() == TEXT_DIRECT_RTL) {
        animator_.secondLapOffsetAngle_ = maxTextAngle;
    }
}

uint16_t UIArcLabel::GetArcLength()
{
    const char* text = arcLabelText_->GetText();
    if (text == nullptr) {
        return 0;
    }

    return TypedText::GetTextWidth(&text[arcTextInfo_.lineStart], arcLabelText_->GetFontId(),
                                   arcLabelText_->GetFontSize(), (arcTextInfo_.lineEnd - arcTextInfo_.lineStart),
                                   style_->letterSpace_);
}

void UIArcLabel::Start()
{
    if (arcLabelText_->GetDirect() == TEXT_DIRECT_RTL) {
        arcLabelText_->SetAlign(TEXT_ALIGNMENT_RIGHT, TEXT_ALIGNMENT_CENTER);
    } else {
        arcLabelText_->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
    }
    if (hasAnimator_) {
        static_cast<ArcLabelAnimator*>(animator_.animator)->SetRollCount(animator_.rollCount);
    } else {
        ArcLabelAnimator* animator = new ArcLabelAnimator(animator_.rollCount, this);
        if (animator == nullptr) {
            GRAPHIC_LOGE("new ArcLabelAnimator fail");
            return;
        }
        animator->SetRollSpeed(animator_.speed);
        animator->RegisterScrollListener(animator_.scrollListener);
        animator_.animator = animator;
        hasAnimator_ = true;
    }
    animator_.animator->Start();
}

void UIArcLabel::Stop()
{
    if (hasAnimator_) {
        static_cast<ArcLabelAnimator*>(animator_.animator)->Stop();
    }
}

void UIArcLabel::SetRollCount(const uint16_t rollCount)
{
    if (hasAnimator_) {
        static_cast<ArcLabelAnimator*>(animator_.animator)->SetRollCount(rollCount);
    } else {
        animator_.rollCount = rollCount;
    }
}

void UIArcLabel::RegisterScrollListener(ArcLabelScrollListener* scrollListener)
{
    if (hasAnimator_) {
        static_cast<ArcLabelAnimator*>(animator_.animator)->RegisterScrollListener(scrollListener);
    } else {
        animator_.scrollListener = scrollListener;
    }
}

void UIArcLabel::SetRollSpeed(const uint16_t speed)
{
    if (hasAnimator_) {
        static_cast<ArcLabelAnimator*>(animator_.animator)->SetRollSpeed(speed);
    } else {
        animator_.speed = speed;
    }
}

uint16_t UIArcLabel::GetRollSpeed() const
{
    if (hasAnimator_) {
        return static_cast<ArcLabelAnimator*>(animator_.animator)->GetRollSpeed();
    }

    return animator_.speed;
}
} // namespace OHOS
