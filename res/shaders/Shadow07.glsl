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

layout(location = 250) uniform mat4 light_transform;


void main() {
    
    
    gl_Position = light_transform*self.transform *vec4(position, 1.0);  // keep in world space
}

#endif


#ifdef fragment_shader
layout(location = 0) out vec4 color;
#define NEAR_PLANE 0.1
#define FAR_PLANE 100.0
// 输入为 [0, 1] 的投影深度
// 输出为 [0, 1] 的线性深度
float getLinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    // 简单通过代入 [-1, 1] 验证结果为 [NEAR_PLANE, FAR_PLANE]
    z =(2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
    return (z - NEAR_PLANE)/(FAR_PLANE - NEAR_PLANE);
}

layout(location = 5) uniform bool vsm;

void main() {
     //if(!vsm)
     //return;

    float depth = getLinearizeDepth(gl_FragCoord.z);
    color.r = depth;
    color.g = depth * depth;
    //color.b=1.0;
    color.a=1.0;

    
}

#endif
