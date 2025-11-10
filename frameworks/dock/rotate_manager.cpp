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

void RotateManager::Add(RotateEventListener* listener)
{
    if (listener == nullptr) {
        GRAPHIC_LOGE("RotateManager::Add invalid param\n");
        return;
    }

    // 遍历链表，避免重复添加
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        if (it->data_ == listener) {
            GRAPHIC_LOGW("RotateManager::Add listener already registered\n");
            return;
        }
        it = it->next_;
    }

    rotateList_.PushBack(listener);
}

void RotateManager::Remove(RotateEventListener* listener)
{
    if (listener == nullptr) {
        return;
    }

    bool found = false;
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        if (it->data_ == listener) {
            rotateList_.Remove(it);
            found = true;
            break;
        }
        it = it->next_;
    }

    if (!found) {
        GRAPHIC_LOGW("RotateManager::Remove listener not found\n");
    }
}

void RotateManager::Clear()
{
    rotateList_.Clear();
}

const List<RotateEventListener*>& RotateManager::GetRegisteredListeners() const
{
    return rotateList_;
}

void RotateManager::OnRotateStart(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            listener->OnRotateStartEvent(event);
        }
        it = it->next_;
    }
}

void RotateManager::OnRotate(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            listener->OnRotateEvent(event);
        }
        it = it->next_;
    }
}

void RotateManager::OnRotateEnd(const RotateEvent& event)
{
    ListNode<RotateEventListener*>* it = rotateList_.Begin();
    while (it != rotateList_.End()) {
        RotateEventListener* listener = it->data_;
        if (listener != nullptr) {
            listener->OnRotateEndEvent(event);
        }
        it = it->next_;
    }
}
} // namespace OHOS