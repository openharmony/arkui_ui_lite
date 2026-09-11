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
#include "svg/svg_path_parser.h"

using namespace testing::ext;

namespace OHOS {

class SvgPathPointAtTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgPathPointAtCubicNonDegenerate_001
 * @tc.desc: Verify SvgPathPointAt flattens a non-degenerate cubic with the correct mt^3 weight.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtCubicNonDegenerate_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    ASSERT_TRUE(SvgPathPointAt("M100 100 C0 100 0 0 100 0", 0.25f, x, y, angle));
    EXPECT_NEAR(x, 52.0f, 1.0f);
    EXPECT_NEAR(y, 88.0f, 1.0f);

    ASSERT_TRUE(SvgPathPointAt("M100 100 C0 100 0 0 100 0", 0.5f, x, y, angle));
    EXPECT_NEAR(x, 25.0f, 1.0f);
    EXPECT_NEAR(y, 50.0f, 1.0f);

    ASSERT_TRUE(SvgPathPointAt("M100 100 C0 100 0 0 100 0", 0.75f, x, y, angle));
    EXPECT_NEAR(x, 52.0f, 1.0f);
    EXPECT_NEAR(y, 11.0f, 1.0f);

    ASSERT_TRUE(SvgPathPointAt("M100 100 C0 100 0 0 100 0", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 100.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);
}

/**
 * @tc.name: SvgPathPointAtArc_001
 * @tc.desc: Verify SvgPathPointAt consumes all seven arc parameters and samples the
 *           approximated line to the arc end point.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtArc_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    ASSERT_TRUE(SvgPathPointAt("M0 0 L50 0 A30 30 0 0 1 50 50 L50 100", 1.0f / 3.0f, x, y, angle));
    EXPECT_NEAR(x, 50.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);
    EXPECT_NEAR(angle, 0.0f, 0.01f);

    ASSERT_TRUE(SvgPathPointAt("M0 0 L50 0 A30 30 0 0 1 50 50 L50 100", 2.0f / 3.0f, x, y, angle));
    EXPECT_NEAR(x, 50.0f, 0.01f);
    EXPECT_NEAR(y, 50.0f, 0.01f);
    EXPECT_NEAR(angle, 90.0f, 0.01f);

    ASSERT_TRUE(SvgPathPointAt("M0 0 L50 0 A30 30 0 0 1 50 50 L50 100", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 50.0f, 0.01f);
    EXPECT_NEAR(y, 100.0f, 0.01f);
}

/**
 * @tc.name: SvgPathPointAtAdversarial_001
 * @tc.desc: Verify SvgPathPointAt terminates on tokens strtof cannot consume.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtAdversarial_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    EXPECT_FALSE(SvgPathPointAt("M!", 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("M0 0 L!", 0.5f, x, y, angle));

    ASSERT_TRUE(SvgPathPointAt("M0 0 L10 10 Z 5", 0.5f, x, y, angle));
    EXPECT_NEAR(x, 10.0f, 0.01f);
    EXPECT_NEAR(y, 10.0f, 0.01f);

    EXPECT_FALSE(SvgPathPointAt("A1 1 0 0 0", 0.5f, x, y, angle));
}

/**
 * @tc.name: SvgPathPointAtTruncatedCommands_001
 * @tc.desc: Verify SvgPathPointAt terminates on commands missing trailing coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtTruncatedCommands_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    EXPECT_FALSE(SvgPathPointAt("M0 0 L", 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("M0 0 H", 0.5f, x, y, angle));
    (void)SvgPathPointAt("M0 0 C0 100 100", 0.5f, x, y, angle);
    (void)SvgPathPointAt("M0 0 Q1 2", 0.5f, x, y, angle);
    (void)SvgPathPointAt("M0 0 A1", 0.5f, x, y, angle);
}

/**
 * @tc.name: SvgPathPointAtEdge_001
 * @tc.desc: Verify SvgPathPointAt handles null/empty/single-point input and clamps progress.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtEdge_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_FALSE(SvgPathPointAt(nullptr, 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("", 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("M0 0", 0.5f, x, y, angle));

    ASSERT_TRUE(SvgPathPointAt("M0 0 L10 0", -0.5f, x, y, angle));
    EXPECT_NEAR(x, 0.0f, 0.01f);
    ASSERT_TRUE(SvgPathPointAt("M0 0 L10 0", 1.5f, x, y, angle));
    EXPECT_NEAR(x, 10.0f, 0.01f);
}

/**
 * @tc.name: SvgPathPointAtSmooth_001
 * @tc.desc: Verify SvgPathPointAt honours the smooth S/T commands: the reflected control
 *           point is used and the segment is NOT dropped. Endpoints prove the S/T segment
 *           is present (x reaches 200, not the prior C/Q endpoint of 100).
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtSmooth_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    // C followed by S: S reflects the previous C control point (100,100) through (100,0)
    // to (100,-100) and ends at (200,0). If S were dropped, the endpoint would be (100,0).
    ASSERT_TRUE(SvgPathPointAt("M0 0 C0 100 100 100 100 0 S200 -100 200 0", 0.0f, x, y, angle));
    EXPECT_NEAR(x, 0.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);
    ASSERT_TRUE(SvgPathPointAt("M0 0 C0 100 100 100 100 0 S200 -100 200 0", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 200.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);

    // Q followed by T: T reflects the previous Q control point (50,100) through (100,0)
    // to (150,-100) and ends at (200,0). If T were dropped, the endpoint would be (100,0).
    ASSERT_TRUE(SvgPathPointAt("M0 0 Q50 100 100 0 T200 0", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 200.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);

    // The reflected control point pulls the S segment below the x-axis: some sampled y must be negative.
    bool sawNegativeY = false;
    for (int i = 1; i < 10; i++) {
        ASSERT_TRUE(SvgPathPointAt("M0 0 C0 100 100 100 100 0 S200 -100 200 0",
                                    static_cast<float>(i) / 10.0f, x, y, angle));
        if (y < 0.0f) {
            sawNegativeY = true;
        }
    }
    EXPECT_TRUE(sawNegativeY);
}
/**
 * @tc.name: SvgPathPointAtArcSmooth_001
 * @tc.desc: Verify an arc followed by a smooth T command resets the reflection control
 *           point to the current point, so the T segment is a straight line.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathPointAtTest, SvgPathPointAtArcSmooth_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    // With the reset, T after A reflects (10,10) through (10,10) -> straight line to (20,10).
    ASSERT_TRUE(SvgPathPointAt("M0 0 A10 10 0 0 1 10 10 T20 10", 0.0f, x, y, angle));
    EXPECT_NEAR(x, 0.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);

    ASSERT_TRUE(SvgPathPointAt("M0 0 A10 10 0 0 1 10 10 T20 10", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 20.0f, 0.01f);
    EXPECT_NEAR(y, 10.0f, 0.01f);

    // Progress 0.8 is well into the post-arc T segment; it must stay on y=10.
    ASSERT_TRUE(SvgPathPointAt("M0 0 A10 10 0 0 1 10 10 T20 10", 0.8f, x, y, angle));
    EXPECT_NEAR(y, 10.0f, 0.01f);
}
} // namespace OHOS
