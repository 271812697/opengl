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

layout(binding = 1) uniform sampler2D iChannel0;
layout(binding = 0) uniform sampler2D iChannel1;
layout(location = 1) uniform float r;
layout(location = 2) uniform vec3 w;
layout(location = 3) uniform int particles_count;
out vec4 FragColor;

vec3 hsv2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

	return c.z * mix( vec3(1.0), rgb, c.y);
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = vUv;
    vec4 prev_color = texture(iChannel0, uv);  
    vec4 out_color = vec4(0.0, 0.0, 0.0, 1.0);
   
    
    prev_color.y = prev_color.y *0.9;
    
    for (int i = 0; i < particles_count; i++)        
    {
        ivec2 adress = ivec2(i % int(iResolution.x),i / int(iResolution.x));
        vec2 position = texelFetch(iChannel1,adress,0).xy;
    	float radius = float(i % 4) + r;
        float dist = distance(fragCoord.xy, position);
        float diff = (radius - dist) / radius;
		vec3 color2 = hsv2rgb(vec3(0.5 + 0.5 * sin(iTime + float(i)*0.01), 0.5, 1.0)) * diff;
        if ( diff > 0.0)
        {
        	out_color = max(out_color, vec4(color2, 1.0));   
    	}       
    }
    
	fragColor = (out_color *w.x+ prev_color *w.y) * w.z;   
   
}
void main()
{
   mainImage(FragColor,gl_FragCoord.xy);
   //FragColor=vec4(1.0,1.0,0.0,1.0);
}
#endif