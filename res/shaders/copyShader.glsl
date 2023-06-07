#version 460 core
#pragma optimize(off)
#ifdef vertex_shader
layout(location = 0) out _vtx {
    out vec2 vUv;
    out vec2 vL;
    out vec2 vR;
    out vec2 vT;
    out vec2 vB;
};
uniform vec2 texelSize;
void main()
{
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    vUv = (position + 1) * 0.5;
    vL = vUv - vec2(texelSize.x, 0.0);
    vR = vUv + vec2(texelSize.x, 0.0);
    vT = vUv + vec2(0.0, texelSize.y);
    vB = vUv - vec2(0.0, texelSize.y);
    gl_Position = vec4(position, 0.0, 1.0);
}
#endif
#ifdef fragment_shader
precision mediump float;
precision mediump sampler2D;
layout(location = 0) in _vtx {
    in vec2 vUv;
    in vec2 vL;
    in vec2 vR;
    in vec2 vT;
    in vec2 vB;
};
uniform sampler2D uTexture;
out vec4 FragColor;
void main()
{
   FragColor = texture2D(uTexture, vUv);
}
#endif