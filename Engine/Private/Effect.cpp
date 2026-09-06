#include "Effect.h"
#include "GameInstance.h"

CEffect::CEffect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}
CEffect::CEffect(const CEffect& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect::Initialize(void* _pArg)
{
    /* 1. Map Object */
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    /* 2. MAP_INIT_DESC */
    EFFECT_INIT_DESC* pInitDesc = static_cast<EFFECT_INIT_DESC*>(_pArg);
    if (FAILED(Ready_Components(pInitDesc)))
        return E_FAIL;

    return S_OK;
}

void CEffect::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;
}

void CEffect::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pModelCom)
        m_pModelCom->Play_Animation(_fTimeDelta);

    m_tShaderDesc.fTime += _fTimeDelta;

    /* Timeline */
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

    if (m_tTime.fAccTime >= m_tTime.fAccDurationTime)
    {
        if (!m_bDelete)
        {
             //m_pGameInstance->Reserve_DeleteObject(this);
            m_bActive = false;
            m_bDelete = true;
            return;
        }
    }
}

void CEffect::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONLIGHT, this)))
        return;
}

HRESULT CEffect::Render()
{
    if (!m_bActive)
        return S_OK;

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
            /* Camera */
            if (FAILED(m_pGameInstance->Bind_CameraPosition(m_pShaderCom, "g_vCamPosition")))
                return E_FAIL;
            /* Bind */
            if (FAILED(Render_Binding()))
                return E_FAIL;
            /* Bone */
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
            /* Shader */
            if (FAILED(m_pShaderCom->Begin(m_iEffectPassIndex)))
                return E_FAIL;
            /* Mesh */
            if (FAILED(m_pModelCom->Render(i)))
                return E_FAIL;
        }
    }

    return S_OK;
}

void CEffect::Set_Dir(_vector _vDir)
{
    XMStoreFloat4(&m_tShaderDesc.vDirection, _vDir);
}

void CEffect::Set_TextureSlots(_uint main, _uint noise0, _uint noise1, _uint mask, _uint distort, _uint dissolve)
{
    m_iMainTexSlot = main;
    m_iNoise0Slot = noise0;
    m_iNoise1Slot = noise1;
    m_iMaskSlot = mask;
    m_iDistortionSlot = distort;
    m_iDissolveSlot = dissolve;
}

void CEffect::Set_EffectPassIndex(EFFECT_TYPE _eEffectType)
{
    m_iEffectPassIndex = ENUM_TO_UINT(_eEffectType);
}

HRESULT CEffect::Ready_Components(EFFECT_INIT_DESC* _pTags)
{
    EFFECT_INIT_DESC* pInitDesc = static_cast<EFFECT_INIT_DESC*>(_pTags);

    /* Com_Shader */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        // TEXT("Prototype_Component_Shader_VtxMeshEffect"),
        _pTags->tObjectDesc.ShaderTag,
        TEXT("Com_Shader"),
        reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* Com_Model */
    if (pInitDesc->tObjectDesc.ModelTag[0] != 0)
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tObjectDesc.ModelTag,
            TEXT("Com_Model"),
            reinterpret_cast<CComponent**>(&m_pModelCom))))
            return E_FAIL;
    }

    /* Com_VIBuffer */
    if (pInitDesc->tObjectDesc.VIBufferTag[0] != 0)
    {
        /* Com_VIBuffer */
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            TEXT("Prototype_Component_VIBuffer_Plane"),
            TEXT("Com_VIBuffer"),
            reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
            return E_FAIL;
    }

    /* Com_MainTexture */
    if (!pInitDesc->tEffectDesc.MainTextureTag.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.MainTextureTag,
            TEXT("Com_MainTexture"),
            reinterpret_cast<CComponent**>(&m_pMainTexture))))
            return E_FAIL;
    }

    /* Com_NoiseTexture_0 */
    if (!pInitDesc->tEffectDesc.NoiseTag_0.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.NoiseTag_0,
            TEXT("Com_NoiseTexture_0"),
            reinterpret_cast<CComponent**>(&m_pNoiseTexture_0))))
            return E_FAIL;
    }

    /* Com_NoiseTexture_1 */
    if (!pInitDesc->tEffectDesc.NoiseTag_1.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.NoiseTag_1,
            TEXT("Com_NoiseTexture_1"),
            reinterpret_cast<CComponent**>(&m_pNoiseTexture_1))))
            return E_FAIL;
    }

    /* Com_MaskTexture */
    if (!pInitDesc->tEffectDesc.MaskTag.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.MaskTag,
            TEXT("Com_MaskTexture"),
            reinterpret_cast<CComponent**>(&m_pMaskTexture))))
            return E_FAIL;
    }

    /* Com_DistortionTexture */
    if (!pInitDesc->tEffectDesc.DistortionTag.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.DistortionTag,
            TEXT("Com_DistortionTexture"),
            reinterpret_cast<CComponent**>(&m_pDistortionTexture))))
            return E_FAIL;
    }

    /* Com_DissolveTexture */
    if (!pInitDesc->tEffectDesc.DissolveTag.empty())
    {
        if (FAILED(Add_Component(
            RESOURCE_LEVEL_STATIC,
            pInitDesc->tEffectDesc.DissolveTag,
            TEXT("Com_DissolveTexture"),
            reinterpret_cast<CComponent**>(&m_pDissolveTexture))))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect::Render_Binding()
{
    /* Texture */
    if (m_pMainTexture)
        m_pMainTexture->Bind_ShaderResourceView(m_pShaderCom, "g_MainTexture", m_iMainTexSlot);
    if (m_pNoiseTexture_0)
        m_pNoiseTexture_0->Bind_ShaderResourceView(m_pShaderCom, "g_NoiseTexture_0", m_iNoise0Slot);
    if (m_pNoiseTexture_1)
        m_pNoiseTexture_1->Bind_ShaderResourceView(m_pShaderCom, "g_NoiseTexture_1", m_iNoise1Slot);
    if (m_pMaskTexture)
        m_pMaskTexture->Bind_ShaderResourceView(m_pShaderCom, "g_MaskTexture", m_iMaskSlot);
    if (m_pDistortionTexture)
        m_pDistortionTexture->Bind_ShaderResourceView(m_pShaderCom, "g_DistortionTexture", m_iDistortionSlot);
    if (m_pDissolveTexture)
        m_pDissolveTexture->Bind_ShaderResourceView(m_pShaderCom, "g_DissolveTexture", m_iDissolveSlot);

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
    m_pShaderCom->Bind_RawValue("g_iFrameX", &m_tShaderDesc.iFrameX, sizeof(_int));
    m_pShaderCom->Bind_RawValue("g_iFrameY", &m_tShaderDesc.iFrameY, sizeof(_int));
    m_pShaderCom->Bind_RawValue("g_fFrameSpeed", &m_tShaderDesc.fFrameSpeed, sizeof(_float));

    /* Time */
    m_pShaderCom->Bind_RawValue("g_Time", &m_tShaderDesc.fTime, sizeof(_float));

    /* RandValue(CPU) */
    m_pShaderCom->Bind_RawValue("g_fRandom", &m_fRandValue, sizeof(_float));
    
    /* Size */
    m_pShaderCom->Bind_RawValue("g_fSize", &m_fVSScale, sizeof(_float));

    return S_OK;
}

CEffect* CEffect::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CEffect* pInstance = new CEffect(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CEffect::Clone(void* _pArg)
{
    CEffect* pInstance = new CEffect(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CEffect::Free()
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

    if (m_pVIBufferCom)
        Safe_Release(m_pVIBufferCom);
}
