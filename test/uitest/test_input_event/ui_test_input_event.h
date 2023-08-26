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

#ifndef UI_TEST_INPUT_EVENT_H
#define UI_TEST_INPUT_EVENT_H

#include "components/root_view.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "dock/input_device.h"
#include "ui_test.h"

namespace OHOS {
class TestView : public UIView {
public:
    TestView() {}
    virtual ~TestView() {}
    bool OnLongPressEvent(const LongPressEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("long press!");
            label_->Invalidate();
        }
        return UIView::OnLongPressEvent(event);
    }

    bool OnDragEvent(const DragEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("drag!");
            label_->Invalidate();
        }
        return UIView::OnDragEvent(event);
    }

    void SetSentence(const char* sentence)
    {
        sentence_ = sentence;
    }

    bool OnClickEvent(const ClickEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText(sentence_);
            label_->Invalidate();
        }
        return UIView::OnClickEvent(event);
    }

    bool OnPressEvent(const PressEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("press!");
            label_->Invalidate();
        }
        return UIView::OnPressEvent(event);
    }

    bool OnReleaseEvent(const ReleaseEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("release!");
            label_->Invalidate();
        }
        return UIView::OnReleaseEvent(event);
    }

    bool OnCancelEvent(const CancelEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("cancel!");
            label_->Invalidate();
        }
        return UIView::OnCancelEvent(event);
    }

    void SetLabel(UILabel* label)
    {
        label_ = label;
    }

    void SetLabel2(UILabel* label)
    {
        label2_ = label;
    }

private:
    UILabel* label_ = nullptr;
    UILabel* label2_ = nullptr;
    const char* sentence_ = "click";
};

class TestUIScrollView : public UIScrollView {
public:
    TestUIScrollView() {}
    virtual ~TestUIScrollView() {}
    bool OnLongPressEvent(const LongPressEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("long press!");
            label_->Invalidate();
        }
        return UIView::OnLongPressEvent(event);
    }

    bool OnDragEvent(const DragEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("drag!");
            label_->Invalidate();
        }
        return UIScrollView::OnDragEvent(event);
    }

    bool OnDragStartEvent(const DragEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("drag start!");
            label_->Invalidate();
        }
        return UIScrollView::OnDragStartEvent(event);
    }

    bool OnDragEndEvent(const DragEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("drag end!");
            label_->Invalidate();
        }
        return UIScrollView::OnDragEndEvent(event);
    }

    void SetSentence(const char* sentence)
    {
        sentence_ = sentence;
    }

    bool OnClickEvent(const ClickEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText(sentence_);
            label_->Invalidate();
        }
        return UIView::OnClickEvent(event);
    }

    bool OnPressEvent(const PressEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("press!");
            label_->Invalidate();
        }
        return UIView::OnPressEvent(event);
    }

    bool OnReleaseEvent(const ReleaseEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("release!");
            label_->Invalidate();
        }
        return UIView::OnReleaseEvent(event);
    }

    bool OnCancelEvent(const CancelEvent& event) override
    {
        if (label_ != nullptr) {
            label_->SetText("cancel!");
            label_->Invalidate();
        }
        return UIView::OnCancelEvent(event);
    }

    void SetLabel(UILabel* label)
    {
        label_ = label;
    }

private:
    UILabel* label_ = nullptr;
    const char* sentence_ = "click";
};

class TestKeyInputListener : public RootView::OnKeyActListener {
public:
    explicit TestKeyInputListener(UILabel* label) : label_(label) {}
    virtual ~TestKeyInputListener() {}
    bool OnKeyAct(UIView& view, const KeyEvent& event) override
    {
        if (label_ == nullptr) {
            return true;
        }
        switch (event.GetState()) {
            case InputDevice::STATE_PRESS:
                label_->SetText("key pressed!");
                break;
            case InputDevice::STATE_RELEASE:
                label_->SetText("key released!");
                break;
            default:
                label_->SetText("");
                break;
        }
        label_->Invalidate();
        return true;
    }

private:
    UILabel* label_;
};

class TestOnClickListener : public UIView::OnClickListener {
public:
    explicit TestOnClickListener(UILabel* label, char* sentence, bool isConsume)
        : label_(label), sentence_(sentence), isConsume_(isConsume)
    {
    }
    virtual ~TestOnClickListener() {}
    virtual bool OnClick(UIView& view, const ClickEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(sentence_);
            label_->Invalidate();
        }
        return isConsume_;
    }

private:
    UILabel* label_;
    char* sentence_;
    bool isConsume_;
};

class TestOnLongPressListener : public UIView::OnLongPressListener {
public:
    explicit TestOnLongPressListener(UILabel* label, char* sentence, bool isConsume)
        : label_(label), sentence_(sentence), isConsume_(isConsume)
    {
    }
    virtual ~TestOnLongPressListener() {}
    virtual bool OnLongPress(UIView& view, const LongPressEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(sentence_);
            label_->Invalidate();
        }
        return isConsume_;
    }

private:
    UILabel* label_;
    char* sentence_;
    bool isConsume_;
};

class TestOnTouchListener : public UIView::OnTouchListener {
public:
    explicit TestOnTouchListener(UILabel* label, char* strPress, char* strRelease, char* strCancel, bool isConsume)
        : label_(label), strPress_(strPress), strRelease_(strRelease), strCancel_(strCancel), isConsume_(isConsume)
    {
    }
    virtual ~TestOnTouchListener() {}
    virtual bool OnPress(UIView& view, const PressEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strPress_);
            label_->Invalidate();
        }
        return isConsume_;
    }

    virtual bool OnRelease(UIView& view, const ReleaseEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strRelease_);
            label_->Invalidate();
        }
        return isConsume_;
    }

    virtual bool OnCancel(UIView& view, const CancelEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strCancel_);
            label_->Invalidate();
        }
        return isConsume_;
    }

private:
    UILabel* label_;
    char* strPress_;
    char* strRelease_;
    char* strCancel_;
    bool isConsume_;
};

class TestOnDragListener : public UIView::OnDragListener {
public:
    explicit TestOnDragListener(UILabel* label, char* strDragStart, char* strDrag, char* strDragEnd, bool isConsume)
        : label_(label), strDragStart_(strDragStart), strDrag_(strDrag), strDragEnd_(strDragEnd), isConsume_(isConsume)
    {
    }
    virtual ~TestOnDragListener() {}
    virtual bool OnDragStart(UIView& view, const DragEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strDragStart_);
            label_->Invalidate();
        }
        return isConsume_;
    }

    virtual bool OnDrag(UIView& view, const DragEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strDrag_);
            label_->Invalidate();
        }
        return isConsume_;
    }

    virtual bool OnDragEnd(UIView& view, const DragEvent& event)
    {
        if (label_ != nullptr) {
            label_->SetText(strDragEnd_);
            label_->Invalidate();
        }
        return isConsume_;
    }

private:
    UILabel* label_;
    char* strDragStart_;
    char* strDrag_;
    char* strDragEnd_;
    bool isConsume_;
};

class UITestInputEvent : public UITest {
public:
    UITestInputEvent() {}
    ~UITestInputEvent() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    /**
     * @brief Test if dispatch press\release\longpress\cancel event act normal when target is touchable
     */
    void UIKitPointerInputTestDispatchSimpleEvent001();
    /**
     * @brief Test if dispatch press\release\longpress\cancel\drag event act normal when target is untouchable
     */
    void UIKitPointerInputTestDispatchSimpleEvent002();
    /**
     * @brief Test if dispatch drag event act normal when target is touchable and draggable and drag parent instead.
     */
    void UIKitPointerInputTestDispatchDragEvent001();
    /**
     * @brief Test if dispatch drag event act normal when target is touchable and draggable and not drag parent instead.
     */
    void UIKitPointerInputTestDispatchDragEvent002();
    /**
     * @brief Test if dispatch drag event act normal when target is untouchable but draggable.
     */
    void UIKitPointerInputTestDispatchDragEvent003();
    /**
     * @brief Test if dispatch drag event act normal when target is untouchable but draggable.
     */
    void UIKitPointerInputTestDispatchKeyEvent001();
    /**
     * @brief Test if dispatch drag event act normal when target is untouchable but draggable.
     */
    void UIKitPointerInputTestDispatchInVisibleEvent001();
    /**
     * @brief Test click, release or longClick event bubble act normal when both of parent and child is triggered.
     */
    void UIKitPointerInputTestDispatchBubble001();
    /**
     * @brief Test click, release or longClick event bubble act normal when child is triggered but and parent not.
     */
    void UIKitPointerInputTestDispatchBubble002();
    /**
     * @brief Test click, release or longClick event bubble act normal when child is triggered but and parent not.
     */
    void UIKitPointerInputTestDispatchBubble003();
    /**
     * @brief Test click, release or longClick event bubble act normal when parent is triggered but and child not.
     */
    void UIKitPointerInputTestDispatchBubble004();
    /**
     * @brief Test drag event bubble act normal when both of parent and child is triggered.
     */
    void UIKitPointerInputTestDispatchBubble005();
    /**
     * @brief Test drag event bubble act normal when child is triggered but and parent not.
     */
    void UIKitPointerInputTestDispatchBubble006();
    /**
     * @brief Test drag event bubble act normal when child is triggered but and parent not.
     */
    void UIKitPointerInputTestDispatchBubble007();
    /**
     * @brief Test drag event bubble act normal when both of parent and child is not triggered.
     */
    void UIKitPointerInputTestDispatchBubble008();
    /**
     * @brief Test drag event bubble act normal when parent is triggered but and child not.
     */
    void UIKitPointerInputTestDispatchBubble009();
    /**
     * @brief Test drag event bubble act normal when parent is triggered but and child not.
     */
    void UIKitPointerInputTestDispatchBubble010();

private:
    UIScrollView* container_ = nullptr;
    TestKeyInputListener* keyListener_ = nullptr;
    void InnerTest(const char* title, bool touchable, bool draggable, bool dragParent);
    void InnerBubbleTest(const char* title, bool touchable, bool draggable, bool hasListener, bool isBubble);
    void InnerBubbleDragTest(const char* title,
                             bool childDraggable,
                             bool parentDraggable,
                             bool hasListener,
                             bool isBubble);
    void SetViewAndContainerListeners(bool isBubble, OHOS::UIScrollView* parentContainer,
                                      TestView* testView, UILabel* label1, UILabel* label2);
    void SetScrollsListeners(bool isBubble, OHOS::UIScrollView* parentScroll,
                             OHOS::UIScrollView* childScroll, UILabel* label1, UILabel* label2);
};
} // namespace OHOS
#endif // UI_TEST_INPUT_EVENT_H
