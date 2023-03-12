#version 460 core
#pragma optimize(off)
#ifdef vertex_shader
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 0) out _vtx {
    out vec3 _localpos;
    out vec3 _position;
    out vec3 _normal;
    out vec2 _uv;
};
layout(location = 3) uniform mat4 world;
layout(location = 4) uniform mat4 view;
layout(location = 5) uniform mat4 projection;
void main()
{
   _position=vec3(world*vec4(aPos, 1.0));
   _uv=uv;
   _localpos=aPos;
   _normal=vec3(world*vec4(normal, 1.0));
   gl_Position = projection*view*world*vec4(aPos, 1.0);
}
#endif
#ifdef fragment_shader
layout(location = 0) in _vtx {
    in vec3 _localpos;
    in vec3 _position;
    in vec3 _normal;
    in vec2 _uv;
};
layout(binding = 0) uniform samplerCube environment_map;
layout(binding = 1) uniform sampler2D albedo_map;
out vec4 FragColor;
void main()
{
   FragColor = texture(environment_map, _localpos);//vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
#endif