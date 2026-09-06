#include "GameObject/Body_MoriblinSpear.h"
#include "GameInstance.h"
#include "GameObject/MoriblinSpear.h"

CBody_MoriblinSpear::CBody_MoriblinSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CBody{ _pDevice, _pDeviceContext }
{
}


CBody_MoriblinSpear::CBody_MoriblinSpear(const CBody_MoriblinSpear& _Prototype)
    : CBody(_Prototype)
{
}

HRESULT CBody_MoriblinSpear::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBody_MoriblinSpear::Initialize(void* _pArg)
{
    /* 1. Body */
    if (FAILED(CBody::Initialize(_pArg)))
        return E_FAIL;

    /* 6. Collider */
    if (FAILED(Ready_Collider()))
        return E_FAIL;

    return S_OK;
}

void CBody_MoriblinSpear::Priority_Update(_float _fTimeDelta)
{
    CBody::Priority_Update(_fTimeDelta);
}

void CBody_MoriblinSpear::Update(_float _fTimeDelta)
{
    CBody::Update(_fTimeDelta);

    /* Collider */
    if (m_pParentMatrix && m_pSensorCollider)
        m_pSensorCollider->Update(XMLoadFloat4x4(m_pParentMatrix));
}

void CBody_MoriblinSpear::Late_Update(_float _fTimeDelta)
{
    CBody::Late_Update(_fTimeDelta);

}

HRESULT CBody_MoriblinSpear::Render()
{
    if (FAILED(CBody::Render()))
        return E_FAIL;

#ifdef _DEBUG
    //if (m_pSensorCollider) m_pSensorCollider->Render();
#endif // _DEBUG

    return S_OK;
}

void CBody_MoriblinSpear::Collider_SensorEnable(_bool _bActive)
{
    if (m_pSensorCollider)
        m_pSensorCollider->Set_Active(_bActive);
}

void CBody_MoriblinSpear::Set_Owner_Parent(CGameObject* _pOwner)
{
    m_pOwner = _pOwner;

    if (m_pSensorCollider)
        m_pSensorCollider->Set_Owner(_pOwner);
}

HRESULT CBody_MoriblinSpear::Ready_Collider()
{
    /* Sensor */
    CBounding_Sphere::SPHERE_DESC tSphereDesc = {};
    tSphereDesc.fRadius = 8.f;
    tSphereDesc.vCenter = _float3(0.f, 0.f, 0.f);
    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Collider_SPHERE"),
        TEXT("Com_SensorCollider"),
        reinterpret_cast<CComponent**>(&m_pSensorCollider),
        &tSphereDesc)))
        return E_FAIL;

    m_pSensorCollider->Set_Group(GROUP::SENSOR_MONSTER);
    m_pGameInstance->Register_Collider(m_pSensorCollider);

    return S_OK;
}

CBody_MoriblinSpear* CBody_MoriblinSpear::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CBody_MoriblinSpear* pInstance = new CBody_MoriblinSpear(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CBody_MoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBody_MoriblinSpear::Clone(void* _pArg)
{
    CBody_MoriblinSpear* pInstance = new CBody_MoriblinSpear(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CBody_MoriblinSpear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBody_MoriblinSpear::Free()
{
    __super::Free();

    Safe_Release(m_pSensorCollider);
}
