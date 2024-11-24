/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
 *
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
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

const float KERNEL_RADIUS = 3;
  
uniform float g_Sharpness = 0.5;
uniform float _blurRadius = 3;
//vec2(1/width, 0) or vec2(0, 1/height)
uniform vec2 _invResolutionDirection; // either set x to 1/width or y to 1/height
uniform mat4 _projection;

uniform sampler2D _src;
uniform sampler2D _depthMap;

float linearize_depth(float d,float zNear,float zFar)
{
  return zNear * zFar / (zFar + d * (zNear - zFar));
}

in vec2 texCoord;
out vec4 out_Color;


//-------------------------------------------------------------------------

vec4 BlurFunction(vec2 uv, float r, vec4 center_c, float center_d, inout float w_total)
{
  vec4  c = texture2D( _src, uv );
  float d = texture2D( _depthMap, uv).x;
  float near   = _projection[3][2]/(_projection[2][2] - 1.0);
  float far    = _projection[3][2]/(_projection[2][2] + 1.0);
  d = linearize_depth(d, near, far);
  
  const float BlurSigma = float(_blurRadius) * 0.5;
  const float BlurFalloff = 1.0 / (2.0 * BlurSigma * BlurSigma);
  
  float ddiff = (d - center_d) * g_Sharpness;
  float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
  w_total += w;

  return c*w;
}

void main()
{
  vec4  center_c = texture2D( _src, texCoord );
  float center_d = texture2D( _depthMap, texCoord).x;
  float near   = _projection[3][2]/(_projection[2][2] - 1.0);
  float far    = _projection[3][2]/(_projection[2][2] + 1.0);
  center_d = linearize_depth(center_d, near, far);
  
  vec4  c_total = center_c;
  float w_total = 1.0;
  
  for (float r = 1; r <= _blurRadius; ++r)
  {
    vec2 uv = texCoord + _invResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }
  
  for (float r = 1; r <= _blurRadius; ++r)
  {
    vec2 uv = texCoord - _invResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }

  out_Color = vec4(c_total / w_total);
}
