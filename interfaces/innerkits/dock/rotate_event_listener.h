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
 
#ifndef ROTATE_EVENT_LISTENER_H
#define ROTATE_EVENT_LISTENER_H
 
#include "events/rotate_event.h"
 
namespace OHOS {
class RotateEventListener : public HeapBase {
public:
    virtual ~RotateEventListener() {};
    virtual bool OnRotateStartEvent(const RotateEvent& event)
    {
        return false;
    }
    virtual bool OnRotateEvent(const RotateEvent& event)
    {
        return false;
    }
    virtual bool OnRotateEndEvent(const RotateEvent& event)
    {
        return false;
    }
};
}
#endif // ROTATE_EVENT_LISTENER_H
