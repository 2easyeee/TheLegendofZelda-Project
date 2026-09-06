#include "ImGui_Object.h"
#include "GameInstance.h"

CImGui_Object::CImGui_Object(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pDevice { _pDevice }
    , m_pDeviceContext { _pDeviceContext }
    , m_pGameInstance {CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
    Safe_AddRef(m_pGameInstance);
}

void CImGui_Object::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pDevice);
}
