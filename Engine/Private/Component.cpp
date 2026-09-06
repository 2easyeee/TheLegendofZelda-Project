#include "Component.h"

#include "GameInstance.h"

CComponent::CComponent(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pGameInstance { CGameInstance::GetInstance() }
    , m_pDevice { _pDevice }
    , m_pDeviceContext { _pDeviceContext }
    , m_isCloned { false }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

CComponent::CComponent(const CComponent& _Prototype)
    : m_pGameInstance{ _Prototype.m_pGameInstance }
    , m_pDevice{ _Prototype.m_pDevice }
    , m_pDeviceContext{ _Prototype.m_pDeviceContext }
    , m_isCloned { true }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CComponent::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CComponent::Initialize(void* _pArg)
{
    return S_OK;
}

void CComponent::Free()
{
    __super::Free();

    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pDevice);
    Safe_Release(m_pGameInstance);
}
