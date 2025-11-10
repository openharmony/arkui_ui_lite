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
 
#ifndef ROTATE_MANAGER_H
#define ROTATE_MANAGER_H
 
#include "gfx_utils/list.h"
#include "rotate_event_listener.h"
#include "events/rotate_event.h"
 
namespace OHOS {
class RotateManager : public HeapBase {
public:
    static RotateManager& GetInstance(void);
 
    // 注册、删除、注销监听器
    void Add(RotateEventListener* listener);
    void Remove(RotateEventListener* listener);
    void Clear();
 
    // 获取已注册的监听器列表
    const List<RotateEventListener*>& GetRegisteredListeners() const;
 
    // 触发旋转事件（分发给所有注册的监听器）
    void OnRotateStart(const RotateEvent& event);
    void OnRotate(const RotateEvent& event);
    void OnRotateEnd(const RotateEvent& event);
 
private:
    List<RotateEventListener*> rotateList_;
    RotateManager() = default;
    RotateManager(const RotateManager&) = delete;
    RotateManager& operator=(const RotateManager&) = delete;
};
}
#endif // ROTATE_MANAGER_H
