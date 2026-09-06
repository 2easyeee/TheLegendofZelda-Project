#include "PawnObject.h"
#include "GameInstance.h"

CPawnObject::CPawnObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject { _pDevice, _pDeviceContext }
{
}

CPawnObject::CPawnObject(const CPawnObject& _Prototype)
    : CWorldObject (_Prototype)
{
}

HRESULT CPawnObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPawnObject::Initialize(void* _pArg)
{
    ACTOR_INIT_DESC* pInitDesc = static_cast<ACTOR_INIT_DESC*>(_pArg);

    /* 0. CGameObject ÀÇ Init */
    if (FAILED(CGameObject::Initialize(&pInitDesc->tObjectDesc)))
        return E_FAIL;
    if (FAILED(m_pTransformCom->Initialize(nullptr)))
        return E_FAIL;

    /* 1. WorldObject */
    if (FAILED(__super::Initialize(&pInitDesc->tWorldDesc)))
        return E_FAIL;

    /* 2. Component */
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    /* 2. ActorObject */

    return S_OK;
}

void CPawnObject::Priority_Update(_float _fTimeDelta)
{
}

void CPawnObject::Update(_float _fTimeDelta)
{
    /* EFFECT (Shader) */
    Update_ShaderEffect(_fTimeDelta);

    /* EFFECT */
    if (m_tKnockBack.bActive)
    {
        MoveWithKnockBack(_fTimeDelta);

        /* 2. State */
        Change_State();
    }
    else
    {
        /* 1. FSM */
        if (m_pState)
            m_pState->OnStateStay(this, _fTimeDelta);

        /* 2. State */
        Change_State();
    }

    /* 3. Animation */
    if (m_pModelCom)
        m_pModelCom->Play_Animation(_fTimeDelta);
}

void CPawnObject::Late_Update(_float _fTimeDelta)
{
}

HRESULT CPawnObject::Render()
{
    /* Transform */
    if (FAILED(m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
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

HRESULT CPawnObject::Render_Shadow()
{
    /* Transform */
    if (FAILED(m_pTransformCom->Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
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

void CPawnObject::Register_State(IPawnState* _pState)
{
    if (!_pState)
        return;
    _uint ID = _pState->Get_StateID();
    m_StateMap.emplace(ID, _pState);

    if (!m_pState)
        m_pState = _pState;
}

void CPawnObject::RequestToChangeState(_uint _StateID)
{
    m_NexState = _StateID;
    m_bRequestStateChange = true;
}

void CPawnObject::Change_State()
{
    if (!m_bRequestStateChange)
        return;

    auto iter = m_StateMap.find(m_NexState);
    if (iter == m_StateMap.end())
        return;

    if (m_pState)
        m_pState->OnStateExit(this);

    m_pState = iter->second;
    m_pState->OnStateEnter(this);

    m_bRequestStateChange = false;
}

void CPawnObject::Set_SpeedMulti(_float _fMulti)
{
    if (m_pTransformCom)
        m_pTransformCom->Set_SpeedMulti(_fMulti);
}

_bool CPawnObject::MoveWithNav(_vector _vMoveDir, _float _fTimeDelta)
{
    /* Nav + KnockBack + Block + Slide */

    if (!m_pTransformCom)
        return false;

    _vector vPrevPos = m_pTransformCom->Get_State(STATE::POSITION);

    /* 1. KnockBack */
    if (m_tKnockBack.bActive)
    {
        _vector vKnockBackDir = XMVector3Normalize(m_vKnockBackDir);
        _vector vTargetPos = vPrevPos + (vKnockBackDir * m_pTransformCom->Get_MoveSpeed() * _fTimeDelta);

        if (m_pNavCom && !m_pNavCom->IsMove(vTargetPos))
        {
            m_tKnockBack.bActive = false;
            return false;
        }

        m_pTransformCom->Set_State(STATE::POSITION, vTargetPos);
        //m_pTransformCom->LookAt(vTargetPos - vKnockBackDir);
        
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        return true;
    }

    /* 2. No Input */
    if (XMVector3Equal(_vMoveDir, XMVectorZero()))
        return false;

    _vector vMoveDir = XMVectorSetY(_vMoveDir, 0.f);
    vMoveDir = XMVector3Normalize(vMoveDir);

    _vector vTargetPos = vPrevPos + (vMoveDir * m_pTransformCom->Get_MoveSpeed() * _fTimeDelta);

    /* 3. Navigation */
    if (m_pNavCom && !m_pNavCom->IsMove(vTargetPos))
        return false;

    /* 4. Move */
    m_pTransformCom->Set_State(STATE::POSITION, vTargetPos);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 5. Block */
    _vector vNormal = XMVectorZero();
    _float fDepth = 0.f;
    if (!m_pGameInstance->Check_Block(m_pBodyCollider, &vNormal, &fDepth))
    {
        m_pTransformCom->LookAt(vTargetPos - vMoveDir);
        return true;
    }

    /* 6. Collider + Slide */
    m_pTransformCom->Set_State(STATE::POSITION, vPrevPos);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    _vector vMove = vTargetPos - vPrevPos;
    vMove = XMVectorSetY(vMove, 0.f);

    vNormal = XMVectorSetY(vNormal, 0.f);
    vNormal = XMVector3Normalize(vNormal);

    _float fDot = XMVectorGetX(XMVector3Dot(vMove, vNormal));
    _vector vSlide = vMove - vNormal * fDot;

    if (XMVector3LengthSq(vSlide).m128_f32[0] < 0.000001f)
        return false;

    _vector vSlidePos = vPrevPos + vSlide;

    if (m_pNavCom && !m_pNavCom->IsMove(vSlidePos))
        return false;

    m_pTransformCom->Set_State(STATE::POSITION, vSlidePos);
    m_pTransformCom->LookAt(vSlidePos - XMVector3Normalize(vSlide));
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 5. Stuck */
    if (fDepth > 0.0001f)
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        vPos += vNormal * (fDepth * 0.5f);
        m_pTransformCom->Set_State(STATE::POSITION, vPos);
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }

    return true;
}

_bool CPawnObject::ReflectWithNav(_vector& _vMoveDir, _float _fTimeDelta)
{
    if (!m_pTransformCom)
        return false;

    _vMoveDir = XMVectorSetY(_vMoveDir, 0.f);
    _vMoveDir = XMVector3Normalize(_vMoveDir);

    _vector vPrevPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vTargetPos = vPrevPos + (_vMoveDir * m_pTransformCom->Get_MoveSpeed() * _fTimeDelta);

    /* 1. Move */
    m_pTransformCom->Set_State(STATE::POSITION, vTargetPos);
    m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* 2. Check Block Wall */
    _vector vNormal = XMVectorZero();
    _float fDepth = 0.1f;
    int reflectCount = 0;

    while (m_pGameInstance->Check_Block(m_pBodyCollider, &vNormal, &fDepth) && reflectCount < 3)
    {
        /* Return Position */
        m_pTransformCom->Set_State(STATE::POSITION, vPrevPos);
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        /* Force reflect */
        _matrix RotMatrix = XMMatrixRotationY(XMConvertToRadians(45.f));
        _vMoveDir = XMVector3TransformNormal(_vMoveDir, RotMatrix);
        _vMoveDir = XMVector3Normalize(_vMoveDir);

        /* Retry */
        _vector vReflectPos = vPrevPos + _vMoveDir * m_pTransformCom->Get_MoveSpeed() * _fTimeDelta;

        m_pTransformCom->Set_State(STATE::POSITION, vReflectPos);
        m_pBodyCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

        return true;
    }

    /* 3. Dir */
    m_pTransformCom->LookAt(vTargetPos + _vMoveDir);

    return false;
}


void CPawnObject::Set_Animation(_string _AnimName, _bool _bLoop, _bool _isForce)
{
    if (!m_pModelCom)
        return;

    if (FAILED(m_pModelCom->Set_Animation_Name(_AnimName, _bLoop, _isForce)))
        return;
}

void CPawnObject::Reset_Animation_TrackPosition()
{
    if (m_pModelCom)
        m_pModelCom->Reset_Animation_TrackPosition();
}

void CPawnObject::Set_AnimationSpeedMulti(_float _fAnimSpeedMulti)
{
    if (m_pModelCom)
        m_pModelCom->Set_AnimationSpeedMulti(_fAnimSpeedMulti);
}

_bool CPawnObject::IsAnimFinished() const
{
    if (m_pModelCom)
        return m_pModelCom->IsAnimFinished();

    return false;
}

_float CPawnObject::Get_PlayRatio() const
{
    if (m_pModelCom)
        return m_pModelCom->Get_PlayRatio();

    return 0.f;
}

void CPawnObject::Set_BoneOffset(const _char* _BoneName, _fmatrix _offset)
{
    if (m_pModelCom)
        m_pModelCom->Set_BoneOffset(_BoneName, _offset);
}

void CPawnObject::Clear_BoneOffset()
{
    if (m_pModelCom)
        m_pModelCom->Clear_BoneOffset();
}

void CPawnObject::Set_BoneSnap(const _char* _TargetBoneName, const _char* _SourceBoneName)
{
    if (m_pModelCom)
        m_pModelCom->Set_BoneSnap(_TargetBoneName, _SourceBoneName);
}

void CPawnObject::Clear_BoneSnap()
{
    if (m_pModelCom)
        m_pModelCom->Clear_BoneSnap();
}

HRESULT CPawnObject::Ready_Components(OBJECT_DESC* _Desc)
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

void CPawnObject::Start_KnockBack(_vector _vHitPos, _float _fPower, _float _fDuration)
{
    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDir = vMyPos - _vHitPos;
    vDir = XMVectorSetY(vDir, 0.f);
    vDir = XMVector3Normalize(vDir);

    m_vKnockBackDir = vDir;
    m_fKnockBackSpeed = _fPower;
    m_tKnockBack.bActive = true;
    m_tKnockBack.fAccTime = 0.f;
    m_tKnockBack.fAccDurationTime = _fDuration;
}

void CPawnObject::MoveWithKnockBack(_float _fTimeDelta)
{
    if (!m_tKnockBack.bActive)
        return;
    m_tKnockBack.fAccTime += _fTimeDelta;

    _float t = m_tKnockBack.fAccTime / m_tKnockBack.fAccDurationTime;
    _float fRatio = EaseOutCubic(t);

    _float endMulti = 0.2f;
    _float curMulti = m_fKnockBackSpeed * (1.f - fRatio) + endMulti * fRatio;
    m_pTransformCom->Set_SpeedMulti(curMulti);

    /* Åö */
    if (m_tKnockBack.fAccTime < 0.06f)
        m_pTransformCom->Set_SpeedMulti(curMulti * 1.6f);

    MoveWithNav(m_vKnockBackDir, _fTimeDelta);

    if (t >= 1.f)
    {
        m_tKnockBack.bActive = false;
        m_pTransformCom->Set_SpeedMulti(1.f);
    }
}

void CPawnObject::Start_HitFlash()
{
    m_tHitTime.bActive = true;
    m_tHitTime.fAccTime = 0.f;

    /* SFX */
    m_pGameInstance->Play_Sound(L"ENEMY_Common_Damage.wav", SOUND::EFFECT);
}

void CPawnObject::Start_Dissolve()
{
    m_tDissovleTime.bActive = true;
    m_tDissovleTime.fAccTime = 0.f;
}

void CPawnObject::Start_DeleteVFX()
{
    /* Effect Pos */
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f);

    /* Star */
    _vector basePos = vPos;
    _int iCount = 8;

    for (_int i = 0; i < iCount; ++i)
    {
        /* Effect */
        CEffectManager::EFFECT_EVENT tEvent{};
        tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
        tEvent.EffectName = TEXT("Star");

        _float fAngle = XM_2PI * ((rand() % 1000) / 1000.f);
        _float fX = cosf(fAngle);
        _float fZ = sinf(fAngle);
        _float fY = ((rand() % 100) / 100.f) * 0.5f;

        _vector vDir = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));

        _float foffsetScale = 0.1f + ((rand() % 100) / 100.f) * 0.3f;
        _vector vOffset = vDir * foffsetScale;

        tEvent.vPosition = basePos + vOffset;
        tEvent.vDirection = vDir;

        m_pGameInstance->Push_EffectEVENT(tEvent);
    }

    /* Purple_Smoke */
    CEffectManager::EFFECT_EVENT tEvent{};
    tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
    tEvent.EffectName = TEXT("Plane");
    tEvent.vPosition = vPos;
    m_pGameInstance->Push_EffectEVENT(tEvent);

    /* SFX */
    m_pGameInstance->Play_Sound(L"ENEMY_Common_Delete_0.wav", SOUND::EFFECT, 3.5f);
}

void CPawnObject::Update_ShaderEffect(_float _fTimeDelta)
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

        _float fRatio = m_tDissovleTime.fAccTime / m_tDissovleTime.fAccDurationTime;
        if (!m_bDeleteVFX && fRatio > 0.2f)
        {
            Start_DeleteVFX();
            m_bDeleteVFX = true;
        }

        if (m_tDissovleTime.fAccTime >= m_tDissovleTime.fAccDurationTime)
        {
            m_tDissovleTime.fAccTime = m_tDissovleTime.fAccDurationTime;
        }
    }
}

HRESULT CPawnObject::Render_Binding()
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

void CPawnObject::Free()
{
    __super::Free();

    for (auto& Pair : m_StateMap)
        Safe_Delete(Pair.second);
    m_StateMap.clear();

    m_pState = nullptr;

    Safe_Release(m_pNavCom);
    Safe_Release(m_pBodyCollider);

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
