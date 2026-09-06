#include "AssimpEditor.h"
#include "aiConverter.h"
#include "ImGui_Manager.h"
#include "Hierarchy_Model.h"
#include "Inspector_Mesh.h"
#include "Inspector_Material.h"
#include "Texture_Preview.h"
#include "Camera_Free.h"
#include "StaticMapObject.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Layer.h"
#include "Transform.h"
#include "MapObject.h"
#include "Inspector_GameObject.h"
#include "Inspector_Animation.h"

CAssimpEditor::CAssimpEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel { _pDevice, _pDeviceContext }
{
}

HRESULT CAssimpEditor::Initialize(CImGui_Manager* _pImGuiManager)
{
    /* ImGui */
    if (!m_pImGuiManager)
        m_pImGuiManager = _pImGuiManager;
    
    /* Register ImGui Object */
    m_pImGuiManager->Register_ImGui_Objcet(TEXT("Hierarchy"), CHierarchy_Model::Create(m_pDevice, m_pDeviceContext, this));
    m_pImGuiManager->Register_ImGui_Objcet(TEXT("Inspector_Mesh"), CInspector_Mesh::Create(m_pDevice, m_pDeviceContext, this));
    m_pImGuiManager->Register_ImGui_Objcet(TEXT("Inspector_Material"), CInspector_Material::Create(m_pDevice, m_pDeviceContext, this));
    m_pImGuiManager->Register_ImGui_Objcet(TEXT("Inspector_GameObject"), CInspector_GameObject::Create(m_pDevice, m_pDeviceContext));
    m_pImGuiManager->Register_ImGui_Objcet(TEXT("Inspector_Animation"), CInspector_Animation::Create(m_pDevice, m_pDeviceContext, this));
    
    /* Texture Preview */
    m_pTexturePreview = CTexture_Preview::Create(m_pDevice, m_pDeviceContext);

    /* FBX -> Memeory */
    m_pAIConverter = CaiConverter::Create();
    if (!m_pAIConverter)
        return E_FAIL;

    /* aiScene InJection */
    m_paiScene = m_pAIConverter->Get_aiScene();

    /* GameObject */
    if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
        return E_FAIL;

    return S_OK;
}

void CAssimpEditor::Update(_float _fTimeDelta)
{
}

HRESULT CAssimpEditor::Render()
{
    SetWindowText(g_hWnd, TEXT("Assimp EDITOR"));

    /* IO */
    ImGui::Begin("IO");
    Render_Import_FBX(); ImGui::SameLine();
    Render_Export_FBX(); ImGui::SameLine();
    Render_Model();
    ImGui::End();

    return S_OK;
}

void CAssimpEditor::Set_SelectedNode(const aiNode* _pNode)
{
    m_pSelectedNode = _pNode;
    m_SelectedMeshIndex = -1; // Node 변경 시, 초기화

    if (_pNode->mNumMeshes > 0)
    {
        m_SelectedMeshIndex = _pNode->mMeshes[0];
    }
}

aiAnimation* CAssimpEditor::Get_Animation(_uint _iIndex)
{
    if (!m_paiScene || _iIndex >= m_paiScene->mNumAnimations)
        return nullptr;
    return m_paiScene->mAnimations[_iIndex];
}

HRESULT CAssimpEditor::Ready_Layer_Camera(const _wstring& strLayerTag)
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
        ENUM_TO_UINT(LEVEL::ASSIMPEDITOR),
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CAssimpEditor::Create_GameObject()
{
    /* Remove */
    if (m_pPreviewObject)
        m_pGameInstance->Reserve_DeleteObject(m_pPreviewObject);

    /* XML */
    _wstring basePath = Extract_AssetRelativePath(m_wstrFullFilePath);
    _wstring tagXMLPath =
        TEXT("../../Resources/Models/") +
        basePath +
        TEXT("_TAGS.xml");

    EXPORT_TAGS tags;
    if (FAILED(Load_GameObjectXML(tagXMLPath, tags)))
        return E_FAIL;

    /* ReBuild */
    _string paramPath = WSTRTOCHAR(basePath);
    m_pAIConverter->Replace(paramPath, "\\", "/");

    _wstring meshPath = TEXT("../../Resources/Models/") + STRTOWSTR(paramPath) + TEXT(".mesh");

    /* Blocking Dup Prototype */
    _wstring newModelTag = tags.ModelTag + TEXT("_Preview");
    
    if (FAILED(m_pGameInstance->Add_Prototype(
        RESOURCE_LEVEL_STATIC,
        newModelTag,
        CModel::Create(
            m_pDevice,
            m_pDeviceContext,
            WSTRTOCHAR(meshPath).c_str()))))
        return E_FAIL;

    /* Spawn */
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, tags.ObjectID.c_str());
    wcscpy_s(tObjectDesc.ObjectTag, tags.GameObjectTag.c_str());
    wcscpy_s(tObjectDesc.LayerTag, tags.LayerTag.c_str());
    wcscpy_s(tObjectDesc.ShaderTag, tags.ShaderTag.c_str());
    wcscpy_s(tObjectDesc.ModelTag, newModelTag.c_str());

    CMapObject::MAP_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tWorldDesc.vPosition = _float3{ 0.f, 0.f, 0.f };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::ASSIMPEDITOR),
        tObjectDesc.LayerTag,
        &tInitDesc, &m_pPreviewObject)))
        return E_FAIL;

    return S_OK;
}

void CAssimpEditor::Render_Import_FBX()
{
    if (ImGui::Button("Load"))
    {
        Import_FBX();
    }
}

void CAssimpEditor::Render_Export_FBX()
{
    if (ImGui::Button("Export"))
    {
        Export_FBX();
    }
}

void CAssimpEditor::Render_Model()
{
    if (ImGui::Button("Show Model"))
    {
        Create_GameObject();
    }
}

HRESULT CAssimpEditor::Export_FBX()
{
    if (!m_paiScene)
    {
        MSG_BOX("FAILED TO EXPORT : Import an asset before exporting.");
        return S_OK;
    }

    m_pAIConverter->Export_ModelData();
    m_pAIConverter->Export_MaterialData();
    Export_GameObjectXML(m_pAIConverter->Get_XMLPath());

    MSG_BOX("SUCCESS : Export .mesh and .xml");

    return S_OK;
}

HRESULT CAssimpEditor::Import_FBX()
{
    _tchar filePath[MAX_PATH] = {};

    OPENFILENAMEW openFileName = {};
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = g_hWnd;
    openFileName.lpstrFilter = TEXT("FBX Files (*.fbx)\0*.fbx\0");
    openFileName.lpstrFile = filePath;
    openFileName.nMaxFile = MAX_PATH;
    openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&openFileName))
    {
        Safe_Delete(m_pAIConverter);

        m_pAIConverter = CaiConverter::Create();
        m_pAIConverter->Read_AssetFile(Extract_AssetRelativePath(filePath));
        m_paiScene = m_pAIConverter->Get_aiScene();
    }

    /* Save File Path To Texture Preview */
    m_wstrFullFilePath = filePath;

    return S_OK;
}

_wstring CAssimpEditor::Extract_AssetRelativePath(const _wstring& _FullPath)
{
    filesystem::path path = filesystem::canonical(_FullPath);
    filesystem::path result;

    bool found = false;
    for (auto it = path.begin(); it != path.end(); ++it)
    {
        if (*it == L"Assets")
        {
            ++it;
            for (; it != path.end(); ++it)
                result /= *it;
            found = true;
            break;
        }
    }

    result.replace_extension();

    return found ? result.wstring() : L"";
}

void CAssimpEditor::Export_GameObjectXML(_wstring _XMLPath)
{
    /* Get Struct */
    auto* pInspector = static_cast<CInspector_GameObject*>(
        m_pImGuiManager->Find_ImGui_Object(TEXT("Inspector_GameObject")));
    if (!pInspector)
        return;

    EXPORT_TAGS tTags = pInspector->Get_ExportTags();

    /* XML */
    tinyxml2::XMLDocument document;
    auto* pRoot = document.NewElement("GameObject");
    document.InsertFirstChild(pRoot);

    auto AddTag = [&](const char* name, const wstring& value)
        {
            auto* element = document.NewElement(name);
            element->SetText(WSTRTOCHAR(value).c_str());
            pRoot->InsertEndChild(element);
        };

    AddTag("ObjectID", Extract_ObjectID(_XMLPath));
    AddTag("LayerTag", tTags.LayerTag);
    AddTag("GameObjectTag", tTags.GameObjectTag);
    AddTag("ShaderTag", tTags.ShaderTag);
    AddTag("ModelTag", TEXT("Prototype_Component_Model_Map_") + Extract_ObjectID(_XMLPath));

    /* Save MeshPath */
    filesystem::path meshPath = filesystem::path(_XMLPath).filename();
    meshPath += TEXT(".mesh");
    _wstring relativeMeshPath 
        = filesystem::path(_XMLPath).parent_path().parent_path().filename().wstring()
        + TEXT("/")
        + filesystem::path(_XMLPath).parent_path().filename().wstring()
        + TEXT("/") + meshPath.wstring();
    AddTag("MeshPath", relativeMeshPath);

    /* Export .xml */
    _wstring savePath = _XMLPath + TEXT("_TAGS") + TEXT(".xml");
    _string ParameterPath = WSTRTOCHAR(savePath).c_str();
    m_pAIConverter->Replace(ParameterPath, "\\", "/");
    document.SaveFile(ParameterPath.c_str());
}

HRESULT CAssimpEditor::Load_GameObjectXML(_wstring _XMLPath, EXPORT_TAGS& _outTags)
{
    tinyxml2::XMLDocument document;

    _string path = WSTRTOCHAR(_XMLPath);
    m_pAIConverter->Replace(path, "\\", "/");

    if (document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
        return false;

    auto* pRoot = document.FirstChildElement("GameObject");
    if (!pRoot)
        return false;

    auto ReadTag = [&](const char* name, wstring& out)
        {
            auto* e = pRoot->FirstChildElement(name);
            if (e && e->GetText())
                out = STRTOWSTR(e->GetText());
        };

    ReadTag("ObjectID", _outTags.ObjectID);
    ReadTag("LayerTag", _outTags.LayerTag);
    ReadTag("GameObjectTag", _outTags.GameObjectTag);
    ReadTag("ShaderTag", _outTags.ShaderTag);
    ReadTag("ModelTag", _outTags.ModelTag);
    ReadTag("MeshPath", _outTags.MeshPath);

    return S_OK;
}

_wstring CAssimpEditor::Extract_ObjectID(_wstring _XMLPath)
{
    filesystem::path path(_XMLPath);
    return path.stem().wstring();
}

CAssimpEditor* CAssimpEditor::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CImGui_Manager* _pImGuiManager)
{
    CAssimpEditor* pInstance = new CAssimpEditor(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_pImGuiManager)))
    {
        MSG_BOX("FAILED TO CREATED : CAssimpEditor");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAssimpEditor::Free()
{
    __super::Free();

    Safe_Delete(m_pTexturePreview);
    Safe_Delete(m_pAIConverter);
}