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

#include "animator/path_animator_callback.h"

#include <cmath>

#include "graphic_config.h"
#include "hal_tick.h"
#include "gfx_utils/graphic_log.h"
#include "gfx_utils/graphic_math.h"

namespace OHOS {
namespace {
constexpr uint16_t POINTS_MIN = 2;           // a valid path needs at least 2 points
constexpr int16_t RECT_PAD_BOTH_SIDES = 2;   // padding expands on both sides of the rect
constexpr float ROUNDING_OFFSET = 0.5f;      // half divisor for rounding/centering math

/* tangent angle (degrees) of the segment starting at segIdx;
   zero-length segments fall back to the nearest previous non-zero segment,
   and to 0 when every point coincides */
float SegmentTangentDeg(const PathPolyline& polyline, uint8_t segIdx)
{
    // reverse iteration without negative index: i = segIdx+1 down to 1, idx = i-1
    for (uint8_t i = segIdx + 1; i > 0; i--) {
        uint8_t idx = i - 1;
        if ((idx + 1) >= polyline.count) {
            continue;
        }
        float dx = static_cast<float>(polyline.x[idx + 1] - polyline.x[idx]);
        float dy = static_cast<float>(polyline.y[idx + 1] - polyline.y[idx]);
        if ((dx != 0.0f) || (dy != 0.0f)) {
            return std::atan2(dy, dx) * RADIAN_TO_ANGLE;
        }
    }
    return 0.0f;
}
} // namespace

PathAnimatorCallback::PathAnimatorCallback() = default;

void PathAnimatorCallback::SetDuration(uint32_t duration)
{
    durationMs_ = duration;
    ResetProgressState(); // reconfigure replays from the start and re-syncs the base
}

void PathAnimatorCallback::ResetProgressState()
{
    startTime_ = 0;      // replay from the start: next Callback records a new HALTick
    cycled_ = false;     // reset the end-of-cycle detection
    baseSynced_ = false; // re-sync the path base at the next execution frame
}

void PathAnimatorCallback::SetEasingFunc(EasingFunc easing)
{
    easing_ = (easing != nullptr) ? easing : EasingEquation::LinearEaseNone;
}

bool PathAnimatorCallback::SetPath(const Vector2<int16_t>* points, uint8_t count)
{
    if ((points == nullptr) || (count < POINTS_MIN)) {
        GRAPHIC_LOGE("PathAnimatorCallback SetPath invalid points");
        return false;
    }
    if (count > PathPolyline::MAX_POINTS) {
        GRAPHIC_LOGW("PathAnimatorCallback SetPath points exceed max limit, truncated");
    }
    uint8_t copyCount = (count > PathPolyline::MAX_POINTS) ? PathPolyline::MAX_POINTS : count;
    for (uint8_t i = 0; i < copyCount; i++) {
        polyline_.x[i] = points[i].x_;
        polyline_.y[i] = points[i].y_;
    }
    polyline_.count = copyCount;
    OffsetPathParser::BuildCumulativeLength(polyline_);
    mode_ = MODE_POLYLINE;
    ResetProgressState(); // path changed: replay from the first frame and re-sync the base
    return true;
}

bool PathAnimatorCallback::SetPath(const PathPolyline& polyline)
{
    if (polyline.count < POINTS_MIN) {
        GRAPHIC_LOGE("PathAnimatorCallback SetPath invalid polyline");
        return false;
    }
    polyline_ = polyline;
    mode_ = MODE_POLYLINE;
    ResetProgressState();
    return true;
}

bool PathAnimatorCallback::SetBezier(const Vector2<int16_t>& start, const Vector2<int16_t>& control,
    const Vector2<int16_t>& end)
{
    bezierStart_ = start;
    bezierCtrl_ = control;
    bezierEnd_ = end;
    mode_ = MODE_BEZIER;
    ResetProgressState();
    return true;
}

bool PathAnimatorCallback::SetPathString(const char* pathStr)
{
    if (!OffsetPathParser::ParseToPolyline(pathStr, polyline_)) {
        GRAPHIC_LOGE("PathAnimatorCallback SetPathString parse failed");
        return false;
    }
    mode_ = MODE_POLYLINE;
    ResetProgressState();
    return true;
}

void PathAnimatorCallback::SetFixedRotate(int16_t deg)
{
    rotateMode_ = (deg == 0) ? PATH_ROTATE_NONE : PATH_ROTATE_FIXED;
    rotateDeg_ = deg;
}

void PathAnimatorCallback::SetAutoRotate(int16_t offsetDeg)
{
    rotateMode_ = PATH_ROTATE_AUTO;
    rotateDeg_ = offsetDeg;
}

void PathAnimatorCallback::Callback(UIView* view)
{
    if ((view == nullptr) || (mode_ == MODE_NONE) || (durationMs_ == 0)) {
        return;
    }
    uint32_t now = HALTick::GetInstance().GetTime();
    if (startTime_ == 0) {
        startTime_ = now; // first frame records the start timestamp
    }
    uint32_t elapsed = now - startTime_;
    uint32_t current = elapsed;
    if (elapsed >= durationMs_) {
        if (!cycled_) {
            // first time we reach the end: clamp to the final frame.
            // for a non-repeating Animator this is the last Callback; for a repeating
            // Animator the next Callback will take the wrap branch below.
            current = durationMs_;
            cycled_ = true;
        } else {
            // repeating animation: wrap around. at exact cycle boundaries show the
            // end frame for one Callback, then continue from the start on the next.
            current = elapsed % durationMs_;
            if (current == 0) {
                current = durationMs_;
            }
        }
    }
    int16_t dist = easing_(0, PROGRESS_PERMILLE_MAX, static_cast<uint16_t>(current),
                           static_cast<uint16_t>(durationMs_));
    ApplyFrame(view, static_cast<uint16_t>(dist));
}

void PathAnimatorCallback::ApplyFrame(UIView* view, uint16_t distPermille)
{
    if (view == nullptr) {
        return;
    }
    if (!baseSynced_) {
        baseX_ = view->GetX(); // record the view position at the first frame as the path base
        baseY_ = view->GetY();
        baseSynced_ = true;
    }
    int16_t x = 0;
    int16_t y = 0;
    float tangentDeg = 0.0f;
    bool needTangent = (rotateMode_ == PATH_ROTATE_AUTO);
    if (CalculatePointPermille(distPermille, x, y, needTangent ? &tangentDeg : nullptr)) {
        UpdateViewPosition(view, x, y);
        ApplyPathRotate(view, tangentDeg);
    }
}

bool PathAnimatorCallback::CalculatePolylinePoint(const PathPolyline& polyline, uint16_t dist,
    int16_t& outX, int16_t& outY, uint16_t distMax, float* outTangentDeg)
{
    if (polyline.count < POINTS_MIN) {
        return false;
    }
    dist = (dist > distMax) ? distMax : dist;
    // arc-length conversion: target = totalLen * dist / distMax
    // totalLen (<= UINT16_MAX) * dist (<= 1000) fits in uint32
    uint32_t target = 0;
    if (distMax != 0) {
        target = static_cast<uint32_t>(polyline.totalLen) * dist / distMax;
    }

    uint8_t segIdx = 0;
    while (((segIdx + 1) < polyline.count) && (polyline.cumLen[segIdx + 1] < target)) {
        segIdx++;
    }
    if ((segIdx + 1) >= polyline.count) {
        outX = polyline.x[polyline.count - 1];
        outY = polyline.y[polyline.count - 1];
        if (outTangentDeg != nullptr) {
            *outTangentDeg = SegmentTangentDeg(polyline, polyline.count - POINTS_MIN);
        }
        return true;
    }
    uint16_t segStart = polyline.cumLen[segIdx];
    uint16_t segEnd = polyline.cumLen[segIdx + 1];
    float ratio = (segEnd > segStart)
                      ? static_cast<float>(target - segStart) / static_cast<float>(segEnd - segStart)
                      : 0.0f;
    outX = polyline.x[segIdx] +
           static_cast<int16_t>((polyline.x[segIdx + 1] - polyline.x[segIdx]) * ratio);
    outY = polyline.y[segIdx] +
           static_cast<int16_t>((polyline.y[segIdx + 1] - polyline.y[segIdx]) * ratio);
    if (outTangentDeg != nullptr) {
        *outTangentDeg = SegmentTangentDeg(polyline, segIdx);
    }
    return true;
}

bool PathAnimatorCallback::CalculatePoint(uint16_t dist, int16_t& outX, int16_t& outY, float* outTangentDeg) const
{
    dist = (dist > PROGRESS_MAX) ? PROGRESS_MAX : dist;
    // public API keeps the percent semantics (0~100); forward to the permille engine
    return CalculatePointPermille(static_cast<uint16_t>(dist * (PROGRESS_PERMILLE_MAX / PROGRESS_MAX)),
                                  outX, outY, outTangentDeg);
}

bool PathAnimatorCallback::CalculatePointPermille(uint16_t dist, int16_t& outX, int16_t& outY,
    float* outTangentDeg) const
{
    dist = (dist > PROGRESS_PERMILLE_MAX) ? PROGRESS_PERMILLE_MAX : dist;
    if (mode_ == MODE_POLYLINE) {
        return CalculatePolylinePoint(polyline_, dist, outX, outY, PROGRESS_PERMILLE_MAX, outTangentDeg);
    }
    if (mode_ == MODE_BEZIER) {
        // quadratic Bezier: B(t) = (1-t)^2*P0 + 2(1-t)t*Pc + t^2*P1, t = dist/1000
        float t = static_cast<float>(dist) / PROGRESS_PERMILLE_MAX;
        float u = 1.0f - t;
        // round to the nearest integer instead of truncating; branch on the sign so that
        // negative coordinates (e.g. an upward arched path with y < 0) are also rounded
        // correctly, aligning with the convention used in OffsetPathParser::AppendPoint.
        float rawX = u * u * bezierStart_.x_ + 2.0f * u * t * bezierCtrl_.x_ + t * t * bezierEnd_.x_;
        float rawY = u * u * bezierStart_.y_ + 2.0f * u * t * bezierCtrl_.y_ + t * t * bezierEnd_.y_;
        outX = static_cast<int16_t>(rawX + ((rawX >= 0) ? ROUNDING_OFFSET : -ROUNDING_OFFSET));
        outY = static_cast<int16_t>(rawY + ((rawY >= 0) ? ROUNDING_OFFSET : -ROUNDING_OFFSET));
        if (outTangentDeg != nullptr) {
            // derivative B'(t) = 2(1-t)(Pc-P0) + 2t(P1-Pc): the path direction at t
            float dx = 2.0f * u * (bezierCtrl_.x_ - bezierStart_.x_) +
                       2.0f * t * (bezierEnd_.x_ - bezierCtrl_.x_);
            float dy = 2.0f * u * (bezierCtrl_.y_ - bezierStart_.y_) +
                       2.0f * t * (bezierEnd_.y_ - bezierCtrl_.y_);
            *outTangentDeg = ((dx != 0.0f) || (dy != 0.0f)) ? (std::atan2(dy, dx) * RADIAN_TO_ANGLE) : 0.0f;
        }
        return true;
    }
    return false;
}

void PathAnimatorCallback::UpdateViewPosition(UIView* view, int16_t x, int16_t y)
{
    // split into two Invalidate calls to avoid Join() creating a large bounding box
    // that covers the whole path area (old + new positions may be far apart)
    Rect oldRect = view->GetRect();
    if (rotateMode_ != PATH_ROTATE_NONE) {
        ExpandRectForRotation(view, oldRect);
    }
    view->InvalidateRect(oldRect);

    view->SetPosition(baseX_ + x, baseY_ + y);

    Rect newRect = view->GetRect();
    if (rotateMode_ != PATH_ROTATE_NONE) {
        ExpandRectForRotation(view, newRect);
    }
    view->InvalidateRect(newRect);
}

void PathAnimatorCallback::ApplyPathRotate(UIView* view, float tangentDeg)
{
    if (rotateMode_ == PATH_ROTATE_NONE) {
        return;
    }
    int16_t angle = (rotateMode_ == PATH_ROTATE_FIXED)
                        ? rotateDeg_
                        : (static_cast<int16_t>(tangentDeg + ((tangentDeg >= 0) ? ROUNDING_OFFSET : -ROUNDING_OFFSET)) +
                           rotateDeg_);
    angle = ((angle % CIRCLE_IN_DEGREE) + CIRCLE_IN_DEGREE) % CIRCLE_IN_DEGREE; // normalize, negative-safe
    TransformMap transMap(view->GetOrigRect());
    Vector2<float> pivot((view->GetWidth() - 1) / 2.0f, (view->GetHeight() - 1) / 2.0f);
    transMap.Rotate(angle, pivot);
    view->SetTransformMap(transMap);
}

void PathAnimatorCallback::ExpandRectForRotation(UIView* view, Rect& rect) const
{
    float w = static_cast<float>(view->GetWidth());
    float h = static_cast<float>(view->GetHeight());
    float maxWH = (w > h) ? w : h;
    // half of (diagonal - max side), rounded up: corners of any rotation are covered
    int16_t pad = static_cast<int16_t>((std::sqrt(w * w + h * h) - maxWH) / 2.0f + ROUNDING_OFFSET) + 1;
    int16_t newX = rect.GetX() - pad;
    int16_t newY = rect.GetY() - pad;
    rect.SetRect(newX, newY, newX + rect.GetWidth() + RECT_PAD_BOTH_SIDES * pad - 1,
                 newY + rect.GetHeight() + RECT_PAD_BOTH_SIDES * pad - 1);
}

} // namespace OHOS
