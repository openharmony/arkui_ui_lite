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

#include "gtest/gtest.h"
#include "svg/svg_engine.h"
#include "components/ui_view.h"
#include "components/ui_view_group.h"

using namespace testing::ext;

namespace OHOS {

class SvgEngineTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(SvgEngineTest, BuildRenderDestroy_001, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    ASSERT_TRUE(doc != nullptr);
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgEngine::SetAttribute(root, "viewBox", "0 0 100 100");
    SvgElementHandle rect = SvgEngine::CreateElement(doc, SVG_RECT);
    SvgEngine::SetAttribute(rect, "x", "10");
    SvgEngine::SetAttribute(rect, "width", "50");
    SvgEngine::SetAttribute(rect, "fill", "#ff0000");
    SvgEngine::AppendChild(root, rect);
    SvgEngine::SetRoot(doc, root);
    UIViewGroup host;
    host.SetPosition(0, 0, 200, 200);
    UIView* view = SvgEngine::AttachToView(doc, &host);
    EXPECT_TRUE(view != nullptr);
    EXPECT_EQ(host.GetChildrenHead(), view);
    SvgEngine::Render(doc);
    SvgEngine::Invalidate(doc);
    SvgEngine::DestroyDocument(doc);
}

HWTEST_F(SvgEngineTest, GradientWithStops_002, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgElementHandle defs = SvgEngine::CreateElement(doc, SVG_DEFS);
    SvgElementHandle grad = SvgEngine::CreateElement(doc, SVG_LINEAR_GRADIENT);
    SvgEngine::SetAttribute(grad, "id", "g1");
    SvgElementHandle stop = SvgEngine::CreateElement(doc, SVG_STOP);
    SvgEngine::SetAttribute(stop, "offset", "0");
    SvgEngine::SetAttribute(stop, "stop-color", "red");
    SvgEngine::AppendChild(grad, stop);
    SvgEngine::AppendChild(defs, grad);
    SvgEngine::AppendChild(root, defs);
    SvgElementHandle rect = SvgEngine::CreateElement(doc, SVG_RECT);
    SvgEngine::SetAttribute(rect, "fill", "url(#g1)");
    SvgEngine::AppendChild(root, rect);
    SvgEngine::SetRoot(doc, root);
    SvgEngine::DestroyDocument(doc);
}

HWTEST_F(SvgEngineTest, FullVocabularyCreatable_003, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    for (uint16_t t = SVG_ROOT; t <= SVG_UNKNOWN; t++) {
        SvgElementHandle e = SvgEngine::CreateElement(doc, static_cast<SvgElementType>(t));
        EXPECT_TRUE(e != nullptr) << "type=" << t;
    }
    SvgEngine::DestroyDocument(doc);
}

HWTEST_F(SvgEngineTest, UseSwitchAnchor_004, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgElementHandle g = SvgEngine::CreateElement(doc, SVG_GROUP);
    SvgEngine::SetAttribute(g, "id", "shape");
    SvgEngine::AppendChild(root, g);
    SvgElementHandle use = SvgEngine::CreateElement(doc, SVG_USE);
    SvgEngine::SetAttribute(use, "xlink:href", "#shape");
    SvgEngine::AppendChild(root, use);
    SvgEngine::SetRoot(doc, root);
    SvgEngine::DestroyDocument(doc);
    SUCCEED();
}

HWTEST_F(SvgEngineTest, SyncViewportFallbackAndViewBoxTransform_005, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    ASSERT_TRUE(doc != nullptr);
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgEngine::SetAttribute(root, "viewBox", "0 0 200 100");
    SvgElementHandle rect = SvgEngine::CreateElement(doc, SVG_RECT);
    SvgEngine::SetAttribute(rect, "x", "0");
    SvgEngine::SetAttribute(rect, "y", "0");
    SvgEngine::SetAttribute(rect, "width", "100");
    SvgEngine::SetAttribute(rect, "height", "100");
    SvgEngine::AppendChild(root, rect);
    SvgEngine::SetRoot(doc, root);

    UIViewGroup host;
    host.SetPosition(0, 0, 480, 320);
    UIView* view = SvgEngine::AttachToView(doc, &host);
    EXPECT_TRUE(view != nullptr);

    // Root should fallback to host content size.
    EXPECT_EQ(view->GetWidth(), 480);
    EXPECT_EQ(view->GetHeight(), 320);

    // Child rect should be scaled by uniform meet = min(480/200, 320/100) = 2.4
    // width = 100 * 2.4 = 240.
    UIView* child = static_cast<UIViewGroup*>(view)->GetChildrenHead();
    ASSERT_TRUE(child != nullptr);
    EXPECT_EQ(child->GetWidth(), 240);

    SvgEngine::DestroyDocument(doc);
}

HWTEST_F(SvgEngineTest, SyncViewportExplicitSizeOverridesHost_006, TestSize.Level1)
{
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    ASSERT_TRUE(doc != nullptr);
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgEngine::SetAttribute(root, "width", "100");
    SvgEngine::SetAttribute(root, "height", "50");
    SvgEngine::SetAttribute(root, "viewBox", "0 0 100 50");
    SvgEngine::SetRoot(doc, root);

    UIViewGroup host;
    host.SetPosition(0, 0, 480, 320);
    UIView* view = SvgEngine::AttachToView(doc, &host);
    EXPECT_TRUE(view != nullptr);

    // Explicit width/height should override host size.
    EXPECT_EQ(view->GetWidth(), 100);
    EXPECT_EQ(view->GetHeight(), 50);

    SvgEngine::DestroyDocument(doc);
}

// Cover the null/invalid guards in SvgEngine facade methods (svg_engine.cpp).
HWTEST_F(SvgEngineTest, NullGuards_007, TestSize.Level1)
{
    UIViewGroup host;
    host.SetPosition(0, 0, 200, 200);
    UIView notGroup;

    // null document / null parent guards
    SvgEngine::SetAttribute(nullptr, "x", "1");
    SvgEngine::AppendChild(nullptr, nullptr);
    SvgEngine::SetRoot(nullptr, nullptr);
    SvgEngine::UpdateRootViewport(nullptr, 10, 10);
    SvgEngine::Render(nullptr);
    SvgEngine::Invalidate(nullptr);
    SvgEngine::StartAnimation(nullptr);
    SvgEngine::StopAnimation(nullptr);
    SvgEngine::DestroyDocument(nullptr);
    EXPECT_EQ(SvgEngine::AttachToView(nullptr, &host), nullptr);
    EXPECT_EQ(SvgEngine::AttachToView(nullptr, nullptr), nullptr);

    // doc with a root but parent is not a UIViewGroup -> dynamic_cast guard
    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgEngine::SetRoot(doc, root);
    EXPECT_EQ(SvgEngine::AttachToView(doc, &notGroup), nullptr);

    // doc without a root -> root guard
    SvgDocumentHandle doc2 = SvgEngine::CreateDocument();
    EXPECT_EQ(SvgEngine::AttachToView(doc2, &host), nullptr);

    SvgEngine::DestroyDocument(doc);
    SvgEngine::DestroyDocument(doc2);
}

// Cover the new PauseAnimations / UnpauseAnimations facade passthroughs (svg_engine.cpp).
HWTEST_F(SvgEngineTest, PauseUnpauseGuard_008, TestSize.Level1)
{
    // Null document and not-yet-started documents must be tolerated (no animator -> no-op).
    SvgEngine::PauseAnimations(nullptr);
    SvgEngine::UnpauseAnimations(nullptr);

    SvgDocumentHandle doc = SvgEngine::CreateDocument();
    SvgElementHandle root = SvgEngine::CreateElement(doc, SVG_ROOT);
    SvgEngine::SetRoot(doc, root);
    UIViewGroup host;
    host.SetPosition(0, 0, 200, 200);
    UIView* view = SvgEngine::AttachToView(doc, &host);
    EXPECT_TRUE(view != nullptr);
    SvgEngine::PauseAnimations(doc);
    SvgEngine::UnpauseAnimations(doc);
    SvgEngine::DestroyDocument(doc);
    SUCCEED();
}

} // namespace OHOS
