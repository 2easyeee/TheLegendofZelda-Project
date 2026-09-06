#include "Hierarchy_Level.h"
#include "MapEditor.h"

CHierarchy_Level::CHierarchy_Level(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pEditor)
    : CImGui_Object { _pDevice, _pDeviceContext }
    , m_pMapEditor { _pEditor }
{
}

HRESULT CHierarchy_Level::Initialize()
{
    m_strTitle = TEXT("Hierarchy");

    return S_OK;
}

void CHierarchy_Level::Update(_float fTimeDelta)
{
}

void CHierarchy_Level::LateUpdate(_float fTimeDelta)
{
}

HRESULT CHierarchy_Level::Render()
{
    if (!m_pMapEditor)
        return E_FAIL;

    return S_OK;
}

CHierarchy_Level* CHierarchy_Level::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pEditor)
{
    CHierarchy_Level* pInstance = new CHierarchy_Level(_pDevice, _pDeviceContext, _pEditor);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CHierarchy_Level");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CHierarchy_Level::Free()
{
    __super::Free();
}
