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
layout(location = 3) uniform vec2 texelSize;
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
layout(binding = 0) uniform sampler2D uVelocity;
layout(binding = 1) uniform sampler2D uCurl;
layout(location = 4) uniform float curl;
layout(location = 5) uniform float dt;
out vec4 FragColor;
void main()
{
        float L = texture2D(uCurl, vL).x;
        float R = texture2D(uCurl, vR).x;
        float T = texture2D(uCurl, vT).x;
        float B = texture2D(uCurl, vB).x;
        float C = texture2D(uCurl, vUv).x;

        vec2 force = 0.5 * vec2(abs(T) - abs(B), abs(R) - abs(L));
        force /= length(force) + 0.0001;
        force *= curl * C;
        force.y *= -1.0;

        vec2 velocity = texture2D(uVelocity, vUv).xy;
        velocity += force * dt;
        velocity = min(max(velocity, -2000.0), 2000.0);
        FragColor = vec4(velocity, 0.0, 1.0);
}
#endif