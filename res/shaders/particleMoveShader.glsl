#version 460 core
#pragma optimize(off)

layout(std140, binding = 11) uniform RendererInput {
    ivec2 iResolution;
    float iTime;
    float iTimeDelta;
    float iFrameRate;
    ivec2 iMouse;   
    int iFrame;       
 } ;
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
precision mediump float;
precision mediump sampler2D;
layout(location = 0) in _vtx {
    in vec2 vUv;

};
layout(binding = 0) uniform sampler2D iChannel0;
out vec4 FragColor;

vec2 rotate(vec2 v, float angle) {
	float a = sin(angle);
	float b = cos(angle);
	mat2 m = mat2(b, -a, a, b);
	return m * v;
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec4 out_color = vec4(0.0, 0.0, 0.0, 0.0);
    ivec2 frag_coord_int = ivec2(fragCoord);
	int index = frag_coord_int.x + frag_coord_int.y * int(iResolution.x);
    int particles_count = 500;
    
    if (index < particles_count)
    {
        if (iFrame < 4)
        {
            out_color = vec4(
                fragCoord.x, 
                20 + sin(fragCoord.x * 0.01),
                cos(fragCoord.x*321.0 + fragCoord.x*fragCoord.x*4.0),
                -sin(fragCoord.x*321.0 + fragCoord.x*fragCoord.x*4.0)); 
                

        }
        else
        {
            vec4 color = texelFetch(iChannel0, frag_coord_int,0).xyzw;
            float speed = 100.0 + 40.0 * sin(float(index));
            float move_step = speed * iTimeDelta;
            vec2 dir = color.zw;
            float new_x = color.x + move_step * dir.x;
            float new_y = color.y + move_step * dir.y;

            if (new_x < -30.0)
            {
                dir.x = -dir.x;
            }
            else if (new_y < -30.0)
            {
                dir.y = -dir.y;
            }
            else if (new_x > (iResolution.x ))
            {
                dir.x = -dir.x;
            }
            else if (new_y > (iResolution.y ))
            {
                dir.y = -dir.y;
            }
            else
            {
                float angle = 0.01;
                if (index % 2 == 0)
                {
                    angle = -0.01;
                }
                dir.xy = rotate(dir.xy, angle);
            }
            out_color = vec4(new_x, new_y,dir.x,dir.y); 
                  
        }

   // out_color=vec4(1.0,0.0,0.0,1.0);
    }

    fragColor = out_color;
}

void main()
{
   mainImage(FragColor,gl_FragCoord.xy);
   //FragColor=vec4(1.0,1.0,0.0,1.0);
}
#endif