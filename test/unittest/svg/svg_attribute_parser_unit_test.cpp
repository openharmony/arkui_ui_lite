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

#include <gtest/gtest.h>
#include <limits>
#include <string>
#include "gfx_utils/graphic_types.h"
#include "gfx_utils/list.h"
#include "svg/svg_attribute_parser.h"

using namespace testing::ext;

namespace OHOS {
namespace {
constexpr uint32_t URL_BUFFER_LEN = 128;
}

class SvgAttributeParserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgAttributeParserParseInt_001
 * @tc.desc: Verify ParseInt handles valid, invalid and null input.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseInt_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseInt(nullptr), 0);
    EXPECT_EQ(SvgAttributeParser::ParseInt("0"), 0);
    EXPECT_EQ(SvgAttributeParser::ParseInt("123"), 123);
    EXPECT_EQ(SvgAttributeParser::ParseInt("-456"), -456);
    EXPECT_EQ(SvgAttributeParser::ParseInt("abc"), 0);
}

/**
 * @tc.name: SvgAttributeParserParseFloat_001
 * @tc.desc: Verify ParseFloat handles valid, invalid and null input.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseFloat_001, TestSize.Level1)
{
    EXPECT_FLOAT_EQ(SvgAttributeParser::ParseFloat(nullptr), 0.0f);
    EXPECT_FLOAT_EQ(SvgAttributeParser::ParseFloat("0"), 0.0f);
    EXPECT_FLOAT_EQ(SvgAttributeParser::ParseFloat("3.14"), 3.14f);
    EXPECT_FLOAT_EQ(SvgAttributeParser::ParseFloat("-2.5"), -2.5f);
    EXPECT_FLOAT_EQ(SvgAttributeParser::ParseFloat("abc"), 0.0f);
}

/**
 * @tc.name: SvgAttributeParserParseLength_001
 * @tc.desc: Verify ParseLength clamps values to int16_t range.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseLength_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseLength(nullptr), 0);
    EXPECT_EQ(SvgAttributeParser::ParseLength("0"), 0);
    EXPECT_EQ(SvgAttributeParser::ParseLength("100"), 100);
    EXPECT_EQ(SvgAttributeParser::ParseLength("-5"), -5);
    EXPECT_EQ(SvgAttributeParser::ParseLength("abc"), 0);
    EXPECT_EQ(SvgAttributeParser::ParseLength("100000"), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ParseLength("-100000"), INT16_MIN);
    // Invalid value falls back to the supplied initial value (stroke-width initial is 1).
    EXPECT_EQ(SvgAttributeParser::ParseLength("abc", 1), 1);
    EXPECT_EQ(SvgAttributeParser::ParseLength(nullptr, 1), 1);
    // A valid 0 is still 0 even with a non-zero fallback.
    EXPECT_EQ(SvgAttributeParser::ParseLength("0", 1), 0);
}

/**
 * @tc.name: SvgAttributeParserParseLength_002
 * @tc.desc: Verify ParseLength boundary values at the int16_t clamp limits.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseLength_002, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseLength("32767"), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ParseLength("-32768"), INT16_MIN);
    EXPECT_EQ(SvgAttributeParser::ParseLength("32767.9"), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ParseLength("-32768.1"), INT16_MIN);
}

/**
 * @tc.name: SvgAttributeParserParseColor_001
 * @tc.desc: Verify ParseColor handles hex, named, rgb/rgba and edge cases.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_001, TestSize.Level1)
{
    ColorType color;
    color.full = 0;

    color = SvgAttributeParser::ParseColor(nullptr);
    EXPECT_EQ(color.full, 0U);

    color = SvgAttributeParser::ParseColor("");
    EXPECT_EQ(color.full, 0U);

    color = SvgAttributeParser::ParseColor("#ff0000");
    EXPECT_EQ(color.full, 0xFFFF0000U);

    color = SvgAttributeParser::ParseColor("#f00");
    EXPECT_EQ(color.full, 0xFFFF0000U);

    color = SvgAttributeParser::ParseColor("#ff0000ff");
    EXPECT_EQ(color.full, 0xFF0000FFU);

    color = SvgAttributeParser::ParseColor("red");
    EXPECT_EQ(color.full, 0xFFFF0000U);

    color = SvgAttributeParser::ParseColor("none");
    EXPECT_EQ(color.full, 0U);

    color = SvgAttributeParser::ParseColor("rgb(0,128,255)");
    EXPECT_EQ(color.full, 0xFF0080FFU);

    color = SvgAttributeParser::ParseColor("rgba(0,0,0,0)");
    EXPECT_EQ(color.full, 0U);

    color = SvgAttributeParser::ParseColor("unknown");
    EXPECT_EQ(color.full, 0xFF000000U);
}

/**
 * @tc.name: SvgAttributeParserParseColor_002
 * @tc.desc: Verify ParseColor handles 4-digit hex and unsupported hex lengths.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_002, TestSize.Level1)
{
    ColorType color;

    color = SvgAttributeParser::ParseColor("#abcd");
    EXPECT_EQ(color.full, 0xAABBCCDDU);

    // Unsupported hex lengths return transparent, not opaque black.
    color = SvgAttributeParser::ParseColor("#ff");
    EXPECT_EQ(color.full, 0x00000000U);

    color = SvgAttributeParser::ParseColor("#12345");
    EXPECT_EQ(color.full, 0x00000000U);

    color = SvgAttributeParser::ParseColor("#");
    EXPECT_EQ(color.full, 0x00000000U);
}

/**
 * @tc.name: SvgAttributeParserParseColor_003
 * @tc.desc: Verify ParseColor handles every supported named color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_003, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseColor("black").full, 0xFF000000U);
    EXPECT_EQ(SvgAttributeParser::ParseColor("white").full, 0xFFFFFFFFU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("red").full, 0xFFFF0000U);
    EXPECT_EQ(SvgAttributeParser::ParseColor("green").full, 0xFF008000U);
    EXPECT_EQ(SvgAttributeParser::ParseColor("blue").full, 0xFF0000FFU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("yellow").full, 0xFFFFFF00U);
    EXPECT_EQ(SvgAttributeParser::ParseColor("cyan").full, 0xFF00FFFFU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("magenta").full, 0xFFFF00FFU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("none").full, 0U);
}

/**
 * @tc.name: SvgAttributeParserParseColor_004
 * @tc.desc: Verify ParseColor handles malformed rgb/rgba and channel clamping.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_004, TestSize.Level1)
{
    ColorType color;

    color = SvgAttributeParser::ParseColor("rgbx(1,2,3)");
    EXPECT_EQ(color.full, 0xFF000000U);

    color = SvgAttributeParser::ParseColor("rgb");
    EXPECT_EQ(color.full, 0xFF000000U);

    color = SvgAttributeParser::ParseColor("rgb(");
    EXPECT_EQ(color.full, 0xFF000000U);

    color = SvgAttributeParser::ParseColor("rgb(10");
    EXPECT_EQ(color.full, 0xFF0A0000U);

    color = SvgAttributeParser::ParseColor("rgb(300,-5,10)");
    EXPECT_EQ(color.full, 0xFFFF000AU);

    color = SvgAttributeParser::ParseColor("rgb(-5,10,20)");
    EXPECT_EQ(color.full, 0xFF000A14U);

    color = SvgAttributeParser::ParseColor("rgba(1,2,3,1)");
    EXPECT_EQ(color.full, 0xFF010203U);

    color = SvgAttributeParser::ParseColor("rgba(1,2,3,2)");
    EXPECT_EQ(color.full, 0xFF010203U);
}

/**
 * @tc.name: SvgAttributeParserParseOpacity_001
 * @tc.desc: Verify ParseOpacity clamps values to [0, 255].
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseOpacity_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseOpacity(nullptr), OPA_OPAQUE);
    EXPECT_EQ(SvgAttributeParser::ParseOpacity("0"), 0);
    EXPECT_EQ(SvgAttributeParser::ParseOpacity("0.5"), OPA_OPAQUE / 2);
    EXPECT_EQ(SvgAttributeParser::ParseOpacity("1"), OPA_OPAQUE);
    EXPECT_EQ(SvgAttributeParser::ParseOpacity("2"), OPA_OPAQUE);
    EXPECT_EQ(SvgAttributeParser::ParseOpacity("-0.5"), 0);
}

/**
 * @tc.name: SvgAttributeParserParsePoints_001
 * @tc.desc: Verify ParsePoints extracts points from comma/space separated lists.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePoints_001, TestSize.Level1)
{
    List<Point> points;
    EXPECT_FALSE(SvgAttributeParser::ParsePoints(nullptr, points));

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("", points));
    EXPECT_EQ(points.Size(), 0U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("0,0 10,20", points));
    EXPECT_EQ(points.Size(), 2U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("0 0 10 20", points));
    EXPECT_EQ(points.Size(), 2U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("0,0 10,20 30,40", points));
    EXPECT_EQ(points.Size(), 3U);
}

/**
 * @tc.name: SvgAttributeParserParsePoints_002
 * @tc.desc: Verify ParsePoints handles garbage separators, truncation and negative decimals.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePoints_002, TestSize.Level1)
{
    List<Point> points;

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints(" , , ", points));
    EXPECT_EQ(points.Size(), 0U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("10,", points));
    EXPECT_EQ(points.Size(), 0U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("-5.5,3.7", points));
    EXPECT_EQ(points.Size(), 1U);
    EXPECT_EQ(points.Front().x, -5);
    EXPECT_EQ(points.Front().y, 3);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("abc-2,4zzz", points));
    EXPECT_EQ(points.Size(), 1U);
    EXPECT_EQ(points.Front().x, -2);
    EXPECT_EQ(points.Front().y, 4);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("1,2 3,4 5", points));
    EXPECT_EQ(points.Size(), 2U);
    EXPECT_EQ(points.Back().x, 3);
    EXPECT_EQ(points.Back().y, 4);
}

/**
 * @tc.name: SvgAttributeParserParsePoints_004
 * @tc.desc: Verify ParsePoints accepts scientific notation and explicit plus signs.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePoints_004, TestSize.Level1)
{
    List<Point> points;

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("1e2,3e-1", points));
    EXPECT_EQ(points.Size(), 1U);
    EXPECT_EQ(points.Front().x, 100);
    EXPECT_EQ(points.Front().y, 0);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("+10 -20 1.5E+1,2.5E-1", points));
    EXPECT_EQ(points.Size(), 2U);
    EXPECT_EQ(points.Front().x, 10);
    EXPECT_EQ(points.Back().x, 15);
}

/**
 * @tc.name: SvgAttributeParserParseUrlReference_001
 * @tc.desc: Verify ParseUrlReference extracts ids from url(#id) and #id forms.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseUrlReference_001, TestSize.Level1)
{
    char buffer[URL_BUFFER_LEN] = { 0 };

    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference(nullptr, buffer, URL_BUFFER_LEN));
    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("#id", nullptr, URL_BUFFER_LEN));
    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("noid", buffer, URL_BUFFER_LEN));

    EXPECT_TRUE(SvgAttributeParser::ParseUrlReference("url(#grad1)", buffer, URL_BUFFER_LEN));
    EXPECT_STREQ(buffer, "grad1");

    EXPECT_TRUE(SvgAttributeParser::ParseUrlReference("#grad1", buffer, URL_BUFFER_LEN));
    EXPECT_STREQ(buffer, "grad1");

    EXPECT_TRUE(SvgAttributeParser::ParseUrlReference("url(#grad1 )", buffer, URL_BUFFER_LEN));
    EXPECT_STREQ(buffer, "grad1");
}

/**
 * @tc.name: SvgAttributeParserParseUrlReference_002
 * @tc.desc: Verify ParseUrlReference handles zero size, empty id, tab terminators and truncation.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseUrlReference_002, TestSize.Level1)
{
    char buffer[URL_BUFFER_LEN] = { 0 };

    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("#id", buffer, 0));

    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("url(#)", buffer, URL_BUFFER_LEN));

    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("#", buffer, URL_BUFFER_LEN));

    EXPECT_TRUE(SvgAttributeParser::ParseUrlReference("#a b", buffer, URL_BUFFER_LEN));
    EXPECT_STREQ(buffer, "a");

    EXPECT_TRUE(SvgAttributeParser::ParseUrlReference("#a\tb", buffer, URL_BUFFER_LEN));
    EXPECT_STREQ(buffer, "a");

    char small[3] = { 0 };
    // Buffer too small: the function must reject the truncated id rather than
    // return a partial reference that would silently fail later.
    EXPECT_FALSE(SvgAttributeParser::ParseUrlReference("#abcdef", small, sizeof(small)));
}

/**
 * @tc.name: SvgAttributeParserParseTransform_001
 * @tc.desc: Verify ParseTransform handles all supported transform commands.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseTransform_001, TestSize.Level1)
{
    TransAffine matrix;
    EXPECT_FALSE(SvgAttributeParser::ParseTransform(nullptr, matrix));
    EXPECT_FALSE(SvgAttributeParser::ParseTransform("", matrix));
    EXPECT_FALSE(SvgAttributeParser::ParseTransform("unknown(1)", matrix));

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10, 20)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2, 3)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(90)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(90, 10, 20)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("skewX(45)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("skewY(45)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("matrix(1,0,0,1,5,5)", matrix));
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10, 20) scale(2)", matrix));
}

/**
 * @tc.name: SvgAttributeParserParseTransform_002
 * @tc.desc: Verify ParseTransform produces the expected matrix values for each command.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseTransform_002, TestSize.Level1)
{
    TransAffine matrix;
    const float* data = nullptr;

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10, 20)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 0.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2, 3)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 2.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("matrix(2,0,0,3,5,6)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);
    EXPECT_FLOAT_EQ(data[2], 5.0f);
    EXPECT_FLOAT_EQ(data[5], 6.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(90)", matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[0], 0.0f, 0.001f);
    EXPECT_NEAR(data[3], 1.0f, 0.001f);

    // Per the SVG spec, rotate(angle, cx, cy) rotates about (cx, cy), i.e. the matrix is
    // T(cx,cy) * R * T(-cx,-cy). A 90-degree rotation about (10, 20) yields e=30, f=10.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(90, 10, 20)", matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[2], 30.0f, 0.01f);
    EXPECT_NEAR(data[5], 10.0f, 0.01f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("skewX(45)", matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[1], 1.0f, 0.001f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("skewY(45)", matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[3], 1.0f, 0.001f);

    // Per the SVG spec, transform lists compose left to right: "translate(10, 20) scale(2)"
    // is T * S, so the translation is NOT scaled. The translation components stay (10, 20).
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10, 20) scale(2)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 2.0f);
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);
}

/**
 * @tc.name: SvgAttributeParserParseTransform_003
 * @tc.desc: Verify ParseTransform tolerates missing parens, signs, exponents and separators.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseTransform_003, TestSize.Level1)
{
    TransAffine matrix;
    const float* data = nullptr;

    EXPECT_FALSE(SvgAttributeParser::ParseTransform("   ", matrix));

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate 10 20", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10,20", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(+1e1,+2e1)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2 +3)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);

    // T * S composition: the translation (1, 2) is applied after the scale, so it is not scaled.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(1,\t2\n)scale(\r3)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 3.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);
    EXPECT_FLOAT_EQ(data[2], 1.0f);
    EXPECT_FLOAT_EQ(data[5], 2.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(45) junk skewX(30)", matrix));
}

/**
 * @tc.name: SvgAttributeParserParseTransform_004
 * @tc.desc: Verify composed transforms map points per the SVG spec (left-to-right composition).
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseTransform_004, TestSize.Level1)
{
    TransAffine matrix;

    // "translate(10,20) scale(2)" must map (10,10) to (30,40): scale first, then translate.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate(10,20) scale(2)", matrix));
    float x = 10.0f;
    float y = 10.0f;
    matrix.Transform(&x, &y);
    EXPECT_NEAR(x, 30.0f, 0.01f);
    EXPECT_NEAR(y, 40.0f, 0.01f);

    // The reverse list must map (10,10) to (40,60): translate first, then scale.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale(2) translate(10,20)", matrix));
    x = 10.0f;
    y = 10.0f;
    matrix.Transform(&x, &y);
    EXPECT_NEAR(x, 40.0f, 0.01f);
    EXPECT_NEAR(y, 60.0f, 0.01f);

    // rotate(90, 10, 20) rotates about (10,20): (20,20) maps to (10,30).
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(90,10,20)", matrix));
    x = 20.0f;
    y = 20.0f;
    matrix.Transform(&x, &y);
    EXPECT_NEAR(x, 10.0f, 0.01f);
    EXPECT_NEAR(y, 30.0f, 0.01f);
}

/**
 * @tc.name: SvgAttributeParserParseColor_005
 * @tc.desc: Verify rgba() parses fractional alpha as a float scaled to [0, 255].
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_005, TestSize.Level1)
{
    // 0.5 * 255 = 127.5, rounded to 128.
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(255,0,0,0.5)").full, 0x80FF0000U);
    // 0.25 * 255 = 63.75, rounded to 64.
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(10,20,30,0.25)").full, 0x400A141EU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(10,20,30,1)").full, 0xFF0A141EU);
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(10,20,30,0)").full, 0x000A141EU);
    // Out-of-range alpha is clamped.
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(10,20,30,2)").full, 0xFF0A141EU);
}

/**
 * @tc.name: SvgAttributeParserParsePoints_003
 * @tc.desc: Verify ParsePoints terminates on adversarial tokens that strtof cannot consume.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePoints_003, TestSize.Level1)
{
    List<Point> points;

    // A lone '.' or '-' cannot be consumed by strtof; the parser must not loop forever.
    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints(".", points));
    EXPECT_EQ(points.Size(), 0U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("-", points));
    EXPECT_EQ(points.Size(), 0U);

    // The '-' after the first pair is not a number; parsing must still terminate.
    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("10 20 - 30", points));
    EXPECT_EQ(points.Size(), 1U);
    EXPECT_EQ(points.Front().x, 10);
    EXPECT_EQ(points.Front().y, 20);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("1,2,.", points));
    EXPECT_EQ(points.Size(), 1U);

    points.Clear();
    EXPECT_TRUE(SvgAttributeParser::ParsePoints("10,20,-,30,40", points));
    EXPECT_EQ(points.Size(), 2U);
    EXPECT_EQ(points.Back().x, 30);
    EXPECT_EQ(points.Back().y, 40);
}

/**
 * @tc.name: SvgAttributeParserParseViewBox_001
 * @tc.desc: Verify ParseViewBox extracts min-x, min-y, width and height.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseViewBox_001, TestSize.Level1)
{
    float values[4] = {0};

    EXPECT_TRUE(SvgAttributeParser::ParseViewBox("0 0 100 50", values));
    EXPECT_FLOAT_EQ(values[0], 0.0f);
    EXPECT_FLOAT_EQ(values[1], 0.0f);
    EXPECT_FLOAT_EQ(values[2], 100.0f);
    EXPECT_FLOAT_EQ(values[3], 50.0f);

    EXPECT_TRUE(SvgAttributeParser::ParseViewBox("10,20,30,40", values));
    EXPECT_FLOAT_EQ(values[0], 10.0f);
    EXPECT_FLOAT_EQ(values[1], 20.0f);
    EXPECT_FLOAT_EQ(values[2], 30.0f);
    EXPECT_FLOAT_EQ(values[3], 40.0f);

    EXPECT_FALSE(SvgAttributeParser::ParseViewBox(nullptr, values));
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("0 0 100", values));
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("0 0 0 0", values));
}

/**
 * @tc.name: SvgAttributeParserParsePreserveAspectRatio_001
 * @tc.desc: Verify ParsePreserveAspectRatio covers all 9 alignments, none, meet and slice.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePreserveAspectRatio_001, TestSize.Level1)
{
    uint8_t align = 0;
    bool slice = true;

    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMinYMin meet", align, slice));
    EXPECT_EQ(align, 0U);
    EXPECT_FALSE(slice);

    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMidYMid meet", align, slice));
    EXPECT_EQ(align, 4U);
    EXPECT_FALSE(slice);

    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMaxYMax slice", align, slice));
    EXPECT_EQ(align, 8U);
    EXPECT_TRUE(slice);

    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("none", align, slice));
    EXPECT_EQ(align, 9U);
    EXPECT_FALSE(slice);

    EXPECT_FALSE(SvgAttributeParser::ParsePreserveAspectRatio(nullptr, align, slice));
    EXPECT_FALSE(SvgAttributeParser::ParsePreserveAspectRatio("invalid", align, slice));
}

/**
 * @tc.name: SvgAttributeParserClampToInt16_001
 * @tc.desc: Verify ClampToInt16 saturates to int16_t range and passes in-range values.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserClampToInt16_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(0.0f), 0);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(100.5f), 100);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(-100.5f), -100);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(static_cast<float>(INT16_MAX)), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(static_cast<float>(INT16_MIN)), INT16_MIN);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(100000.0f), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(-100000.0f), INT16_MIN);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(32767.5f), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(-32768.5f), INT16_MIN);
}

/**
 * @tc.name: SvgAttributeParserParseClockValue_001
 * @tc.desc: Verify ParseClockValue converts SVG clock values to milliseconds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseClockValue_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::ParseClockValue(nullptr), 0U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("indefinite"), 0xFFFFFFFFU);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("2s"), 2000U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("500ms"), 500U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("2min"), 120000U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("1h"), 3600000U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("00:01:30"), 90000U);
    // MM:SS is also accepted as a valid colon clock value.
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("01:30"), 90000U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("-1s"), 0U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("abc"), 0U);
}

/**
 * @tc.name: SvgAttributeParserParseSemicolonFloats_001
 * @tc.desc: Verify ParseSemicolonFloats splits semicolon-separated floats.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseSemicolonFloats_001, TestSize.Level1)
{
    float* array = nullptr;

    EXPECT_EQ(SvgAttributeParser::ParseSemicolonFloats(nullptr, array), 0U);
    EXPECT_EQ(array, nullptr);
    EXPECT_EQ(SvgAttributeParser::ParseSemicolonFloats("", array), 0U);
    EXPECT_EQ(array, nullptr);
    EXPECT_EQ(SvgAttributeParser::ParseSemicolonFloats("abc", array), 0U);
    EXPECT_EQ(array, nullptr);

    uint32_t count = SvgAttributeParser::ParseSemicolonFloats("0;0.5;1", array);
    EXPECT_EQ(count, 3U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 0.0f);
    EXPECT_FLOAT_EQ(array[1], 0.5f);
    EXPECT_FLOAT_EQ(array[2], 1.0f);
    delete[] array;
    array = nullptr;
}

/**
 * @tc.name: SvgAttributeParserParseDashArray_001
 * @tc.desc: Verify ParseDashArray extracts comma/space separated non-negative dashes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseDashArray_001, TestSize.Level1)
{
    float* array = nullptr;

    EXPECT_EQ(SvgAttributeParser::ParseDashArray(nullptr, array), 0U);
    EXPECT_EQ(array, nullptr);
    EXPECT_EQ(SvgAttributeParser::ParseDashArray("none", array), 0U);
    EXPECT_EQ(array, nullptr);
    EXPECT_EQ(SvgAttributeParser::ParseDashArray("", array), 0U);
    EXPECT_EQ(array, nullptr);

    uint32_t count = SvgAttributeParser::ParseDashArray("5,3", array);
    EXPECT_EQ(count, 2U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 5.0f);
    EXPECT_FLOAT_EQ(array[1], 3.0f);
    delete[] array;
    array = nullptr;

    count = SvgAttributeParser::ParseDashArray("5", array);
    EXPECT_EQ(count, 2U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 5.0f);
    EXPECT_FLOAT_EQ(array[1], 5.0f);
    delete[] array;
    array = nullptr;
}

/**
 * @tc.name: SvgAttributeParserRoundToInt16_001
 * @tc.desc: Verify RoundToInt16 rounds to nearest and saturates at the int16_t bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserRoundToInt16_001, TestSize.Level1)
{
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(0.0f), 0);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(100.4f), 100);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(100.6f), 101);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(-100.4f), -100);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(-100.6f), -101);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(0.5f), 1);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(-0.5f), -1);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(static_cast<float>(INT16_MAX)), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(static_cast<float>(INT16_MIN)), INT16_MIN);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(100000.0f), INT16_MAX);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(-100000.0f), INT16_MIN);
}

/**
 * @tc.name: SvgAttributeParserNanInput_001
 * @tc.desc: Verify the int16_t converters map NaN to 0 instead of invoking undefined behaviour.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserNanInput_001, TestSize.Level1)
{
    const float nanValue = std::numeric_limits<float>::quiet_NaN();
    EXPECT_EQ(SvgAttributeParser::ClampToInt16(nanValue), 0);
    EXPECT_EQ(SvgAttributeParser::RoundToInt16(nanValue), 0);
    EXPECT_EQ(SvgAttributeParser::ParseLength("nan"), 0);
}

/**
 * @tc.name: SvgAttributeParserParseColor_006
 * @tc.desc: Verify rgb()/rgba() tolerate missing components and a missing alpha term.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseColor_006, TestSize.Level1)
{
    // No digits at all: every component parses as 0 and the alpha defaults to opaque.
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgb()").full, 0xFF000000U);
    // rgba without an alpha term: strtof yields 0, so the color becomes fully transparent.
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgba(1,2,3)").full, 0x00010203U);
    EXPECT_EQ(SvgAttributeParser::ParseColor("rgb(1 2 3)").full, 0xFF010203U);
}

/**
 * @tc.name: SvgAttributeParserParseTransform_005
 * @tc.desc: Verify transform commands with missing or partial argument lists fall back to defaults.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseTransform_005, TestSize.Level1)
{
    TransAffine matrix;
    const float* data = nullptr;

    // A command with no argument list at all still counts as a parsed command.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("translate", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[2], 0.0f);
    EXPECT_FLOAT_EQ(data[5], 0.0f);

    // scale() with no argument falls back to 0, which is then mirrored onto y.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("scale()", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 0.0f);
    EXPECT_FLOAT_EQ(data[4], 0.0f);

    // matrix() with fewer than six arguments zero-fills the missing ones.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("matrix(2,0,0)", matrix));
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 0.0f);

    // rotate() with a center x but no center y treats the missing value as 0.
    EXPECT_TRUE(SvgAttributeParser::ParseTransform("rotate(0 10)", matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[2], 0.0f, 0.01f);
    EXPECT_NEAR(data[5], 0.0f, 0.01f);
}

/**
 * @tc.name: SvgAttributeParserPostMultiply_001
 * @tc.desc: Verify PostMultiply computes matrix * rhs rather than rhs * matrix.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserPostMultiply_001, TestSize.Level1)
{
    TransAffine matrix = TransAffine::TransAffineTranslation(10, 20);
    SvgAttributeParser::PostMultiply(matrix, TransAffine::TransAffineScaling(2.0f, 3.0f));
    const float* data = matrix.GetData();
    // T * S keeps the translation unscaled and adopts the scale factors.
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);

    // Post-multiplying by the identity must leave the matrix untouched.
    TransAffine identity;
    SvgAttributeParser::PostMultiply(matrix, identity);
    data = matrix.GetData();
    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[4], 3.0f);
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);
}

/**
 * @tc.name: SvgAttributeParserParseViewBox_002
 * @tc.desc: Verify ParseViewBox rejects a null output, garbage tokens and non-positive sizes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseViewBox_002, TestSize.Level1)
{
    float values[4] = {0};

    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("0 0 100 50", nullptr));
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("", values));
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("0 0 abc 10", values));

    // A negative width fails the final size check while the values are still written out.
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("1 2 -10 10", values));
    EXPECT_FLOAT_EQ(values[0], 1.0f);
    EXPECT_FLOAT_EQ(values[1], 2.0f);
    EXPECT_FLOAT_EQ(values[2], -10.0f);

    // A positive width with a zero height fails the second half of the size check.
    EXPECT_FALSE(SvgAttributeParser::ParseViewBox("0 0 10 0", values));
}

/**
 * @tc.name: SvgAttributeParserParsePreserveAspectRatio_002
 * @tc.desc: Verify the remaining alignment keywords and the default meet/slice handling.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParsePreserveAspectRatio_002, TestSize.Level1)
{
    uint8_t align = 0;
    bool slice = true;

    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMidYMin meet", align, slice));
    EXPECT_EQ(align, 1U);
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMaxYMin meet", align, slice));
    EXPECT_EQ(align, 2U);
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMinYMid meet", align, slice));
    EXPECT_EQ(align, 3U);
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMaxYMid meet", align, slice));
    EXPECT_EQ(align, 5U);
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMinYMax meet", align, slice));
    EXPECT_EQ(align, 6U);
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMidYMax meet", align, slice));
    EXPECT_EQ(align, 7U);

    // Without a meet/slice suffix the default (meet) is kept.
    slice = true;
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("xMinYMin", align, slice));
    EXPECT_EQ(align, 0U);
    EXPECT_FALSE(slice);

    // Leading separators are skipped before the alignment keyword.
    EXPECT_TRUE(SvgAttributeParser::ParsePreserveAspectRatio("  xMidYMid slice", align, slice));
    EXPECT_EQ(align, 4U);
    EXPECT_TRUE(slice);
}

/**
 * @tc.name: SvgAttributeParserParseClockValue_002
 * @tc.desc: Verify malformed colon clock values, negative seconds and fractional units.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseClockValue_002, TestSize.Level1)
{
    // The hour field is not a number, so the first colon check rejects the value.
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("a:1:2"), 0U);
    // Negative seconds inside a full clock value are clamped to zero.
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("00:00:-5"), 0U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("01:02:03"), 3723000U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("1.5s"), 1500U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("0.5s"), 500U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("0"), 0U);
    EXPECT_EQ(SvgAttributeParser::ParseClockValue(""), 0U);
    // A bare number without a unit is interpreted as seconds.
    EXPECT_EQ(SvgAttributeParser::ParseClockValue("3"), 3000U);
}

/**
 * @tc.name: SvgAttributeParserParseSemicolonFloats_002
 * @tc.desc: Verify empty segments, trailing separators and unparsable tails are handled.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseSemicolonFloats_002, TestSize.Level1)
{
    float* array = nullptr;

    // An empty segment between two values is skipped instead of producing a zero.
    uint32_t count = SvgAttributeParser::ParseSemicolonFloats("0;;1", array);
    EXPECT_EQ(count, 2U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 0.0f);
    EXPECT_FLOAT_EQ(array[1], 1.0f);
    delete[] array;
    array = nullptr;

    // Parsing stops at the first token strtof cannot consume.
    count = SvgAttributeParser::ParseSemicolonFloats("1;abc", array);
    EXPECT_EQ(count, 1U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 1.0f);
    delete[] array;
    array = nullptr;

    // A trailing separator run terminates the scan without adding a value.
    count = SvgAttributeParser::ParseSemicolonFloats("1; ", array);
    EXPECT_EQ(count, 1U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 1.0f);
    delete[] array;
    array = nullptr;

    count = SvgAttributeParser::ParseSemicolonFloats(";", array);
    EXPECT_EQ(count, 0U);
    EXPECT_EQ(array, nullptr);
}

/**
 * @tc.name: SvgAttributeParserParseSemicolonFloats_003
 * @tc.desc: Verify ParseSemicolonFloats stops at its internal 64 value limit.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseSemicolonFloats_003, TestSize.Level1)
{
    std::string input;
    for (uint32_t i = 0; i < 70; i++) {
        if (i > 0) {
            input += ";";
        }
        input += "1";
    }

    float* array = nullptr;
    uint32_t count = SvgAttributeParser::ParseSemicolonFloats(input.c_str(), array);
    EXPECT_EQ(count, 64U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 1.0f);
    EXPECT_FLOAT_EQ(array[63], 1.0f);
    delete[] array;
    array = nullptr;
}

/**
 * @tc.name: SvgAttributeParserParseDashArray_002
 * @tc.desc: Verify dash parsing stops at negative or unparsable values and duplicates odd lists.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAttributeParserTest, SvgAttributeParserParseDashArray_002, TestSize.Level1)
{
    float* array = nullptr;

    // A negative dash terminates the scan; the single remaining dash is duplicated.
    uint32_t count = SvgAttributeParser::ParseDashArray("5,-3", array);
    EXPECT_EQ(count, 2U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 5.0f);
    EXPECT_FLOAT_EQ(array[1], 5.0f);
    delete[] array;
    array = nullptr;

    // A leading negative value yields no dashes at all.
    EXPECT_EQ(SvgAttributeParser::ParseDashArray("-1", array), 0U);
    EXPECT_EQ(array, nullptr);

    EXPECT_EQ(SvgAttributeParser::ParseDashArray("abc", array), 0U);
    EXPECT_EQ(array, nullptr);

    EXPECT_EQ(SvgAttributeParser::ParseDashArray("   ", array), 0U);
    EXPECT_EQ(array, nullptr);

    // An odd number of dashes is repeated once to form an even pattern.
    count = SvgAttributeParser::ParseDashArray("1 2 3", array);
    EXPECT_EQ(count, 6U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[0], 1.0f);
    EXPECT_FLOAT_EQ(array[1], 2.0f);
    EXPECT_FLOAT_EQ(array[2], 3.0f);
    EXPECT_FLOAT_EQ(array[3], 1.0f);
    EXPECT_FLOAT_EQ(array[4], 2.0f);
    EXPECT_FLOAT_EQ(array[5], 3.0f);
    delete[] array;
    array = nullptr;

    // An even number of dashes is kept as is.
    count = SvgAttributeParser::ParseDashArray("1 2 3 4", array);
    EXPECT_EQ(count, 4U);
    ASSERT_NE(array, nullptr);
    EXPECT_FLOAT_EQ(array[3], 4.0f);
    delete[] array;
    array = nullptr;
}
} // namespace OHOS
