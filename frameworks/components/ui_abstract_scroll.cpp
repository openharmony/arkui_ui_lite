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

#include "components/ui_abstract_scroll.h"

#include "securec.h"

#include "animator/interpolation.h"
#include "common/screen.h"
#include "components/ui_abstract_scroll_bar.h"
#include "components/ui_arc_scroll_bar.h"
#include "components/ui_box_scroll_bar.h"
#if GRAPHIC_ENABLE_SCROLL_FLAG
#include "gfx_utils/graphic_log.h"
#endif
#if DEFAULT_ANIMATION
#include "graphic_timer.h"
#endif

namespace OHOS {
#if DEFAULT_ANIMATION
class BarEaseInOutAnimator final : public AnimatorCallback {
public:
    BarEaseInOutAnimator() = delete;
    BarEaseInOutAnimator(const BarEaseInOutAnimator&) = delete;
    BarEaseInOutAnimator& operator=(const BarEaseInOutAnimator&) = delete;
    BarEaseInOutAnimator(BarEaseInOutAnimator&&) = delete;
    BarEaseInOutAnimator& operator=(BarEaseInOutAnimator&&) = delete;

    explicit BarEaseInOutAnimator(UIAbstractScroll& scrollView)
        : scrollView_(scrollView),
          timer_(APPEAR_PERIOD, TimerCb, this),
          animator_(this, nullptr, ANIMATOR_DURATION, false)
    {
    }

    ~BarEaseInOutAnimator()
    {
        timer_.Stop();
        animator_.Stop();
    }

    void RefreshBar()
    {
        if (animator_.GetState() == Animator::START) {
            if (!isEaseIn_) {
                animator_.SetRunTime(ANIMATOR_DURATION - animator_.GetRunTime());
            }
        } else if (scrollView_.yScrollBar_->GetOpacity() == OPA_TRANSPARENT) {
            animator_.Start();
        } else {
            timer_.Start(); // updates the start time of timer, ensuring that timer is triggered two seconds after the
                            // last operation
        }
        isEaseIn_ = true;
    }

private:
    void Callback(UIView* view) override
    {
        uint8_t opa = OPA_OPAQUE * animator_.GetRunTime() / ANIMATOR_DURATION;
        if (!isEaseIn_) {
            opa = OPA_OPAQUE - opa;
        }
        float bezielY = opa;
        bezielY =
            Interpolation::GetBezierY(bezielY / OPA_OPAQUE, BEZIER_CONTROL_POINT_X_1, 0, BEZIER_CONTROL_POINT_X_2, 1);
        opa = static_cast<uint8_t>(bezielY * opa);
        if (scrollView_.yScrollBarVisible_) {
            scrollView_.yScrollBar_->SetOpacity(opa);
        }
        if (scrollView_.xScrollBarVisible_) {
            scrollView_.xScrollBar_->SetOpacity(opa);
        }
        scrollView_.Invalidate();
    }

    void OnStop(UIView& view) override
    {
        if (isEaseIn_) {
            if (scrollView_.yScrollBarVisible_) {
                scrollView_.yScrollBar_->SetOpacity(OPA_OPAQUE);
            }
            if (Screen::GetInstance().GetScreenShape() == ScreenShape::RECTANGLE && scrollView_.xScrollBarVisible_) {
                scrollView_.xScrollBar_->SetOpacity(OPA_OPAQUE);
            }
            timer_.Start(); // The timer is triggered when animation stops.
        } else {
            if (scrollView_.yScrollBarVisible_) {
                scrollView_.yScrollBar_->SetOpacity(OPA_TRANSPARENT);
            }
            if (scrollView_.xScrollBarVisible_) {
                scrollView_.xScrollBar_->SetOpacity(OPA_TRANSPARENT);
            }
        }
        scrollView_.Invalidate();
    }

    static void TimerCb(void* arg)
    {
        BarEaseInOutAnimator* barAnimator = reinterpret_cast<BarEaseInOutAnimator*>(arg);
        barAnimator->isEaseIn_ = false;
        barAnimator->animator_.Start();
    }
    static constexpr uint16_t ANIMATOR_DURATION = 250;
    static constexpr uint16_t APPEAR_PERIOD = 2000;
    static constexpr float BEZIER_CONTROL_POINT_X_1 = 0.33f;
    static constexpr float BEZIER_CONTROL_POINT_X_2 = 0.67f;
    UIAbstractScroll& scrollView_;
    GraphicTimer timer_;
    Animator animator_;
    bool isEaseIn_ = true;
};
#endif

UIAbstractScroll::UIAbstractScroll()
    : direction_(VERTICAL),
      deltaIndex_(0),
      rotateIndex_(0),
      reserve_(0),
      easingFunc_(EasingEquation::CubicEaseOut),
      scrollAnimator_(&animatorCallback_, this, 0, true),
      scrollBarSide_(SCROLL_BAR_RIGHT_SIDE),
      scrollBarCenter_({0, 0}),
      scrollBarCenterSetFlag_(false)
{
#if defined(ENABLE_FOCUS_MANAGER) && ENABLE_FOCUS_MANAGER
    focusable_ = true;
#endif
#if defined(ENABLE_ROTATE_INPUT) && ENABLE_ROTATE_INPUT
    rotateFactor_ = DEFAULT_SCROLL_VIEW_ROTATE_FACTOR;
    rotateThrowthreshold_ = ABSTRACT_ROTATE_THROW_THRESHOLD;
    rotateAccCoefficient_ = ABSTRACT_ROTATE_DISTANCE_COEFF;
    isRotating_ = false;
#endif
    isViewGroup_ = true;
    touchable_ = true;
    draggable_ = true;
    dragParentInstead_ = false;
#if GRAPHIC_ENABLE_SCROLL_FLAG
    indicatorWidthSet_ = false;
    indicatorColorSet_ = false;
    indicatorBorderRadiusSet_ = false;
    indicatorMinLengthSet_ = false;
    indicatorOpacitySet_ = false;
    indicatorStyle_.width = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH;
    indicatorStyle_.minLength = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN;
    indicatorStyle_.borderRadius = 0;
    indicatorStyle_.color = StyleDefault::GetScrollBarForegroundStyle().bgColor_;
    indicatorStyle_.opacity = OPA_OPAQUE;
#endif
}

UIAbstractScroll::~UIAbstractScroll()
{
#if defined(DEFAULT_ANIMATION) && DEFAULT_ANIMATION
    if (barEaseInOutAnimator_ != nullptr) {
        delete barEaseInOutAnimator_;
        barEaseInOutAnimator_ = nullptr;
    }
#endif
    if (xScrollBar_ != nullptr) {
        delete xScrollBar_;
        xScrollBar_ = nullptr;
    }
    if (yScrollBar_ != nullptr) {
        delete yScrollBar_;
        yScrollBar_ = nullptr;
    }
}

void UIAbstractScroll::MoveChildByOffset(int16_t offsetX, int16_t offsetY)
{
    if ((offsetX == 0) && (offsetY == 0)) {
        return;
    }
    UIView* view = GetChildrenHead();
    while (view != nullptr) {
        int16_t x = view->GetX() + offsetX;
        int16_t y = view->GetY() + offsetY;
        view->SetPosition(x, y);
        view = view->GetNextSibling();
    }
    Invalidate();
}

int16_t UIAbstractScroll::GetMaxDelta() const
{
    int16_t result = 0;
    for (int16_t i = 0; i < MAX_DELTA_SIZE; i++) {
        if (result < MATH_ABS(lastDelta_[i])) {
            result = MATH_ABS(lastDelta_[i]);
        }
    }
    return result;
}

int16_t UIAbstractScroll::GetMaxRotate() const
{
    int16_t result = 0;
    for (int16_t i = 0; i < MAX_DELTA_SIZE; i++) {
        if (MATH_ABS(result) < MATH_ABS(lastRotate_[i])) {
            result = lastRotate_[i];
        }
    }
    return result;
}

void UIAbstractScroll::InitDelta()
{
    if (memset_s(lastDelta_, sizeof(lastDelta_), 0, sizeof(lastDelta_)) != EOK) {
        GRAPHIC_LOGE("memset_s error");
    }
}

void UIAbstractScroll::InitRotate()
{
    if (memset_s(lastRotate_, sizeof(lastRotate_), 0, sizeof(lastRotate_)) != EOK) {
        GRAPHIC_LOGE("memset_s error");
    }
}

void UIAbstractScroll::StopAnimator()
{
    scrollAnimator_.Stop();
    animatorCallback_.ResetCallback();
    isDragging_ = false;
}

bool UIAbstractScroll::DragThrowAnimator(Point currentPos, Point lastPos, uint8_t dragDirection, bool dragBack)
{
    if (!throwDrag_ && (reboundSize_ == 0)) {
        return false;
    }
    int16_t dragDistanceX = 0;
    int16_t dragDistanceY = 0;
    if (throwDrag_) {
        CalculateDragDistance(currentPos, lastPos, dragDirection, dragDistanceX, dragDistanceY);
    }
    if (reboundSize_ != 0) {
        CalculateReboundDistance(dragDistanceX, dragDistanceY);
    }

    if (!dragBack) {
        FixDistance(dragDistanceX, dragDistanceY);
    }

    StartAnimator(dragDistanceX, dragDistanceY);
    return true;
}

void UIAbstractScroll::StartAnimator(int16_t dragDistanceX, int16_t dragDistanceY)
{
    int16_t dragTimes = MATH_MAX(MATH_ABS(dragDistanceX), MATH_ABS(dragDistanceY)) / DRAG_TIMES_COEFFICIENT;
    if (dragTimes < MIN_DRAG_TIMES) {
        dragTimes = MIN_DRAG_TIMES;
    }
    animatorCallback_.ResetCallback();
    animatorCallback_.SetDragStartValue(0, 0);
    animatorCallback_.SetDragEndValue(dragDistanceX, dragDistanceY);
    animatorCallback_.SetDragTimes(dragTimes * DRAG_ACC_FACTOR / GetDragACCLevel());
    scrollAnimator_.Start();
}

void UIAbstractScroll::CalculateDragDistance(Point currentPos,
                                             Point lastPos,
                                             uint8_t dragDirection,
                                             int16_t& dragDistanceX,
                                             int16_t& dragDistanceY)
{
    if ((direction_ == VERTICAL) || (direction_ == HORIZONTAL_AND_VERTICAL)) {
        dragDistanceY = currentPos.y - lastPos.y;
        if (isRotating_) {
            dragDistanceY *= rotateAccCoefficient_;
        } else {
            dragDistanceY *= DRAG_DISTANCE_COEFFICIENT;
            if (dragDistanceY > 0 || (dragDistanceY == 0 && dragDirection == DragEvent::DIRECTION_TOP_TO_BOTTOM)) {
                dragDistanceY += GetMaxDelta() * GetSwipeACCLevel() / DRAG_ACC_FACTOR;
            } else if (dragDistanceY < 0 ||
                       (dragDistanceY == 0 && dragDirection == DragEvent::DIRECTION_BOTTOM_TO_TOP)) {
                dragDistanceY -= GetMaxDelta() * GetSwipeACCLevel() / DRAG_ACC_FACTOR;
            }
        }
    }

    if ((direction_ == HORIZONTAL) || (direction_ == HORIZONTAL_AND_VERTICAL)) {
        dragDistanceX = currentPos.x - lastPos.x;
        if (isRotating_) {
            dragDistanceX *= rotateAccCoefficient_;
        } else {
            dragDistanceX *= DRAG_DISTANCE_COEFFICIENT;
            if (dragDistanceX > 0 || (dragDistanceX == 0 && dragDirection == DragEvent::DIRECTION_LEFT_TO_RIGHT)) {
                dragDistanceX += GetMaxDelta() * GetSwipeACCLevel() / DRAG_ACC_FACTOR;
            } else if (dragDistanceX < 0 ||
                       (dragDistanceX == 0 && dragDirection == DragEvent::DIRECTION_RIGHT_TO_LEFT)) {
                dragDistanceX -= GetMaxDelta() * GetSwipeACCLevel() / DRAG_ACC_FACTOR;
            }
        }
    }

    if (maxScrollDistance_ != 0) {
        if (MATH_ABS(dragDistanceY) > maxScrollDistance_) {
            int16_t calculatedValue = (dragDistanceY > 0) ? 1 : -1;
            dragDistanceY = calculatedValue * maxScrollDistance_;
        }
        if (MATH_ABS(dragDistanceX) > maxScrollDistance_) {
            int16_t calculatedValue = (dragDistanceX > 0) ? 1 : -1;
            dragDistanceX = calculatedValue * maxScrollDistance_;
        }
    }
}

void UIAbstractScroll::ListAnimatorCallback::Callback(UIView* view)
{
    if (view == nullptr) {
        return;
    }

    UIAbstractScroll* scrollView = static_cast<UIAbstractScroll*>(view);
    scrollView->isDragging_ = true;
    curtTime_++;
    if (curtTime_ <= dragTimes_) {
        bool needStopX = false;
        bool needStopY = false;
        if (startValueY_ != endValueY_) {
            int16_t actY = scrollView->easingFunc_(startValueY_, endValueY_, curtTime_, dragTimes_);
            if (!scrollView->DragYInner(actY - previousValueY_)) {
                needStopY = true;
            }
            previousValueY_ = actY;
        } else {
            needStopY = true;
        }
        if (startValueX_ != endValueX_) {
            int16_t actX = scrollView->easingFunc_(startValueX_, endValueX_, curtTime_, dragTimes_);
            if (!scrollView->DragXInner(actX - previousValueX_)) {
                needStopX = true;
            }
            previousValueX_ = actX;
        } else {
            needStopX = true;
        }
        if (needStopX && needStopY) {
            scrollView->StopAnimator();
        }
    } else {
        scrollView->StopAnimator();
    }
}

#if ENABLE_ROTATE_INPUT
bool UIAbstractScroll::OnRotateStartEvent(const RotateEvent& event)
{
    isRotating_ = true;
    if (scrollAnimator_.GetState() != Animator::STOP) {
        UIAbstractScroll::StopAnimator();
    }
    return UIView::OnRotateStartEvent(event);
}

bool UIAbstractScroll::OnRotateEvent(const RotateEvent& event)
{
    int16_t rotateLen = static_cast<int16_t>(event.GetRotate() * rotateFactor_);
    RefreshRotate(rotateLen);
    if (direction_ == HORIZONTAL) {
        DragXInner(rotateLen);
    } else {
        DragYInner(rotateLen);
    }
    return UIView::OnRotateEvent(event);
}

bool UIAbstractScroll::OnRotateEndEvent(const RotateEvent& event)
{
    InitDelta();

    uint8_t dir;
    int16_t lastRotateLen = GetMaxRotate();
    if (direction_ == HORIZONTAL) {
        dir = (lastRotateLen >= 0) ? DragEvent::DIRECTION_LEFT_TO_RIGHT : DragEvent::DIRECTION_RIGHT_TO_LEFT;
    } else {
        dir = (lastRotateLen >= 0) ? DragEvent::DIRECTION_TOP_TO_BOTTOM : DragEvent::DIRECTION_BOTTOM_TO_TOP;
    }
    bool triggerAnimator = (MATH_ABS(lastRotateLen) >= rotateThrowthreshold_);
    if (throwDrag_ && triggerAnimator) {
        Point current;
        if (direction_ == HORIZONTAL) {
            current = {lastRotateLen, 0};
        } else {
            current = {0, lastRotateLen};
        }
        DragThrowAnimator(current, {0, 0}, dir, dragBack_);
    } else {
        DragThrowAnimator({0, 0}, {0, 0}, dir, dragBack_);
    }
    isRotating_ = false;
    InitRotate();
    return UIView::OnRotateEndEvent(event);
}
#endif

void UIAbstractScroll::SetXScrollBarVisible(bool visible)
{
    if (Screen::GetInstance().GetScreenShape() == ScreenShape::CIRCLE) {
        return;
    } else if (visible && xScrollBar_ == nullptr) {
        xScrollBar_ = new UIBoxScrollBar();
#if GRAPHIC_ENABLE_SCROLL_FLAG
        SyncIndicatorStyle(xScrollBar_);
#endif
    }
    xScrollBarVisible_ = visible;
#if DEFAULT_ANIMATION
    if (xScrollBarVisible_ && barEaseInOutAnimator_ == nullptr) {
        barEaseInOutAnimator_ = new BarEaseInOutAnimator(*this);
    }
#endif
}

void UIAbstractScroll::SetYScrollBarVisible(bool visible)
{
    yScrollBarVisible_ = visible;
    if (yScrollBarVisible_ && yScrollBar_ == nullptr) {
        if (Screen::GetInstance().GetScreenShape() == ScreenShape::CIRCLE) {
            yScrollBar_ = new UIArcScrollBar();
        } else {
            yScrollBar_ = new UIBoxScrollBar();
        }
#if GRAPHIC_ENABLE_SCROLL_FLAG
        SyncIndicatorStyle(yScrollBar_);
#endif
    }
#if DEFAULT_ANIMATION
    if (yScrollBarVisible_ && barEaseInOutAnimator_ == nullptr) {
        barEaseInOutAnimator_ = new BarEaseInOutAnimator(*this);
    }
#endif
}

#if GRAPHIC_ENABLE_SCROLL_FLAG
void UIAbstractScroll::RefreshScrollBarParams()
{
    if (!xScrollBarVisible_ && !yScrollBarVisible_) {
        return;
    }
    Rect childrenRect = GetAllChildRelativeRect();
    // use int32_t to avoid int16_t overflow when content is large
    // 2: two blank spaces on both sides
    int32_t totalLen = static_cast<int32_t>(childrenRect.GetHeight()) + 2 * scrollBlankSize_;
    int32_t len = GetHeight();
    if (yScrollBarVisible_ && (totalLen > 0)) {
        yScrollBar_->SetForegroundProportion(static_cast<float>(len) / totalLen);
        if (totalLen > len) {
            yScrollBar_->SetScrollProgress(static_cast<float>(scrollBlankSize_ - childrenRect.GetTop()) /
                                           (totalLen - len));
        }
    }
    if (xScrollBarVisible_) {
        // 2: two blank spaces on both sides
        totalLen = static_cast<int32_t>(childrenRect.GetWidth()) + 2 * scrollBlankSize_;
        len = GetWidth();
        if (totalLen > 0) {
            xScrollBar_->SetForegroundProportion(static_cast<float>(len) / totalLen);
            if (totalLen > len) {
                xScrollBar_->SetScrollProgress(static_cast<float>(scrollBlankSize_ - childrenRect.GetLeft()) /
                                               (totalLen - len));
            }
        }
    }
}

void UIAbstractScroll::DrawScrollBarOnRect(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea,
                                           const Rect& scrollRect, uint8_t opa)
{
    const int16_t barWidth = static_cast<int16_t>(GetScrollBarWidth());
    constexpr int16_t PIXEL_ALIGN_OFFSET = 1;
    if (yScrollBarVisible_) {
        if (scrollBarSide_ == SCROLL_BAR_RIGHT_SIDE) {
            yScrollBar_->SetPosition(scrollRect.GetRight() - barWidth + PIXEL_ALIGN_OFFSET,
                                     scrollRect.GetTop(), barWidth,
                                     scrollRect.GetHeight());
        } else {
            yScrollBar_->SetPosition(scrollRect.GetLeft(), scrollRect.GetTop(), barWidth, scrollRect.GetHeight());
        }
        yScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
    }
    if (xScrollBarVisible_) {
        if (scrollBarSide_ == SCROLL_BAR_RIGHT_SIDE) {
            xScrollBar_->SetPosition(scrollRect.GetLeft(),
                                     scrollRect.GetBottom() - barWidth + PIXEL_ALIGN_OFFSET,
                                     scrollRect.GetWidth() - barWidth, barWidth);
        } else {
            xScrollBar_->SetPosition(scrollRect.GetLeft() + barWidth,
                                     scrollRect.GetBottom() - barWidth + PIXEL_ALIGN_OFFSET,
                                     scrollRect.GetWidth() - barWidth, barWidth);
        }
        xScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
    }
}

void UIAbstractScroll::DrawScrollBarOnCircle(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea,
                                             const Rect& scrollRect, uint8_t opa)
{
    if (!yScrollBarVisible_) {
        return;
    }
    yScrollBar_->SetScrollBarSide(scrollBarSide_);
    int16_t x;
    int16_t y;
    if (scrollBarCenterSetFlag_) {
        x = scrollRect.GetX() + scrollBarCenter_.x;
        y = scrollRect.GetY() + scrollBarCenter_.y;
    } else {
        x = scrollRect.GetX() + (GetWidth() / 2);  // 2: half
        y = scrollRect.GetY() + (GetHeight() / 2); // 2: half
    }
    yScrollBar_->SetPosition(x, y, GetScrollBarWidth(), GetWidth() / 2); // 2: half
    yScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
}
#endif // GRAPHIC_ENABLE_SCROLL_FLAG

void UIAbstractScroll::OnPostDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    Rect scrollRect = GetRect();
    uint8_t opa = GetMixOpaScale();
#if GRAPHIC_ENABLE_SCROLL_FLAG
    DrawScrollBars(gfxDstBuffer, invalidatedArea, scrollRect, opa);
#else
    if (Screen::GetInstance().GetScreenShape() == ScreenShape::RECTANGLE) {
        if (yScrollBarVisible_) {
            if (scrollBarSide_ == SCROLL_BAR_RIGHT_SIDE) {
                yScrollBar_->SetPosition(scrollRect.GetRight() - SCROLL_BAR_WIDTH + 1, scrollRect.GetTop(),
                                         SCROLL_BAR_WIDTH, scrollRect.GetHeight());
            } else {
                yScrollBar_->SetPosition(scrollRect.GetLeft(), scrollRect.GetTop(), SCROLL_BAR_WIDTH,
                                         scrollRect.GetHeight());
            }
            yScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
        }
        if (xScrollBarVisible_) {
            if (scrollBarSide_ == SCROLL_BAR_RIGHT_SIDE) {
                xScrollBar_->SetPosition(scrollRect.GetLeft(), scrollRect.GetBottom() - SCROLL_BAR_WIDTH + 1,
                                         scrollRect.GetWidth() - SCROLL_BAR_WIDTH, SCROLL_BAR_WIDTH);
            } else {
                xScrollBar_->SetPosition(scrollRect.GetLeft() + SCROLL_BAR_WIDTH,
                                         scrollRect.GetBottom() - SCROLL_BAR_WIDTH + 1,
                                         scrollRect.GetWidth() - SCROLL_BAR_WIDTH, SCROLL_BAR_WIDTH);
            }
            xScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
        }
    } else {
        if (yScrollBarVisible_) {
            yScrollBar_->SetScrollBarSide(scrollBarSide_);
            int16_t x;
            int16_t y;
            if (scrollBarCenterSetFlag_) {
                x = scrollRect.GetX() + scrollBarCenter_.x;
                y = scrollRect.GetY() + scrollBarCenter_.y;
            } else {
                x = scrollRect.GetX() + (GetWidth() / 2);  // 2: half
                y = scrollRect.GetY() + (GetHeight() / 2); // 2: half
            }
            yScrollBar_->SetPosition(x, y, SCROLL_BAR_WIDTH, GetWidth() / 2); // 2: half
            yScrollBar_->OnDraw(gfxDstBuffer, invalidatedArea, opa);
        }
    }
#endif
    UIView::OnPostDraw(gfxDstBuffer, invalidatedArea);
}

void UIAbstractScroll::RefreshAnimator()
{
#if DEFAULT_ANIMATION
    barEaseInOutAnimator_->RefreshBar();
#endif
}

#if GRAPHIC_ENABLE_SCROLL_FLAG
void UIAbstractScroll::SetIndicatorStyle(const ScrollIndicatorStyle& style)
{
    indicatorStyle_ = style;
    indicatorWidthSet_ = true;
    indicatorColorSet_ = true;
    indicatorBorderRadiusSet_ = true;
    indicatorMinLengthSet_ = true;
    indicatorOpacitySet_ = true;
    if ((indicatorStyle_.width == 0) || (indicatorStyle_.width > INT16_MAX)) {
        GRAPHIC_LOGW("UIAbstractScroll::SetIndicatorStyle invalid width %u, restore default %u",
                     indicatorStyle_.width, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
        indicatorStyle_.width = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH;
    }
    if ((indicatorStyle_.minLength == 0) || (indicatorStyle_.minLength > INT16_MAX)) {
        GRAPHIC_LOGW("UIAbstractScroll::SetIndicatorStyle invalid minLength %u, restore default %u",
                     indicatorStyle_.minLength, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
        indicatorStyle_.minLength = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN;
    }
    if (xScrollBar_ != nullptr) {
        SyncIndicatorStyle(xScrollBar_);
    }
    if (yScrollBar_ != nullptr) {
        SyncIndicatorStyle(yScrollBar_);
    }
    Invalidate();
}

void UIAbstractScroll::ResetIndicatorStyle()
{
    indicatorWidthSet_ = false;
    indicatorColorSet_ = false;
    indicatorBorderRadiusSet_ = false;
    indicatorMinLengthSet_ = false;
    indicatorOpacitySet_ = false;
    indicatorStyle_.width = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH;
    indicatorStyle_.minLength = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN;
    indicatorStyle_.borderRadius = 0;
    indicatorStyle_.color = StyleDefault::GetScrollBarForegroundStyle().bgColor_;
    indicatorStyle_.opacity = OPA_OPAQUE;
    if (xScrollBar_ != nullptr) {
        xScrollBar_->ResetIndicatorStyle();
    }
    if (yScrollBar_ != nullptr) {
        yScrollBar_->ResetIndicatorStyle();
    }
    Invalidate();
}

void UIAbstractScroll::SetIndicatorWidth(uint16_t width)
{
    if ((width == 0) || (width > INT16_MAX)) {
        GRAPHIC_LOGW("UIAbstractScroll::SetIndicatorWidth invalid width %u, restore default %u",
                     width, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
        width = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH;
    }
    indicatorStyle_.width = width;
    indicatorWidthSet_ = true;
    SyncIndicatorStyle(xScrollBar_);
    SyncIndicatorStyle(yScrollBar_);
    Invalidate();
}

void UIAbstractScroll::SetIndicatorColor(ColorType color)
{
    indicatorStyle_.color = color;
    indicatorColorSet_ = true;
    SyncIndicatorStyle(xScrollBar_);
    SyncIndicatorStyle(yScrollBar_);
    Invalidate();
}

void UIAbstractScroll::SetIndicatorBorderRadius(uint16_t borderRadius)
{
    indicatorStyle_.borderRadius = borderRadius;
    indicatorBorderRadiusSet_ = true;
    SyncIndicatorStyle(xScrollBar_);
    SyncIndicatorStyle(yScrollBar_);
    Invalidate();
}

void UIAbstractScroll::SetIndicatorMinLength(uint16_t minLength)
{
    if ((minLength == 0) || (minLength > INT16_MAX)) {
        GRAPHIC_LOGW("UIAbstractScroll::SetIndicatorMinLength invalid minLength %u, restore default %u",
                     minLength, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
        minLength = UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN;
    }
    indicatorStyle_.minLength = minLength;
    indicatorMinLengthSet_ = true;
    SyncIndicatorStyle(xScrollBar_);
    SyncIndicatorStyle(yScrollBar_);
    Invalidate();
}

void UIAbstractScroll::SetIndicatorOpacity(uint8_t opacity)
{
    indicatorStyle_.opacity = opacity;
    indicatorOpacitySet_ = true;
    SyncIndicatorStyle(xScrollBar_);
    SyncIndicatorStyle(yScrollBar_);
    Invalidate();
}

void UIAbstractScroll::SyncIndicatorStyle(UIAbstractScrollBar* scrollBar)
{
    if (scrollBar == nullptr) {
        return;
    }
    scrollBar->ResetIndicatorStyle();
    if (indicatorWidthSet_) {
        scrollBar->SetIndicatorWidth(indicatorStyle_.width);
    }
    if (indicatorColorSet_) {
        scrollBar->SetIndicatorColor(indicatorStyle_.color);
    }
    if (indicatorBorderRadiusSet_) {
        scrollBar->SetIndicatorBorderRadius(indicatorStyle_.borderRadius);
    }
    if (indicatorMinLengthSet_) {
        scrollBar->SetIndicatorMinLength(indicatorStyle_.minLength);
    }
    if (indicatorOpacitySet_) {
        scrollBar->SetIndicatorOpacity(indicatorStyle_.opacity);
    }
}

uint16_t UIAbstractScroll::GetScrollBarWidth() const
{
    return indicatorWidthSet_ ? indicatorStyle_.width : UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH;
}

void UIAbstractScroll::DrawScrollBars(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea,
                                      const Rect& scrollRect, uint8_t opa)
{
    // refresh scrollbar params by content size, so the slider length is correct before the first scroll.
    // Do not call RefreshAnimator here to avoid triggering the fade-in/out animation at startup.
    RefreshScrollBarParams();
    if (Screen::GetInstance().GetScreenShape() == ScreenShape::RECTANGLE) {
        DrawScrollBarOnRect(gfxDstBuffer, invalidatedArea, scrollRect, opa);
    } else {
        DrawScrollBarOnCircle(gfxDstBuffer, invalidatedArea, scrollRect, opa);
    }
}
#endif // GRAPHIC_ENABLE_SCROLL_FLAG
} // namespace OHOS
