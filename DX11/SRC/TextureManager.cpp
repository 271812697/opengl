#include "stb_image.h"
#include "TextureManager.h"
#include "XUtil.h"
#include "DXTrace.h"

#include <filesystem>
#include <DirectXPackedVector.h>
using namespace Microsoft::WRL;

namespace
{
    // TextureManager单例
    TextureManager* s_pInstance = nullptr;
}

TextureManager::TextureManager()
{
    if (s_pInstance)
        throw std::exception("TextureManager is a singleton!");
    s_pInstance = this;
}

TextureManager::~TextureManager()
{
}

TextureManager& TextureManager::Get()
{
    if (!s_pInstance)
        throw std::exception("TextureManager needs an instance!");
    return *s_pInstance;
}

void TextureManager::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());

    // 加一张1x1的空纹理
    ComPtr<ID3D11Texture2D> pTex;
    CD3D11_TEXTURE2D_DESC nullTexDesc(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1);
    D3D11_SUBRESOURCE_DATA initData{};
    uint32_t white = (uint32_t)-1;
    initData.pSysMem = &white;
    initData.SysMemPitch = 4;
    m_pDevice->CreateTexture2D(&nullTexDesc, &initData, pTex.GetAddressOf());

    ComPtr<ID3D11ShaderResourceView> pSRV;
    m_pDevice->CreateShaderResourceView(pTex.Get(), nullptr, pSRV.GetAddressOf());
    m_TextureSRVs.try_emplace(0, pSRV.Get()).second;
}

ID3D11ShaderResourceView* TextureManager::CreateFromFile(std::string_view filename, bool enableMips, bool forceSRGB)
{
    XID fileID = StringToID(filename);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();

    auto& res = m_TextureSRVs[fileID];
    ComPtr<ID3D11Texture2D> pTex;
    std::wstring wstr = UTF8ToWString(filename);
    if (true)
    {
        bool is_hdr=stbi_is_hdr(filename.data());
        int width, height, comp;
        void* img_data = nullptr;
        if (is_hdr) {
            img_data = stbi_loadf(filename.data(), &width, &height, &comp, 4);
        }
        else
            img_data = stbi_load(filename.data(), &width, &height, &comp, STBI_rgb_alpha);
        if (img_data)
        {
            CD3D11_TEXTURE2D_DESC texDesc(is_hdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM,
                width, height, 1,
                enableMips ? 0 : 1,
                D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
                D3D11_USAGE_DEFAULT, 0, 1, 0,
                enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
            HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
            // 上传纹理数据
            if (is_hdr) {
                m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(float) * 4, 0);
            }
            else
                m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data,  width * sizeof(uint32_t), 0);
            
            
            CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
                is_hdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM);
            // 创建SRV
            HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
            // 生成mipmap
            if (enableMips)
                m_pDeviceContext->GenerateMips(res.Get());
            stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
            SetDebugObjectName(res.Get(), std::filesystem::path(filename).filename().string());
#endif
        }
        else
        {
            std::string warning = "[Warning]: TextureManager::CreateFromFile, couldn't find \"";
            warning += filename;
            warning += "\"\n";
            OutputDebugStringA(warning.c_str());


        }
    }

    return res.Get();
}

ID3D11ShaderResourceView* TextureManager::CreateFromMemory(std::string_view name, void* data, size_t byteWidth, bool enableMips, bool forceSRGB)
{
    XID fileID = StringToID(name);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();

    auto& res = m_TextureSRVs[fileID];
    int width, height, comp;
    stbi_uc* img_data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data), (int)byteWidth, &width, &height, &comp, STBI_rgb_alpha);
    if (img_data)
    {
        CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
            width, height, 1,
            enableMips ? 0 : 1,
            D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
            D3D11_USAGE_DEFAULT, 0, 1, 0,
            enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
        // 上传纹理数据
        m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
        CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
            forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
        // 创建SRV
        HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
        // 生成mipmap
        if (enableMips)
            m_pDeviceContext->GenerateMips(res.Get());
        stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
        SetDebugObjectName(res.Get(), name);
#endif
    }
    else
    {
        std::string warning = "[Warning]: TextureManager::CreateFromMemory, failed to create texture \"";
        warning += name;
        warning += "\"\n";
        OutputDebugStringA(warning.c_str());
    }
    return res.Get();
}

bool TextureManager::AddTexture(std::string_view name, ID3D11ShaderResourceView* texture)
{
    XID nameID = StringToID(name);
    return m_TextureSRVs.try_emplace(nameID, texture).second;
}

void TextureManager::RemoveTexture(std::string_view name)
{
    XID nameID = StringToID(name);
    m_TextureSRVs.erase(nameID);
}

ID3D11ShaderResourceView* TextureManager::GetTexture(std::string_view filename)
{
    XID fileID = StringToID(filename);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();
    return nullptr;
}

ID3D11ShaderResourceView* TextureManager::GetNullTexture()
{
    return m_TextureSRVs[0].Get();
}
