#include "NavigationEditor.h"
#include "ImGui_Manager.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "Inspector_NavMesh.h"

CNavigationEditor::CNavigationEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel{ _pDevice, _pDeviceContext }
{
}

HRESULT CNavigationEditor::Initialize(CImGui_Manager* _pImGuiManager)
{
    /* ImGui */
    if (!m_pImGuiManager)
        m_pImGuiManager = _pImGuiManager;

    m_pImGuiManager->Register_ImGui_Objcet(TEXT("NavMesh"), CInspector_NavMesh::Create(m_pDevice, m_pDeviceContext));

    /* GameObject */
    if (FAILED(Ready_Layer_Camera(TEXT("Camera"))))
        return E_FAIL;

    return S_OK;
}

void CNavigationEditor::Update(_float _fTimeDelta)
{
}

HRESULT CNavigationEditor::Render()
{
    SetWindowText(g_hWnd, TEXT("Navigation Editor"));

    return S_OK;
}

HRESULT CNavigationEditor::Ready_Layer_Camera(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Camera_Free"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Camera_Free"));

    CCamera::CAMERA_DESC CameraDesc = {};
    CameraDesc.vEye = _float3(0.f, 10.f, -8.f);
    CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
    CameraDesc.fFovy = XMConvertToRadians(60.0f);
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 1000.f;
    CameraDesc.fSpeedPerSec = 10.f;
    CameraDesc.fRotationPerSec = 90.0f;
    CameraDesc.fSensor = 0.07f;

    CCamera::CAMERA_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tCameraDesc = CameraDesc;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::NAVIGATIONEDITOR),
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

CNavigationEditor* CNavigationEditor::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CImGui_Manager* _pImGuiManager)
{
    CNavigationEditor* pInstance = new CNavigationEditor(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_pImGuiManager)))
    {
        MSG_BOX("FAILED TO CREATED : CNavigationEditor");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNavigationEditor::Free()
{
    __super::Free();
}
