#include "GameObject/Boomerang.h"
#include "GameInstance.h"

CBoomerang::CBoomerang(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject{ _pDevice, _pDeviceContext }
{
}

CBoomerang::CBoomerang(const CBoomerang& _Prototype)
    : CWorldObject(_Prototype)
{
}

HRESULT CBoomerang::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoomerang::Initialize(void* _pArg)
{
    BOOMERANG_INIT_DESC* tInitDesc = static_cast<BOOMERANG_INIT_DESC*>(_pArg);

    /* 2. CGameObject*/
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

    if (&tInitDesc->tBoomerangDesc)
    {
        _matrix WorldMatrix = XMLoadFloat4x4(&tInitDesc->tBoomerangDesc.WorldMatrix);
        for (_int i = 0; i < 3; ++i)
        {
            WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
        }
        m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
        m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
        m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
    }

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    /* Here Transform */
    m_pTransformCom->Set_Scale(1.f, 1.f, 5.f);
    //m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 45.f);
    //m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(0.f, 0.5f, 0.f, 1.f));

    return S_OK;
}

void CBoomerang::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CWorldObject::Priority_Update(_fTimeDelta);
}

void CBoomerang::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CWorldObject::Update(_fTimeDelta);

    /* Boomerang */
    if (m_eBoomState == BOOM_STATE::FLY)
    {
        m_BoomTime.fAccTime += _fTimeDelta;

        if (m_bUseHoming)
        {
            _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

            _vector vDesiredDir = XMVector3Normalize(m_vHomingTarget - vPos);
            _vector vCurrentDir = XMLoadFloat3(&m_vDir);

            vCurrentDir = XMVector3Normalize(XMVectorLerp(vCurrentDir, vDesiredDir, _fTimeDelta * m_fHomingStrengh));
            XMStoreFloat3(&m_vDir, vCurrentDir);
        }
        else
        {
            _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
            _vector vDir = XMLoadFloat3(&m_vDir);

            vPos += vDir * m_fSpeed * _fTimeDelta * -1.f;
            m_pTransformCom->Set_State(STATE::POSITION, vPos);

            m_fSpinAngle += 720.f * _fTimeDelta;
            _vector vAxis = m_pTransformCom->Get_State(STATE::LOOK);
            m_pTransformCom->Rotation(vAxis, m_fSpinAngle);
        }

        if (m_BoomTime.fAccTime >= m_BoomTime.fAccDurationTime)
        {
            m_eBoomState = BOOM_STATE::RETURN;
        }
    }
    else if (m_eBoomState == BOOM_STATE::RETURN)
    {
        if (!m_bHasTarget)
            return;

        _matrix Target = XMLoadFloat4x4(&m_ReturnTargetMatrix);
        _vector vTargetPos = Target.r[3];

        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

        _vector vDir = XMVector3Normalize(vTargetPos - vPos);
        vPos += vDir * m_fSpeed * _fTimeDelta;

        m_pTransformCom->Set_State(STATE::POSITION, vPos);

        m_fSpinAngle += 720.f * _fTimeDelta;
        _vector vAxis = m_pTransformCom->Get_State(STATE::LOOK);
        m_pTransformCom->Rotation(vAxis, m_fSpinAngle);

        if (XMVectorGetX(XMVector3Length(vTargetPos - vPos)) < 0.5f)
        {
            m_eBoomState = BOOM_STATE::ATTACHED;
            m_pColliderCom->Set_Active(false);
        }
    }

    /* 6. Collider */
    if (m_pColliderCom)
        m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBoomerang::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CWorldObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CBoomerang::Render()
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
    //if (m_pColliderCom)
    //    m_pColliderCom->Render();
#endif // _DEBUG

    return S_OK;
}

void CBoomerang::Throw(_float3 _vDir)
{
    if (m_eBoomState != BOOM_STATE::ATTACHED)
        return;

    m_eBoomState = BOOM_STATE::FLY;
    m_vDir = _vDir;
    m_BoomTime.fAccTime = 0.f;
    m_fSpinAngle = 0.f;

    XMStoreFloat3(&m_vDir, XMVector3Normalize(XMLoadFloat3(&m_vDir)));

    XMStoreFloat3(&m_vStartPos, m_pTransformCom->Get_State(STATE::POSITION));

    m_pColliderCom->Set_Active(true);
}

void CBoomerang::Set_ReturnTargetMatrix(_float4x4 _Matrix)
{
    /* Return */
    m_ReturnTargetMatrix = _Matrix;
    m_bHasTarget = true;
}

void CBoomerang::Update_WorldMatrix(_float4x4 _WorldMatrix)
{
    /* WorldMatrix */
    _matrix WorldMatrix = XMLoadFloat4x4(&_WorldMatrix);

    _float3 vScale = m_pTransformCom->Get_Scaled();

    for (_int i = 0; i < 3; ++i)
    {
        WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
    }

    WorldMatrix.r[0] *= vScale.x;
    WorldMatrix.r[1] *= vScale.y;
    WorldMatrix.r[2] *= vScale.z;

    m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
    m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
    m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
}

void CBoomerang::Set_HomingTarget(_vector _vTargetPos)
{
    m_vHomingTarget = _vTargetPos;
    m_bUseHoming = true;
}

void CBoomerang::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::WEAPON_PLAYER)
    {
        if (_Dst->Get_Group() == GROUP::MONSTER)
        {
            if (m_eBoomState != BOOM_STATE::FLY)
                return;
            m_eBoomState = BOOM_STATE::RETURN;

            m_pColliderCom->Set_Active(false);
        }
    }
}

void CBoomerang::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CBoomerang::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::WEAPON_PLAYER)
    {
        if (_Dst->Get_Group() == GROUP::MONSTER)
        {
            m_pColliderCom->Set_Active(true);
        }
    }
}

HRESULT CBoomerang::Ready_Components(OBJECT_DESC* _Desc)
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

HRESULT CBoomerang::Ready_Collider()
{
    /* Collider */
    CBounding_OBB::OBB_DESC tOBBDesc = {};
    tOBBDesc.vExtents = _float3(0.35f, 0.2f, 0.05f);
    tOBBDesc.vCenter = _float3(0.f, tOBBDesc.vExtents.y, 0.f);
    tOBBDesc.vAngles = _float3(0.f, 0.f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_OBB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pColliderCom),
        &tOBBDesc)))
        return E_FAIL;

    m_pColliderCom->Set_Owner(this);
    m_pColliderCom->Set_Group(GROUP::WEAPON_PLAYER);
    m_pColliderCom->Set_Active(false);
    m_pGameInstance->Register_Collider(m_pColliderCom);

    return S_OK;
}

CBoomerang* CBoomerang::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CBoomerang* pInstance = new CBoomerang(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CBoomerang");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBoomerang::Clone(void* _pArg)
{
    CBoomerang* pInstance = new CBoomerang(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CBoomerang");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoomerang::Free()
{
    __super::Free();

    Safe_Release(m_pColliderCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
