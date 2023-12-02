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
	//PNG��ʽ
	void* getScreenRect(float prex, float prey, float endx, float endy,int* w,int*h);
	void drawRect(float prex,float prey,float endx,float endy);
	bool InitEffect();
	bool InitResource();
	bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);
	std::string getModelName(std::string path);
private:

	//�б����
	 const char* item[601];

	 //Ҫ��ǵ�ģ�Ͳο��б�·�������ģ�Ͳο�������ģ�ͺ���Ȥ��
	 std::vector<std::string>markmodellist;
	 Geometry::MeshData<VertexPosNormalColor>makr_meshdata;//���ص���άģ��
	 //�ο�����Ȥ��
	 std::vector<std::pair<DirectX::XMFLOAT3, Geometry::MeshData<VertexPosNormalColor>>>mark_poilShperelist;
	 //�ֶ���ǵ���Ȥ��
	 std::vector<std::pair<DirectX::XMFLOAT3, Geometry::MeshData<VertexPosNormalColor>>>_mark_poilShperelist;//��Ȥ��
	 const char* markitem[601];
	 
	 //��ǰ�Ļ���ģʽ
	 DrawSceneMode curDraw;

	CModelViewerCamera g_camera;
	CameraMode curMode;
	//std::shared_ptr<Camera> camera;
	ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // �������벼��
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// ���㻺����
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// ����������
	UINT m_IndexCount;							    // ������������������С

	DirectionalLight m_DirLight;					// Ĭ�ϻ�����
	PointLight m_PointLight;						// Ĭ�ϵ��
	SpotLight m_SpotLight;						    // Ĭ�ϻ�۹�
	ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// ��դ��״̬: �߿�ģʽ
	ComPtr<ID3D11RasterizerState> m_pRSnoCull;
	bool m_IsWireframeMode;							// ��ǰ�Ƿ�Ϊ�߿�ģʽ
};
#endif