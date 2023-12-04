#define EPS      1e-5
#define PI       3.141592653589793
#define PI2      6.283185307179586
#define INV_PI   0.318309886183791  // 1 over PI
#define HLF_PI   1.570796326794897  // half PI
#define SQRT2    1.414213562373095
#define SQRT3    1.732050807568877
#define SQRT5    2.236067977499789
#define CBRT2    1.259921049894873  // cube root 2
#define CBRT3    1.442249570307408  // cube root 3
#define G_PHI    1.618033988749894  // golden ratio
#define EULER_E  2.718281828459045  // natural exponent e
#define LN2      0.693147180559945
#define LN10     2.302585092994046
#define INV_LN2  1.442695040888963  // 1 over ln2
#define INV_LN10 0.434294481903252  // 1 over ln10
// 方向光
struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

// 点光
struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Att;
    float Pad;
};

// 聚光灯
struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Direction;
    float Spot;

    float3 Att;
    float Pad;
};
// 物体表面材质
struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecPower
    float4 Reflect;
};
struct GSOutput
{
    float4 PosH : SV_POSITION;//裁剪的齐次坐标
    float3 PosW : POSITION;     // 在世界中的位置
    float3 Nor :NORMAL;
    float4 Color : COLOR;
    float4 texcoord: TEXCOORD;
};
Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);
// sampler binding points (texture units) 17-19 are reserved for PBR IBL
TextureCube irradiance_map : register(t17);
TextureCube prefilter_map : register(t18);
Texture2D BRDF_LUT : register(t19);
cbuffer PSConstantBuffer : register(b1)
{
    DirectionalLight g_DirLight;
    PointLight g_PointLight[5];
    SpotLight g_SpotLight;
    Material g_Material;
    float3 g_EyePosW;
    float g_Pad;
    int mode;
}
cbuffer ST : register(b3)
{
    float basic;//基础颜色强度
    int moade;
    int poiflag;
    int pbrflag;
    float4 poi_color;
    float4 m_color;//
    float4 c_color;
    int _Clip;
    float k;//裁剪值
    int coffi;//映射系数
}
cbuffer PBR : register(b4)
{
    float roughness = 0.1;
    float metallic = 0.1;

}
//计算平行光
void ComputeDirectionalLight(Material mat, DirectionalLight L,
    float3 normal, float3 toEye,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    

    // 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 光向量与照射方向相反
    float3 lightVec = -L.Direction;

    // 添加环境光
    ambient = mat.Ambient * L.Ambient;

    // 添加漫反射光和镜面光
    float diffuseFactor = dot(lightVec, normal);
   // diffuseFactor = (diffuseFactor + 1) / 2;

    // 展开，避免动态分支
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}
//计算点光源的光照
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 从表面到光源的向量
    float3 lightVec = L.Position - pos;

    // 表面到光线的距离
    float d = length(lightVec);

    // 灯光范围测试
    if (d > L.Range)
        return;

    // 标准化光向量
    lightVec /= d;

    // 环境光计算
    ambient = mat.Ambient * L.Ambient;

    // 漫反射和镜面计算
    float diffuseFactor = dot(lightVec, normal);
    //diffuseFactor = (diffuseFactor + 1) / 2;
    // 展开以避免动态分支
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    // 光的衰弱
    float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// 像素着色器
// pixel data definition

float3 ComputeF0(const float3 albedo, float metalness, float specular)
{
    float3 dielectric_F0 = float3(0.16 * specular * specular, 0.16 * specular * specular, 0.16 * specular * specular);
    return lerp(dielectric_F0, albedo, metalness);
}

float3 ComputeAlbedo(const float3 albedo, float metalness)
{
    return albedo * (1.0 - metalness);
}


struct Pixel
{
    float3 _position;
    float3 _normal;
    float3 position;

    float3 V;
    float3 N;
    float3 R;
    float3 GN;
    float3 GR;
    float NoV;
    float4 albedo;
    float roughness;
    float alpha;
    float3 ao;
    float3 diffuse_color;
    float3 F0;
    float3 DFG;
    float metalness;
    float specular;
    float Ec;

};

// initializes the current pixel (fragment), values are computed from the material inputs
void InitPixel(inout Pixel px, const float3 camera_pos)
{
    px.position = px._position;
    px.V = normalize(camera_pos - px.position);
    px.N = px._normal;
    px.R = reflect(-px.V, px.N);
    px.NoV = max(dot(px.N, px.V), 1e-4);
    px.GN = px._normal; // geometric normal vector, unaffected by normal map
    px.GR = reflect(-px.V, px._normal); // geometric reflection vector
    px.roughness = roughness;
    px.roughness = clamp(px.roughness, 0.045, 1.0);
    px.alpha = pow(px.roughness,2);

    px.ao = 1.0;
    px.metalness = metallic;
    px.specular = clamp(0.1, 0.35, 1.0);
    px.diffuse_color = ComputeAlbedo(px.albedo.rgb, px.metalness);
    px.F0 = ComputeF0(px.albedo.rgb, px.metalness, px.specular); 
    px.DFG = BRDF_LUT.Sample(g_Sam, float2(px.NoV, px.roughness)).rgb;
    px.Ec = 1.0 + px.F0 * (1.0 / px.DFG.y - 1.0); // energy compensation factor >= 1.0
   

}

/* ------------------------- Specular D - Normal Distribution Function ------------------------- */

// Trowbridge-Reitz GGX normal distribution function (long tail distribution)
// Bruce Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
float D_TRGGX(float alpha, float NoH)
{
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}
// Smith's height-correlated visibility function (V = G / normalization term)
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_SmithGGX(float alpha, float NoV, float NoL)
{
    float a2 = alpha * alpha;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}
/* ------------------------- Specular F - Fresnel Reflectance Function ------------------------- */


// Schlick's approximation when grazing angle reflectance F90 is assumed to be 1
float3 F_Schlick(const float3 F0, float HoV)
{
    return F0 + (1.0 - F0) * pow(1.0 - HoV,5); // HoV = HoL
}

float3 EvalSpecularLobe(const Pixel px, const float3 L, const float3 H, float NoV, float NoL, float NoH, float HoL)
{
    float D = 0.0;
    float V = 0.0;
    float3 F = float3(0.0,0.0,0.0);
    
    D = D_TRGGX(px.alpha, NoH);
    V = V_SmithGGX(px.alpha, NoV, NoL);
    F = F_Schlick(px.F0, HoL);

    return (D * V) * F;
}
// Disney's diffuse BRDF that takes account of roughness (not energy conserving though)
// Brent Burley 2012, "Physically Based Shading at Disney"
float Fd_Burley(float alpha, float NoV, float NoL, float HoL)
{
    float F90 = 0.5 + 2.0 * HoL * HoL * alpha;
    float a = 1.0 + (F90 - 1.0) * pow(1.0 - NoL,5);
    float b = 1.0 + (F90 - 1.0) * pow(1.0 - NoV,5);
    return a * b * INV_PI;
}
// evaluates base material's specular BRDF lobe


// evaluates base material's diffuse BRDF lobe
float3 EvalDiffuseLobe(const Pixel px, float NoV, float NoL, float HoL)
{
    return px.diffuse_color * Fd_Burley(px.alpha, NoV, NoL, HoL);
}
// evaluates the contribution of a white analytical light source of unit intensity
float3 EvaluateAL(const Pixel px, const float3 L)
{
    float NoL = dot(px.N, L);
    if (NoL <= 0.0)
        return float3(0.0,0.0,0.0);

    float3 H = normalize(px.V + L);
    float3 Fr = float3(0.0,0.0,0.0);
    float3 Fd = float3(0.0,0.0,0.0);
    float3 Lo = float3(0.0,0.0,0.0);

    float NoV = px.NoV;
    float NoH = max(dot(px.N, H), 0.0);
    float HoL = max(dot(H, L), 0.0);
// standard model
    Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec; // compensate energy
    Fd = EvalDiffuseLobe(px, NoV, NoL, HoL);
    Lo = (Fd + Fr) * NoL;
    return Lo;
}
// evaluates the contribution of a white directional light of unit intensity
float3 EvaluateADL(const Pixel px, const float3 L, float visibility)
{
    return visibility <= 0.0 ? float3(0.0,0.0,0.0) : (EvaluateAL(px, L) * visibility);
}
// evaluates the contribution of a white point light of unit intensity
float3 EvaluateAPL(const Pixel px, const float3 position, float range, float linea, float quadratic, float visibility)
{
    float3 L = normalize(position - px.position);

    // distance attenuation: inverse square falloff
    float d = distance(position, px.position);
    float attenuation = (d >= range) ? 0.0 : (1.0 / (1.0 +linea * d
    +quadratic * d * d));

    return (attenuation <= 0.0 || visibility <= 0.0) ? float3(0,0,0) : (attenuation * visibility * EvaluateAL(px, L));
}

float3 ApproxACES(float3 radiance)
{
    float3 v = radiance * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
}
float3 Linear2Gamma(float3 color)
{
    return pow(color, 1.0 / 2.2); // component-wise
}
// Ground truth based colored ambient occlussion (colored GTAO)
// Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
float3 MultiBounceGTAO(float visibility, const float3 albedo)
{
    float3 a = 2.0404 * albedo - 0.3324;
    float3 b = -4.7951 * albedo + 0.6417;
    float3 c = 2.7552 * albedo + 0.6903;
    float v = visibility;
    return max(float3(v, v, v), ((v * a + b) * v + c) * v);
}
float3 ComputeRAR(const Pixel px)
{
    float3 R = px.R;


    return lerp(R, px.N, px.alpha * px.alpha);
}
float QuarticEaseIn(float x)
{
    return x * x * x * x;
}
float3 ComputeLD(const float3 R, float roughness)
{
    const float max_level = 11.0;
    float miplevel = max_level * QuarticEaseIn(roughness);
    return prefilter_map.SampleLevel(g_Sam, R, miplevel).rgb;
}
// evaluates the contribution of environment IBL at the pixel
float3 EvaluateIBL(const Pixel px)
{
    float3 Fr = float3(0.0, 0.0, 0.0); // specular reflection (the Fresnel effect), weighted by E
    float3 Fd = float3(0.0, 0.0, 0.0); // diffuse reflection, weighted by (1 - E) * (1 - transmission)
    float3 Ft = float3(0.0, 0.0, 0.0); // diffuse refraction, weighted by (1 - E) * transmission

    float3 E = float3(0.0, 0.0, 0.0); // specular BRDF's total energy contribution (integral after the LD term)
    float3 AO = 1; // diffuse ambient occlusion
    E = lerp(px.DFG.xxx, px.DFG.yyy, px.F0);
    AO *= MultiBounceGTAO(AO.r, px.diffuse_color);
    Fr = ComputeLD(ComputeRAR(px), px.roughness) * E;
    Fr *= px.Ec; // apply multi-scattering energy compensation (Kulla-Conty 17 and Lagarde 18)

    // the irradiance map already includes the Lambertian BRDF so we multiply the texel by
    // diffuse color directly. Do not divide by PI here cause that will be double-counting
    // for spherical harmonics, INV_PI should be rolled into SH9 during C++ precomputation

    Fd = irradiance_map.Sample(g_Sam, px.N).rgb * px.diffuse_color * (1.0 - E);
    Fd *= AO; // apply ambient occlussion and multi-scattering colored GTAO



    return Fr + Fd + Ft;
}
// optimizes lower power functions, removes the implicit `exp/log` call
float pow2(float x)
{
    return x * x;
}
float pow3(float x)
{
    return x * x * x;
}
float pow4(float x)
{
    return x * x * x * x;
}
float pow5(float x)
{
    return x * x * x * x * x;
}

float QuadraticEaseOut(float x)
{
    return x * (2 - x);
}

float QuadraticEaseInOut(float x)
{
    return x < 0.5 ? (2 * x * x) : (4 * x - 2 * x * x - 1);
}

float CubicEaseIn(float x)
{
    return x * x * x;
}

float CubicEaseOut(float x)
{
    return 1 + pow(x - 1,3);
}

float CubicEaseInOut(float x)
{
    return (x < 0.5) ? (4 * x * x * x) : (0.5 * pow3(2 * x - 2) + 1);
}


float QuarticEaseOut(float x)
{
    return pow3(x - 1) * (1 - x) + 1;
}

float QuarticEaseInOut(float x)
{
    return (x < 0.5) ? (8 * x * x * x * x) : (1 - 8 * pow4(x - 1));
}

float QuinticEaseIn(float x)
{
    return x * x * x * x * x;
}

float QuinticEaseOut(float x)
{
   
    return pow5(x - 1) + 1;
   // return pow(x - 1,coffi) + 1;
}

float QuinticEaseInOut(float x)
{
    return (x < 0.5) ? (16 * x * x * x * x * x) : (0.5 * pow5((2 * x) - 2) + 1);
}

float SineEaseIn(float x)
{
    return 1 - cos(x * HLF_PI);
}

float SineEaseOut(float x)
{
    return sin(x * HLF_PI);
}

float SineEaseInOut(float x)
{
    return 0.5 * (1 - cos(PI * x));
}

float CircularEaseIn(float x)
{
    return 1 - sqrt(1 - x * x);
}

float CircularEaseOut(float x)
{
    return sqrt((2 - x) * x);
}

float CircularEaseInOut(float x)
{
    return (x < 0.5) ? (0.5 * (1 - sqrt(1 - 4 * x * x))) : (0.5 * (sqrt((3 - 2 * x) * (2 * x - 1)) + 1));
}

float ExponentialEaseIn(float x)
{
    return (x == 0.0) ? x : pow(2, 10 * (x - 1));
}

float ExponentialEaseOut(float x)
{
    return (x == 1.0) ? x : 1 - pow(2, -10 * x);
}

float ExponentialEaseInOut(float x)
{
    return (x == 0.0 || x == 1.0) ? x :
        ((x < 0.5) ? (0.5 * pow(2, (20 * x) - 10)) : (1 - 0.5 * pow(2, (-20 * x) + 10)));
}

float ElasticEaseIn(float x)
{
    return sin(13 * HLF_PI * x) * pow(2, 10 * (x - 1));
}

float ElasticEaseOut(float x)
{
    return 1 - sin(13 * HLF_PI * (x + 1)) * pow(2, -10 * x);
}

float ElasticEaseInOut(float x)
{
    return (x < 0.5)
        ? (0.5 * sin(26 * HLF_PI * x) * pow(2, 10 * (2 * x - 1)))
        : (0.5 * (2 - sin(26 * HLF_PI * x) * pow(2, -10 * (2 * x - 1))));
}

float BackEaseIn(float x)
{
    return x * x * x - x * sin(x * PI);
}

float BackEaseOut(float x)
{
    float f = 1 - x;
    return 1 - (f * f * f - f * sin(f * PI));
}

float BackEaseInOut(float x)
{
    if (x < 0.5)
    {
        float f = 2 * x;
        return 0.5 * (f * f * f - f * sin(f * PI));
    }

    float f = 2 - 2 * x;
    return 0.5 * (1 - (f * f * f - f * sin(f * PI))) + 0.5;
}

float BounceEaseOut(float x)
{
    if (x < 0.36363636363)
    {
        return x * x * 7.5625;
    }
    if (x < 0.72727272727)
    {
        return x * x * 9.075 - x * 9.9 + 3.4;
    }
    if (x < 0.90000000000)
    {
        return x * x * 12.0664819945 - x * 19.6354570637 + 8.89806094183;
    }
    return x * x * 10.8 - x * 20.52 + 10.72;
}

float BounceEaseIn(float x)
{
    return 1 - BounceEaseOut(1 - x);
}

float BounceEaseInOut(float x)
{
    return (x < 0.5) ? (0.5 * BounceEaseIn(x * 2)) : (0.5 * BounceEaseOut(x * 2 - 1) + 0.5);
}
float4 PBRColor(GSOutput pIn)
{
    Pixel px;
    px._position =pIn.PosW;
    px._normal = pIn.Nor;
    //ExponentialEaseOut
    //CircularEaseOut
    //QuinticEaseOut
    float alpha = 1;
    if (coffi==0)
        alpha = QuarticEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 1)
        alpha = QuadraticEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 2)
        alpha = CubicEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 3)
        alpha = QuinticEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 4)
        alpha = SineEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 5)
        alpha = CircularEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 6)
        alpha = ExponentialEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 7)
        alpha = ElasticEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 8)
        alpha = BackEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    if (coffi == 9)
        alpha = BounceEaseOut(pIn.Color.r); //pow(pIn.Color.r,0.25);
    px.albedo = pIn.Color;//alpha * m_color + (1 - alpha) * c_color;
    if (poiflag)
        //px.albedo = float4(poi_color.r,poi_color.g,poi_color.b,1.0);
        px.albedo = float4(poi_color.rgb, 1.0);
    InitPixel(px, g_EyePosW);

    float3 Lo = float3(0.0,0.0,0.0);

    float density = 1.0f;
    // contribution of directional light
    Lo += EvaluateADL(px, g_DirLight.Direction.xyz, 1.0) * g_DirLight.Diffuse.rgb * density;
    Lo += EvaluateIBL(px);    
    //Lo += float3(0.2, 0.2, 0.2) * px.diffuse_color;
    Lo = ApproxACES(Lo);
    Lo = Linear2Gamma(Lo);
    return float4(Lo, px.albedo.a);
}
float4 PS(GSOutput pIn) : SV_Target
{    float alpha = pIn.Color.x;   
    //光滑插值
    //alpha = alpha * alpha * (3.0f - 2.0f * alpha);
     //可视化法线
    //color = float4(pIn.Nor * 0.5 + 0.5, 1.0);
    //color = g_DiffuseMap.Sample(g_Sam, float2(alpha,alpha));
   // return color;
    //color = float4(1.0, 1.0, 0, 1.0);
    if(_Clip)
        clip(pow(alpha, 0.25) - k);
    if (pbrflag==1) {
        return PBRColor(pIn);
    }
    float4 A = float4(0,0,0,0);
    float4 D = float4(0, 0, 0, 0);
    float4 S = float4(0, 0, 0, 0);
    ComputeDirectionalLight(g_Material, g_DirLight, pIn.Nor, g_EyePosW, A, D, S);
    float4 color = A + D;
    [unroll]
    for (int i = 0; i < 5; i++)
    {
    ComputePointLight(g_Material, g_PointLight[i], pIn.PosW, pIn.Nor, g_EyePosW, A, D, S);
    color+= D+A;  
    }
   

    //diffuse项
    float4 res = alpha * m_color + (1 - alpha) * c_color;
    if (poiflag == 1) {
        //这里设置兴趣点的颜色
        return m_color*color;
    }
    if (moade == 0) {
        //light
        float3 Lo = res * color.xyz;
       // Lo = Lo / (Lo + 1.0);
        //Lo = pow(Lo, 1.0 / 2.2);
        return float4(Lo, 1.0);
          //return res * color;
        //return pIn.Color*color;
    }
    else if (moade == 1) {
        //probility
       return res;//pIn.Color;
    }
    else if (moade==2) {
        //poi
        return pIn.Color;
    }
    
    //绘制深度信息
     //float d=pIn.texcoord.w/5;
     // d=pow2(d);
    //return  float4(d,d,d,1.0);
    basic
    return basic*float4(1, 1, 1, 1.0) * pIn.Color;  
}
