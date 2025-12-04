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
#include "dock/rotate_manager.h"
#include "gfx_utils/graphic_log.h"

namespace OHOS {

RotateManager& RotateManager::GetInstance(void)
{
    static RotateManager instance;
    return instance;
}

bool RotateManager::Add(RotateEventListener* listener)
{
    if (listener == nullptr) {
        GRAPHIC_LOGE("RotateManager::Add invalid param\n");
        return false;
    }
    if (!rotateList_.IsEmpty()) {
        rotateList_.Clear();
    }
    rotateList_.PushFront(listener);
    return true;
}

bool RotateManager::Remove(RotateEventListener* listener)
{
    if (listener == nullptr) {
        return false;
    }

    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        if (it->data_ == listener) {
            rotateList_.Remove(it);
            return true;
        }
        it = it->next_;
    }
    GRAPHIC_LOGW("RotateManager::Remove listener not found\n");
    return false;
}

bool RotateManager::Clear()
{
    rotateList_.Clear();
    return true;
}

const List<RotateEventListener*>& RotateManager::GetRegisteredListeners() const
{
    return rotateList_;
}

bool RotateManager::OnRotateStart(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            if (listener->OnRotateStartEvent(event)) {
                return true;
            };
        }
        it = it->next_;
    }
    return false;
}

bool RotateManager::OnRotate(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            if (listener->OnRotateEvent(event)) {
                return true;
            };
        }
        it = it->next_;
    }
    return false;
}

bool RotateManager::OnRotateEnd(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            if (listener->OnRotateEndEvent(event)) {
                return true;
            };
        }
        it = it->next_;
    }
    return false;
}
} // namespace OHOS