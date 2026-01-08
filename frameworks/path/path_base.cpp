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

#include "path/path_base.h"
#include <float.h>

namespace OHOS {
PathBase* PathBase::pathBase_ = nullptr;

void PathBase::TranslateAngle(float& startAngle, float& endAngle)
{
    endAngle = QUARTER_IN_DEGREE - endAngle;
    startAngle = QUARTER_IN_DEGREE - startAngle;

    if (startAngle < endAngle) {
        endAngle -= CIRCLE_IN_DEGREE;
    }
}

void PathBase::CalcFirstHalfControlPoint(FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float radius, bool end)
{
    if (end) {
        p1.x = radius;
        p1.y = -radius * TANGENT;
        p2.x = -p1.y;
        p2.y = -p1.x;
        p3.x = 0;
        p3.y = -radius;
    } else {
        p1.x = -radius;
        p1.y = radius * TANGENT;
        p2.x = -p1.y;
        p2.y = -p1.x;
        p3.x = 0;
        p3.y = radius;
    }
}

void PathBase::CalcSecondHalfControlPoint(FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float radius, bool end)
{
    if (end) {
        p1.x = -radius * TANGENT;
        p1.y = -radius;
        p2.x = p1.y;
        p2.y = p1.x;
        p3.x = -radius;
        p3.y = 0;
    } else {
        p1.x = radius * TANGENT;
        p1.y = radius;
        p2.x = p1.y;
        p2.y = p1.x;
        p3.x = radius;
        p3.y = 0;
    }
}

CubicBezierPoints PathBase::CalcCubicBezierPoints(float radius, FloatPoint center, float startAngleDegree,
    float endAngleDegree)
{
    if (IsFloatEqual(startAngleDegree, endAngleDegree)) {
        CubicBezierPoints points;
        SetInvalidCubicBezierPoints(points);
        return points;
    }

    FloatPoint p0;
    float startAngle = startAngleDegree / SEMICIRCLE_IN_DEGREE * (float)UI_PI;
    p0.x = radius * cos(startAngle) + center.x;
    p0.y = -radius * sin(startAngle) + center.y;

    FloatPoint p3;
    float endAngle = endAngleDegree / SEMICIRCLE_IN_DEGREE * (float)UI_PI;
    p3.x = radius * cos(endAngle) + center.x;
    p3.y = -radius * sin(endAngle) + center.y;

    // 贝塞尔曲线控制点计算公式
    float deltaAngle = (endAngle - startAngle) / 2.0f; // 2.0:h计算公式取圆心角的一半
    float bezierRatio = 4 * (1 - cos(deltaAngle)) / (3 * sin(deltaAngle)); // 4 3:h计算公式
    float bezierControlRadius = sqrt(pow(radius, 2) + pow(bezierRatio * radius, 2)); // 2:平方运算

    float controlPointDeltaAngle = atan(bezierRatio);

    FloatPoint p1;
    float p1Angle = startAngle + controlPointDeltaAngle;
    p1.x = cos(p1Angle) * bezierControlRadius + center.x;
    p1.y = -sin(p1Angle) * bezierControlRadius + center.y;

    FloatPoint p2;
    float p2Angle = endAngle - controlPointDeltaAngle;
    p2.x = cos(p2Angle) * bezierControlRadius + center.x;
    p2.y = -sin(p2Angle) * bezierControlRadius + center.y;

    CubicBezierPoints result = { p0, p1, p2, p3 };
    return result;
}

CubicBezierPoints PathBase::CalcRoundedRectOuterPoint(const Rect& rect, float radius, float width)
{
    FloatPoint p0;
    p0.x = rect.GetRight() - radius + 1;
    p0.y = rect.GetTop();

    FloatPoint translate;
    translate.x = rect.GetRight() - radius + 1;
    translate.y = rect.GetTop() + radius - 1;

    FloatPoint p1;
    FloatPoint p2;
    FloatPoint p3;
    CalcSecondHalfControlPoint(p1, p2, p3, radius - 1, false);
    p1.x += translate.x;
    p2.x += translate.x;
    p3.x += translate.x;
    p1.y = translate.y - p1.y;
    p2.y = translate.y - p2.y;
    p3.y = translate.y - p3.y;

    CubicBezierPoints result = { p0, p1, p2, p3 };
    return result;
}

CubicBezierPoints PathBase::CalcRoundedRectInnerPoint(const Rect& rect, float radius, float width)
{
    FloatPoint p0;
    p0.x = rect.GetRight() - radius + 1;
    p0.y = rect.GetTop() + width;

    FloatPoint translate;
    translate.x = rect.GetRight() - radius + 1;
    translate.y = rect.GetTop() + radius - 1;

    FloatPoint p1;
    FloatPoint p2;
    FloatPoint p3;
    CalcSecondHalfControlPoint(p1, p2, p3, radius - width - 1, false);
    p1.x += translate.x;
    p2.x += translate.x;
    p3.x += translate.x;
    p1.y = translate.y - p1.y;
    p2.y = translate.y - p2.y;
    p3.y = translate.y - p3.y;

    CubicBezierPoints result = { p0, p1, p2, p3 };
    return result;  
}

void PathBase::VerticalMirror(FloatPoint& p0, FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float y)
{
    float mirrorY = MIRROR * y;
    p0.y = mirrorY - p0.y;
    p1.y = mirrorY - p1.y;
    p2.y = mirrorY - p2.y;
    p3.y = mirrorY - p3.y;
}

void PathBase::HorizionMirror(FloatPoint& p0, FloatPoint& p1, FloatPoint& p2, FloatPoint& p3, float x)
{
    float mirrorX = MIRROR * x;
    p0.x = mirrorX - p0.x;
    p1.x = mirrorX - p1.x;
    p2.x = mirrorX - p2.x;
    p3.x = mirrorX - p3.x;
}

void PathBase::SetInvalidCubicBezierPoints(CubicBezierPoints& points)
{
    points.p0 = { INVALID_COORD, INVALID_COORD };
    points.p1 = points.p0;
    points.p2 = points.p0;
    points.p3 = points.p0;
}

bool PathBase::IsFloatEqual(float x, float y)
{
    return (fabs(x - y) < 1e-6f);
}

bool PathBase::IsInvalidPoint(const FloatPoint& p0)
{
    return (IsFloatEqual(p0.x, INVALID_COORD) || IsFloatEqual(p0.y, INVALID_COORD));
}

bool PathBase::IsInvalidCurve(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2)
{
    return (IsInvalidPoint(p0) || IsInvalidPoint(p1) || IsInvalidPoint(p2));
}
} // namespace OHOS