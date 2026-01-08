/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_PATH_BASE_H
#define GRAPHIC_LITE_PATH_BASE_H

#include <float.h>
#include "gfx_utils/graphic_type.h"
#include "gfx_utils/heap_base.h"
#include "gfx_utils/rect.h"

namespace OHOS {
enum PathCmd {
    PATH_DONE,
    PATH_CLOSE,
    MOVE_TO,
    MOVE_TO_REL,
    LINE_TO,
    LINE_TO_REL,
    QUAD_TO,
    QUAD_TO_REL,
    CUBIC_TO,
    CUBIC_TO_REL,
};

struct ArcPathInfo {
    FloatPoint center;
    float radius;
    float width;
    float startAngle;
    float endAngle;
};

struct CubicBezierPoints {
    FloatPoint p0;
    FloatPoint p1;
    FloatPoint p2;
    FloatPoint p3;
};

constexpr float TANGENT = 0.5522847498307933f;
constexpr float MIRROR = 2.0f;
constexpr int32_t HALF_SIZE = 2;
constexpr float INVALID_COORD = FLT_MAX;
constexpr float EDGE_RATIO = 70.0f;

class PathBase : public HeapBase {
public:
    virtual bool Reset() = 0;

    virtual void MoveTo(FloatPoint& p0) = 0;

    virtual void LineTo(FloatPoint& p0) = 0;

    virtual void AddLine(float x0, float y0, float x1, float y1) = 0;

    virtual void AddArc(ArcPathInfo& arcInfo, bool rounded) = 0;

    virtual void DoneOrClosePath(PathCmd cmd) = 0;

    void TranslateAngle(float& startAngle, float& endAngle);

    void CalcFirstHalfControlPoint(FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float radius, bool end);

    void CalcSecondHalfControlPoint(FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float radius, bool end);

    CubicBezierPoints CalcCubicBezierPoints(float radius, FloatPoint center,
        float startAngleDegree, float endAngleDegree);

    CubicBezierPoints CalcRoundedRectOuterPoint(const Rect& rect, float radius, float width);

    CubicBezierPoints CalcRoundedRectInnerPoint(const Rect& rect, float radius, float width);

    void VerticalMirror(FloatPoint& p0, FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float y);

    void HorizionMirror(FloatPoint& p0, FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float x);

    void SetInvalidCubicBezierPoints(CubicBezierPoints& points);

    bool InInvalidCurve(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2);

    static PathBase* GetInstance()
    {
        return pathBase_;
    }

    static void InitPathBase(PathBase* pathBase)
    {
        pathBase_ = pathBase;
    }

protected:
    bool IsFloatEqual(float x, float y);

    bool IsInvalidPoint(const FloatPoint& p0);

    static PathBase* pathBase_;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_PATH_BASE_H