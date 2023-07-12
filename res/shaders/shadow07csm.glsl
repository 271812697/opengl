#version 460 core
#pragma optimize(off)


// uniform blocks >= 10 are reserved for internal use only
layout(std140, binding = 10) uniform RendererInput {
    ivec2 resolution;     // viewport size in pixels
    ivec2 cursor_pos;     // cursor position relative to viewport's upper-left corner
    float near_clip;      // frustum near clip distance
    float far_clip;       // frustum far clip distance
    float time;           // number of seconds since window created
    float delta_time;     // number of seconds since the last frame
    bool  depth_prepass;  // early z test
    uint  shadow_index;   // index of the shadow map render pass (one index per light source)
} rdr_in;

// default-block (loose) uniform locations >= 1000 are reserved for internal use only
struct self_t {
    mat4 transform;    // 1000, model matrix of the current entity
    uint material_id;  // 1001, current mesh's material id
    uint ext_1002;
    uint ext_1003;
    uint ext_1004;
    uint ext_1005;
    uint ext_1006;
    uint ext_1007;
};

layout(location = 1000) uniform self_t self;

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec2 uv2;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 binormal;
layout(location = 6) in ivec4 bone_id;
layout(location = 7) in vec4 bone_wt;

void main() {
    gl_Position = self.transform *vec4(position, 1.0);  // keep in world space
}
#endif
#ifdef geometry_shader

layout(triangles,invocations=5) in;
layout(triangle_strip, max_vertices = 3) out;


layout(std140, binding = 3) uniform DL {

    mat4 lightSpaceMatrices[16];//csm各级阴影贴图对应的光空间矩阵
    float cascadePlaneDistances[16];
    int cascadeCount;   // number of frusta - 1
    vec4 lightDir;
} dl;

void main() {
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = dl.lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}

#endif

#ifdef fragment_shader
void main() {
}
#endif
