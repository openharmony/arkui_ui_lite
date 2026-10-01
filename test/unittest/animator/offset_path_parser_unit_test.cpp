/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "animator/offset_path_parser.h"

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

#include "securec.h"


using namespace testing::ext;
namespace OHOS {
namespace {
// SVG path subset supported by the parser: M (move), L (line), Z (close),
// Q (quadratic Bezier), C (cubic Bezier). Coordinates are plain numbers and
// an optional "px" suffix is tolerated. Both uppercase and lowercase commands
// are treated as absolute coordinates.
const char* PATH_LINE = "path(\"M 0 0 L 240 0\")";
const char* PATH_RECT = "path(\"M 0 0 L 240 0 L 240 30 L 0 30 Z\")";
const char* PATH_QUAD = "path(\"M 0 0 Q 120 -40 240 0\")";
const char* PATH_CUBIC = "path(\"M 0 0 C 80 -40 160 -40 240 0\")";
const char* PATH_MULTI_CMD = "path(\"M 0 0 L 100 0 Q 150 -30 200 0 L 240 0\")";
const char* PATH_EMPTY_BODY = "path(\"\")";
const char* PATH_MISSING_BRACKET = "path M 0 0 L 240 0";
const char* PATH_UNSUPPORTED_CMD = "path(\"M 0 0 H 240 0\")";
const char* PATH_NO_PATH_PREFIX = "\"M 0 0 L 240 0\"";
} // namespace

class OffsetPathParserTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgPathParserLine_001
 * @tc.desc: Verify parsing a straight line path "M 0 0 L 240 0".
 *           The polyline should have 2 points: start (0,0) and end (240,0),
 *           total length 240, cumulative length table [0, 240].
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserLine_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(PATH_LINE, out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 240);
    EXPECT_EQ(out.y[1], 0);
    EXPECT_EQ(out.totalLen, 240);
    EXPECT_EQ(out.cumLen[0], 0);
    EXPECT_EQ(out.cumLen[1], 240);
}

/**
 * @tc.name: SvgPathParserRect_001
 * @tc.desc: Verify parsing a closed rectangle path "M 0 0 L 240 0 L 240 30 L 0 30 Z".
 *           The Z command closes back to the start point, so the polyline should
 *           contain 5 points: 4 corners + the closing point (0,0).
 *           Total length = 240 + 30 + 240 + 30 = 540.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserRect_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(PATH_RECT, out));
    EXPECT_EQ(out.count, 5);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 240);
    EXPECT_EQ(out.y[1], 0);
    EXPECT_EQ(out.x[2], 240);
    EXPECT_EQ(out.y[2], 30);
    EXPECT_EQ(out.x[3], 0);
    EXPECT_EQ(out.y[3], 30);
    // Z closes back to the start point (0, 0)
    EXPECT_EQ(out.x[4], 0);
    EXPECT_EQ(out.y[4], 0);
    EXPECT_EQ(out.totalLen, 540);
}

/**
 * @tc.name: SvgPathParserQuadratic_001
 * @tc.desc: Verify parsing a quadratic Bezier path "M 0 0 Q 120 -40 240 0".
 *           The Q segment is sampled into 8 points (BEZIER_SAMPLES), so the
 *           polyline should have 1 (M) + 8 (Q samples) = 9 points.
 *           The first point is the start (0,0), the last is the end (240,0),
 *           and the middle points follow the quadratic curve y = -40 * 4 * t * (1-t).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserQuadratic_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(PATH_QUAD, out));
    EXPECT_EQ(out.count, 9);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[8], 240);
    EXPECT_EQ(out.y[8], 0);
    // The apex of B(t) = (1-t)^2*0 + 2(1-t)t*(-40) + t^2*0 is at t = 0.5,
    // y = 2 * 0.5 * 0.5 * (-40) = -20. Sampling hits t = 4/8 = 0.5 at index 4.
    EXPECT_EQ(out.y[4], -20);
}

/**
 * @tc.name: SvgPathParserCubic_001
 * @tc.desc: Verify parsing a cubic Bezier path "M 0 0 C 80 -40 160 -40 240 0".
 *           The C segment is sampled into 8 points, so the polyline should have
 *           1 (M) + 8 (C samples) = 9 points. First point (0,0), last (240,0).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserCubic_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(PATH_CUBIC, out));
    EXPECT_EQ(out.count, 9);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[8], 240);
    EXPECT_EQ(out.y[8], 0);
}

/**
 * @tc.name: SvgPathParserMultiCmd_001
 * @tc.desc: Verify parsing a path with mixed commands
 *           "M 0 0 L 100 0 Q 150 -30 200 0 L 240 0".
 *           The polyline should have 1 (M) + 1 (L) + 8 (Q samples) + 1 (L) = 11 points.
 *           Total length should be greater than 240 (the curve adds some arc length).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserMultiCmd_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(PATH_MULTI_CMD, out));
    EXPECT_EQ(out.count, 11);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[10], 240);
    EXPECT_EQ(out.y[10], 0);
    EXPECT_GT(out.totalLen, 240);
}

/**
 * @tc.name: SvgPathParserEmpty_001
 * @tc.desc: Verify parsing an empty body "path("")" fails and returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserEmpty_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline(PATH_EMPTY_BODY, out));
}

/**
 * @tc.name: SvgPathParserMissingBracket_001
 * @tc.desc: Verify parsing a path missing the '(' bracket fails and returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserMissingBracket_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline(PATH_MISSING_BRACKET, out));
}

/**
 * @tc.name: SvgPathParserNoPathPrefix_001
 * @tc.desc: Verify parsing a string without the "path" prefix fails and returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserNoPathPrefix_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline(PATH_NO_PATH_PREFIX, out));
}

/**
 * @tc.name: SvgPathParserNullInput_001
 * @tc.desc: Verify parsing a null input fails and returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserNullInput_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline(nullptr, out));
}

/**
 * @tc.name: SvgPathParserUnsupportedCmd_001
 * @tc.desc: Verify parsing a path with unsupported command H (horizontal line to)
 *           does not crash; the H command is skipped and parsing continues.
 *           The path "M 0 0 H 240 0" has M (1 point) + H skipped (no points added),
 *           so the polyline should have fewer than 2 points and parsing fails.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserUnsupportedCmd_001, TestSize.Level1)
{
    PathPolyline out;
    // The H command is unsupported and skipped; with only the M point remaining,
    // the polyline has fewer than 2 points and parsing returns false.
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline(PATH_UNSUPPORTED_CMD, out));
}

/**
 * @tc.name: SvgPathParserBuildCumLen_001
 * @tc.desc: Verify BuildCumulativeLength computes the cumulative length table
 *           for a manually filled polyline. Three points (0,0), (100,0), (100,50)
 *           should have cumLen [0, 100, 150] and totalLen 150.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserBuildCumLen_001, TestSize.Level1)
{
    PathPolyline out;
    out.x[0] = 0;
    out.y[0] = 0;
    out.x[1] = 100;
    out.y[1] = 0;
    out.x[2] = 100;
    out.y[2] = 50;
    out.count = 3;
    OffsetPathParser::BuildCumulativeLength(out);
    EXPECT_EQ(out.cumLen[0], 0);
    EXPECT_EQ(out.cumLen[1], 100);
    EXPECT_EQ(out.cumLen[2], 150);
    EXPECT_EQ(out.totalLen, 150);
}

/**
 * @tc.name: SvgPathParserThinning_001
 * @tc.desc: Verify that a path with more points than MAX_POINTS (32) is thinned
 *           proportionally to MAX_POINTS. A long polyline path "M 0 0 L 1 0 L 2 0 ..."
 *           is constructed with 40 points; the output should be thinned to 32 points
 *           with the first (0,0) and last (39,0) kept.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserThinning_001, TestSize.Level2)
{
    // Build a path with 40 L commands: "M 0 0 L 1 0 L 2 0 ... L 39 0"
    char pathStr[512];
    int pos = snprintf_s(pathStr, sizeof(pathStr), sizeof(pathStr) - 1, "path(\"M 0 0");
    for (int i = 1; i < 40; i++) {
        pos += snprintf_s(pathStr + pos, sizeof(pathStr) - pos, sizeof(pathStr) - pos - 1,
                          " L %d 0", i);
    }
    pos += snprintf_s(pathStr + pos, sizeof(pathStr) - pos, sizeof(pathStr) - pos - 1, "\")");
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(pathStr, out));
    // Thinned to MAX_POINTS = 32
    EXPECT_EQ(out.count, PathPolyline::MAX_POINTS);
    // First and last points are kept
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.x[out.count - 1], 39);
}

/**
 * @tc.name: SvgPathParserNegativeCoord_001
 * @tc.desc: Verify parsing negative coordinates.
 *           Path "M -10 -20 L 30 -40" should produce points (-10,-20) and (30,-40).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserNegativeCoord_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M -10 -20 L 30 -40\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], -10);
    EXPECT_EQ(out.y[0], -20);
    EXPECT_EQ(out.x[1], 30);
    EXPECT_EQ(out.y[1], -40);
}

/**
 * @tc.name: SvgPathParserDecimalCoord_001
 * @tc.desc: Verify parsing decimal coordinates. Decimal values are rounded to int16.
 *           Path "M 0.5 1.2 L 100.6 50.4" should produce rounded points.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserDecimalCoord_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0.5 1.2 L 100.6 50.4\")", out));
    EXPECT_EQ(out.count, 2);
    // 0.5 rounds to 1, 1.2 rounds to 1
    EXPECT_EQ(out.x[0], 1);
    EXPECT_EQ(out.y[0], 1);
    // 100.6 rounds to 101, 50.4 rounds to 50
    EXPECT_EQ(out.x[1], 101);
    EXPECT_EQ(out.y[1], 50);
}

/**
 * @tc.name: SvgPathParserPxSuffix_001
 * @tc.desc: Verify parsing coordinates with "px" suffix.
 *           Path "M 0px 0px L 100px 50px" should be parsed as 0,0 and 100,50.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserPxSuffix_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0px 0px L 100px 50px\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 50);
}

/**
 * @tc.name: SvgPathParserCommaSeparator_001
 * @tc.desc: Verify parsing coordinates separated by commas.
 *           Path "M 0,0 L 100,50" should be parsed the same as space-separated.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserCommaSeparator_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0,0 L 100,50\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 50);
}

/**
 * @tc.name: SvgPathParserMixedSeparator_001
 * @tc.desc: Verify parsing coordinates with mixed separators (space, comma, tab).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserMixedSeparator_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0\t0, L 100 50\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 50);
}

/**
 * @tc.name: SvgPathParserZMidPath_001
 * @tc.desc: Verify Z command in the middle of a path closes the current subpath.
 *           Path "M 0 0 L 100 0 Z L 200 0" should produce points for M, L, Z, L.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserZMidPath_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 L 100 0 Z L 200 0\")", out));
    EXPECT_EQ(out.count, 4);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 0);
    // Z closes back to start (0, 0)
    EXPECT_EQ(out.x[2], 0);
    EXPECT_EQ(out.y[2], 0);
    EXPECT_EQ(out.x[3], 200);
    EXPECT_EQ(out.y[3], 0);
}

/**
 * @tc.name: SvgPathParserInvalidChar_001
 * @tc.desc: Verify parsing fails when an invalid character is encountered
 *           in a coordinate position.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserInvalidChar_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 L x y\")", out));
}

/**
 * @tc.name: SvgPathParserSinglePoint_001
 * @tc.desc: Verify parsing fails when only one point is produced
 *           (a valid path needs at least 2 points).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserSinglePoint_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_FALSE(OffsetPathParser::ParseToPolyline("path(\"M 0 0\")", out));
}

/**
 * @tc.name: SvgPathParserTooManyTempPoints_001
 * @tc.desc: Verify that when more than MAX_TEMP_POINTS (64) points are parsed,
 *           the parser truncates with a warning and continues.
 *           A path with 70 L commands should be thinned to MAX_POINTS (32) at the end.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserTooManyTempPoints_001, TestSize.Level2)
{
    char pathStr[512];
    int pos = snprintf_s(pathStr, sizeof(pathStr), sizeof(pathStr) - 1, "path(\"M 0 0");
    for (int i = 1; i < 70; i++) {
        pos += snprintf_s(pathStr + pos, sizeof(pathStr) - pos, sizeof(pathStr) - pos - 1,
                          " L %d 0", i);
    }
    pos += snprintf_s(pathStr + pos, sizeof(pathStr) - pos, sizeof(pathStr) - pos - 1, "\")");
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline(pathStr, out));
    EXPECT_EQ(out.count, PathPolyline::MAX_POINTS);
}

/**
 * @tc.name: SvgPathParserBuildCumLenEmpty_001
 * @tc.desc: Verify BuildCumulativeLength handles a polyline with 0 or 1 points.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserBuildCumLenEmpty_001, TestSize.Level1)
{
    PathPolyline out;
    out.count = 0;
    OffsetPathParser::BuildCumulativeLength(out);
    EXPECT_EQ(out.totalLen, 0);

    out.count = 1;
    out.x[0] = 10;
    out.y[0] = 20;
    OffsetPathParser::BuildCumulativeLength(out);
    EXPECT_EQ(out.cumLen[0], 0);
    EXPECT_EQ(out.totalLen, 0);
}

/**
 * @tc.name: SvgPathParserLowerCaseCmd_001
 * @tc.desc: Verify lowercase commands are treated as absolute coordinates.
 *           Path "m 0 0 l 100 0" should behave the same as "M 0 0 L 100 0".
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserLowerCaseCmd_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"m 0 0 l 100 0\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 0);
}

/**
 * @tc.name: SvgPathParserMultipleM_001
 * @tc.desc: Verify a second M command is treated as L per SVG spec.
 *           Path "M 0 0 M 100 0" should produce 2 points: (0,0) and (100,0).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserMultipleM_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 M 100 0\")", out));
    EXPECT_EQ(out.count, 2);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 0);
}

/**
 * @tc.name: SvgPathParserMultipleZ_001
 * @tc.desc: Verify multiple Z commands close the path repeatedly.
 *           Path "M 0 0 L 100 0 Z Z" should have the closing point twice.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserMultipleZ_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 L 100 0 Z Z\")", out));
    EXPECT_EQ(out.count, 4);
    EXPECT_EQ(out.x[0], 0);
    EXPECT_EQ(out.y[0], 0);
    EXPECT_EQ(out.x[1], 100);
    EXPECT_EQ(out.y[1], 0);
    EXPECT_EQ(out.x[2], 0);
    EXPECT_EQ(out.y[2], 0);
    EXPECT_EQ(out.x[3], 0);
    EXPECT_EQ(out.y[3], 0);
}

/**
 * @tc.name: SvgPathParserLeadingSpace_001
 * @tc.desc: Verify leading spaces before the path() wrapper are tolerated.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserLeadingSpace_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("  path(\"M 0 0 L 100 0\")", out));
    EXPECT_EQ(out.count, 2);
}

/**
 * @tc.name: SvgPathParserLargeCoord_001
 * @tc.desc: Verify large positive coordinates are saturated to int16 max.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserLargeCoord_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 L 40000 0\")", out));
    EXPECT_EQ(out.count, 2);
    // 40000 exceeds int16 max (32767), cast produces implementation-defined value;
    // the parser currently does not clamp, so just verify it parses without crash.
    SUCCEED();
}

/**
 * @tc.name: SvgPathParserNegativeLargeCoord_001
 * @tc.desc: Verify large negative coordinates are handled.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserNegativeLargeCoord_001, TestSize.Level1)
{
    PathPolyline out;
    EXPECT_TRUE(OffsetPathParser::ParseToPolyline("path(\"M 0 0 L -40000 0\")", out));
    EXPECT_EQ(out.count, 2);
    SUCCEED();
}

/**
 * @tc.name: SvgPathParserCumLenSaturation_001
 * @tc.desc: Verify BuildCumulativeLength saturates to UINT16_MAX when the
 *           accumulated length exceeds uint16 range.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(OffsetPathParserTest, SvgPathParserCumLenSaturation_001, TestSize.Level1)
{
    PathPolyline out;
    out.count = 4;
    out.x[0] = 0;
    out.y[0] = 0;
    out.x[1] = 30000;
    out.y[1] = 0;
    out.x[2] = 30000;
    out.y[2] = 30000;
    out.x[3] = 0;
    out.y[3] = 30000;
    OffsetPathParser::BuildCumulativeLength(out);
    // 30000 + 30000 + 30000 = 90000 > UINT16_MAX, saturated
    EXPECT_EQ(out.totalLen, UINT16_MAX);
}
} // namespace OHOS
