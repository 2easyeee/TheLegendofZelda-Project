#include "ShaderPreview.h"
#include "GameInstance.h"

CShaderPreview::CShaderPreview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CShaderPreview::CShaderPreview(const CShaderPreview& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CShaderPreview::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CShaderPreview::Initialize(void* _pArg)
{
    /* 1. Map Object */
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    /* 2. MAP_INIT_DESC */
    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    /* Shader Desc */
    m_tShaderDescDefault = m_tShaderDesc;
    m_vDefaultScale = { 1.f, 1.f, 1.f};

    return S_OK;
}

void CShaderPreview::Priority_Update(_float _fTimeDelta)
{
}

void CShaderPreview::Update(_float _fTimeDelta)
{
    if (m_pModelCom)
        m_pModelCom->Play_Animation(_fTimeDelta);

    m_tShaderDesc.fTime += _fTimeDelta;

    /* Timeline */
    if (m_bUseTimeline)
    {
        m_tTime.fAccTime += _fTimeDelta;

        float duration = max(m_tTime.fAccDurationTime, 0.0001f);
        float t = m_tTime.fAccTime / duration;

        if (t > 1.f)
            t = 1.f;

        /* Scale */
        _float3 scale = Lerp3(m_tTimeline.vScaleStart, m_tTimeline.vScaleEnd, t);
        m_pTransformCom->Set_Scale(scale.x, scale.y, scale.z);

        /* Rotation */
        if (m_tTimeline.fRotationSpeed != 0.f)
        {
            _vector axis = XMLoadFloat3(&m_tTimeline.vRotationAxis);
            m_pTransformCom->Turn(axis, m_tTimeline.fRotationSpeed * _fTimeDelta);
        }

        /* Alpha */
        m_tShaderDesc.fAlpha = Lerp(m_tTimeline.fAlphaStart, m_tTimeline.fAlphaEnd, t);

        /* Dissolve */
        m_tShaderDesc.fDissolve = Lerp(m_tTimeline.fDissolveStart, m_tTimeline.fDissolveEnd, t);
    }

}

void CShaderPreview::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLEND, this)))
        return;
}

HRESULT CShaderPreview::Render()
{
    /* Transform */
    if (FAILED(CGameObject::m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Model */
    if (m_pModelCom)
    {
        _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
        for (size_t i = 0; i < iNumMeshes; i++)
        {
            /* Material */
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
                return E_FAIL;
            /* Bind */
            if (FAILED(Render_Binding()))
                return E_FAIL;
            /* Bone */
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
            /* Shader */
            if (FAILED(m_pShaderCom->Begin(0)))
                return E_FAIL;
            /* Mesh */
            if (FAILED(m_pModelCom->Render(i)))
                return E_FAIL;
        }
    }

    return S_OK;
}

void CShaderPreview::Set_Texture(TEXTURE_SLOT _eSlot, ID3D11ShaderResourceView* _SRV, _wstring _Path)
{
    ID3D11ShaderResourceView*& target = Get_TextureSlot(_eSlot);

    Safe_Release(target);
    target = _SRV;

    if (target)
        target->AddRef();

    /* Save Texture Path */
    switch (_eSlot)
    {
    case TEXTURE_SLOT::MAIN:
        m_tTextures.MainTexture = _Path;
        break;

    case TEXTURE_SLOT::NOISE_0:
        m_tTextures.Noise_0 = _Path;
        break;

    case TEXTURE_SLOT::NOISE_1:
        m_tTextures.Noise_1 = _Path;
        break;

    case TEXTURE_SLOT::MASK:
        m_tTextures.Mask = _Path;
        break;

    case TEXTURE_SLOT::DISTORTION:
        m_tTextures.Distortion = _Path;
        break;

    case TEXTURE_SLOT::DISSOLVE:
        m_tTextures.Dissolve = _Path;
        break;
    }
}

ID3D11ShaderResourceView* CShaderPreview::Get_Texture(TEXTURE_SLOT _eSlot)
{
    /* For. Only Read */
    return Get_TextureSlot(_eSlot);
}

ID3D11ShaderResourceView*& CShaderPreview::Get_TextureSlot(TEXTURE_SLOT _eSlot)
{
    switch (_eSlot)
    {
    case TEXTURE_SLOT::MAIN:        return m_pMainTexture;
    case TEXTURE_SLOT::NOISE_0:     return m_pNoiseTexture_0;
    case TEXTURE_SLOT::NOISE_1:     return m_pNoiseTexture_1;
    case TEXTURE_SLOT::MASK:        return m_pMaskTexture;
    case TEXTURE_SLOT::DISTORTION:  return m_pDistortionTexture;
    case TEXTURE_SLOT::DISSOLVE:    return m_pDissolveTexture;
    }

    return m_pMainTexture;
}

HRESULT CShaderPreview::Change_Model(_wstring _ModelTag)
{
    if (m_pModelCom)
    {
        Remove_Component(TEXT("Com_Model"));
        Safe_Release(m_pModelCom);
        m_pModelCom = nullptr;
    }

    const wstring prefix = TEXT("Prototype_Component_Model_Map_");

    if (_ModelTag.find(prefix) == 0)
    {
        m_tShaderDesc.EffectName = _ModelTag.substr(prefix.length());
    }
    else
    {
        m_tShaderDesc.EffectName = _ModelTag;
    }

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _ModelTag,
        TEXT("Com_Model"),
        reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;
    
    return S_OK;
}

void CShaderPreview::Apply_Effect(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Texture)
{
    m_tShaderDesc = _Param;
    m_tTimeline = _Timeline;
    m_tTextures = _Texture;

    _wstring modelTag = L"Prototype_Component_Model_Map_" + _Param.EffectName;
    Change_Model(modelTag);

    Load_Texture_FromPath(TEXTURE_SLOT::MAIN, _Texture.MainTexture);
    Load_Texture_FromPath(TEXTURE_SLOT::NOISE_0, _Texture.Noise_0);
    Load_Texture_FromPath(TEXTURE_SLOT::NOISE_1, _Texture.Noise_1);
    Load_Texture_FromPath(TEXTURE_SLOT::MASK, _Texture.Mask);
    Load_Texture_FromPath(TEXTURE_SLOT::DISTORTION, _Texture.Distortion);
    Load_Texture_FromPath(TEXTURE_SLOT::DISSOLVE, _Texture.Dissolve);
}

HRESULT CShaderPreview::Ready_Components(OBJECT_DESC* _pTags)
{
    /* Com_Shader */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Shader_VtxMeshEffect"),
        TEXT("Com_Shader"),
        reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* Com_Model */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Model_Map_Grass"),
        TEXT("Com_Model"),
        reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CShaderPreview::Render_Binding()
{
    /* Texture */
    if (m_pMainTexture)
        m_pShaderCom->Bind_ShaderResourceView("g_MainTexture", m_pMainTexture);
    if (m_pNoiseTexture_0)
        m_pShaderCom->Bind_ShaderResourceView("g_NoiseTexture_0", m_pNoiseTexture_0);
    if (m_pNoiseTexture_1)
        m_pShaderCom->Bind_ShaderResourceView("g_NoiseTexture_1", m_pNoiseTexture_1);
    if (m_pMaskTexture)
        m_pShaderCom->Bind_ShaderResourceView("g_MaskTexture", m_pMaskTexture);
    if (m_pDistortionTexture)
        m_pShaderCom->Bind_ShaderResourceView("g_DistortionTexture", m_pDistortionTexture);
    if (m_pDissolveTexture)
        m_pShaderCom->Bind_ShaderResourceView("g_DissolveTexture", m_pDissolveTexture);

    /* Raw Value */
    /* Color */
    m_pShaderCom->Bind_RawValue("g_vColor", &m_tShaderDesc.vColor, sizeof(_float4));
    
    /* Object Whole Alpha */
    m_pShaderCom->Bind_RawValue("g_fAlpha", &m_tShaderDesc.fAlpha, sizeof(_float));
    
    /* UV */
    m_pShaderCom->Bind_RawValue("g_vUVScale", &m_tShaderDesc.vUVScale, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_vUVSpeed", &m_tShaderDesc.vUVSpeed, sizeof(_float2));
    
    /* Noise_0 */
    m_pShaderCom->Bind_RawValue("g_vNoiseScale_0", &m_tShaderDesc.vNoiseScale_0, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_vNoiseSpeed_0", &m_tShaderDesc.vNoiseSpeed_0, sizeof(_float2));
    
    /* Noise_1*/
    m_pShaderCom->Bind_RawValue("g_vNoiseScale_1", &m_tShaderDesc.vNoiseScale_1, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_vNoiseSpeed_1", &m_tShaderDesc.vNoiseSpeed_1, sizeof(_float2));
    
    /* Distortion */
    m_pShaderCom->Bind_RawValue("g_fDistortion", &m_tShaderDesc.fDistortion, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_vDistortionDir", &m_tShaderDesc.vDistortionDir, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_fDistortionSpeed", &m_tShaderDesc.fDistortionSpeed, sizeof(_float));
    
    /* Dissolve */
    m_pShaderCom->Bind_RawValue("g_fDissolve", &m_tShaderDesc.fDissolve, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_fDissolveEdge", &m_tShaderDesc.fDissolveEdge, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_vDissolveColor", &m_tShaderDesc.vDissolveColor, sizeof(_float4));
    
    /* Emissive */
    m_pShaderCom->Bind_RawValue("g_fEmissive", &m_tShaderDesc.fEmissive, sizeof(_float));
    
    /* Flipbook */
    // m_pShaderCom->Bind_RawValue("g_iFrameX", &m_tShaderDesc.iFrameX, sizeof(_int));
    // m_pShaderCom->Bind_RawValue("g_iFrameY", &m_tShaderDesc.iFrameY, sizeof(_int));
    // m_pShaderCom->Bind_RawValue("g_fFrameSpeed", &m_tShaderDesc.fFrameSpeed, sizeof(_float));
    
    /* Time */
    m_pShaderCom->Bind_RawValue("g_Time", &m_tShaderDesc.fTime, sizeof(_float));

    return S_OK;
}

HRESULT CShaderPreview::Load_Texture_FromPath(TEXTURE_SLOT slot, const std::wstring& path)
{
    if (path.empty())
        return E_FAIL;

    ID3D11ShaderResourceView* pSRV = nullptr;

    std::wstring ext = path.substr(path.find_last_of(L'.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    HRESULT hr;

    if (ext == L"dds")
    {
        hr = DirectX::CreateDDSTextureFromFile(
            m_pDevice,
            path.c_str(),
            nullptr,
            &pSRV);
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(
            m_pDevice,
            m_pDeviceContext,
            path.c_str(),
            nullptr,
            &pSRV);
    }

    if (FAILED(hr))
        return E_FAIL;

    Set_Texture(slot, pSRV, path);

    Safe_Release(pSRV);

    return S_OK;
}

CShaderPreview* CShaderPreview::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CShaderPreview* pInstance = new CShaderPreview(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CShaderPreview");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CShaderPreview::Clone(void* _pArg)
{
    CShaderPreview* pInstance = new CShaderPreview(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CShaderPreview");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CShaderPreview::Free()
{
    __super::Free();

    Safe_Release(m_pMainTexture);
    Safe_Release(m_pNoiseTexture_0);
    Safe_Release(m_pNoiseTexture_1);
    Safe_Release(m_pMaskTexture);
    Safe_Release(m_pDistortionTexture);
    Safe_Release(m_pDissolveTexture);

    Safe_Release(m_pShaderCom);

    if (m_pModelCom)
        Safe_Release(m_pModelCom);
}
