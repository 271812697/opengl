#version 460 core
#pragma optimize(off)
#ifdef vertex_shader
layout(location = 0) out _vtx {
    out vec2 vUv;
};
void main()
{
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    vUv = (position + 1) * 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
#endif
#ifdef fragment_shader
layout(location = 0) in _vtx {
    in vec2 vUv;
};

out vec4 FragColor;
void main()
{
   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
#endif