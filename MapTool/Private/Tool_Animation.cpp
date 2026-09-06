#include "Tool_Animation.h"
#include "MapEditor.h"

CTool_Animation::CTool_Animation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pMapEditor{ _pMapEditor }
{
}

HRESULT CTool_Animation::Initialize()
{
    return S_OK;
}

void CTool_Animation::Update(_float fTimeDelta)
{
}

void CTool_Animation::LateUpdate(_float fTimeDelta)
{
}

HRESULT CTool_Animation::Render()
{
    ImGui::Begin("Inspector (Animation)");
    Render_Animation();
    ImGui::End();

    return S_OK;
}

void CTool_Animation::Render_Animation()
{
    if (!m_pMapEditor)
        return;

}

const char* CTool_Animation::Get_AnimationNameList(_uint _iAnimIndex)
{
    if (!m_pMapEditor)
        return "";
}

CTool_Animation* CTool_Animation::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
{
    CTool_Animation* pInstance = new CTool_Animation(_pDevice, _pDeviceContext, _pMapEditor);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CTool_Animation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTool_Animation::Free()
{
    __super::Free();
}
