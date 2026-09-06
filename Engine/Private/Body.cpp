#include "Body.h"
#include "GameInstance.h"

CBody::CBody(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CPartObject{ _pDevice, _pDeviceContext }
{
}

CBody::CBody(const CBody& _Prototype)
    : CPartObject(_Prototype)
{
}

HRESULT CBody::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBody::Initialize(void* _pArg)
{
    /* 1. PartOBject */
    PART_CREATE_DESC* tInitDesc = static_cast<PART_CREATE_DESC*>(_pArg);
    if (FAILED(CPartObject::Initialize(tInitDesc)))
        return E_FAIL;

    if (&tInitDesc->tObjectDesc)
    {
        /* 2. Component */
        if (FAILED(Ready_Components(&tInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    return S_OK;
}

void CBody::Priority_Update(_float _fTimeDelta)
{
}

void CBody::Update(_float _fTimeDelta)
{
    /* EFFECT (Shader) */
    Update_ShaderEffect(_fTimeDelta);

    CPartObject::Update_WorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 3. Animation */
    if (m_pModelCom)
        m_pModelCom->Play_Animation(_fTimeDelta);
}

void CBody::Late_Update(_float _fTimeDelta)
{
    CPartObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this)))
        return;
}

HRESULT CBody::Render()
{
    /* Transform */
    if (FAILED(CPartObject::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Model */
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        /* Material */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
            return E_FAIL;
        /* Material (Normal) */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MATERIAL::NORMALS)))
            return E_FAIL;
        /* Material (AO) */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_AOTexture", i, MATERIAL::AO)))
            return E_FAIL;
        /* Material (MTL) */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MTLTexture", i, MATERIAL::METALLIC)))
            return E_FAIL;
        /* Material (SMT)*/
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_SMTTexture", i, MATERIAL::ROUGHNESS)))
            return E_FAIL;
        /* Bone */
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        /* Bind */
        if (FAILED(Render_Binding()))
            return E_FAIL;
        /* Shader */
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        /* Mesh */
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBody::Render_Shadow()
{
    /* Transform */
    if (FAILED(CPartObject::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    /* Perspective */
    if (FAILED(m_pGameInstance->Bind_ShadowTransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_ShadowTransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    /* Model */
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        /* Material */
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
            return E_FAIL;
        /* Bone */
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        /* Shader */
        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;
        /* Mesh */
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

const _float4x4* CBody::Get_SocketMatrixPtr(const _char* _pBoneName)
{
    return m_pModelCom->Get_BoneMatrixPtr(_pBoneName);
}

void CBody::Set_Animation(_string _AnimName, _bool _bLoop, _bool _isForce)
{
    if (!m_pModelCom)
        return;

    if (FAILED(m_pModelCom->Set_Animation_Name(_AnimName, _bLoop, _isForce)))
        return;
}

void CBody::Reset_Animation_TrackPosition()
{
    if (m_pModelCom)
        m_pModelCom->Reset_Animation_TrackPosition();
}

void CBody::Set_AnimationSpeedMulti(_float _fAnimSpeedMulti)
{
    if (m_pModelCom)
        m_pModelCom->Set_AnimationSpeedMulti(_fAnimSpeedMulti);
}

_bool CBody::IsAnimFinished() const
{
    if (m_pModelCom)
        return m_pModelCom->IsAnimFinished();

    return false;
}

_float CBody::Get_PlayRatio() const
{
    if (m_pModelCom)
        return m_pModelCom->Get_PlayRatio();

    return 0.f;
}

HRESULT CBody::Ready_Components(OBJECT_DESC* _Desc)
{
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _Desc->ShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(
        _Desc->iLevel,
        _Desc->ModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    /* Shader Effect */
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Texture_Dissolve"),
        TEXT("Com_DissolveTexture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
        return E_FAIL;

    return S_OK;
}

void CBody::Start_HitFlash()
{
    m_tHitTime.bActive = true;
    m_tHitTime.fAccTime = 0.f;
}

void CBody::Start_Dissolve()
{
    m_tDissovleTime.bActive = true;
    m_tDissovleTime.fAccTime = 0.f;
}

void CBody::Update_ShaderEffect(_float _fTimeDelta)
{
    /* Hit Flash */
    if (m_tHitTime.bActive)
    {
        m_tHitTime.fAccTime += _fTimeDelta;

        if (m_tHitTime.fAccTime >= m_tHitTime.fAccDurationTime)
        {
            m_tHitTime.bActive = false;
            m_fHitStrength = 0.f;
        }
        else
        {
            m_fHitStrength = 1.f;
        }
    }


    /* Dissovle */
    if (m_tDissovleTime.bActive)
    {
        m_tDissovleTime.fAccTime += _fTimeDelta;
        if (m_tDissovleTime.fAccTime >= m_tDissovleTime.fAccDurationTime)
        {
            m_tDissovleTime.fAccTime = m_tDissovleTime.fAccDurationTime;
        }
    }
}

HRESULT CBody::Render_Binding()
{
    /* Hit Effect */
    _float3 vHitColor = { 1.f, 0.f, 0.f };
    m_pShaderCom->Bind_RawValue("g_vHitColor", &vHitColor, sizeof(_float3));

    _float fHitStrength = m_fHitStrength;
    m_pShaderCom->Bind_RawValue("g_fHitStrength", &fHitStrength, sizeof(_float));

    /* Dissolve */
    if (m_pTextureCom)
        m_pTextureCom->Bind_ShaderResourceView(m_pShaderCom, "g_DissolveTexture", 0);

    _float fDissolve = m_tDissovleTime.fAccTime / m_tDissovleTime.fAccDurationTime;
    m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float));

    return S_OK;
}

void CBody::Start_DeleteVFX()
{
    ///* Effect Pos */
    //_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    //vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f);

    ///* Star */
    //_vector basePos = vPos;
    //_int iCount = 8;

    //for (_int i = 0; i < iCount; ++i)
    //{
    //    /* Effect */
    //    CEffectManager::EFFECT_EVENT tEvent{};
    //    tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
    //    tEvent.EffectName = TEXT("Star");

    //    _float fAngle = XM_2PI * ((rand() % 1000) / 1000.f);
    //    _float fX = cosf(fAngle);
    //    _float fZ = sinf(fAngle);
    //    _float fY = ((rand() % 100) / 100.f) * 0.5f;

    //    _vector vDir = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));

    //    _float foffsetScale = 0.1f + ((rand() % 100) / 100.f) * 0.3f;
    //    _vector vOffset = vDir * foffsetScale;

    //    tEvent.vPosition = basePos + vOffset;
    //    tEvent.vDirection = vDir;

    //    m_pGameInstance->Push_EffectEVENT(tEvent);
    //}

    ///* Purple_Smoke */
    //CEffectManager::EFFECT_EVENT tEvent{};
    //tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
    //tEvent.EffectName = TEXT("Plane");
    //tEvent.vPosition = vPos;
    //m_pGameInstance->Push_EffectEVENT(tEvent);

    ///* SFX */
    //m_pGameInstance->Play_Sound(L"ENEMY_Common_Delete_0.wav", SOUND::EFFECT, 3.5f);
}

void CBody::Free()
{
    __super::Free();

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
