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

namespace LedMapper {

static const string VERTEX_SHADER_GRAB = STRINGIFY(
    #version 330

    precision mediump float;
    uniform mat4 modelViewProjectionMatrix;
    uniform ivec2 outTexResolution;

    layout(location = 0) in vec3 VertexPosition;
    out vec2 ledPos;

    void main()
    {
        ledPos = VertexPosition.xy;
        gl_Position = modelViewProjectionMatrix
                      * vec4(gl_VertexID % outTexResolution.x, 1+floor(gl_VertexID / outTexResolution.x),
                             0.0, 1.0);
    }
);

static const string FRAGMENT_SHADER_GRAB = STRINGIFY(
    #version 330

    uniform sampler2DRect texIn;
    in vec2 ledPos;
    out vec4 vFragColor;

    void main()
    {
        vFragColor = texture(texIn, ledPos);
    }
);



} // namespace LedMapper