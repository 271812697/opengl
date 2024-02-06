#ifndef GAMEAPP_H
#define GAMEAPP_H
#include "d3dApp.h"
#include"LightHelper.h"
#include "Geometry.h"
#include"DXCamera.h"
class GameApp : public D3DApp
{

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();
    enum  CameraMode{FP,TP};
	enum DrawSceneMode{VIEW,MARK};
	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	//PNG格式
	void* getScreenRect(float prex, float prey, float endx, float endy,int* w,int*h);
	void drawRect(float prex,float prey,float endx,float endy);
	void DrawLight(ID3D11DeviceContext* m_pd3dImmediateContext);
	bool InitEffect();
	bool InitResource();
	bool ResetMesh(const Geometry::MeshData<VertexPosNormalColorTex>& meshData);
	std::string getModelName(std::string path);
    void VertexDraw(bool left);
    
private:

	//列表参数
	 const char* item[601];

	 //要标记的模型参考列表路径，这个模型参考包括了模型和兴趣点
	 std::vector<std::string>markmodellist;
	 Geometry::MeshData<VertexPosNormalColorTex>makr_meshdata;//加载的三维模型
	 //参考的兴趣点
	 std::vector<std::pair<DirectX::XMFLOAT3, Geometry::MeshData<VertexPosNormalColorTex>>>mark_poilShperelist;
	 //手动标记的兴趣点
	 std::vector<std::pair<DirectX::XMFLOAT3, Geometry::MeshData<VertexPosNormalColorTex>>>_mark_poilShperelist;//兴趣点
	 const char* markitem[601];
	 
	 //当前的绘制模式
	 DrawSceneMode curDraw;

	CModelViewerCamera g_camera;
	CameraMode curMode;
	//std::shared_ptr<Camera> camera;
	ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // 顶点输入布局
    ComPtr<ID3D11InputLayout> m_pVertexLayoutInstance;	    // 顶点输入布局
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
	UINT m_IndexCount;							    // 绘制物体的索引数组大小

	DirectionalLight m_DirLight;					// 默认环境光
	PointLight m_PointLight[4];						// 默认点光
	SpotLight m_SpotLight;						    // 默认汇聚光
	ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
	ComPtr<ID3D11RasterizerState> m_pRSnoCull;
	bool m_IsWireframeMode;							// 当前是否为线框模式
};
#endif