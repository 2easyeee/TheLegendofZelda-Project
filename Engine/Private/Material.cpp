#include "Material.h"
#include "GameInstance.h"

CMaterial::CMaterial(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice{ _pDevice }
    , m_pDeviceContext{ _pDeviceContext }
{
    Safe_AddRef(_pDevice);
    Safe_AddRef(_pDeviceContext);
}

HRESULT CMaterial::Initialize(tinyxml2::XMLElement* _pNode, const _char* _pMaterialXMLPath)
{
    Create_DefaultTexture(DEFAULT_SRV::WHITE);
    Create_DefaultTexture(DEFAULT_SRV::BLACK);

    /* File */
    for (tinyxml2::XMLElement* element = _pNode->FirstChildElement();
        element; element = element->NextSiblingElement())
    {
        const char* tag = element->Name();
        const char* text = element->GetText();
        if (!text || text[0] == '\0')
            continue;

        MATERIAL eType = Get_MaterialTypeFromTag(tag);
        if (eType == MATERIAL::NONE)
            continue;

        /* 원본 경로 */
        filesystem::path xmlPath(_pMaterialXMLPath);
        filesystem::path basePath = xmlPath.parent_path();
        filesystem::path texPath = basePath / text;

        /* .dds 경로 */
        filesystem::path ddsPath = texPath;
        ddsPath.replace_extension(".dds");

        /* Convert Path */
        auto ConvertPath = [&](filesystem::path p)
            {
                _string FullPath = p.string();
                Replace(FullPath, "\\", "/");
                Replace(FullPath, "Models", "Assets");
                return filesystem::path(FullPath);
            };

        filesystem::path FinalPath;
        if (filesystem::exists(ConvertPath(ddsPath)))
            FinalPath = ConvertPath(ddsPath);
        else
            FinalPath = ConvertPath(texPath);

        /* Load */
        ID3D11ShaderResourceView* pSRV = { nullptr };
        if (!filesystem::exists(FinalPath))
        {
            pSRV = m_pDefaultSRV_White;
        }
        else
        {
            HRESULT hr = {};

            if (FinalPath.extension() == ".dds")
            {
                hr = CreateDDSTextureFromFile(m_pDevice,
                    FinalPath.wstring().c_str(),
                    nullptr, &pSRV);
            }
            else
            {
                hr = CreateWICTextureFromFile(m_pDevice,
                    FinalPath.wstring().c_str(),
                    nullptr, &pSRV);
            }

            if (FAILED(hr) | !pSRV)
                pSRV = m_pDefaultSRV_White;
        }
        m_vecMaterials[static_cast<_uint>(eType)].push_back(pSRV);
    }

    /* Color */
    auto LoadColor = [](tinyxml2::XMLElement* element, _float4& out)
        {
            element->QueryFloatAttribute("R", &out.x);
            element->QueryFloatAttribute("G", &out.y);
            element->QueryFloatAttribute("B", &out.z);
            element->QueryFloatAttribute("A", &out.w);
        };

    LoadColor(_pNode->FirstChildElement("Ambient"), m_tMaterialDesc.vAmbient);
    LoadColor(_pNode->FirstChildElement("Diffuse"), m_tMaterialDesc.vDiffuse);
    LoadColor(_pNode->FirstChildElement("Specular"), m_tMaterialDesc.vSpecular);
    LoadColor(_pNode->FirstChildElement("Emissive"), m_tMaterialDesc.vEmissive);

    return S_OK;
}

HRESULT CMaterial::Bind_Material(CShader* _pShader, const _char* _pConstantName, MATERIAL _eMaterialType, _uint _iTextureIndex)
{
    if (!_pShader)
        return E_FAIL;

    _pShader->Bind_ShaderResourceView(_pConstantName, Get_SRV(_eMaterialType, _iTextureIndex));
    
    return S_OK;
}

MATERIAL CMaterial::Get_MaterialTypeFromTag(const char* tag)
{
    if (!strcmp(tag, "DiffuseFile"))    return MATERIAL::DIFFUSE;
    if (!strcmp(tag, "SpecularFile"))   return MATERIAL::SPECULAR;
    if (!strcmp(tag, "NormalFile"))     return MATERIAL::NORMALS;
    if (!strcmp(tag, "EmissiveFile"))   return MATERIAL::EMISSIVE;
    if (!strcmp(tag, "MetallicFile"))   return MATERIAL::METALLIC;
    if (!strcmp(tag, "RoughnessFile"))  return MATERIAL::ROUGHNESS;
    if (!strcmp(tag, "AOFile"))         return MATERIAL::AO;

    return MATERIAL::NONE;
}

HRESULT CMaterial::Create_DefaultTexture(DEFAULT_SRV _eSRVColor)
{
    if (_eSRVColor == DEFAULT_SRV::WHITE && m_pDefaultSRV_White)
        return S_OK;

    if (_eSRVColor == DEFAULT_SRV::BLACK && m_pDefaultSRV_Black)
        return S_OK;

    _uint whitePixel = 0xFFFFFFFF;
    _uint blackPixel = 0xFF000000;
     //_uint magentaPixel = 0xFFFF00FF;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 1;
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data = {};
    switch (_eSRVColor)
    {
    case Engine::CMaterial::DEFAULT_SRV::WHITE:
        data.pSysMem = &whitePixel;
        break;
    case Engine::CMaterial::DEFAULT_SRV::BLACK:
        data.pSysMem = &blackPixel;
        break;
    }
    data.SysMemPitch = sizeof(unsigned int);

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = m_pDevice->CreateTexture2D(&desc, &data, &pTexture);
    if (FAILED(hr))
        return hr;

    switch (_eSRVColor)
    {
    case Engine::CMaterial::DEFAULT_SRV::WHITE:
        hr = m_pDevice->CreateShaderResourceView(pTexture, nullptr, &m_pDefaultSRV_White);
        break;
    case Engine::CMaterial::DEFAULT_SRV::BLACK:
        hr = m_pDevice->CreateShaderResourceView(pTexture, nullptr, &m_pDefaultSRV_Black);
        break;
    }
    Safe_Release(pTexture);

    return hr;
}

ID3D11ShaderResourceView* CMaterial::Get_SRV(MATERIAL _eType, _uint _iIndex)
{
    auto& vecMaterial = m_vecMaterials[ENUM_TO_UINT(_eType)];
    if (!vecMaterial.empty() && vecMaterial[_iIndex])
        return vecMaterial[_iIndex];

    switch (_eType)
    {
    case Engine::MATERIAL::DIFFUSE:
        return m_pDefaultSRV_White;
        break;
    case Engine::MATERIAL::METALLIC:
        return m_pDefaultSRV_Black;
        break;
    case Engine::MATERIAL::ROUGHNESS:
        return m_pDefaultSRV_Black;
        break;
    case Engine::MATERIAL::AO:
        return m_pDefaultSRV_White; /* AO File 이 비었을 때, 흰색이면 1.0 이라서 안전함. */
        break;
    default:
        return m_pDefaultSRV_White;
        break;
    }
    return nullptr;
}

CMaterial* CMaterial::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
    tinyxml2::XMLElement* _pNode, const _char* _pMaterialXMLPath)
{
    CMaterial* pInstance = new CMaterial(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_pNode, _pMaterialXMLPath)))
    {
        MSG_BOX("FAILED TO CREATED : CMaterial");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMaterial::Free()
{
    __super::Free();

    /* vector release */
    for (auto& Materials : m_vecMaterials)
    {
        for (auto& pSRV : Materials)
        {
            if (pSRV == m_pDefaultSRV_White ||pSRV == m_pDefaultSRV_Black)
                continue;
            Safe_Release(pSRV);
        }

        Materials.clear();
    }

    Safe_Release(m_pDefaultSRV_Black);
    Safe_Release(m_pDefaultSRV_White);
    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pDevice);
}
