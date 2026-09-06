#include "GameObject/Rupee.h"
#include "GameInstance.h"

CRupee::CRupee(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CRupee::CRupee(const CRupee& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CRupee::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CRupee::Initialize(void* _pArg)
{
    /* MapObject */
    if (FAILED(CMapObject::Initialize(_pArg)))
        return E_FAIL;

    MAP_INIT_DESC* pInitDesc = static_cast<MAP_INIT_DESC*>(_pArg);
    if (FAILED(Ready_Components(&pInitDesc->tObjectDesc)))
        return E_FAIL;

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    m_pCollider->Set_Active(false);
    m_tDelayTime.bActive = false;
    m_tDelayTime.fAccTime = 0.f;

    return S_OK;
}

void CRupee::Priority_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;
}

void CRupee::Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (!m_tDelayTime.bActive)
    {
        m_tDelayTime.fAccTime += _fTimeDelta;
        if (m_tDelayTime.fAccTime >= m_tDelayTime.fAccDurationTime)
        {
            m_tDelayTime.bActive = true;
            if (m_pCollider)
                m_pCollider->Set_Active(true);
        }
    }

    /* Collider */
    if (m_pCollider)
        m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    /* EFFECT */
    if (m_bBurstMode)
    {
        Update_Burst(_fTimeDelta);
    }
    else
    {
        Bounce(_fTimeDelta);
        Float(_fTimeDelta);
    }

    if (m_bBillboard)
        Update_Billboard(_fTimeDelta);
}

void CRupee::Late_Update(_float _fTimeDelta)
{
    if (!m_bActive)
        return;

    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLEND, this)))
        return;
}

HRESULT CRupee::Render()
{
    if (!m_bActive)
        return S_OK;

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
    //if (m_pCollider) m_pCollider->Render();
#endif

    return S_OK;
}

void CRupee::OnCollisionEnter(CCollider* _pSrc, CCollider* _Dst)
{
    if (_pSrc->Get_Group() == GROUP::ITEM)
    {
        if (_Dst->Get_Group() == GROUP::PLAYER)
        {
            UI_Event();
            // TODO : Player ¿¬µ¿
            Destroy();
        }
    }
}

void CRupee::OnCollisionStay(CCollider* _pSrc, CCollider* _Dst)
{
}

void CRupee::OnCollisionExit(CCollider* _pSrc, CCollider* _Dst)
{
}

void CRupee::Set_Type(RUPEE_TYPE _eType)
{
    m_eType = _eType;

    _wstring ModelTag = {};
    switch (_eType)
    {
    case CRupee::RUPEE_TYPE::GREEN:
    {
        m_iValue = 1;
        ModelTag = TEXT("Prototype_Component_Model_Map_RupeeGreen");
    }
        break;
    case CRupee::RUPEE_TYPE::RED:
    {
        m_iValue = 20;
        ModelTag = TEXT("Prototype_Component_Model_Map_RupeeRed");
    }
    break;
    case CRupee::RUPEE_TYPE::PURPLE:
    {
        m_iValue = 50;
        ModelTag = TEXT("Prototype_Component_Model_Map_RupeePurple");
    }
        break;
    case CRupee::RUPEE_TYPE::GOLD:
    {
        m_iValue = 1000;
        ModelTag = TEXT("Prototype_Component_Model_Map_RupeeGold");
    }
    break;
    }

    Update_ModelCom(ModelTag);
}

_int CRupee::Get_Value() const
{
    return m_iValue;
}

void CRupee::Set_Burst(_vector _vVelocity)
{
    m_bBurstMode = true;
    m_vVelocity = _vVelocity;

    /* InActive BounceMode */
    m_bSpawnBounce = false;
    m_fBaseHeight = 0.f;
}

void CRupee::Set_Billboard(_bool _bActive)
{
    m_bBillboard = _bActive;
}

HRESULT CRupee::Ready_Components(OBJECT_DESC* _pTags)
{
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Shader_VtxMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(
        _pTags->iLevel,
        _pTags->ModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRupee::Update_ModelCom(_wstring _pModelTag)
{
    if (m_pModelCom)
    {
        Remove_Component(TEXT("Com_Model"));
        Safe_Release(m_pModelCom);
        m_pModelCom = nullptr;
    }

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _pModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CRupee::Ready_Collider()
{
    /* Collider */
    CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
    tSphereDesc.fRadius = 0.5f;
    tSphereDesc.vCenter = _float3(0.f, tSphereDesc.fRadius * 0.5f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_SPHERE"),
        TEXT("Com_Collider"),
        reinterpret_cast<CComponent**>(&m_pCollider),
        &tSphereDesc)))
        return E_FAIL;

    m_pCollider->Set_Owner(this);
    m_pCollider->Set_Group(GROUP::ITEM);
    m_pCollider->Set_Active(true);
    m_pGameInstance->Register_Collider(m_pCollider);

    return S_OK;
}

void CRupee::UI_Event()
{
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::RUPEE_ADD;
    tUIEvent.iValue = m_iValue;

    m_pGameInstance->Push_UIEVENT(tUIEvent);
}

void CRupee::Destroy()
{
    /* SFX */
    m_pGameInstance->Play_Sound(L"OBJ_Rupee.wav", SOUND::EFFECT, 2.f);
    this->Set_Active(false);

    if (m_pCollider)
        m_pCollider->Set_Active(false);
}

void CRupee::Update_Billboard(_float _fTimeDelta)
{
}

void CRupee::Bounce(_float _fTimeDelta)
{
    if (m_bSpawnBounce)
    {
        m_fSpawnVelocity += m_fGravity * _fTimeDelta;
        m_fBaseHeight += m_fSpawnVelocity * _fTimeDelta;

        if (m_fBaseHeight <= 0.f)
        {
            m_fBaseHeight = 0.f;
            m_bSpawnBounce = false;
        }
    }
}

void CRupee::Float(_float _fTimeDelta)
{
    m_fFloatTime += _fTimeDelta;

    _float fOffset = sinf(m_fFloatTime * 3.f) * 0.05f;

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos = XMVectorSetY(vPos, m_fBaseHeight + fOffset);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

void CRupee::Rotate(_float _fTimeDelta)
{
    m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180.f * _fTimeDelta);
}

void CRupee::Update_Burst(_float _fTimeDelta)
{
    m_vVelocity += XMVectorSet(0.f, -9.8f * _fTimeDelta, 0.f, 0.f);

   _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos += m_vVelocity * _fTimeDelta;

    /* Floor */
    _float fGround = 0.5f;
    if (XMVectorGetY(vPos) <= fGround)
    {
        vPos = XMVectorSetY(vPos, fGround);
        m_vVelocity *= 0.3f;
        m_vVelocity = XMVectorSetY(m_vVelocity, 0.f);
        if (XMVectorGetX(XMVector3Length(m_vVelocity)) < 0.1f)
        {
            m_bBurstMode = false;
            m_fBaseHeight = fGround;
        }
    }

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

CRupee* CRupee::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CRupee* pInstance = new CRupee(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CRupee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRupee::Clone(void* _pArg)
{
    CRupee* pInstance = new CRupee(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CRupee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRupee::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pCollider);
}
