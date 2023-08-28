/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#ifndef UI_TEST_IMAGE_ANIMATOR_H
#define UI_TEST_IMAGE_ANIMATOR_H

#include "components/ui_image_animator.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "layout/grid_layout.h"
#include "ui_test.h"

namespace OHOS {
class TestAnimatorStopListener : public UIImageAnimatorView::AnimatorStopListener {
public:
    explicit TestAnimatorStopListener(UIViewGroup* viewGroup) : viewGroup_(viewGroup)
    {
        viewGroup_ = viewGroup;
    }
    ~TestAnimatorStopListener() {}

    void Init()
    {
        label_ = new UILabel();
        viewGroup_->Add(label_);
        label_->SetPosition(24, 50, 200, 29); // 24: position x 50: position y 200: width 29: height
        label_->SetText("OnAnimatorStop");
        label_->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
        label_->SetStyle(STYLE_TEXT_COLOR, Color::Black().full);
        label_->SetStyle(STYLE_BACKGROUND_COLOR, Color::White().full);
        label_->SetStyle(STYLE_BACKGROUND_OPA, 255); // 255: value
        label_->SetVisible(false);
    }

    void OnAnimatorStop(UIView& view) override
    {
        label_->SetVisible(true);
        label_->Invalidate();
    }

private:
    UILabel* label_ = nullptr;
    UIViewGroup* viewGroup_ = nullptr;
};

class UITestImageAnimator : public UITest, public UIView::OnClickListener {
public:
    UITestImageAnimator() {}
    ~UITestImageAnimator() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    void SetUpButton(UILabelButton* btn, const char* title);

    bool OnClick(UIView& view, const ClickEvent& event) override;

    void UIKitImageAnimatorTestStart001();
    void UIKitImageAnimatorTestStop002();
    void UIKitImageAnimatorTestPause003();
    void UIKitImageAnimatorTestResume004();
    void UIKitImageAnimatorTestSetImageAnimatorSrc005();
    void UIKitImageAnimatorTestSetTickOfUpdate006();
    void UIKitImageAnimatorTestSetSizeFixed007();
    void UIKitImageAnimatorTestSetRepeat008();
    void UIKitImageAnimatorTestSetReverse009();
    void UIKitImageAnimatorTestSetAnimatorStopListener010();
    void UIKitImageAnimatorTestSetRepeatTimes011();
    void UIKitImageAnimatorTestSetFillModeTrueForward012();
    void UIKitImageAnimatorTestSetFillModeTrueBackward013();
    void UIKitImageAnimatorTestSetFillModeFalseForward014();
    void UIKitImageAnimatorTestSetFillModeFalseBackward015();
    void UIKitImageAnimatorTestSetImageInfo016();

private:
    void InitImageInfo();
    void SetSrcData3(uint8_t* srcData3, uint8_t* srcData2, uint8_t* srcData1,
                     uint32_t dataSize, bool& isReturn, const uint16_t pixelByteSize);
    void SetImageInfoPara(ImageInfo* imageInfo,
                          const uint16_t width,
                          const uint16_t height,
                          uint32_t dataSize,
                          uint8_t* srcData);
    void SetimageAnimatorImageInfos();
    UIScrollView* container_ = nullptr;
    GridLayout* layout_ = nullptr;
    UIImageAnimatorView* imageAnimator_ = nullptr;
    TestAnimatorStopListener* listener_ = nullptr;

    UILabelButton* startBtn_ = nullptr;
    UILabelButton* stopBtn_ = nullptr;
    UILabelButton* pauseBtn_ = nullptr;
    UILabelButton* resumeBtn_ = nullptr;
    UILabelButton* setImageBtn_ = nullptr;
    UILabelButton* setSpeedBtn_ = nullptr;
    UILabelButton* fixedBtn_ = nullptr;
    UILabelButton* repeatBtn_ = nullptr;
    UILabelButton* noRepeatBtn_ = nullptr;
    UILabelButton* reverseOrderBtn_ = nullptr;
    UILabelButton* positiveOrderBtn_ = nullptr;
    UILabelButton* listenerBtn_ = nullptr;
    UILabelButton* repeatTimesBtn_ = nullptr;
    UILabelButton* fillModeTrueForwardBtn_ = nullptr;
    UILabelButton* fillModeTrueBackwardBtn_ = nullptr;
    UILabelButton* fillModeFalseForwardBtn_ = nullptr;
    UILabelButton* fillModeFalseBackwardBtn_ = nullptr;
    UILabelButton* setImageInfoBtn_ = nullptr;

    ImageInfo* imageInfo1_ = nullptr;
    ImageInfo* imageInfo2_ = nullptr;
    ImageInfo* imageInfo3_ = nullptr;
    ImageAnimatorInfo imageAnimatorImageInfo_[3] = { 0 };
};
} // namespace OHOS
#endif // UI_TEST_IMAGE_ANIMATOR_H
