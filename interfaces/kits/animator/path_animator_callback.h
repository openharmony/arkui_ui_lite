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

#ifndef PATH_ANIMATOR_CALLBACK_H
#define PATH_ANIMATOR_CALLBACK_H

#include "animator/animator.h"
#include "animator/easing_equation.h"
#include "animator/offset_path_parser.h"
#include "components/ui_view.h"
#include "gfx_utils/graphic_math.h"

namespace OHOS {

/**
 * @brief AnimatorCallback that produces path-animation effects on a UIView:
 *        moves the view along a preset path (polyline / quadratic Bezier) with
 *        configurable duration and easing, plus optional rotation.
 *
 *        Single responsibility: everything needed to PRODUCE the effect inside
 *        Callback(view) — path data, progress state (frame counting), path base,
 *        easing, positioning and rotation. The timeline (Start/Stop/Pause/repeat/
 *        runTime) belongs to Animator, which drives this callback strictly one-way:
 *
 *        PathAnimatorCallback cb;
 *        cb.SetDuration(2000);
 *        cb.SetPathString("path(\"M 0 0 L 240 0\")");
 *        Animator animator(&cb, target, 2000, true); // injects and drives the callback
 *        animator.Start();
 *
 *        The callback is self-contained: each Callback() call advances exactly one
 *        frame (the 16ms task beat), so tests can drive it directly without any
 *        Animator — construct, configure, then call Callback(&view) frame by frame.
 */
class PathAnimatorCallback final : public AnimatorCallback {
public:
    PathAnimatorCallback();

    ~PathAnimatorCallback() override = default;

    /* non-copyable and non-movable: the effect state machine is instance-bound */
    PathAnimatorCallback(const PathAnimatorCallback&) = delete;
    PathAnimatorCallback& operator=(const PathAnimatorCallback&) = delete;
    PathAnimatorCallback(PathAnimatorCallback&&) = delete;
    PathAnimatorCallback& operator=(PathAnimatorCallback&&) = delete;

    /**
     * @brief Sets the effect duration in milliseconds. It is the denominator of
     *        easing progress evaluation, NOT the period of any Animator — when
     *        driven by an Animator, pass the same duration to the Animator
     *        constructor. 0 means the animation never advances (Callback is a no-op).
     */
    void SetDuration(uint32_t duration);

    /**
     * @brief Sets the easing function applied to the whole-path progress,
     *        e.g. EasingEquation::LinearEaseNone (default), QuadEaseInOut.
     *        nullptr falls back to LinearEaseNone.
     */
    void SetEasingFunc(EasingFunc easing);

    /**
     * @brief Sets a polyline path by a point array (relative offsets to the view's
     *        original position, px). Overwrites the previous path input and resets
     *        the state machine (replays from the first frame on the next Callback).
     *
     * @param points point array (at least 2 points)
     * @param count  point count, truncated to PathPolyline::MAX_POINTS
     * @return true if the path is valid; false otherwise (nullptr or count < 2)
     */
    bool SetPath(const Vector2<int16_t>* points, uint8_t count);

    /**
     * @brief Sets a polyline path directly by a PathPolyline
     *        (e.g. the output of OffsetPathParser::ParseToPolyline).
     *        Resets the state machine as well.
     * @return true if the polyline is valid (count >= 2); false otherwise.
     */
    bool SetPath(const PathPolyline& polyline);

    /**
     * @brief Sets a quadratic Bezier path by three points (relative offsets, px).
     *        Evaluated by the quadratic Bezier formula at runtime. Resets the state machine.
     */
    bool SetBezier(const Vector2<int16_t>& start, const Vector2<int16_t>& control,
        const Vector2<int16_t>& end);

    /**
     * @brief Sets a path by an SVG path string, e.g. path("M 0 0 Q 30 -80 60 0").
     *        Supported command subset: M/L/Q/C/Z; Bezier commands are sampled
     *        into a polyline (8 points per segment). Resets the state machine.
     * @return true if parsing succeeds; false for invalid input.
     */
    bool SetPathString(const char* pathStr);

    /* rotate modes of the target view (W3C offset-rotate alike) */
    enum PathRotateMode : uint8_t {
        PATH_ROTATE_NONE = 0, // no rotation (default)
        PATH_ROTATE_FIXED,    // constant angle in degrees
        PATH_ROTATE_AUTO      // follow the path tangent direction, plus an offset angle
    };

    /**
     * @brief Sets a constant rotation angle (degrees, clockwise) applied to the
     *        target view inside Callback. 0 resets to PATH_ROTATE_NONE.
     */
    void SetFixedRotate(int16_t deg);

    /**
     * @brief Sets the target view to follow the path tangent direction inside
     *        Callback, plus an additive offset angle (degrees).
     *        SetAutoRotate(0) equals CSS "offset-rotate: auto";
     *        SetAutoRotate(180) equals "offset-rotate: reverse".
     */
    void SetAutoRotate(int16_t offsetDeg);

    /**
     * @brief Calculates the point on the path by progress.
     *        For polyline mode the point is located by arc-length ratio;
     *        for Bezier mode the progress is used as the formula parameter.
     *
     * @param dist progress in 0~100 (clamped)
     * @param outX output x (relative offset, px)
     * @param outY output y (relative offset, px)
     * @param outTangentDeg optional output: path direction at the point (degrees;
     *        polyline: segment atan2; Bezier: derivative B'(t) atan2; nullptr to skip)
     * @return true if a valid path mode is configured and the point is calculated.
     */
    bool CalculatePoint(uint16_t dist, int16_t& outX, int16_t& outY, float* outTangentDeg = nullptr) const;

    /**
     * @brief Frame callback invoked by the driving Animator (or directly by tests).
     *        Advances the state machine by exactly one frame: eases the progress,
     *        then applies it via ApplyFrame.
     *        NOTE: framework-invoked callback — do NOT call it directly from
     *        application code (tests may drive it frame by frame).
     *
     * @param view target view provided by the driving Animator; may be nullptr,
     *             in which case the callback is a no-op. ApplyFrame is only
     *             reached when view is non-null.
     */
    void Callback(UIView* view) override;

    /**
     * @brief Applies ONE frame of the path effect at the given progress — the single
     *        effect-execution entry shared by all driving modes:
     *        syncs the path base from the view's current position on the first call,
     *        locates the path point at distPermille, moves the view to base + point,
     *        applies the configured rotation, and invalidates the affected area.
     *
     *        Two driving modes
     *        - self-contained: Callback() uses HALTick to compute elapsed time,
     *          eases it into a progress permille, and calls ApplyFrame;
     *        - external progress: an outer animation system (e.g. ace TransitionImpl
     *          with CSS easing/iterations/fill semantics) computes the progress and
     *          calls ApplyFrame directly. ApplyFrame does NOT touch durationMs_ or
     *          startTime_, so no SetDuration is required in this mode.
     *
     * @param view         target view to update (non-null)
     * @param distPermille progress in 0~1000 (clamped by CalculatePointPermille)
     */
    void ApplyFrame(UIView* view, uint16_t distPermille);

private:
    /* path input mode */
    enum PathMode : uint8_t {
        MODE_NONE = 0,
        MODE_POLYLINE, // polyline (point array / PathPolyline / SVG path string)
        MODE_BEZIER    // quadratic Bezier (formula evaluation)
    };

    /* move the view to base + (x, y) and invalidate the joined area
       (expanded when rotation applies) */
    void UpdateViewPosition(UIView* view, int16_t x, int16_t y);
    /* reset the progress state machine (timing/base): called by every
       Set* configuration so reconfiguring replays from the start */
    void ResetProgressState();
    /* apply the configured rotate mode to the target view
       (tangentDeg is used by PATH_ROTATE_AUTO); no-op for PATH_ROTATE_NONE */
    void ApplyPathRotate(UIView* view, float tangentDeg);
    /* expand the invalidated rect so the rotated bounding box corners are covered */
    void ExpandRectForRotation(UIView* view, Rect& rect) const;

    static constexpr uint16_t PROGRESS_MAX = 100;           // progress percentage upper limit
    static constexpr uint16_t PROGRESS_PERMILLE_MAX = 1000; // permille progress upper limit

    /**
     * @brief Permille form of CalculatePoint: finer progress resolution (0~1000),
     *        avoiding visible position steps caused by percent-level quantization.
     *        The public CalculatePoint (0~100) forwards to it with dist * 10.
     */
    bool CalculatePointPermille(uint16_t distPermille, int16_t& outX, int16_t& outY,
        float* outTangentDeg = nullptr) const;

    /**
     * @brief Calculates the point on a polyline by progress (arc-length positioning),
     *        static form that works without an instance.
     *
     * @param polyline polyline with x/y/count/cumLen/totalLen (count >= 2 required)
     * @param dist     progress in 0~distMax (clamped to distMax)
     * @param outX     output x (relative offset, px)
     * @param outY     output y (relative offset, px)
     * @param distMax  progress upper limit of dist semantics: 100 (percent, default)
     *                 or 1000 (permille, for smooth positioning)
     * @param outTangentDeg optional output: tangent angle of the segment at the
     *                 calculated point (degrees; zero-length segments fall back to
     *                 the nearest previous non-zero segment; nullptr to skip)
     * @return true if the polyline is valid (count >= 2) and the point is calculated.
     */
    static bool CalculatePolylinePoint(const PathPolyline& polyline, uint16_t dist,
        int16_t& outX, int16_t& outY, uint16_t distMax = 100, float* outTangentDeg = nullptr);

    /* state machine — timing */
    uint32_t durationMs_ = 0;   // effect duration in milliseconds (0 = never advances)
    uint32_t startTime_ = 0;    // HALTick timestamp recorded on the first Callback (0 = not started)
    bool cycled_ = false;   // true once elapsed >= durationMs_, used to wrap repeating animations
    /* state machine — effect */
    EasingFunc easing_ = EasingEquation::LinearEaseNone; // easing function, default LinearEaseNone
    /* state machine — base */
    bool baseSynced_ = false;   // false until the first Callback records the path base
    int16_t baseX_ = 0;         // view position recorded at the first Callback as the path base
    int16_t baseY_ = 0;
    /* state machine — path */
    PathMode mode_ = MODE_NONE;
    PathPolyline polyline_;          // polyline path (with cumulative lengths)
    Vector2<int16_t> bezierStart_;   // quadratic Bezier points
    Vector2<int16_t> bezierCtrl_;
    Vector2<int16_t> bezierEnd_;
    /* state machine — rotation */
    PathRotateMode rotateMode_ = PATH_ROTATE_NONE; // rotation behavior of the target view
    int16_t rotateDeg_ = 0;         // FIXED: constant angle; AUTO: additive offset angle (deg)
};

} // namespace OHOS

#endif // PATH_ANIMATOR_CALLBACK_H
