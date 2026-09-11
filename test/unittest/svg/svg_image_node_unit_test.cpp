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
#include "svg/svg_image_node.h"

using namespace testing::ext;

namespace OHOS {

namespace {

class TestSvgImageNode : public SvgImageNode {
public:
    using SvgImageNode::SetGeometryAttribute;
    using SvgImageNode::Clone;
};

} // namespace

class SvgImageNodeTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgImageNodeTest, ImageBounds_001, TestSize.Level1)
{
    TestSvgImageNode image;
    EXPECT_TRUE(image.SetGeometryAttribute("x", "10"));
    EXPECT_TRUE(image.SetGeometryAttribute("y", "20"));
    EXPECT_TRUE(image.SetGeometryAttribute("width", "30"));
    EXPECT_TRUE(image.SetGeometryAttribute("height", "40"));
    Rect b = image.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 10);
    EXPECT_EQ(b.GetTop(), 20);
    EXPECT_EQ(b.GetWidth(), 30);
    EXPECT_EQ(b.GetHeight(), 40);
}

HWTEST_F(SvgImageNodeTest, ImageHref_002, TestSize.Level1)
{
    TestSvgImageNode image;
    EXPECT_TRUE(image.SetGeometryAttribute("href", "/common/icon.png"));
    EXPECT_TRUE(image.SetGeometryAttribute("xlink:href", "/common/other.png"));
    EXPECT_FALSE(image.SetGeometryAttribute("unknown", "value"));
}

HWTEST_F(SvgImageNodeTest, ImageEmptyBoundsForNonPositiveSize_003, TestSize.Level1)
{
    TestSvgImageNode image;
    EXPECT_TRUE(image.SetGeometryAttribute("x", "10"));
    EXPECT_TRUE(image.SetGeometryAttribute("y", "20"));
    EXPECT_TRUE(image.SetGeometryAttribute("width", "0"));
    EXPECT_TRUE(image.SetGeometryAttribute("height", "40"));
    Rect b = image.GetLocalBounds();
    EXPECT_EQ(b.GetWidth(), 0);
    EXPECT_EQ(b.GetHeight(), 0);
}

HWTEST_F(SvgImageNodeTest, ImageGeometryAttributeUnknownRejected_004, TestSize.Level1)
{
    TestSvgImageNode image;
    EXPECT_FALSE(image.SetGeometryAttribute("fill", "red"));
}

HWTEST_F(SvgImageNodeTest, ImageSetAttributeNullRejected_005, TestSize.Level1)
{
    TestSvgImageNode image;
    EXPECT_FALSE(image.SetAttribute(nullptr, "value"));
    EXPECT_FALSE(image.SetAttribute("x", nullptr));
}

HWTEST_F(SvgImageNodeTest, ImageCloneCopiesGeometry_006, TestSize.Level1)
{
    TestSvgImageNode image;
    image.SetGeometryAttribute("x", "5");
    image.SetGeometryAttribute("y", "6");
    image.SetGeometryAttribute("width", "7");
    image.SetGeometryAttribute("height", "8");
    image.SetGeometryAttribute("href", "/common/icon.png");
    SvgElementBase* cloneBase = image.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgImageNode* clone = static_cast<SvgImageNode*>(cloneBase);
    Rect b = clone->GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 5);
    EXPECT_EQ(b.GetTop(), 6);
    EXPECT_EQ(b.GetWidth(), 7);
    EXPECT_EQ(b.GetHeight(), 8);
    delete cloneBase;
}

} // namespace OHOS
