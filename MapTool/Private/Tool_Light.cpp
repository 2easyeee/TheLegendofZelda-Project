#include "Tool_Light.h"
#include "MapEditor.h"
#include "GameObject/MapObject_Light.h"
#include "Light.h"
#include "GameInstance.h"
#include "Layer.h"

CTool_Light::CTool_Light(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pMapEditor{ _pMapEditor }
    , m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CTool_Light::Initialize(CMapEditor* _pMapEditor)
{
    if (!_pMapEditor)
        return E_FAIL;

    m_pMapEditor = _pMapEditor;

    return S_OK;
}

void CTool_Light::Update(_float fTimeDelta)
{
}

void CTool_Light::LateUpdate(_float fTimeDelta)
{
}

HRESULT CTool_Light::Render()
{
    ImGui::Begin("Inspector (Light)");
    Render_Light();
    ImGui::End();

    ImGui::Begin("Light File System");
    Render_Save_Button();
    Render_Load_Button();
    ImGui::End();

    return S_OK;
}

void CTool_Light::Render_Light()
{
    CGameObject* pSelected = m_pMapEditor->Get_ImGui_SelectedObject();
    if (!pSelected)
    {
        ImGui::Text("No Selected Object");
        return;
    }

    CMapObject_Light* pLightObj = dynamic_cast<CMapObject_Light*>(pSelected);
    if (!pLightObj)
    {
        ImGui::Text("Selected Object is NOT Light");
        return;
    }

    if (m_pTarget != pLightObj)
    {
        m_pTarget = pLightObj;
        m_EditDesc = pLightObj->Get_LightDesc();
    }


    _bool bChanged = { false };

    /* Type */
    int type = (int)m_EditDesc.eType;
    ImGui::Text("Light Type");
    bChanged |= ImGui::RadioButton("Directional", &type, (int)LIGHT::DIRECTIONAL);
    ImGui::SameLine();
    bChanged |= ImGui::RadioButton("Point", &type, (int)LIGHT::POINT);
    ImGui::SameLine();
    bChanged |= ImGui::RadioButton("Spot", &type, (int)LIGHT::SPOT);
    m_EditDesc.eType = (LIGHT)type;

    ImGui::Separator();

    /* Usage */
    int usage = (int)m_EditDesc.eUsage;
    ImGui::Text("Usage");
    bChanged |= ImGui::RadioButton("Static", &usage, (int)LIGHT_USAGE::STATIC);
    ImGui::SameLine();
    bChanged |= ImGui::RadioButton("Dynamic", &usage, (int)LIGHT_USAGE::DYNAMIC);
    m_EditDesc.eUsage = (LIGHT_USAGE)usage;
    ImGui::Separator();
    bChanged |= ImGui::ColorEdit3("Diffuse", (float*)&m_EditDesc.vDiffuse);
    bChanged |= ImGui::ColorEdit3("Ambient", (float*)&m_EditDesc.vAmbient);
    bChanged |= ImGui::ColorEdit3("Specular", (float*)&m_EditDesc.vSpecular);
    ImGui::Separator();
    bChanged |= ImGui::DragFloat3("Direction", (float*)&m_EditDesc.vDirection, 0.1f);
    ImGui::Separator();
    bChanged |= ImGui::DragFloat3("Position", (float*)&m_EditDesc.vPosition, 0.1f);
    bChanged |= ImGui::DragFloat("Range", &m_EditDesc.fRange, 0.1f, 0.f, 100.f);
    bChanged |= ImGui::DragFloat("Intensity", &m_EditDesc.fIntensity, 0.01f, 0.f, 10.f);

    if (bChanged)
    {
        m_pTarget->Set_LightDesc(m_EditDesc);
        CTransform* pTransform = static_cast<CTransform*>(m_pTarget->Get_Component(TEXT("Com_Transform")));
        if (pTransform)
        {
            _vector pos = XMVectorSet(m_EditDesc.vPosition.x, m_EditDesc.vPosition.y, m_EditDesc.vPosition.z, 1.f);
            pTransform->Set_State(STATE::POSITION, pos);

            _vector dir = XMVector3Normalize(XMLoadFloat4(&m_EditDesc.vDirection));
            pTransform->LookAt(pTransform->Get_State(STATE::POSITION) + dir);
        }
    }
}

HRESULT CTool_Light::Render_Save_Button()
{
    ImGui::Text("Save Level:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(30);
    ImGui::InputInt("##SaveLevelLight", &m_iFileIndex_Save, 0);

    ImGui::SameLine();
    if (ImGui::Button("Click!##SaveLight"))
    {
        int iResult = MessageBox(nullptr, L"저장 잘못누른거 아니지 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
        if (iResult == IDOK)
        {
            Save_LightBinary(ENUM_TO_UINT(LEVEL::END), m_iFileIndex_Save);
        }
    }

    return S_OK;
}

HRESULT CTool_Light::Render_Load_Button()
{
    ImGui::Text("Load Level:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(30);
    ImGui::InputInt("##LoadLevelLight", &m_iFileIndex_Load, 0);

    ImGui::SameLine();
    if (ImGui::Button("Click!##LoadLight"))
    {
        int iResult = MessageBox(nullptr, L"로드하기 전에 저장 했나 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
        if (iResult == IDOK)
        {
            m_pGameInstance->Clone_LightBinary(ENUM_TO_UINT(LEVEL::MAPEDITOR), m_iFileIndex_Load);
        }
    }

    return S_OK;
}

HRESULT CTool_Light::Save_LightBinary(_uint _iLevelTotalNum, _uint _iFileIndex)
{
    string strFileName = "MAP_LIGHT_" + to_string(_iFileIndex) + ".dat";
    ofstream file("../../Resources/Data/dat/" + strFileName, ios::binary | ios::trunc);
    if (!file.is_open())
        return E_FAIL;

    auto pLayers = m_pGameInstance->Get_Layers();
    if (!pLayers)
        return E_FAIL;

    _uint iLightCount = 0;

    /* 1. Count */
    for (_uint i = 0; i < _iLevelTotalNum; ++i)
    {
        auto& layers = pLayers[i];

        for (auto& Pair : layers)
        {
            const _wstring& wstrLayerTag = Pair.first;

            if (wstrLayerTag.rfind(TEXT("Light"), 0) != 0)
                continue;

            CLayer* pLayer = Pair.second;
            if (!pLayer)
                continue;

            for (auto& pGameObject : pLayer->Get_GameObjects())
            {
                if (!pGameObject)
                    continue;

                if (dynamic_cast<CMapObject_Light*>(pGameObject))
                    ++iLightCount;
            }
        }
    }

    /* Count 저장 */
    file.write(reinterpret_cast<const char*>(&iLightCount), sizeof(_uint));

    /* 2. 실제 데이터 */
    for (_uint i = 0; i < _iLevelTotalNum; ++i)
    {
        auto& layers = pLayers[i];

        for (auto& Pair : layers)
        {
            const _wstring& wstrLayerTag = Pair.first;

            if (wstrLayerTag.rfind(TEXT("Light"), 0) != 0)
                continue;

            CLayer* pLayer = Pair.second;
            if (!pLayer)
                continue;

            for (auto& pGameObject : pLayer->Get_GameObjects())
            {
                if (!pGameObject)
                    continue;

                CMapObject_Light* pLight = dynamic_cast<CMapObject_Light*>(pGameObject);
                if (!pLight)
                    continue;

                const LIGHT_DESC& desc = pLight->Get_LightDesc();
                file.write(reinterpret_cast<const char*>(&desc), sizeof(LIGHT_DESC));
            }
        }
    }

    file.close();

    MSG_BOX("SUCCESS TO SAVED : MAP_LIGHT Binary");

    return S_OK;
}

CTool_Light* CTool_Light::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
{
    CTool_Light* pInstance = new CTool_Light(_pDevice, _pDeviceContext, _pMapEditor);
    if (FAILED(pInstance->Initialize(_pMapEditor)))
    {
        MSG_BOX("FAILED TO CREATED : CTool_Light");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTool_Light::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
