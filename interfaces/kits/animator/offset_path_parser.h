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

#ifndef OFFSET_PATH_PARSER_H
#define OFFSET_PATH_PARSER_H

#include <stdint.h>

namespace OHOS {

/**
 * @brief Sampled polyline of an SVG path with a cumulative length table,
 *        used for arc-length positioning driven by offset-distance.
 *        Self-contained (no dependency on animation params), reusable by ui_lite.
 */
struct PathPolyline {
    /* max sampled polyline points (fixed-size storage to avoid heap allocation) */
    static constexpr uint8_t MAX_POINTS = 32;
    /* x/y of sampled polyline points, relative offset to the view's original position (px) */
    int16_t x[MAX_POINTS];
    int16_t y[MAX_POINTS];
    /* cumulative arc length of each point (px, monotonically increasing, the first is 0;
       saturated to UINT16_MAX when exceeding the uint16 range) */
    uint16_t cumLen[MAX_POINTS];
    /* total arc length of the path (px, saturated as above) */
    uint16_t totalLen;
    /* actual sampled point count, 0 means no path animation */
    uint8_t count;

    PathPolyline() : totalLen(0), count(0)
    {
        for (uint8_t i = 0; i < MAX_POINTS; i++) {
            x[i] = 0;
            y[i] = 0;
            cumLen[i] = 0;
        }
    }
};

/**
 * @brief Parser for CSS Motion Path strings in the form of path("...").
 *        Supported command subset: M (move to), L (line to), Q (quadratic Bezier),
 *        C (cubic Bezier) and Z (close path); both uppercase and lowercase commands
 *        are treated as absolute coordinates. Coordinates are plain numbers
 *        (an optional "px" suffix is tolerated).
 *        The output is a sampled polyline with a cumulative length table,
 *        used for arc-length positioning driven by offset-distance.
 */
class OffsetPathParser final {
public:
    /* static utility class, instantiation is not allowed */
    OffsetPathParser() = delete;

    /**
     * @brief Parse path("...") into a sampled polyline.
     *
     * @param pathStr input string, e.g. "path(\"M 0 0 Q 30 -80 60 0\")"
     * @param out     output polyline; points are thinned proportionally
     *                (first and last points are kept) when exceeding MAX_POINTS
     * @return true if parsing succeeds and out.count >= 2; false for invalid input
     */
    static bool ParseToPolyline(const char* pathStr, PathPolyline& out);

    /**
     * @brief Build the cumulative length table of a polyline in place
     *        (saturated to UINT16_MAX to keep monotonicity).
     *        used when the polyline points are filled by the caller directly
     *        (e.g. PathAnimatorCallback::SetPath with a point array).
     *
     * @param out polyline with x/y/count already filled; cumLen/totalLen are computed
     */
    static void BuildCumulativeLength(PathPolyline& out);
};

} // namespace OHOS

#endif // OFFSET_PATH_PARSER_H
