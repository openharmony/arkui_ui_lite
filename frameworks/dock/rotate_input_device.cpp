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

#include "dock/rotate_input_device.h"
#include "gfx_utils/graphic_log.h"
#include "dock/focus_manager.h"


#if ENABLE_ROTATE_INPUT
namespace {
#ifdef _WIN32
constexpr int16_t ROTATE_INPUT_THRESHOLD = 1;
#else
constexpr int16_t ROTATE_INPUT_THRESHOLD = 10;
#endif
constexpr uint8_t ROTATE_END_ZERO_COUNT = 2;
}

namespace OHOS {
RotateInputDevice::RotateInputDevice()
    : rotateStart_(false), threshold_(ROTATE_INPUT_THRESHOLD), cachedRotation_(0), zeroCount_(0)
{
}

void RotateInputDevice::DispatchEvent(const DeviceData& data)
{
    bool cachedToRotate = false;
    if (data.rotate == 0 || rotateStart_) {
        cachedRotation_ = 0;
    } else {
        cachedRotation_ += data.rotate;
        if (MATH_ABS(cachedRotation_) >= threshold_) {
            cachedToRotate = true;
        }
    }

    if (!cachedToRotate && !rotateStart_) {
        return;
    }

    UIView* view = FocusManager::GetInstance()->GetFocusedView();
    RotateManager& manager = RotateManager::GetInstance();

    if (!(IsViewValidAndVisible(view)) && !IsGlobalListener(manager)) {
        GRAPHIC_LOGW("No focus view and no global table crown registered, unable to schedule events!\n");
        return;
    }

    if (IsDispatchGlobalEvent(manager)) {
        DispatchToGlobal(data, manager);
    }
    if (IsDispatchFocusedEvent(view)) {
        DispatchToFocusedView(data, view);
    }
}

void RotateInputDevice::DispatchToGlobal(const DeviceData& data, RotateManager& manager)
{
    if (data.rotate == 0 && rotateStart_) {
        zeroCount_++;
        if (zeroCount_ >= ROTATE_END_ZERO_COUNT) {
            manager.OnRotateEnd(data.rotate);
        }
        zeroCount_ = 0;
        rotateStart_ = false;
        globalRotateEventStatus_ = false;
        GRAPHIC_LOGW("RotateInputDevice dispatched 0-value event!\n");
        return;
    }
    globalRotateEventStatus_ = true;
    if (!rotateStart_) {
        manager.OnRotateStart(data.rotate);
    }
    manager.OnRotate(data.rotate);
    rotateStart_ = true;
}

void RotateInputDevice::DispatchToFocusedView(const DeviceData& data, UIView* view)
{
    if (data.rotate == 0 && rotateStart_) {
        zeroCount_++;
        if (zeroCount_ >= ROTATE_END_ZERO_COUNT) {
            view->OnRotateEnd(data.rotate);
        }
        zeroCount_ = 0;
        rotateStart_ = false;
        focusEventStatus_ = false;
        GRAPHIC_LOGW("RotateInputDevice dispatched 0-value event!\n");
        return;
    }
    focusEventStatus_ = true;
    if (!rotateStart_) {
        view->OnRotateStartEvent(data.rotate);
    }
    view->OnRotateEvent(data.rotate);
    rotateStart_ = true;
    GRAPHIC_LOGI("RotateInputDevice dispatched rotate event, targetView Type = %{public}d,\
        rotate value = %{public}d\n!", static_cast<uint8_t>(view->GetViewType()), data.rotate);
}

bool RotateInputDevice::IsViewValidAndVisible(UIView* view)
{
    if (view == nullptr) {
        return false;
    }
    UIView* parent = view;
    while (parent != nullptr && parent->GetParent() != nullptr) {
        if (!parent->IsVisible()) {
            return false;
        }
        parent = parent->GetParent();
    }
    if (parent->GetViewType() == UI_ROOT_VIEW) {
        return true;
    } else {
        GRAPHIC_LOGW("RotateInputDevice failed to dispatch event without target view attached!\n");
        return false;
    }
}

bool WearRotateInputDevice::IsDispatchFocusedEvent(UIView* view)
{
    /* Global events are being distributed. */
    if (globalRotateEventStatus_) {
        return false;
    }

    /* The focus view is deregistered during the rotation of the focus view. */
    if ((!IsViewValidAndVisible(view))  && focusEventStatus_) {
        focusEventStatus_ = false;
        rotateStart_ = false;
        return false;
    }
    return true;
}

bool RotateInputDevice::IsDispatchGlobalEvent(RotateManager& manager)
{
    /* focues events are being distributed. */
    if (focusEventStatus_) {
        return false;
    }
    /* Global is deregistered during global rotation. */
    if (manager.GetRegisteredListeners().IsEmpty() && globalRotateEventStatus_)) {
        globalRotateEventStatus_ = false;
        rotateStart_ = false;
        return false;
    }
    return !manager.GetRegisteredListeners().IsEmpty();
}
} // namespace OHOS
#endif
