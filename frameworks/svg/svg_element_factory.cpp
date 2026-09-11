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

#include "svg/svg_element_factory.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_text_nodes.h"
#include "svg/svg_image_node.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_generic_node.h"
#include "svg/svg_animation.h"

namespace OHOS {

namespace {

SvgElementBase* TryCreateContainer(SvgElementType type)
{
    switch (type) {
        case SVG_ROOT:
            return new SvgRootNode();
        case SVG_GROUP:
            return new SvgGroupNode();
        case SVG_DEFS:
            return new SvgDefsResource();
        case SVG_USE:
            return new SvgUseNode();
        case SVG_SWITCH:
            return new SvgSwitchNode();
        default:
            return nullptr;
    }
}

SvgElementBase* TryCreateShape(SvgElementType type)
{
    switch (type) {
        case SVG_RECT:
            return new SvgRectNode();
        case SVG_CIRCLE:
            return new SvgCircleNode();
        case SVG_ELLIPSE:
            return new SvgCircleNode(SvgCircleNode::Mode::ELLIPSE);
        case SVG_LINE:
            return new SvgLineNode(SvgLineNode::LINE);
        case SVG_POLYLINE:
            return new SvgLineNode(SvgLineNode::POLYLINE);
        case SVG_POLYGON:
            return new SvgLineNode(SvgLineNode::POLYGON);
        case SVG_PATH:
            return new SvgPathNode();
        default:
            return nullptr;
    }
}

SvgElementBase* TryCreateText(SvgElementType type)
{
    switch (type) {
        case SVG_TEXT:
            return new SvgTextNode();
        case SVG_TEXT_AREA:
            return new SvgTextAreaNode();
        case SVG_TSPAN:
            return new SvgTSpanNode();
        default:
            return nullptr;
    }
}

SvgElementBase* TryCreatePaintServer(SvgElementType type)
{
    switch (type) {
        case SVG_LINEAR_GRADIENT:
            return new SvgLinearGradientResource();
        case SVG_RADIAL_GRADIENT:
            return new SvgRadialGradientResource();
        case SVG_STOP:
            return new SvgStopResource();
        case SVG_SOLID_COLOR:
            return new SvgSolidColorResource();
        default:
            return nullptr;
    }
}

SvgElementBase* TryCreateImage(SvgElementType type)
{
    if (type == SVG_IMAGE) {
        return new SvgImageNode();
    }
    return nullptr;
}

SvgElementBase* TryCreateAnimation(SvgElementType type)
{
    switch (type) {
        case SVG_ANIMATE:
            return new SvgAnimate();
        case SVG_ANIMATE_TRANSFORM:
            return new SvgAnimateTransform();
        case SVG_ANIMATE_COLOR:
            return new SvgAnimateColor();
        case SVG_ANIMATE_MOTION:
            return new SvgAnimateMotion();
        case SVG_SET:
            return new SvgSet();
        case SVG_MPATH:
            return new SvgMPath();
        default:
            return nullptr;
    }
}

} // namespace

SvgElementBase* CreateSvgElementByType(SvgElementType type)
{
    SvgElementBase* node = TryCreateContainer(type);
    if (node != nullptr) {
        return node;
    }
    node = TryCreateShape(type);
    if (node != nullptr) {
        return node;
    }
    node = TryCreateText(type);
    if (node != nullptr) {
        return node;
    }
    node = TryCreatePaintServer(type);
    if (node != nullptr) {
        return node;
    }
    node = TryCreateImage(type);
    if (node != nullptr) {
        return node;
    }
    node = TryCreateAnimation(type);
    if (node != nullptr) {
        return node;
    }
    return new SvgGenericNode(type);
}

} // namespace OHOS
