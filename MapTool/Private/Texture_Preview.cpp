#include "Texture_Preview.h"

CTexture_Preview::CTexture_Preview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice { _pDevice }
    , m_pDeviecContext { _pDeviceContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviecContext);
}

void CTexture_Preview::Clear_SRV()
{
    for (auto& Pair : m_SRVs)
    {
        if (Pair.second)
            Pair.second->Release();
    }
    m_SRVs.clear();
}

ID3D11ShaderResourceView* CTexture_Preview::Get_SRV(const _string& _filePath)
{
    if (_filePath.empty())
        return nullptr;

    auto iter = m_SRVs.find(_filePath);
    if (iter != m_SRVs.end())
        return iter->second;

    ID3D11ShaderResourceView* pSRV = Load_Texture(_filePath);
    if (!pSRV)
        return nullptr;

    m_SRVs.emplace(_filePath, pSRV);
    return pSRV;
}

ID3D11ShaderResourceView* CTexture_Preview::Load_Texture(const _string& _filePath)
{
    filesystem::path FullFilePath 
        = filesystem::path("../Bin/Resources/Assets") / _filePath;

    if (!filesystem::exists(FullFilePath))
        return nullptr;

    ID3D11ShaderResourceView* pSRV = { nullptr };
    HRESULT hr = {};

    _wstring ext = FullFilePath.extension().wstring();
    transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    if (ext == L".dds")
    {
        hr = CreateDDSTextureFromFile(
            m_pDevice,
            FullFilePath.c_str(), nullptr,
            &pSRV);
    }
    else
    {
        hr = CreateWICTextureFromFile(
            m_pDevice, m_pDeviecContext,
            FullFilePath.c_str(), nullptr,
            &pSRV);
    }

    if (FAILED(hr))
        return nullptr;

    return pSRV;
}

CTexture_Preview* CTexture_Preview::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    return new CTexture_Preview(_pDevice, _pDeviceContext);
}

void CTexture_Preview::Free()
{
    Clear_SRV();
    Safe_Release(m_pDeviecContext);
    Safe_Release(m_pDevice);
}
