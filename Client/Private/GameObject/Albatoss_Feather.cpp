#include "GameObject/Albatoss_Feather.h"
#include "GameInstance.h"

CAlbatoss_Feather::CAlbatoss_Feather(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject{ _pDevice, _pDeviceContext }
{
}

CAlbatoss_Feather::CAlbatoss_Feather(const CAlbatoss_Feather& _Prototype)
    : CWorldObject(_Prototype)
{
}

HRESULT CAlbatoss_Feather::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAlbatoss_Feather::Initialize(void* _pArg)
{
    FEATHER_INIT_DESC* tInitDesc = static_cast<FEATHER_INIT_DESC*>(_pArg);

    /* 2. CGameObject */
    if (&tInitDesc->tObjectDesc)
    {
        if (FAILED(CGameObject::Initialize(&tInitDesc->tObjectDesc)))
            return E_FAIL;
        if (FAILED(m_pTransformCom->Initialize(nullptr)))
            return E_FAIL;
    }

    if (&tInitDesc->tObjectDesc)
    {
        /* 2. Component */
        if (FAILED(Ready_Components(&tInitDesc->tObjectDesc)))
            return E_FAIL;
    }

    if (&tInitDesc->tFeatherDesc)
    {
        _matrix WorldMatrix = XMLoadFloat4x4(&tInitDesc->tFeatherDesc.WorldMatrix);
        for (_int i = 0; i < 3; ++i)
        {
            WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
        }
        m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
        m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
        m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
    }

    /* Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 45.f);

    return S_OK;
}

void CAlbatoss_Feather::Priority_Update(_float _fTimeDelta)
{
    CWorldObject::Priority_Update(_fTimeDelta);
}

void CAlbatoss_Feather::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    /* Shoot Feather */
    if (m_tStickIntoTime.bActive)
    {
        m_tStickIntoTime.fAccTime += _fTimeDelta;
        if (m_tStickIntoTime.fAccTime >= m_tStickIntoTime.fAccDurationTime)
        {
            Destroy();
            return;
        }
    }
    else
    {
        Update_Shoot(_fTimeDelta);
    }

    /* Collider */
    if (m_pColliderCom)
        m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* WorldObject */
    CWorldObject::Update(_fTimeDelta);
}

void CAlbatoss_Feather::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CWorldObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CAlbatoss_Feather::Render()
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
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        /* Mesh */
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

#ifdef _DEBUG
    /* Collider */
    //if (m_pColliderCom) m_pColliderCom->Render();
#endif // _DEBUG

    return S_OK;
}

void CAlbatoss_Feather::Shoot_Target(_vector _vTargetPos)
{
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDir = _vTargetPos - vPos;
    vDir = XMVectorSetZ(vDir, 0.f);
    vDir = XMVector3Normalize(vDir);

    m_vDir = vDir;
    m_bShoot = true;
}

void CAlbatoss_Feather::Update_WorldMatrix(_float4x4 _WorldMatrix)
{
    _matrix WorldMatrix = XMLoadFloat4x4(&_WorldMatrix);
    for (_int i = 0; i < 3; ++i)
    {
        WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
    }
    m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
    m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
    m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
}

void CAlbatoss_Feather::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::WEAPON_MONSTER)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER ||
            _Dst->Get_Group() == GROUP::WEAPON_PLAYER ||
            _Dst->Get_Group() == GROUP::SHIELD_PLAYER)
        {
            Destroy();

            /* SFX */
            m_pGameInstance->GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_FeatherY.wav", SOUND::EFFECT);
        }

        if (_Dst->Get_Group() == GROUP::MAP)
        {
            _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
            vPos = XMVectorSetY(vPos, 0.5f);
            m_pTransformCom->Set_State(STATE::POSITION, vPos);

            m_bShoot = false;
            m_tStickIntoTime.bActive = true;
            m_tStickIntoTime.fAccTime = 0.f;
            m_pColliderCom->Set_Active(false);

            /* SFX */
            m_pGameInstance->GetInstance()->Play_Sound(L"ENEMY_Boss_Albatoss_FeatherY.wav", SOUND::EFFECT);
        }
    }
}

void CAlbatoss_Feather::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CAlbatoss_Feather::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

HRESULT CAlbatoss_Feather::Ready_Components(OBJECT_DESC* _Desc)
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

    return S_OK;
}

HRESULT CAlbatoss_Feather::Ready_Collider()
{
    /* Collider */
    CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
    tSphereDesc.fRadius = 0.5f;
    tSphereDesc.vCenter = _float3(0.f, 0.f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_SPHERE"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pColliderCom),
        &tSphereDesc)))
        return E_FAIL;

    m_pColliderCom->Set_Owner(this);
    m_pColliderCom->Set_Group(GROUP::WEAPON_MONSTER);
    m_pGameInstance->Register_Collider(m_pColliderCom);

    return S_OK;
}

void CAlbatoss_Feather::Update_Shoot(_float _fTimeDelta)
{
    if (m_bShoot)
    {
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        vPos += m_vDir * m_fSpeed * _fTimeDelta;
        m_pTransformCom->Set_State(STATE::POSITION, vPos);
    }
}

void CAlbatoss_Feather::Destroy()
{
    m_bActive = false;

    if (m_pColliderCom)
        m_pColliderCom->Set_Active(false);
}

CAlbatoss_Feather* CAlbatoss_Feather::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CAlbatoss_Feather* pInstance = new CAlbatoss_Feather(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CAlbatoss_Feather");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CAlbatoss_Feather::Clone(void* _pArg)
{
    CAlbatoss_Feather* pInstance = new CAlbatoss_Feather(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CAlbatoss_Feather");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAlbatoss_Feather::Free()
{
    __super::Free();

    Safe_Release(m_pColliderCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
