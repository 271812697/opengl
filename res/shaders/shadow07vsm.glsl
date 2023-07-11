#version 460 core

#ifdef vertex_shader

layout(location = 0) out vec2 _uv;

void main() {
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    _uv = (position + 1) * 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) in vec2 _uv;
layout(location = 0) out vec4 FragColor;

layout(binding = 14) uniform sampler2D d_d2;
layout(location = 5) uniform bool vertical;

#define R 5
#define R21 11

void main() {
    vec2 d = vec2(0, 0);
    vec2 texelSize = 1.0 / textureSize(d_d2, 0);
    if(vertical) {
        float r = texelSize.y;
        for(int i = -R; i <= R; ++i) {
            d += texture(d_d2, vec2(_uv.x, _uv.y + i*r)).rg;
        }
    } else {
        float r = texelSize.x;
        for(int i = -R; i <= R; ++i) {
            d += texture(d_d2, vec2(_uv.x + i*r, _uv.y)).rg;
        }
    }
    FragColor.rg = d/R21;
    FragColor.ba=vec2(0.0,1.0);
    // FragColor = vec4(texture(d_d2, TexCoords).rgb, 1.0);
}
#endif
