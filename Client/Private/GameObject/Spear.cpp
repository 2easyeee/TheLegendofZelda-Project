#include "GameObject/Spear.h"
#include "GameInstance.h"

CSpear::CSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CWorldObject { _pDevice, _pDeviceContext }
{
}

CSpear::CSpear(const CSpear& _Prototype)
    : CWorldObject (_Prototype)
{
}

HRESULT CSpear::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpear::Initialize(void* _pArg)
{
    SPEAR_INIT_DESC* tInitDesc = static_cast<SPEAR_INIT_DESC*>(_pArg);

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

    if (&tInitDesc->tSpearDesc)
    {
        _matrix WorldMatrix = XMLoadFloat4x4(&tInitDesc->tSpearDesc.WorldMatrix);
        for (_int i = 0; i < 3; ++i)
        {
            WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
        }
        m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
        m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
        m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
    }

    m_fSpeed = 5.f;
    m_tStickIntoTime.fAccTime = 0.f;

    /* Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    return S_OK;
}

void CSpear::Priority_Update(_float _fTimeDelta)
{
    CWorldObject::Priority_Update(_fTimeDelta);
}

void CSpear::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    /* Collider */
    if (m_pColliderCom)
        m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* Throw */
    if (m_tStickIntoTime.bActive)
    {
        m_vVelocity.y += m_fGravity * _fTimeDelta;

        _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
        vPosition += XMLoadFloat3(&m_vVelocity) * _fTimeDelta;

        if (XMVectorGetY(vPosition) <= 0.3f) // TODO : 현재 Y = 0.f 추후 바꿔야함
        {
            StickIntoGround(vPosition);
            m_pColliderCom->Set_Active(false);
            return;
        }
        m_pTransformCom->Set_State(STATE::POSITION, vPosition);
    }
    CWorldObject::Update(_fTimeDelta);
}

void CSpear::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    CWorldObject::Late_Update(_fTimeDelta);

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;

    /* 기존 Weapon 삭제 */
    if (m_bFlyingFinished)
    {
        //m_pGameInstance->Reserve_DeleteObject(this);
    }
}

HRESULT CSpear::Render()
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

void CSpear::Fly()
{
    m_tStickIntoTime.bActive = true;
    m_bOnGround = false;

    _vector vLook = m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
    vLook = XMVector3Normalize(vLook);

    XMStoreFloat3(&m_vVelocity, vLook * (9.5f)); // m_fSpeed * 1.8f
    m_vVelocity.y += 12.f; // 살짝 위로 던지기 6.f

    m_pColliderCom->Set_Active(true);
}

void CSpear::Fly_Target(_vector _vTargetPos)
{
    m_tStickIntoTime.bActive = true;
    m_bOnGround = false;

    _vector vStart = m_pTransformCom->Get_State(STATE::POSITION);

    _vTargetPos = XMVectorSetY(_vTargetPos, XMVectorGetY(_vTargetPos) + 0.8f);
    _vector vDir = _vTargetPos - vStart;
    vDir = XMVector3Normalize(vDir);

    _float fThrowSpeed = 10.f;
    
    XMStoreFloat3(&m_vVelocity, vDir * fThrowSpeed);

    m_pTransformCom->Set_State(STATE::LOOK, -vDir);

    m_pColliderCom->Set_Active(true);
}

void CSpear::Update_WorldMatrix(_float4x4 _WorldMatrix)
{
    _matrix WorldMatrix = XMLoadFloat4x4(&_WorldMatrix);
    for (_int i = 0; i < 3; ++i)
    {
        WorldMatrix.r[i] = XMVector3Normalize(WorldMatrix.r[i]);
    }
    m_pTransformCom->Set_State(STATE::RIGHT, WorldMatrix.r[0]);
    // 0x0000002771d7dfe0 {{m128_f32=0x0000002771d7dfe0 {0.562120020, -0.0763912052, -0.823520184, 0.00000000} ...}, ...}
    m_pTransformCom->Set_State(STATE::UP, WorldMatrix.r[1]);
    // {m128_f32=0x0000002771d7dff0 {0.812645316, -0.134051383, 0.567131221, 0.00000000} m128_u64=0x0000002771d7dff0 {...} ...}
    m_pTransformCom->Set_State(STATE::LOOK, WorldMatrix.r[2]);
    // {m128_f32=0x0000002771d7e000 {-0.153717831, -0.988025606, -0.0132743130, 0.00000000} m128_u64=0x0000002771d7e000 {...} ...}
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(WorldMatrix.r[3], 1.f));
    // {m128_f32=0x0000002771d7e010 {41.6419830, 0.582218349, 19.3731918, 1.00000000} m128_u64=0x0000002771d7e010 {...} ...}
}

HRESULT CSpear::Ready_Components(OBJECT_DESC* _Desc)
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

HRESULT CSpear::Ready_Collider()
{
    /* Collider */
    CBounding_OBB::OBB_DESC tOBBDesc = {};
    tOBBDesc.vExtents = _float3(0.2f, 0.1f, 0.85f);
    tOBBDesc.vCenter = _float3(0.f, 0.f, -0.15f);
    tOBBDesc.vAngles = _float3(0.f, 0.f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_OBB"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pColliderCom),
        &tOBBDesc)))
        return E_FAIL;

    m_pColliderCom->Set_Owner(this);
    m_pColliderCom->Set_Group(GROUP::WEAPON_MONSTER);
    m_pColliderCom->Set_Active(false);
    m_pGameInstance->Register_Collider(m_pColliderCom);

    return S_OK;
}

void CSpear::StickIntoGround(_vector _vPosition)
{
    _vPosition = XMVectorSetY(_vPosition, 0.15f);
    m_pTransformCom->Set_State(STATE::POSITION, _vPosition);

    /* Angle */
    _vector vDir = XMVector3Normalize(XMLoadFloat3(&m_vVelocity));
    m_pTransformCom->Set_State(STATE::LOOK, -vDir);

    m_vVelocity = { 0.f, 0.f, 0.f };

    m_tStickIntoTime.bActive = false;
    m_bFlyingFinished = true;
}

CSpear* CSpear::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CSpear* pInstance = new CSpear(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSpear::Clone(void* _pArg)
{
    CSpear* pInstance = new CSpear(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSpear::Free()
{
    __super::Free();

    Safe_Release(m_pColliderCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
