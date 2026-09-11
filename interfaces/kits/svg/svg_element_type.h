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

#ifndef GRAPHIC_LITE_SVG_ELEMENT_TYPE_H
#define GRAPHIC_LITE_SVG_ELEMENT_TYPE_H

#include <cstdint>

namespace OHOS {

// SvgElementType is part of the public SVG facade contract (see SvgEngine::CreateElement),
// so its single definition lives in the public kits layer and is shared by both the
// public facade and the internal engine implementation.
#ifndef GRAPHIC_LITE_SVG_ELEMENT_TYPE_DEFINED
#define GRAPHIC_LITE_SVG_ELEMENT_TYPE_DEFINED
enum SvgElementType : uint8_t {
    SVG_ROOT = 0,             // <svg> root element
    SVG_GROUP,                // <g> container group
    SVG_DEFS,                 // <defs> definition section, not rendered directly, referenced by id
    SVG_DESC,                 // <desc> description text (metadata, inert placeholder)
    SVG_TITLE,                // <title> title (metadata, inert placeholder)
    SVG_METADATA,             // <metadata> metadata (inert placeholder)
    SVG_USE,                  // <use> reference and reuse an existing element
    SVG_IMAGE,                // <image> embedded raster image
    SVG_SWITCH,               // <switch> conditional child selection
    SVG_A,                    // <a> hyperlink container
    SVG_RECT,                 // <rect> rectangle
    SVG_CIRCLE,               // <circle> circle
    SVG_ELLIPSE,              // <ellipse> ellipse
    SVG_LINE,                 // <line> straight line
    SVG_POLYLINE,             // <polyline> polyline
    SVG_POLYGON,              // <polygon> polygon
    SVG_PATH,                 // <path> path
    SVG_TEXT,                 // <text> text
    SVG_TSPAN,                // <tspan> text span
    SVG_TBREAK,               // <tbreak> text break
    SVG_TREF,                 // <tref> reference to other text
    SVG_TEXT_AREA,            // <textArea> multiline text area
    SVG_LINEAR_GRADIENT,      // <linearGradient> linear gradient
    SVG_RADIAL_GRADIENT,      // <radialGradient> radial gradient
    SVG_STOP,                 // <stop> gradient stop
    SVG_SOLID_COLOR,          // <solidColor> solid color
    SVG_FONT,                 // <font> font definition (inert placeholder)
    SVG_GLYPH,                // <glyph> glyph (inert placeholder)
    SVG_MISSING_GLYPH,        // <missing-glyph> missing glyph (inert placeholder)
    SVG_HKERN,                // <hkern> horizontal kerning (inert placeholder)
    SVG_FONT_FACE,            // <font-face> font face (inert placeholder)
    SVG_FONT_FACE_SRC,        // <font-face-src> font source (inert placeholder)
    SVG_FONT_FACE_URI,        // <font-face-uri> font URI (inert placeholder)
    SVG_STYLE,                // <style> style sheet (inert placeholder)
    SVG_SCRIPT,               // <script> script (inert placeholder)
    SVG_VIDEO,                // <video> video (inert placeholder)
    SVG_AUDIO,                // <audio> audio (inert placeholder)
    SVG_ANIMATION,            // <animation> animation (inert placeholder)
    SVG_FOREIGN_OBJECT,       // <foreignObject> embedded foreign content (inert placeholder)
    SVG_OBJECT,               // <object> embedded object (inert placeholder)
    SVG_IFRAME,               // <iframe> embedded external SVG document
    SVG_APPLET,               // <applet> applet (inert placeholder)
    SVG_HANDLER,              // <handler> event handler (inert placeholder)
    SVG_LISTENER,             // <listener> event listener (inert placeholder)
    SVG_PREFETCH,             // <prefetch> prefetch (inert placeholder)
    SVG_DISCARD,              // <discard> discard (inert placeholder)
    SVG_ANIMATE,              // <animate> attribute animation
    SVG_ANIMATE_COLOR,        // <animateColor> color animation
    SVG_ANIMATE_TRANSFORM,    // <animateTransform> transform animation
    SVG_ANIMATE_MOTION,       // <animateMotion> motion animation
    SVG_SET,                  // <set> set attribute
    SVG_MPATH,                // <mpath> motion path
    SVG_UNKNOWN,              // unknown / unrecognized element
};
#endif // GRAPHIC_LITE_SVG_ELEMENT_TYPE_DEFINED

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ELEMENT_TYPE_H
