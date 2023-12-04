
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"Settings.h"
#include"EffectHelper.h"
#include"std_image_write.h"
#include"Texture2D.h"
#include"TextureManager.h"
#include<direct.h>
#include<filesystem>
#include <assert.h>
#include <io.h>
#include"Renderstate.h"
using namespace DirectX;
TextureManager textureManager;
TextureCube* skybox = nullptr;
TextureCube* irradiance = nullptr;
TextureCube* prefilter_map = nullptr;
Texture2D* BRDF_LUT = nullptr;
void PreComputeIBL(ID3D11DeviceContext* context) {
	ID3D11Device* m_pd3dDevice = nullptr;
	context->GetDevice(&m_pd3dDevice);
	{
		EffectHelper effect;
		effect.CreateShaderFromFile("CS", L"HLSL//CS.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
		EffectPassDesc pass;
		pass.nameVS = "";
		pass.nameGS = "";
		pass.namePS = "";
		pass.nameDS = "";
		pass.nameHS = "";
		pass.nameCS = "CS";
		effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
		TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
		skybox = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
		ID3D11UnorderedAccessView* ppUAView = nullptr;
		m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
		effect.SetUnorderedAccessByName("cubemap", ppUAView);
		auto view = textureManager.CreateFromFile("SkyBox//outdoor.hdr", false, true);
		effect.SetShaderResourceByName("HDR", view);
		effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
		effect.GetEffectPass("CSPASS")->Apply(context);
		context->Dispatch(64, 64, 6);
		context->CopyResource(skybox->GetTexture(), cubemap->GetTexture());
		delete cubemap;
	}
	{
		EffectHelper effect;
		effect.CreateShaderFromFile("CS", L"HLSL//irradiance_map.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
		EffectPassDesc pass;
		pass.nameVS = "";
		pass.nameGS = "";
		pass.namePS = "";
		pass.nameDS = "";
		pass.nameHS = "";
		pass.nameCS = "CS";
		effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
		TextureCube* cubemap = new TextureCube(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
		irradiance = new TextureCube(m_pd3dDevice, 128, 128, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32G32B32A32_FLOAT);
		ID3D11UnorderedAccessView* ppUAView = nullptr;
		m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
		effect.SetUnorderedAccessByName("irradiance_map", ppUAView);
		effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
		effect.SetShaderResourceByName("g_TexCube", skybox->GetShaderResource());
		effect.GetEffectPass("CSPASS")->Apply(context);
		context->Dispatch(4, 4, 6);
		context->CopyResource(irradiance->GetTexture(), cubemap->GetTexture());
		delete cubemap;

	}

	{
		EffectHelper effect;
		effect.CreateShaderFromFile("CS", L"HLSL//prefilter_envmap.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
		EffectPassDesc pass;
		pass.nameVS = "";
		pass.nameGS = "";
		pass.namePS = "";
		pass.nameDS = "";
		pass.nameHS = "";
		pass.nameCS = "CS";
		effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
		TextureCube* cubemap = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_UNORDERED_ACCESS);
		prefilter_map = new TextureCube(m_pd3dDevice, 2048, 2048, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_BIND_SHADER_RESOURCE);
		D3D11_BOX box = { 0,0,0,2048,2048,1 };
		for (int i = 0; i < 6; i++) {

			context->CopySubresourceRegion(cubemap->GetTexture(),
				D3D11CalcSubresource(0, i, 12), 0, 0, 0,
				skybox->GetTexture(),
				D3D11CalcSubresource(0, i, 1), nullptr);

		}
		effect.SetSamplerStateByName("g_SamLinear", RenderStates::SSAnistropicWrap16x.Get());
		effect.SetShaderResourceByName("environment_map", skybox->GetShaderResource());
		for (int level = 1; level < 12; level++) {
			float roughness = (level) / 12.0f;
			CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
				DXGI_FORMAT_R32G32B32A32_FLOAT, level);
			ID3D11UnorderedAccessView* ppUAView = nullptr;
			m_pd3dDevice->CreateUnorderedAccessView(cubemap->GetTexture(), &uavDesc, &ppUAView);
			effect.SetUnorderedAccessByName("prefilter_map", ppUAView);
			effect.GetConstantBufferVariable("roughness")->SetFloat(roughness);
			int resolution = 2048 / pow(2, level);
			int groups = resolution > 32 ? resolution / 32 : 1;
			effect.GetEffectPass("CSPASS")->Apply(context);
			context->Dispatch(groups, groups, 6);
		}
		context->CopyResource(prefilter_map->GetTexture(), cubemap->GetTexture());
		delete cubemap;
	}


	{
		EffectHelper effect;
		effect.CreateShaderFromFile("CS", L"HLSL//environment_BRDF.hlsl", m_pd3dDevice, "CS", "cs_5_0", nullptr, nullptr);
		EffectPassDesc pass;
		pass.nameVS = "";
		pass.nameGS = "";
		pass.namePS = "";
		pass.nameDS = "";
		pass.nameHS = "";
		pass.nameCS = "CS";
		effect.AddEffectPass("CSPASS", m_pd3dDevice, &pass);
		Texture2D* BRDF = new Texture2D(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS);
		BRDF_LUT = new Texture2D(m_pd3dDevice, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, D3D11_BIND_SHADER_RESOURCE);
		effect.SetUnorderedAccessByName("BRDF_LUT", BRDF->GetUnorderedAccess());
		effect.GetEffectPass("CSPASS")->Apply(context);
		context->Dispatch(32, 32, 1);
		context->CopyResource(BRDF_LUT->GetTexture(), BRDF->GetTexture());
	}
}





#define PI 3.141592654
//特效助理
EffectHelper effect;
using namespace std;
vector<string> listFiles(string dir)
{
	vector<string>ans;
	
	std::filesystem::directory_entry p_directory(dir);
	for (auto& item : std::filesystem::directory_iterator(p_directory))
		if (!item.is_directory()) {
         ans.push_back(item.path().generic_string());
			
		}
	return ans;
}
std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory)
{
	std::vector<std::filesystem::path> files;
	for (auto const& directory_entry : std::filesystem::recursive_directory_iterator{ directory })
	{
		if (directory_entry.is_regular_file())
		{
			files.push_back(directory_entry);
		}
	}
	return files;
}
static Settings g_settings;
static bool listenClicked = false;
static bool _pbrflag = false;
static bool _poiflag = true;
static bool _drawmodel = true;
static bool _modelline = true;
static bool _WireFrame = false;
static bool _DrawVertexX = false;
static bool _DrawVertexY = false;
static float _vertex_clip = 0.0f;
static float _VertexRadiusX = 1.0f;
static float _VertexRadiusY = 1.0f;
static float VertexRadiusXcolor[4] = { 1,0,0,1.0 };
static float VertexRadiusYcolor[4] = { 0,1,1,1.0 };
static bool _Clip = false;
static bool _Mirroring = false;
static char savepicpath[128] = "ALL";
static int len = 128;
static float curraduis = 3.0;//xiangji
static float sphere_radius = 1;
float dis = 10;
struct picture {
	int w;
	int h;
	ID3D11ShaderResourceView* SRV;
	void* data;
};
static std::vector<picture>picArray;
struct Rect {
	float x0, y0;
	float x1, y1;
};
static std::vector<Rect>rectArray;
#pragma region 重构代码
using XID = size_t;
inline XID StrToID(std::string_view str)
{
	static std::hash<std::string_view> hash;
	return hash(str);
}
//当前选中的模型 
std::string cur_model = "alien-1";
//结果模型的路径
std::unordered_map <size_t, std::string>resultmodel_path;
std::unordered_map<size_t, std::tuple<ID3D11Buffer*, ID3D11Buffer*, int>>resultmodel_Buffer;
std::unordered_map<size_t, Geometry::MeshData<VertexPosNormalColorTex>>result_mesh;
//结果poi的路径
std::unordered_map <size_t, std::string>resultpoi_path;
std::unordered_map<size_t, std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>>>resultpoi_pos;
//groundtruth路径包括模型和poi
std::unordered_map <size_t, std::string>groundtruth_path;
std::unordered_map<size_t, std::tuple<ID3D11Buffer*, ID3D11Buffer*, int>>groundtruth_Buffer;
std::unordered_map<size_t, std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>>>groundtruth_poi_pos;
std::tuple<ID3D11Buffer*, ID3D11Buffer*, int> make_Buffer(Geometry::MeshData<VertexPosNormalColorTex>& meshData, ID3D11DeviceContext* m_pd3dImmediateContext) {
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	int m_IndexCount = 0;
	ID3D11Device* m_pd3dDevice = nullptr;
	m_pd3dImmediateContext->GetDevice(&m_pd3dDevice);
	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalColorTex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, &m_pVertexBuffer));
	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, &m_pIndexBuffer));
	return std::make_tuple(m_pVertexBuffer, m_pIndexBuffer, m_IndexCount);
}
void Draw(std::string cur_model, ID3D11DeviceContext* m_pd3dImmediateContext) {
	//检查模型是否已经加载
	if (resultmodel_Buffer.find(StrToID(cur_model)) == resultmodel_Buffer.end()) {
		auto meshData = Geometry::loadFromFile(resultmodel_path[StrToID(cur_model)].c_str());
		resultmodel_Buffer[StrToID(cur_model)] = make_Buffer(meshData, m_pd3dImmediateContext);
		result_mesh[StrToID(cur_model)] = meshData;
	}
	auto [V, I, M] = resultmodel_Buffer[StrToID(cur_model)];
	UINT stride = sizeof(VertexPosNormalColorTex);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, &V, &stride, &offset);
	m_pd3dImmediateContext->IASetIndexBuffer(I, DXGI_FORMAT_R32_UINT, 0);
	m_pd3dImmediateContext->DrawIndexed(M, 0, 0);

#ifdef test
float theta = 0;
	float omiga = 0;
	float r = 2.5;
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 4; j++) {
			float u=omiga + (j * PI / 3.0);
			float v = theta + (i * 2 * PI / 6.0);
			DirectX::XMFLOAT3 pos = {r*sin(u)*cos(v),r * sin(u)*sin(v),r * cos(u)};
			auto m = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos),{0,0,0},{0,1,0}));
			m=DirectX::XMMatrixInverse(nullptr,m);
			effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)m.r);
			effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext);
			m_pd3dImmediateContext->DrawIndexed(M, 0, 0);
		}
	}
	
	DirectX::XMMatrixIdentity();
	effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)DirectX::XMMatrixIdentity().r);
	effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext);;
#endif // DEBUG

	



}
void DrawRightModel(std::string cur_model, ID3D11DeviceContext* m_pd3dImmediateContext) {
	//检查模型是否已经加载
	if (groundtruth_Buffer.find(StrToID(cur_model)) == groundtruth_Buffer.end()) {
		auto it = Geometry::loadFromFile(groundtruth_path[StrToID(cur_model)].c_str());
		groundtruth_Buffer[StrToID(cur_model)] = make_Buffer(it, m_pd3dImmediateContext);
       
	}
	auto [V, I, M] = groundtruth_Buffer[StrToID(cur_model)];
	UINT stride = sizeof(VertexPosNormalColorTex);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, &V, &stride, &offset);
	m_pd3dImmediateContext->IASetIndexBuffer(I, DXGI_FORMAT_R32_UINT, 0);
	m_pd3dImmediateContext->DrawIndexed(M, 0, 0);
}
void GameApp::VertexDraw(bool left)
{
    static ID3D11Buffer* m_pVertexBuffer = nullptr;			// 顶点缓冲区
    static ID3D11Buffer* m_pIndexBuffer = nullptr;
    static int m_IndexCount = 0;
    if (m_pIndexBuffer == nullptr) {
        auto meshData = Geometry::CreateSphere<VertexPosNormalTex>(0.003, 20, 20, { 0.0f,1.0f,0.0f,1.0f });
        D3D11_BUFFER_DESC pDesc;
        ZeroMemory(&pDesc, sizeof(pDesc));
        pDesc.Usage = D3D11_USAGE_IMMUTABLE;
        pDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        pDesc.CPUAccessFlags = 0;
        pDesc.ByteWidth = meshData.vertexVec.size() * sizeof(VertexPosNormalTex);
        D3D11_SUBRESOURCE_DATA pInit = { meshData.vertexVec.data() ,0,0 };
        m_pd3dDevice->CreateBuffer(&pDesc, &pInit, &m_pVertexBuffer);
        m_IndexCount = meshData.indexVec.size();
        pDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        pDesc.ByteWidth = meshData.indexVec.size() * sizeof(DWORD);;
        pInit.pSysMem = meshData.indexVec.data();
        m_pd3dDevice->CreateBuffer(&pDesc, &pInit, &m_pIndexBuffer);
    }

    auto [V, I, M] = left ? resultmodel_Buffer[StrToID(cur_model)] : groundtruth_Buffer[StrToID(cur_model)];
    UINT stride[2] = { sizeof(VertexPosNormalTex),sizeof(VertexPosNormalColorTex) };
    UINT offset[2] = { 0 ,0 };

    ID3D11Buffer* Vb[2] = {m_pVertexBuffer,V };
    D3D11_BUFFER_DESC pDesc;
    V->GetDesc(&pDesc);
    m_pd3dImmediateContext->IASetVertexBuffers(0, 2, Vb, stride, offset);
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayoutInstance.Get());
    m_pd3dImmediateContext->DrawIndexedInstanced(m_IndexCount, pDesc.ByteWidth/ sizeof(VertexPosNormalColorTex), 0, 0, 0);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());




}

void Drawpoi(std::string cur_model, ID3D11DeviceContext* m_pd3dImmediateContext, bool left = true) {
	static ID3D11Buffer* m_pVertexBuffer = nullptr;			// 顶点缓冲区
	static ID3D11Buffer* m_pIndexBuffer = nullptr;
	static int m_IndexCount = 0;
	if (m_pIndexBuffer == nullptr) {
		auto meshData = Geometry::CreateSphere<VertexPosNormalColorTex>(0.015, 60, 60, { 0.0f,1.0f,0.0f,1.0f });
		auto [VertexBuffer, IndexBuffer, IndexCount] = make_Buffer(meshData, m_pd3dImmediateContext);
		m_pVertexBuffer = VertexBuffer;
		m_pIndexBuffer = IndexBuffer;
		m_IndexCount = IndexCount;
	}

	if (left && resultpoi_pos.find(StrToID(cur_model)) == resultpoi_pos.end()) {

		resultpoi_pos[StrToID(cur_model)] = Geometry::loadSpherePosFromFile(resultpoi_path[StrToID(cur_model)].c_str());
	}
	UINT stride = sizeof(VertexPosNormalColorTex);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	auto pos = resultpoi_pos[StrToID(cur_model)];
	if (!left)
	{
		pos = groundtruth_poi_pos[StrToID(cur_model)];
	}
	for (auto it : pos) {
		auto m = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(sphere_radius, sphere_radius, sphere_radius), DirectX::XMMatrixTranslation(it.first.x, it.first.y, it.first.z)));
		effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)m.r);
		float poi_color[4] = { 1,1,0,1 };
		effect.GetConstantBufferVariable("poi_color")->SetFloatVector(3,&it.second.x);
		effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext);;
		m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
	}
	DirectX::XMMatrixIdentity();
	effect.GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (float*)DirectX::XMMatrixIdentity().r);
	effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext);;
}
struct ModelObject {
};
Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> loadTextureFromFile(ID3D11Device* m_pd3dDevice,const char * path=nullptr) {
	ID3D11DeviceContext* m_pd3dImmediateContext = nullptr;
	m_pd3dDevice->GetImmediateContext(&m_pd3dImmediateContext);
	int width, height, comp;
	stbi_uc* img_data = stbi_load(path, &width, &height, &comp, STBI_rgb_alpha);
	if (img_data)
	{
		CD3D11_TEXTURE2D_DESC texDesc(DXGI_FORMAT_R8G8B8A8_UNORM,
			width, height, 1,
			0,
			D3D11_BIND_SHADER_RESOURCE,
			D3D11_USAGE_DEFAULT, 0, 1, 0,
			0);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>res;
		HR(m_pd3dDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));

		// 上传纹理数据
		m_pd3dImmediateContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
		// 创建SRV
		HR(m_pd3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
		
		return res;
	}
	return nullptr;
}
#pragma endregion

XMVECTOR XM_CALLCONV Unproject
(
	FXMVECTOR V,
	float     ViewportX,
	float     ViewportY,
	float     ViewportWidth,
	float     ViewportHeight,
	float     ViewportMinZ,
	float     ViewportMaxZ,
	FXMMATRIX Projection,
	CXMMATRIX View
)
{
	static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };

	//得到逆视口变换的位移变换和缩放变换
	XMVECTOR Scale = XMVectorSet(ViewportWidth * 0.5f, -ViewportHeight * 0.5f, ViewportMaxZ - ViewportMinZ, 1.0f);
	Scale = XMVectorReciprocal(Scale);

	XMVECTOR Offset = XMVectorSet(-ViewportX, -ViewportY, -ViewportMinZ, 0.0f);
	Offset = XMVectorMultiplyAdd(Scale, Offset, D.v);


	XMMATRIX Transform = XMMatrixMultiply(View, Projection);
	Transform = XMMatrixInverse(nullptr, Transform);

	//逆视口变换, 此步到NDC坐标
	XMVECTOR Result = XMVectorMultiplyAdd(V, Scale, Offset);
	//逆VP，此步到世界坐标
	return XMVector3TransformCoord(Result, Transform);
}


struct Ray
{
	Ray();
	Ray(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction);
	static Ray makeFromScreen(D3D11_VIEWPORT& m_ScreenViewport, float x, float y, const XMMATRIX& P, const XMMATRIX& V, const XMVECTOR& position);
	bool XM_CALLCONV Hit(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2, float* pOutDist = nullptr, float maxDist = FLT_MAX);
	DirectX::XMFLOAT3 getPosition(float t) {
		auto o = XMLoadFloat3(&origin);
		auto d = XMLoadFloat3(&direction);
		XMVECTOR ans = d * t + o;
		XMFLOAT3 res;
		XMStoreFloat3(&res, ans);
		return res;
	}
	bool IsHitShpere(DirectX::XMFLOAT3 center, float r) {
		float x = center.x - origin.x;
		float y = center.y - origin.y;
		float z = center.z - origin.z;
		float l = x * direction.x + y * direction.y + z * direction.z;
		l *= l;
		float m = x * x + y * y + z * z;
		if (m - l > r * r)
			return false;
		return true;
	}
	DirectX::XMFLOAT3 origin;		// 射线原点
	DirectX::XMFLOAT3 direction;	// 单位方向向量
};
Ray::Ray()
	: origin(), direction(0.0f, 0.0f, 1.0f)
{
}
Ray::Ray(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction)
	: origin(origin)
{
	// 射线的direction长度必须为1.0f，误差在1e-5f内
	XMVECTOR dirLength = XMVector3Length(XMLoadFloat3(&direction));
	XMVECTOR error = XMVectorAbs(dirLength - XMVectorSplatOne());
	assert(XMVector3Less(error, XMVectorReplicate(1e-5f)));

	XMStoreFloat3(&this->direction, XMVector3Normalize(XMLoadFloat3(&direction)));
}
Ray Ray::makeFromScreen(D3D11_VIEWPORT& m_ScreenViewport, float x, float y, const XMMATRIX& P, const XMMATRIX& V, const XMVECTOR& position)
{
	auto worldpos = Unproject(XMVectorSet(x, y, 0, 1.0), m_ScreenViewport.TopLeftX,
		m_ScreenViewport.TopLeftY, m_ScreenViewport.Width, m_ScreenViewport.Height,
		m_ScreenViewport.MinDepth, m_ScreenViewport.MaxDepth, P, V);
	auto camerpos = position;
	auto direction = worldpos - camerpos;
	direction = XMVector3Normalize(direction);
	XMFLOAT3 o, d;
	XMStoreFloat3(&o, camerpos);
	XMStoreFloat3(&d, direction);
	return Ray(o, d);
}

/// <summary>
/// 检测射线与三角形的相交
/// </summary>
/// <param name="V0"></param>
/// <param name="V1"></param>
/// <param name="V2"></param>
/// <param name="pOutDist"></param>
/// <param name="maxDist"></param>
/// <returns></returns>

bool XM_CALLCONV Ray::Hit(FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, float* pOutDist, float maxDist)
{
	float dist;
	bool res = DirectX::TriangleTests::Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), V0, V1, V2, dist);
	if (res) {
		if (dist < maxDist) {
			*pOutDist = dist;
			return true;
		}


		return false;
	}
	return false;
}
GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance), curMode(CameraMode::TP)
{
}
GameApp::~GameApp()
{

}
Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>it=nullptr;
bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;
	if (!InitEffect())
		return false;
	if (!InitResource())
		return false;
	//加载纹理并创建采样器
	it=loadTextureFromFile(m_pd3dDevice.Get(),"Army.jpg");
	CD3D11_SAMPLER_DESC sampDesc(CD3D11_DEFAULT{});
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MaxAnisotropy = 0;
	ID3D11SamplerState* SSLinearClamp=nullptr;
	// 线性过滤与Clamp模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	HR(m_pd3dDevice->CreateSamplerState(&sampDesc, &SSLinearClamp));
    effect.SetShaderResourceByName("g_DiffuseMap", it.Get());
	effect.SetSamplerStateByName("g_Sam", SSLinearClamp);
	return true;
}
void GameApp::OnResize()
{
	D3DApp::OnResize();
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth) / 2;
	//
	if (effect.GetConstantBufferVariable("g_Proj"))
		effect.GetConstantBufferVariable("g_Proj")->SetVal(XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, AspectRatio() / 2, 0.1f, 1000.0f)));
	g_camera.SetProjParams(XM_PIDIV4, AspectRatio() / 4, 0.1f, 1000.0f);
	g_camera.SetWindow(m_ClientWidth, m_ClientHeight);
	g_camera.SetButtonMasks(0, 0, MOUSE_RIGHT_BUTTON);
}


void GameApp::UpdateScene(float dt)
{
	g_camera.FrameMove(dt);
	curraduis = g_camera.GetRadius();
	XMMATRIX mView = g_camera.GetViewMatrix();
	XMMATRIX mWorld = g_camera.GetWorldMatrix();
	//g_camera.SetDrag(true);
#pragma region 操作相机部分
	effect.GetConstantBufferVariable("g_View")->SetVal(XMMatrixTranspose(XMMatrixMultiply(mWorld, mView)));
	effect.GetConstantBufferVariable("g_EyePosW")->SetVal(g_camera.GetEyePt());
    
#pragma endregion
#ifdef USE_IMGUI
	static bool flag = true;
	ImGui::Begin("Tools", &flag, ImGuiWindowFlags_None);
	if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_None)) {


#pragma region 
		//观察模式
		if (ImGui::BeginTabItem("model")) {
			auto io = ImGui::GetIO();
			ImGui::TextColored({ 1.0,1.0,0.0,1.0 }, "CurPos %f %f", io.MousePos.x, io.MousePos.y);
			static bool light = true;
			static bool probility = false;
			static bool basic = false;
            ImGui::Checkbox("PBR", &_pbrflag);
            ImGui::SameLine();
			if (ImGui::Checkbox("light", &light)) {
				probility = false;
				basic = false;
				effect.GetConstantBufferVariable("moade")->SetSInt(0);
			}
			ImGui::SameLine();

			if (ImGui::Checkbox("probility", &probility)) {
				light = false;
				basic = false;
				effect.GetConstantBufferVariable("moade")->SetSInt(1);
			}
			ImGui::SameLine();

			if (ImGui::Checkbox("basic", &basic)) {
				probility = false;
				light = false;
				effect.GetConstantBufferVariable("moade")->SetSInt(3);
			}
			if (basic) {
				static float density = 0.3;
				ImGui::SliderFloat("density", &density, 0, 1);
				effect.GetConstantBufferVariable("basic")->SetFloat(density);
			}
			if (light || probility) {
				//static float colorA[4] = { 0xEE/256.0,0xBD/256.0,0x89/256.0,1.0 };
				//static float colorB[4] = { 0xD1 / 256.0,0x3A / 256.0,0xBD / 256.0,1.0 };
				static float colorA[4] = { 1,0,0,1.0 };
				static float colorB[4] = { 0,1,1,1.0 };
				ImGui::ColorEdit4("ColorA", colorA);
				ImGui::ColorEdit4("ColorB", colorB);
				//ImGui::Text("%f %f %f %f", color[0], color[1], color[2], color[3]);
				effect.GetConstantBufferVariable("m_color")->SetFloatVector(4, colorA);
				effect.GetConstantBufferVariable("c_color")->SetFloatVector(4, colorB);

			}
			curDraw = VIEW;
#pragma region poi结果对比
			static int meshindex = 0;
			if (ImGui::IsKeyPressed('E', false)) {
				meshindex = (meshindex + 1) % resultmodel_path.size();
				cur_model = item[meshindex];
			}
			if (ImGui::IsKeyPressed('Q', false)) {
				if (meshindex > 0)
					meshindex--;
				cur_model = item[meshindex];
			}
			if (ImGui::Combo("ModelList", &meshindex, item, resultmodel_path.size())) {
				//切换poi 和model
				cur_model = item[meshindex];
			}

			ImGui::Checkbox("listenClicked", &listenClicked);
			ImGui::Checkbox("show poi", &_poiflag);

			if (_poiflag) {
				ImGui::Text("this is a test");
				if (resultpoi_pos.count(StringToID(cur_model))) {
					int id = 0;
					for (auto& it : resultpoi_pos[StringToID(cur_model)]) {
						string a = "color " + to_string(id);
						//static float color[4] = { it.second.x ,it.second.y, it.second.z ,1.0};
						ImGui::ColorEdit4(a.c_str(),&it.second.x);
					//ImGui::Text("poi %f %f %f\n",it.second.x,it.second.y,it.second.z);
						id++;

					}
				}
			}

			
			if (_pbrflag) {
				effect.GetConstantBufferVariable("pbrflag")->SetSInt(1);
				static float roughness = 0.1;
				static float metallic = 0.1;
				ImGui::SliderFloat("roughness", &roughness, 0, 1);
				ImGui::SliderFloat("metallic", &metallic, 0, 1);
				effect.GetConstantBufferVariable("roughness")->SetFloat(roughness);
				effect.GetConstantBufferVariable("metallic")->SetFloat(metallic);
			}
			else {
				effect.GetConstantBufferVariable("pbrflag")->SetSInt(0);
			}
            ImGui::Checkbox("modelDraw",&_drawmodel);
            ImGui::SameLine();
            ImGui::Checkbox("modelLine",&_modelline);
            ImGui::SameLine();
			ImGui::Checkbox("wireframe", &_WireFrame);
            ImGui::SliderFloat("vertex_clip",&_vertex_clip,0.0f,1.0f);
        
			ImGui::Checkbox("Draw vertexX", &_DrawVertexX);
            if (_DrawVertexX) {
                ImGui::SliderFloat("VertexRadiusX",&_VertexRadiusX,0.0,10.0);
                ImGui::ColorEdit4("VertexRadiusXColor", VertexRadiusXcolor);
            }
            ImGui::Checkbox("Draw vertexY",&_DrawVertexY);
            if (_DrawVertexY) {
                ImGui::SliderFloat("VertexRadiusY", &_VertexRadiusY, 0.0, 10.0);
                ImGui::ColorEdit4("VertexRadiusYColor", VertexRadiusYcolor);
            }

			ImGui::Checkbox("Mirroring", &_Mirroring);
            ImGui::SameLine();
			if (ImGui::Checkbox("Clip", &_Clip)) {
				if (!_Clip) {
					effect.GetConstantBufferVariable("_Clip")->SetSInt(0);
				}
				else {
					effect.GetConstantBufferVariable("_Clip")->SetSInt(1);
				}
			}
			if (_Clip) {
				static float clipval = 0;

				ImGui::SliderFloat("clipval", &clipval, 0, 1);
				effect.GetConstantBufferVariable("k")->SetFloat(clipval);
			}

			static int coffi = 0;
			ImGui::RadioButton("QuarticEaseOut",&coffi,0);
			ImGui::SameLine();
			ImGui::RadioButton("QuadraticEaseOut", &coffi, 1);
			ImGui::SameLine();
			ImGui::RadioButton("CubicEaseOut", &coffi, 2);
			ImGui::SameLine();
			ImGui::RadioButton("QuinticEaseOut", &coffi, 3);
			ImGui::SameLine();
			ImGui::RadioButton("SineEaseOut", &coffi, 4);
			ImGui::RadioButton("CircularEaseOut", &coffi, 5);
			ImGui::SameLine();
			ImGui::RadioButton("ExponentialEaseOut", &coffi, 6);
			ImGui::SameLine();
			ImGui::RadioButton("ElasticEaseOut", &coffi, 7);
			ImGui::SameLine();
			ImGui::RadioButton("BackEaseOut", &coffi, 8);
			ImGui::SameLine();
			ImGui::RadioButton("BounceEaseOut", &coffi, 9);
			effect.GetConstantBufferVariable("coffi")->SetSInt(coffi);


			if (_WireFrame) {
				effect.GetEffectPass("model")->SetRasterizerState(m_pRSWireframe.Get());
                
			}
			else {
				effect.GetEffectPass("model")->SetRasterizerState(m_pRSnoCull.Get());
			}

			ImGui::InputText("savepicpath", savepicpath, len);
			ImGui::SliderFloat("zoom", &g_camera.m_fRadius, 0.1, 4);
			if (ImGui::SliderFloat("dis", &dis, 5, 30)) {
				m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -dis);
				effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&m_PointLight);
			
			}
			ImGui::SliderFloat("sphereradius", &sphere_radius, 1, 3);
			if (ImGui::TreeNode("pic")) {
				for (auto it : picArray) {
					//ImGui::SameLine();
					ImGui::Image(it.SRV, { (float)it.w,(float)it.h });
				}

				ImGui::TreePop();
			}

#pragma endregion
			ImGui::EndTabItem();		
			if (listenClicked) {
			if (1) {
				static ImVec2 pos;
				//鼠标的点击动作
			if (ImGui::IsMouseClicked(0)) {
					ImGuiIO& io = ImGui::GetIO();
					pos = io.MouseClickedPos[0];
					//构建射线
					auto it = m_ScreenViewport;
					it.TopLeftX = 0;
					Ray tempRay = Ray::makeFromScreen(it, pos.x, pos.y, XMMatrixPerspectiveFovLH(XM_PIDIV4, AspectRatio() / 2, 0.1f, 1000.0f), XMMatrixMultiply(mWorld, mView), g_camera.GetEyePt());
					//首先与已有小球求交，如果相交则需要销毁点中小球
					int index = -1;

					for (int i = 0; i < resultpoi_pos[StrToID(cur_model)].size(); i++) {
						if (tempRay.IsHitShpere(resultpoi_pos[StrToID(cur_model)][i].first, 0.015)) {
							index = i;
							break;
						}
					}
					if (index != -1) {
						for (int i = index; i < resultpoi_pos[StrToID(cur_model)].size() - 1; i++) {
							resultpoi_pos[StrToID(cur_model)][i] = resultpoi_pos[StrToID(cur_model)][i + 1];
						}
						resultpoi_pos[StrToID(cur_model)].pop_back();
					}
					else {

						//对射线求出与三维模型最近的交点
						float dis = FLT_MAX;
						for (int i = 0; i < result_mesh[StrToID(cur_model)].indexVec.size() / 3; i++) {

							tempRay.Hit(XMLoadFloat3(&result_mesh[StrToID(cur_model)].vertexVec[result_mesh[StrToID(cur_model)].indexVec[3 * i]].pos),
								XMLoadFloat3(&result_mesh[StrToID(cur_model)].vertexVec[result_mesh[StrToID(cur_model)].indexVec[3 * i + 1]].pos),
								XMLoadFloat3(&result_mesh[StrToID(cur_model)].vertexVec[result_mesh[StrToID(cur_model)].indexVec[3 * i + 2]].pos), &dis, dis);

						}
						if (dis < FLT_MAX) {
							DirectX::XMFLOAT3 Worldpos = tempRay.getPosition(dis);
							resultpoi_pos[StrToID(cur_model)].push_back(std::make_pair(Worldpos, DirectX::XMFLOAT3(1.0,0.0,0.0)));

						}





					}
				}
			if (ImGui::IsKeyPressed('W', false)) {
				std::string p = g_settings.correct_poi_path + cur_model + ".off.poicoord.txt";
				Geometry::StoreToFile(resultpoi_pos[StrToID(cur_model)], p.c_str());
				MessageBox(nullptr, L"save done", L"success", 0);
			}
			}

		    }

		}
		//检测鼠标点击



#pragma endregion
#pragma region 
		//标记模式
		if (ImGui::BeginTabItem("mark poi")) {
			curDraw = MARK;
			static int markindex = 0;
			if (ImGui::IsKeyPressed('E', false)) {
				markindex = (markindex + 1) % markmodellist.size();
				makr_meshdata.indexVec.clear();
				makr_meshdata.vertexVec.clear();
				mark_poilShperelist.clear();
				_mark_poilShperelist.clear();
				Geometry::loadFromFile(&makr_meshdata, &mark_poilShperelist, markmodellist[markindex].c_str());
			}
			if (ImGui::IsKeyPressed('Q', false)) {
				if (markindex > 0)
					markindex--;
				makr_meshdata.indexVec.clear();
				makr_meshdata.vertexVec.clear();
				mark_poilShperelist.clear();
				_mark_poilShperelist.clear();
				Geometry::loadFromFile(&makr_meshdata, &mark_poilShperelist, markmodellist[markindex].c_str());
			}
			if (ImGui::IsKeyPressed('W', false)) {
				std::string p = g_settings.markpoipath + getModelName(markmodellist[markindex]) + ".off.FeaturePoint.txt";
				Geometry::StoreToFile(&_mark_poilShperelist, p.c_str());
				MessageBox(nullptr, L"HLSL//save done", L"HLSL//success", 0);
			}
			//展示可用列表

			if (ImGui::Combo("ModelList", &markindex, markitem, markmodellist.size())) {
				makr_meshdata.indexVec.clear();
				makr_meshdata.vertexVec.clear();
				mark_poilShperelist.clear();
				_mark_poilShperelist.clear();
				Geometry::loadFromFile(&makr_meshdata, &mark_poilShperelist, markmodellist[markindex].c_str());
			}
			if (ImGui::Button("undo")) {
				if (!_mark_poilShperelist.empty()) {
					_mark_poilShperelist.pop_back();
				}
			}
			if (ImGui::Button("Reset")) {
				_mark_poilShperelist.clear();
			}
			if (ImGui::Button("copyleft")) {
				_mark_poilShperelist = mark_poilShperelist;
			}
			if (ImGui::Button("save")) {
				std::string p = g_settings.markpoipath + getModelName(markmodellist[markindex]) + ".off.FeaturePoint.txt";
				Geometry::StoreToFile(&_mark_poilShperelist, p.c_str());
				MessageBox(nullptr, L"save done", L"success", 0);
			}
			ImGui::EndTabItem();
#pragma region 鼠标拾取
			//----------鼠标拾取

			static ImVec2 pos;
			//鼠标的点击动作
			if (ImGui::IsMouseClicked(0)) {
				ImGuiIO& io = ImGui::GetIO();
				pos = io.MouseClickedPos[0];
				//构建射线
				Ray tempRay = Ray::makeFromScreen(m_ScreenViewport, pos.x, pos.y, XMMatrixPerspectiveFovLH(XM_PIDIV4, AspectRatio() / 2, 0.1f, 1000.0f), XMMatrixMultiply(mWorld, mView), g_camera.GetEyePt());;//Ray::ScreenToRay(*m_pCamera, pos.x, pos.y);
				//首先与已有小球求交，如果相交则需要销毁点中小球
				int index = -1;
				for (int i = 0; i < _mark_poilShperelist.size(); i++) {
					if (tempRay.IsHitShpere(_mark_poilShperelist[i].first, 0.015)) {
						index = i;
						break;
					}
				}
				if (index != -1) {
					for (int i = index; i < _mark_poilShperelist.size() - 1; i++) {
						_mark_poilShperelist[i] = _mark_poilShperelist[i + 1];
					}
					_mark_poilShperelist.pop_back();
				}
				else {
					//对射线求出与三维模型最近的交点
					float dis = FLT_MAX;

					for (int i = 0; i < makr_meshdata.indexVec.size() / 3; i++) {

						tempRay.Hit(XMLoadFloat3(&makr_meshdata.vertexVec[makr_meshdata.indexVec[3 * i]].pos),
							XMLoadFloat3(&makr_meshdata.vertexVec[makr_meshdata.indexVec[3 * i + 1]].pos), XMLoadFloat3(&makr_meshdata.vertexVec[makr_meshdata.indexVec[3 * i + 2]].pos), &dis, dis);

					}
					if (dis < FLT_MAX) {
						DirectX::XMFLOAT3 Worldpos = tempRay.getPosition(dis);
						//生成一个小球
						auto meshData = Geometry::CreateSphere<VertexPosNormalColorTex>(0.015, 20, 20, { 1.0f,1.0f,0.0f,1.0f });
						for (auto& it : meshData.vertexVec) {
							it.pos = DirectX::XMFLOAT3{ Worldpos.x + it.pos.x,Worldpos.y + it.pos.y,Worldpos.z + it.pos.z };
						}
						_mark_poilShperelist.push_back(std::make_pair(Worldpos, meshData));
					}
				}
			}
#pragma endregion
		}
		ImGui::EndTabBar();
#pragma endregion
	}
	ImGui::End();
#endif // USE_IMGUI
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	static float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   
	effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
	if (curDraw == VIEW) {
		m_ScreenViewport.Width = static_cast<float>(m_ClientWidth) / 2;
		// 设置左边视口
		m_ScreenViewport.TopLeftX = 0;
		m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
       
        if (_drawmodel) {
           Draw(cur_model, m_pd3dImmediateContext.Get());
        }
		
		if (_poiflag) {
			effect.GetConstantBufferVariable("poiflag")->SetSInt(1);
			effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
			Drawpoi(cur_model, m_pd3dImmediateContext.Get());
			effect.GetConstantBufferVariable("poiflag")->SetSInt(0);
			effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
		}
        //
        if (_modelline) {
            effect.GetConstantBufferVariable("vertex_clip")->SetFloat(_vertex_clip);
            effect.GetEffectPass("modelLine")->Apply(m_pd3dImmediateContext.Get());;
            Draw(cur_model, m_pd3dImmediateContext.Get());
        }


		//设置右边视口
       
		m_ScreenViewport.TopLeftX = static_cast<float>(m_ClientWidth) / 2;
		m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
        if (_drawmodel) {
           effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
		   DrawRightModel(cur_model, m_pd3dImmediateContext.Get());
        }

		if (_poiflag) {
			effect.GetConstantBufferVariable("poiflag")->SetSInt(1);
			effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
			Drawpoi(cur_model, m_pd3dImmediateContext.Get(), false);
			effect.GetConstantBufferVariable("poiflag")->SetSInt(0);
			effect.GetEffectPass("model")->Apply(m_pd3dImmediateContext.Get());;
		}
        if (_modelline) {
            effect.GetConstantBufferVariable("vertex_clip")->SetFloat(_vertex_clip);
            effect.GetEffectPass("modelLine")->Apply(m_pd3dImmediateContext.Get());;
            DrawRightModel(cur_model, m_pd3dImmediateContext.Get());
        }


		if (_DrawVertexX) {
            
            effect.GetConstantBufferVariable("vertex_scale")->SetFloat(_VertexRadiusX);
            effect.GetConstantBufferVariable("vertex_color")->SetFloatVector(4, VertexRadiusXcolor);
            effect.GetConstantBufferVariable("vertex_mode")->SetSInt(0);
            effect.GetConstantBufferVariable("vertex_clip")->SetFloat(_vertex_clip);
            m_ScreenViewport.TopLeftX = 0;
            m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
            effect.GetEffectPass("point")->Apply(m_pd3dImmediateContext.Get());;
            VertexDraw(true);
            m_ScreenViewport.TopLeftX = static_cast<float>(m_ClientWidth) / 2;
            m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
            effect.GetEffectPass("point")->Apply(m_pd3dImmediateContext.Get());;
            VertexDraw(false);
		}
        if (_DrawVertexY) {
           
            effect.GetConstantBufferVariable("vertex_scale")->SetFloat(_VertexRadiusY);
            effect.GetConstantBufferVariable("vertex_color")->SetFloatVector(4, VertexRadiusYcolor);
            effect.GetConstantBufferVariable("vertex_mode")->SetSInt(1);
            effect.GetConstantBufferVariable("vertex_clip")->SetFloat(_vertex_clip);
            m_ScreenViewport.TopLeftX = 0;
            m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
            effect.GetEffectPass("point")->Apply(m_pd3dImmediateContext.Get());;
            VertexDraw(true);
            m_ScreenViewport.TopLeftX = static_cast<float>(m_ClientWidth) / 2;
            m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
            effect.GetEffectPass("point")->Apply(m_pd3dImmediateContext.Get());;
            VertexDraw(false);
        }
	}
	else {
		//标记模式，重新设置视口
		m_ScreenViewport.Width = static_cast<float>(m_ClientWidth) / 2;
		// 设置左边视口
		m_ScreenViewport.TopLeftX = 0;
		m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
		ResetMesh(makr_meshdata);
		m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
		for (auto& mesh : mark_poilShperelist) {
			ResetMesh(mesh.second);
			m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
		}
		//设置右边视口
		m_ScreenViewport.TopLeftX = static_cast<float>(m_ClientWidth) / 2;
		m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
		ResetMesh(makr_meshdata);
		m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
		for (auto& mesh : _mark_poilShperelist) {
			ResetMesh(mesh.second);
			m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
		}
	}
#pragma region 
	if (ImGui::IsKeyDown(' ')) {
		ImGuiIO& io = ImGui::GetIO();
		static float prex = 0, prey = 0;
		static float endx = 0, endy = 0;
		static ID3D11ShaderResourceView* pictureview = nullptr;
		static  int w = 0, h = 0;
		//捕捉开始区域截屏的端点
		if (ImGui::IsKeyPressed('A', false)) {
			prex = io.MousePos.x;
			prey = io.MousePos.y;
		}
		//捕捉结束区域截屏的端点
		if (ImGui::IsKeyDown('A')) {
			endx = io.MousePos.x;
			endy = io.MousePos.y;
		}
		//确定一个截屏区域，扔到数组中
		if (ImGui::IsKeyReleased('A')) {
			rectArray.push_back({ prex,prey,endx,endy });
			prex = prey = endx = endy = 0;
		}
		//回退一个截屏区域
		if (ImGui::IsKeyPressed('Z', false)) {
			if (rectArray.size() > 0) {
				rectArray.pop_back();
			}
		}
		//触发截屏操作
		if (ImGui::IsKeyPressed('D', false)) {
			_mkdir(savepicpath);
			auto files = getFiles(savepicpath);
			std::string prefix = std::string(savepicpath) + "/result";
			std::string suffix = ".png";
			int start = files.size() + 1;
			for (auto it : rectArray) {
				void* predestData = getScreenRect(it.x0, it.y0, it.x1, it.y1, &w, &h);
				//创建一张图片扔到picArray中
				{
					ID3D11Texture2D* resolveTexture_ = nullptr;
					D3D11_TEXTURE2D_DESC textureDesc;
					memset(&textureDesc, 0, sizeof textureDesc);
					textureDesc.Width = w;
					textureDesc.Height = h;
					textureDesc.MipLevels = 1;
					textureDesc.ArraySize = 1;
					textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					textureDesc.SampleDesc.Count = 1;
					textureDesc.SampleDesc.Quality = 0;
					textureDesc.Usage = D3D11_USAGE_DEFAULT;
					textureDesc.CPUAccessFlags = 0;
					textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
					D3D11_SUBRESOURCE_DATA pInitialData;
					pInitialData.pSysMem = predestData;
					pInitialData.SysMemPitch = textureDesc.Width * 4;
					m_pd3dDevice->CreateTexture2D(&textureDesc, &pInitialData, &resolveTexture_);
					m_pd3dDevice->CreateShaderResourceView(resolveTexture_, nullptr, &pictureview);
					picArray.push_back({ w,h,pictureview,nullptr });
				}
				std::string savepath = prefix + std::to_string(start) + suffix;
				stbi_write_png(savepath.c_str(), w, h, 4, predestData, 0);
				free(predestData);
				start++;

				if (_Mirroring) {
					void* predestData = getScreenRect(m_ClientWidth / 2.0 + it.x0, it.y0, m_ClientWidth / 2.0 + it.x1, it.y1, &w, &h);
					//创建一张图片扔到picArray中
					{
						ID3D11Texture2D* resolveTexture_ = nullptr;
						D3D11_TEXTURE2D_DESC textureDesc;
						memset(&textureDesc, 0, sizeof textureDesc);
						textureDesc.Width = w;
						textureDesc.Height = h;
						textureDesc.MipLevels = 1;
						textureDesc.ArraySize = 1;
						textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						textureDesc.SampleDesc.Count = 1;
						textureDesc.SampleDesc.Quality = 0;
						textureDesc.Usage = D3D11_USAGE_DEFAULT;
						textureDesc.CPUAccessFlags = 0;
						textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						D3D11_SUBRESOURCE_DATA pInitialData;
						pInitialData.pSysMem = predestData;
						pInitialData.SysMemPitch = textureDesc.Width * 4;
						m_pd3dDevice->CreateTexture2D(&textureDesc, &pInitialData, &resolveTexture_);
						m_pd3dDevice->CreateShaderResourceView(resolveTexture_, nullptr, &pictureview);
						picArray.push_back({ w,h,pictureview,nullptr });
					}
					std::string savepath = prefix + std::to_string(start) + suffix;
					stbi_write_png(savepath.c_str(), w, h, 4, predestData, 0);
					free(predestData);
					start++;
				}
			}
		}
		//画出截屏区域
		{
			drawRect(prex, prey, endx, endy);
			if (_Mirroring)
				drawRect(m_ClientWidth / 2.0 + prex, prey, m_ClientWidth / 2.0 + endx, endy);
			for (auto it : rectArray) {
				drawRect(it.x0, it.y0, it.x1, it.y1);
				if (_Mirroring)
					drawRect(m_ClientWidth / 2.0 + it.x0, it.y0, m_ClientWidth / 2.0 + it.x1, it.y1);
			}
		}
	}
#pragma endregion
#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
	m_pSwapChain->Present(0, 0);
}
LRESULT GameApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = D3DApp::MsgProc(hwnd, msg, wParam, lParam);
	g_camera.HandleMessages(hwnd, msg, wParam, lParam);
	return res;

}
void* GameApp::getScreenRect(float prex, float prey, float endx, float endy, int* w, int* h)
{
	//确定复制屏幕的区域
	int left = min(prex, endx);
	int right = max(prex, endx);
	int top = min(prey, endy);
	int bottom = max(prey, endy);
	*w = right - left;
	*h = bottom - top;
	//首先创建一张分辨率贴图
	auto it = [](int width_, int height_, ID3D11Device* m_pd3dDevice, ID3D11Texture2D** resolveTexture_) {
		D3D11_TEXTURE2D_DESC textureDesc;
		memset(&textureDesc, 0, sizeof textureDesc);
		textureDesc.Width = (UINT)width_;
		textureDesc.Height = (UINT)height_;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		m_pd3dDevice->CreateTexture2D(&textureDesc, nullptr, resolveTexture_);
	};
	ID3D11Texture2D* resolveTexture_ = nullptr;
	it(m_ClientWidth, m_ClientHeight, m_pd3dDevice.Get(), &resolveTexture_);

	//创建一张用来拷贝渲染贴图的数据
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof textureDesc);
	textureDesc.Width = right - left;
	textureDesc.Height = bottom - top;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	ID3D11Texture2D* stagingTexture = nullptr;
	m_pd3dDevice->CreateTexture2D(&textureDesc, nullptr, &stagingTexture);
	ID3D11Resource* source = nullptr;
	m_pRenderTargetView->GetResource(&source);

	m_pd3dImmediateContext->ResolveSubresource(resolveTexture_, 0, source, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	//m_pd3dImmediateContext->CopyResource(stagingTexture,resolveTexture_);
	D3D11_BOX box;
	box.left = left;
	box.top = top;
	box.right = right;
	box.bottom = bottom;
	box.front = 0;
	box.back = 1;
	m_pd3dImmediateContext->CopySubresourceRegion(stagingTexture, 0, 0, 0, 0, resolveTexture_, 0, &box);
	D3D11_MAPPED_SUBRESOURCE mappedData;
	mappedData.pData = nullptr;
	m_pd3dImmediateContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedData);
	unsigned char* destData = new unsigned char[textureDesc.Width * textureDesc.Height * 4];
	unsigned char* predestData = destData;
	for (int y = 0; y < textureDesc.Height; ++y)
	{
		unsigned char* src = (unsigned char*)mappedData.pData + y * mappedData.RowPitch;
		for (int x = 0; x < textureDesc.Width; ++x)
		{
			*destData++ = *src++;
			*destData++ = *src++;
			*destData++ = *src++;
			*destData++ = *src++;
		}
	}
	m_pd3dImmediateContext->Unmap(stagingTexture, 0);
	stagingTexture->Release();
	source->Release();
	resolveTexture_->Release();
	return predestData;
}
void GameApp::drawRect(float prex, float prey, float endx, float endy)
{
	
	int left = min(prex, endx);
	int right = max(prex, endx);
	int top = min(prey, endy);
	int bottom = max(prey, endy);
	float x0 = left * 2.0 / m_ClientWidth - 1.0;
	float y0 = 1.0 - top * 2.0 / m_ClientHeight;
	float x1 = right * 2.0 / m_ClientWidth - 1.0;
	float y1 = 1.0 - bottom * 2.0 / m_ClientHeight;
	float Vertices[35] = {
	x0,y0,0.0,  1.0,0.0,0.0,1.0,
	x1,y0,0.0,  1.0,0.0,0.0,1.0,
	x1,y1,0.0,  1.0,0.0,0.0,1.0,
	x0,y1,0.0,  1.0,0.0,0.0,1.0,
	x0,y0,0.0,  1.0,0.0,0.0,1.0
	};
	static ID3D11Buffer* vertex = nullptr;
	if (vertex) {
		vertex->Release();
	}
	D3D11_BUFFER_DESC pDesc;
	ZeroMemory(&pDesc, sizeof(pDesc));
	pDesc.ByteWidth = 35 * 4;
	pDesc.Usage = D3D11_USAGE_IMMUTABLE;
	pDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = Vertices;
	m_pd3dDevice->CreateBuffer(&pDesc, &data, &vertex);
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	// 将着色器绑定到渲染管线
	effect.GetEffectPass("line")->Apply(m_pd3dImmediateContext.Get());;
	UINT stride = 7 * 4;
	UINT offset = 0;
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, &vertex, &stride, &offset);

	// 设置左边视口
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
	m_pd3dImmediateContext->Draw(5, 0);
}

bool GameApp::InitEffect()
{

	ComPtr<ID3DBlob> blob;
	effect.CreateShaderFromFile("VS", L"HLSL//VS.hlsl", m_pd3dDevice.Get(), "VS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf());
	// 创建并绑定顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColorTex::inputLayout, ARRAYSIZE(VertexPosNormalColorTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
	effect.CreateShaderFromFile("PS", L"HLSL//PS.hlsl", m_pd3dDevice.Get(), "PS", "ps_5_0");
	effect.CreateShaderFromFile("GS", L"HLSL//GS.hlsl", m_pd3dDevice.Get(), "GS", "gs_5_0");
    effect.CreateShaderFromFile("VSLine", L"HLSL//VSLine.hlsl", m_pd3dDevice.Get(), "VS", "vs_5_0");
    effect.CreateShaderFromFile("LineGS", L"HLSL//GSLine.hlsl", m_pd3dDevice.Get(), "GS", "gs_5_0");
    effect.CreateShaderFromFile("LinePS", L"HLSL//PSLine.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");
	effect.CreateShaderFromFile("HS", L"HLSL//HS.hlsl", m_pd3dDevice.Get(), "main", "hs_5_0");
	effect.CreateShaderFromFile("DS", L"HLSL//DS.hlsl", m_pd3dDevice.Get(), "main", "ds_5_0");
	effect.CreateShaderFromFile("LineV", L"HLSL//LineV.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0");
	effect.CreateShaderFromFile("LineP", L"HLSL//LineP.hlsl", m_pd3dDevice.Get(), "main", "ps_5_0");
    effect.CreateShaderFromFile("PointVS", L"HLSL//PointVS.hlsl", m_pd3dDevice.Get(), "main", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf());
    const D3D11_INPUT_ELEMENT_DESC inputdesc[] =
    {
        { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "SPHEREPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "SPHERECOLOR", 0,DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        { "SPHERETEXCOORD", 0,DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1}
    };
    // 创建并绑定顶点布局
    HR(m_pd3dDevice->CreateInputLayout(inputdesc, ARRAYSIZE(inputdesc),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayoutInstance.GetAddressOf()));



	effect.CreateShaderFromFile("PointPS", L"HLSL//PointPS.hlsl", m_pd3dDevice.Get(), "PS", "ps_5_0");
	effect.CreateShaderFromFile("PointGS", L"HLSL//PointGS.hlsl", m_pd3dDevice.Get(), "main", "gs_5_0");
	EffectPassDesc pass;
	pass.nameVS = "VS";
	pass.nameGS = "GS";
	pass.namePS = "PS";

	effect.AddEffectPass("model", m_pd3dDevice.Get(), &pass);
    pass.nameVS = "VSLine";
    pass.nameGS = "LineGS";
    pass.namePS = "LinePS";
    effect.AddEffectPass("modelLine", m_pd3dDevice.Get(), &pass);

	pass.nameVS = "LineV";
	pass.nameGS = "";
	pass.namePS = "LineP";
	pass.nameDS = "";
	pass.nameHS = "";
	effect.AddEffectPass("line", m_pd3dDevice.Get(), &pass);
	pass.nameVS = "PointVS";
	pass.nameGS = "";
	pass.namePS = "PointPS";
	pass.nameDS = "";
	pass.nameHS = "";
	effect.AddEffectPass("point", m_pd3dDevice.Get(), &pass);
	auto it = Geometry::CreateSphere<VertexPos, unsigned int>(0.004, 7, 7);
	float* p = new float[it.vertexVec.size() * 4];
	float* q = p;
	for (int i = 0; i < it.vertexVec.size(); i++) {
		*p++ = it.vertexVec[i].pos.x;
		*p++ = it.vertexVec[i].pos.y;
		*p++ = it.vertexVec[i].pos.z;
		p++;
	}
	effect.GetConstantBufferVariable("spherepos")->SetRaw(q);
	free(q);
	unsigned int* index = new unsigned int[it.indexVec.size() * 4];
	unsigned int* qindex = index;
	for (int i = 0; i < it.indexVec.size(); i++) {
		*index++ = it.indexVec[i];
		index += 3;

	}
	effect.GetConstantBufferVariable("sphereindex")->SetRaw(qindex);
	free(qindex);
	return true;
}
bool GameApp::InitResource()
{
	RenderStates::InitAll(m_pd3dDevice.Get());
	textureManager.Init(m_pd3dDevice.Get());
	PreComputeIBL(m_pd3dImmediateContext.Get());
	effect.SetSamplerStateByName("g_Sam",RenderStates::SSAnistropicClamp16x.Get());;
	effect.SetShaderResourceByName("irradiance_map",irradiance->GetShaderResource());
	effect.SetShaderResourceByName("prefilter_map",prefilter_map->GetShaderResource());
	effect.SetShaderResourceByName("BRDF_LUT",BRDF_LUT->GetShaderResource());
	g_settings.load();
	g_camera.SetViewParams(XMVectorSet(0, 0, -3, 0), g_XMZero);
	g_camera.SetRadius(curraduis, 0.1, 4);
	//设置当前的绘制模式为观察模式
	curDraw = VIEW;
#pragma region VIEW配置

	// ******************
	// 初始化网格模型
	//由于后来改变训练集测试集比例，此处并不对应，需要后期做修改
	int i = 0;
	//auto it = listFiles(g_settings.resultmodelpath);
	for (auto filepath : listFiles(g_settings.resultmodelpath)) {
		resultmodel_path[StrToID(getModelName(filepath))] = filepath;
		item[i] = new char[100];
		strcpy((char*)item[i], getModelName(filepath).c_str());
		i++;
	}
	for (auto filepath : listFiles(g_settings.resultpoipath)) {
		resultpoi_path[StrToID(getModelName(filepath))] = filepath;
	}
#pragma endregion
#pragma region mark配置
	//加载人工标记的模型路径列表
	//markmodellist = listFiles(g_settings.originpath);
    //markmodellist = listFiles("./data/PP2");
    markmodellist = listFiles(g_settings.originpath);
	for (auto filepath : markmodellist) {
		groundtruth_path[StrToID(getModelName(filepath))] = filepath;
          
	}

    //加载
    for (auto filepath : listFiles(g_settings.markpoipath)) {
        
        std::ifstream f;
        f.open(filepath);
        std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>> ans;
 
        float x, y, z;
        while (f >> x >> y >> z) {
              ans.push_back(std::make_pair(DirectX::XMFLOAT3{-x ,y ,z }, DirectX::XMFLOAT3{ 1 ,0 ,0 }));
        }
        f.close();
        groundtruth_poi_pos[StrToID(getModelName(filepath))]=ans;
       
    }
	for (int i = 0; i < 601; i++) {
		markitem[i] = new char[100];
	}
	for (int i = 0; i < markmodellist.size(); i++) {
		strcpy((char*)markitem[i], getModelName(markmodellist[i]).c_str());
	}
	assert(markmodellist.size() > 0, "no modelandpoi");
	//Geometry::loadFromFile(&makr_meshdata, &mark_poilShperelist, markmodellist[0].c_str());
    makr_meshdata=Geometry::loadFromFile(markmodellist[0].c_str());
#pragma endregion
#pragma region 渲染配置

	// 设置视口变换
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth) / 2;
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;



	// ******************
	// 设置常量缓冲区描述
	//


	// ******************
	// 初始化默认光照
	// 方向光
	m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DirLight.direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	// 点光
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PointLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 25.0f;

	// 初始化用于VS的常量缓冲区的值
	auto itt = effect.GetConstantBufferVariable("mode");
	effect.GetConstantBufferVariable("g_World")->SetVal(XMMatrixIdentity());
	effect.GetConstantBufferVariable("g_View")->SetVal(XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	)));
	effect.GetConstantBufferVariable("g_Proj")->SetVal(XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV4, AspectRatio() / 2, 0.1f, 1000.0f)));


	// 初始化用于PS的常量缓冲区的值
	float data[16] = {
	0.58f, 0.58f, 0.58f, 1.0f,
	0.58f, 0.58f, 0.58f, 1.0f,

	};
	effect.GetConstantBufferVariable("g_Material")->SetRaw(data);;
	effect.GetConstantBufferVariable("g_DirLight")->SetRaw(&m_DirLight);
    dis = 3;
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -dis);
	effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&m_PointLight);
	m_PointLight.position= XMFLOAT3(0.0f, 0.0f, dis);
	effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&m_PointLight,sizeof(PointLight)*1,sizeof(PointLight));
	m_PointLight.position = XMFLOAT3(dis, 0.0f, 0.0f);
	effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&m_PointLight, sizeof(PointLight) * 2, sizeof(PointLight));
	m_PointLight.position = XMFLOAT3(-dis, 0.0f, 0.0f);
	effect.GetConstantBufferVariable("g_PointLight")->SetRaw(&m_PointLight, sizeof(PointLight) * 3, sizeof(PointLight));
	effect.GetConstantBufferVariable("g_EyePosW")->Set(XMFLOAT3(0.0f, 0.0f, -5.0f));

	// ******************
	// 初始化光栅化状态
	//
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSnoCull.GetAddressOf()));
	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//
	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
#pragma endregion	
	return true;
}
bool GameApp::ResetMesh(const Geometry::MeshData<VertexPosNormalColorTex>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();
	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalColorTex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosNormalColorTex);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
	// 输入装配阶段的索引缓冲区设置
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	// 设置调试对象名
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
	return true;
}
std::string GameApp::getModelName(std::string path)
{
	int last = -1;
	for (int i = 0; i < path.size(); i++) {
		if (path[i] == '\\' || path[i] == '/')last = i;
	}
	std::string model = path.substr(last + 1);
	model = model.substr(0, model.find('.'));
	return model;
}



