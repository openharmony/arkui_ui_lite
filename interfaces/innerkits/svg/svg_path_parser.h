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

#ifndef GRAPHIC_LITE_SVG_PATH_PARSER_H
#define GRAPHIC_LITE_SVG_PATH_PARSER_H

#include "components/ui_canvas.h"
#include "svg/svg_types.h"
#include "gfx_utils/trans_affine.h"

namespace OHOS {

constexpr double SVG_PI = 3.14159265358979323846;

class SvgPathParser {
public:
    SvgPathParser() = default;

    ~SvgPathParser() = default;

    // Parses path data and records geometry commands on canvas. Paint application is
    // intentionally left to the caller (leaf nodes), keeping geometry separate from style.
    // Optional transform is baked into every recorded coordinate (control points, arc
    // endpoints, and reflected smooth points) so that the resulting vertices live in
    // the caller's target space.
    SvgResult Parse(const char* data, UICanvas& canvas, const TransAffine* transform = nullptr);

private:
    const char* data_ = nullptr;
};

// Samples the point at progress (0..1 by arc length) along the SVG path data d.
// Supports M/L/H/V/C/S/Q/T/Z; A is approximated as a straight line to its end point.
// angle returns the tangent direction in degrees, for rotate="auto" of <animateMotion>.
bool SvgPathPointAt(const char* d, float progress, float& x, float& y, float& angle);

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_PATH_PARSER_H
