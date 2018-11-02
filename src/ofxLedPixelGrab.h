/*
    Copyright (C) 2018 Timofey Tavlintsev [http://tvl.io]

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

#include "ofFbo.h"
#include "ofBufferObject.h"
#include "ofShader.h"

#define STRINGIFY(x) #x

namespace LedMapper {

static const string GLSL_VERSION = "#version 330\n\n";

static const string VERTEX_SHADER_GRAB = STRINGIFY(
    precision mediump float;\n
    uniform mat4 modelViewProjectionMatrix;\n
    uniform ivec2 outTexResolution;\n

    layout(location = 0) in vec3 VertexPosition;\n
    out vec2 ledPos;\n

    void main()\n
    {\n
        ledPos = VertexPosition.xy;\n
        gl_Position = modelViewProjectionMatrix
                      * vec4(gl_VertexID % outTexResolution.x, 1+floor(gl_VertexID / outTexResolution.x),
                             0.0, 1.0);\n
    }
);

static const string FRAGMENT_SHADER_GRAB = STRINGIFY(
    uniform sampler2DRect texIn;\n
    in vec2 ledPos;\n
    out vec4 vFragColor;\n

    %colorConvert%\n

    void main()\n
    {\n
        vFragColor = colorConvert(texture(texIn, ledPos));\n
    }
);
static const string GetColorConvert(GRAB_COLOR_TYPE type) {
    string func = "vec4 colorConvert(vec4 color) { return vec4(";
    switch (type) {
        case BRG:
            func += "color.b, color.r, color.g";
            break;
        case BGR:
            func += "color.b, color.g, color.r";
            break;
        case GRB:
            func += "color.g, color.r, color.b";
            break;
        case GBR:
            func += "color.g, color.b, color.r";
            break;
        case RBG:
            func += "color.r, color.b, color.g";
            break;
        case RGB:
        default: // RGB
            func += "color.r, color.g, color.b";
            break;
    }
    return func + ", 1.0); }";
}

static ofShader GetShaderForColorGrab(GRAB_COLOR_TYPE type) {
    string frag = GLSL_VERSION + FRAGMENT_SHADER_GRAB;
    ofStringReplace(frag,"%colorConvert%", GetColorConvert(type));
    ofShader shader;
    shader.setupShaderFromSource(GL_VERTEX_SHADER, GLSL_VERSION + VERTEX_SHADER_GRAB);
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
    shader.bindDefaults();
    shader.linkProgram();
    return shader;
}

static void FboCopyTo(ofFbo &fbo, ofBufferObject & buffer, GLint internalformat, int width, int height) {
    if(!fbo.isAllocated())
        return;

    int glFormat = ofGetGLFormatFromInternal(internalformat);
    if (buffer.size() <= ofGetNumChannelsFromGLFormat(glFormat))
        buffer.allocate(width * height * ofGetNumChannelsFromGLFormat(glFormat), GL_STATIC_READ);

    fbo.bind();
    buffer.bind(GL_PIXEL_PACK_BUFFER);
    glReadPixels(0, 0, width, height, ofGetGLFormatFromInternal(internalformat),
                 ofGetGLTypeFromInternal(internalformat), NULL);
    buffer.unbind(GL_PIXEL_PACK_BUFFER);
    fbo.unbind();
}


} // namespace LedMapper