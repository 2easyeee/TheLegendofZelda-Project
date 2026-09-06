#include "WorldObject.h"
#include "GameInstance.h"

CWorldObject::CWorldObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject { _pDevice, _pDeviceContext }
{
}

CWorldObject::CWorldObject(const CWorldObject& _Prototype)
    : CGameObject (_Prototype)
{
}

HRESULT CWorldObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWorldObject::Initialize(void* _pArg)
{
    WORLD_DESC* pWorldDesc = static_cast<WORLD_DESC*>(_pArg);

    if (pWorldDesc)
    {
        /* WORLD_DESC */
        memcpy(&m_WorldDesc, pWorldDesc, sizeof(WORLD_DESC));

        m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(m_WorldDesc.vRight.x, m_WorldDesc.vRight.y, m_WorldDesc.vRight.z, 0.f));
        m_pTransformCom->Set_State(STATE::UP, XMVectorSet(m_WorldDesc.vUp.x, m_WorldDesc.vUp.y, m_WorldDesc.vUp.z, 0.f));
        m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(m_WorldDesc.vLook.x, m_WorldDesc.vLook.y, m_WorldDesc.vLook.z, 0.f));
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_WorldDesc.vPosition.x, m_WorldDesc.vPosition.y, m_WorldDesc.vPosition.z, 1.f));

    }
    
    return S_OK;
}

void CWorldObject::Priority_Update(_float _fTimeDelta)
{
}

void CWorldObject::Update(_float _fTimeDelta)
{
}

void CWorldObject::Late_Update(_float _fTimeDelta)
{
}

HRESULT CWorldObject::Render()
{
    return S_OK;
}

void CWorldObject::Free()
{
    __super::Free();
}
