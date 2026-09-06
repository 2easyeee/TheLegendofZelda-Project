#include "FileIO.h"
#include "GameInstance.h"
#include "Layer.h"
#include "Model.h"
#include "TasksSystem.h"

namespace
{
    constexpr _uint MODEL_LOAD_WORKER_COUNT = 4;
    constexpr size_t MODEL_LOAD_BATCH_SIZE = 64;
    constexpr _bool MODEL_LOAD_MULTI_THREAD_ENABLE = true;
}

CFileIO::CFileIO(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : m_pGameInstance { CGameInstance::GetInstance() }
    , m_pDevice{ _pDevice }
    , m_pDeviceContext{ _pDeviceContext }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CFileIO::Initialize()
{
    m_pTasksSystem = CTasksSystem::Create(MODEL_LOAD_WORKER_COUNT);
    if (!m_pTasksSystem)
        return E_FAIL;

    return S_OK;
}

HRESULT CFileIO::Save_MapBinary(_uint _iLevelTotalNum, _uint _iFileIndex)
{
    /* For.MapTool -> Client (Cache 저장) */
    string strFileName = "MAP_LEVEL_" + to_string(_iFileIndex) + ".dat";
    ofstream file("../../Resources/Data/dat/" + strFileName, ios::binary | ios::trunc);
    if (!file.is_open())
        return E_FAIL;

    /* Cache */
    _wstring CachePath = TEXT("../../Resources/Data/Cache/AssetCache.bin");
    Load_AssetCache(CachePath);

    /* LEVEL */
    for (size_t i = 0; i < _iLevelTotalNum; ++i)
    {
        /* Layer */
        for (auto& Pair : m_pGameInstance->Get_Layers()[i])
        {
            const _wstring& wstrLayerTag = Pair.first;

            if (wstrLayerTag.rfind(TEXT("Layer_"), 0) != 0)
                continue;

            /* GameObjects */
            CLayer* pLayer = Pair.second;
            if (!pLayer)
            {
                MSG_BOX("FAILED TO LOAD : Layer 하위 GameObjects");
                continue;
            }

            for (auto& pGameObject : pLayer->Get_GameObjects())
            {
                /* A GameObject */
                if (!pGameObject)
                {
                    MSG_BOX("FAIELD TO LOAD : GameObject");
                    continue;
                }

                const CGameObject::OBJECT_DESC& tObjectDesc = pGameObject->Get_ObjectDesc();
                file.write(reinterpret_cast<const char*>(&tObjectDesc), sizeof(CGameObject::OBJECT_DESC));

                CWorldObject::WORLD_DESC tWorldDesc = {};
                CTransform* pTransform = static_cast<CTransform*>(pGameObject->Get_Component(TEXT("Com_Transform")));
                XMStoreFloat3(&tWorldDesc.vRight, pTransform->Get_State(STATE::RIGHT));
                XMStoreFloat3(&tWorldDesc.vUp, pTransform->Get_State(STATE::UP));
                XMStoreFloat3(&tWorldDesc.vLook, pTransform->Get_State(STATE::LOOK));
                XMStoreFloat3(&tWorldDesc.vPosition, pTransform->Get_State(STATE::POSITION));

                /* Collider */
                _float3 vScale = pTransform->Get_Scaled();
                tWorldDesc.vColliderExtents = _float3(vScale.x, vScale.y, vScale.z);
                tWorldDesc.vColliderCenter = _float3(0.f, 0.f, 0.f);

                file.write(reinterpret_cast<const char*>(&tWorldDesc), sizeof(CWorldObject::WORLD_DESC));

                /* Cache */
                if (wcslen(tObjectDesc.ModelTag) == 0)
                    continue;

                if (m_AssetCache.find(tObjectDesc.ModelTag) == m_AssetCache.end())
                {
                    _wstring MeshPath;
                    if (IsExistContainer(tObjectDesc.ObjectID))
                    {
                        CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
                        _wstring MeshPath = {};
                        Load_ContainerXML(tObjectDesc.iLevel, tObjectDesc.ObjectID, tContainerInitDesc, MeshPath);

                        for (auto& Part : tContainerInitDesc.vecPartsDescs)
                        {
                            if (m_AssetCache.find(Part.tObjectDesc.ModelTag) == m_AssetCache.end())
                            {
                                m_AssetCache.emplace(Part.tObjectDesc.ModelTag, MeshPath);
                            }
                        }
                    }
                    else
                    {
                        if (m_AssetCache.find(tObjectDesc.ModelTag) == m_AssetCache.end())
                        {
                            EXPORT_TAGS tags;
                            if (SUCCEEDED(Parse_XML_TAGS(tObjectDesc.ModelTag, tags)))
                            {
                                m_AssetCache.emplace(tags.ModelTag, tags.MeshPath);
                            }
                        }
                    }
                }
            }
        }
    }

    Write_AssetCache(CachePath);

    file.close();
    MSG_BOX("SUCCESS TO SAVED : .dat + Cache.bin");

    return S_OK;
}

HRESULT CFileIO::Load_ResourceOnly(_uint _iInstanceLevel)
{ 
    /* For.MapTool Init (전체 순회) */
    Load_Full_Prototypes(_iInstanceLevel);
    Bake_AssetCache_FromXML(TEXT(""), TEXT("")); // 일단 fix

    return S_OK;
}

HRESULT CFileIO::Load_InstancesOnly(_uint _iInstanceLevel, _uint _iFileIndex)
{
    /* For.Load in MapTool (Cache 저장) */
    if (FAILED(Load_MapBinary(_iFileIndex)))
        return E_FAIL;

    for (size_t i = 0; i < m_vecInstanceObjectDescs.size(); ++i)
    {
        m_vecInstanceObjectDescs[i].iLevel = _iInstanceLevel;
        Clone_Instance(m_vecInstanceObjectDescs[i], m_vecInstanceWorldDescs[i]);
    }

    return S_OK;
}

HRESULT CFileIO::Load_ResourceAndInstances(_uint _iInstanceLevel, _uint _iFileIndex)
{
    if (!m_pTasksSystem) return E_FAIL;

    /* For.Client (Level 별로 Cache 에서 찾는 구조) */
    _wstring RootPath = TEXT("../../Resources/Models");
    _wstring CachePath = TEXT("../../Resources/Data/Cache/AssetCache.bin");

    if (FAILED(Load_AssetCache(CachePath)))
        return E_FAIL;

    if (FAILED(Load_MapBinary(_iFileIndex)))
        return E_FAIL;

    unordered_set<_wstring> required;
    for (auto& obj : m_vecInstanceObjectDescs)
        required.insert(obj.ModelTag);

    /* Thread */
    vector<MODEL_LOAD_CONTEXT> vecLoadContexts;
    vecLoadContexts.reserve(required.size());
    for (auto& modelTag : required)
    {
        if (modelTag == TEXT("EXCEPTION"))
            continue;

        auto iter = m_AssetCache.find(modelTag);
        if (iter == m_AssetCache.end())
            continue;

        _wstring FullMeshPath = RootPath + TEXT("/") + iter->second;

        MODEL_LOAD_CONTEXT tContext = {};
        tContext.strModelTag = modelTag;
        tContext.strMeshPath = WSTRTOCHAR(FullMeshPath);
        replace(tContext.strMeshPath.begin(), tContext.strMeshPath.end(), '\\', '/');

        /* 참조 횟수 변경을 피하기 위해 CModel 객체는 Main Thread 에서 생성 */
        tContext.pModel = new CModel(m_pDevice, m_pDeviceContext);
        if (!tContext.pModel)
        {
            for (auto& CreatedContext : vecLoadContexts)
                Safe_Release(CreatedContext.pModel);

            return E_FAIL;
        }

        vecLoadContexts.push_back(move(tContext));
    }

    auto ReleaseModels = [&vecLoadContexts]() {
        for (auto& tContext: vecLoadContexts)
            Safe_Release(tContext.pModel);
        };

    /* TEST */
    LARGE_INTEGER Frequency = {};
    LARGE_INTEGER ModelLoadBegin = {};
    LARGE_INTEGER CPULoadEnd = {};
    LARGE_INTEGER ModelLoadEnd = {};

    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&ModelLoadBegin);

    if (MODEL_LOAD_MULTI_THREAD_ENABLE)
    {
        /* 고정 용량 작업 큐가 가득 차지 않도록 일정 개수씩 제출 */
        for (size_t iBatchBegin = 0; iBatchBegin < vecLoadContexts.size(); iBatchBegin += MODEL_LOAD_BATCH_SIZE)
        {
            size_t iBatchEnd = min(iBatchBegin + MODEL_LOAD_BATCH_SIZE, vecLoadContexts.size());

            for (size_t i = iBatchBegin; i < iBatchEnd; ++i)
            {
                CTasksSystem::TASK_DESC tTaskDesc = {};
                tTaskDesc.pFunction = Execute_ModelCPU;
                tTaskDesc.pContext = &vecLoadContexts[i];

                HRESULT hrSubmit = m_pTasksSystem->Submit(tTaskDesc);
                if (FAILED(hrSubmit))
                {
                    /* 이미 제출된 작업이 Context 를 사용 중일 수 있으므로 완료 후 정리 */
                    m_pTasksSystem->Wait_All();
                    ReleaseModels();
                    return hrSubmit;
                }
            }

            /* Worker 가 기록한 CPU 준비 결과를 사용하기 전에 완료를 기다린다 */
            HRESULT hrWait = m_pTasksSystem->Wait_All();
            if (FAILED(hrWait))
            {
                ReleaseModels();
                return hrWait;
            }
        }
    }
    else
    {
        for (auto& tContext : vecLoadContexts)
        {
            HRESULT hrLoad = Execute_ModelCPU(&tContext);
            if (FAILED(hrLoad))
            {
                ReleaseModels();
                return hrLoad;
            }
        }
    }

    QueryPerformanceCounter(&CPULoadEnd);

    /* GPU 생성과 공유 프로토타입 등록은 Loader Thread 에서 순차 처리 */
    for (auto& tContext : vecLoadContexts)
    {
        if (FAILED(tContext.pModel->Initialize_GPU()))
        {
            ReleaseModels();
            return E_FAIL;
        }

        HRESULT hrRegister = Register_PreparedPrototype(_iInstanceLevel, tContext.strModelTag, tContext.pModel);

        /* 등록 함수가 등록 또는 해제를 처리했으므로 이 포인터의 소유권은 더 이상 없다 */
        tContext.pModel = nullptr;

        if (FAILED(hrRegister))
        {
            ReleaseModels();
            return hrRegister;
        }
    }

    QueryPerformanceCounter(&ModelLoadEnd);

    /* TEST */
    const _double fCPULoadTimeMs =
        static_cast<_double>(CPULoadEnd.QuadPart - ModelLoadBegin.QuadPart) * 1000.0 /
        static_cast<_double>(Frequency.QuadPart);

    const _double fModelLoadTimeMs =
        static_cast<_double>(ModelLoadEnd.QuadPart - ModelLoadBegin.QuadPart) * 1000.0 /
        static_cast<_double>(Frequency.QuadPart);

    wchar_t szProfileLog[256] = {};

    swprintf_s(szProfileLog, _countof(szProfileLog),
        L"[Model Load Profile] Mode=%s, Workers=%u, Models=%zu, CPU=%.3f ms, Total=%.3f ms\n",
        MODEL_LOAD_MULTI_THREAD_ENABLE ? L"Parallel" : L"Serial",
        MODEL_LOAD_MULTI_THREAD_ENABLE ? MODEL_LOAD_WORKER_COUNT : 0,
        vecLoadContexts.size(),
        fCPULoadTimeMs,
        fModelLoadTimeMs);

    OutputDebugStringW(szProfileLog);

#ifdef _DEBUG
    for (size_t i = 0; i < vecLoadContexts.size(); ++i)
    {
        wchar_t szLog[256] = {};
        swprintf_s(szLog, _countof(szLog),
            L"[Model CPU Load] Task=%zu, WorkerThreadID=%lu, Tag=%s\n",
            i, vecLoadContexts[i].dwWorkerThreadID, vecLoadContexts[i].strModelTag.c_str());

        OutputDebugStringW(szLog);
    }
#endif // _DEBUG

    return Load_InstancesOnly(_iInstanceLevel, _iFileIndex);
}

HRESULT CFileIO::Load_ContainerXML(_uint _iInstanceLevel, const _wstring& _ContainerID, CContainerObject::CONTAINERR_INIT_DESC& _OutDesc, _wstring _OutMeshPath)
{
    _wstring path = TEXT("../../Resources/Models/Container/") + _ContainerID + TEXT("_CONTAINER.xml");

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(WSTRTOCHAR(path).c_str()) != tinyxml2::XML_SUCCESS)
        return E_FAIL;

    auto* pRoot = doc.FirstChildElement("Container");
    if (!pRoot)
        return E_FAIL;

    /* Container ObjectTag */
    const char* containerID = pRoot->Attribute("id");
    wcscpy_s(_OutDesc.tObjectDesc.ObjectID, CHARTOWSTR(containerID).c_str());
    const char* containerObjectTag = pRoot->Attribute("objectTag");
    wcscpy_s(_OutDesc.tObjectDesc.ObjectTag, CHARTOWSTR(containerObjectTag).c_str());

    /* 기본 Container 정보 */
    _OutDesc.tContainerDesc.iTotalPartsCnt = 0;

    /* Parts */
    for (auto* pPart = pRoot->FirstChildElement("Part"); pPart; pPart = pPart->NextSiblingElement("Part"))
    {
        CPartObject::PART_CREATE_DESC partDesc = {};
        partDesc.ePartType = static_cast<PART>(pPart->IntAttribute("type"));

        /* ObjectID */
        wcscpy_s(partDesc.tObjectDesc.ObjectID, CHARTOWSTR(pPart->Attribute("ObjectID")).c_str());
        wcscpy_s(partDesc.tObjectDesc.LayerTag, CHARTOWSTR(pPart->Attribute("LayerTag")).c_str());
        wcscpy_s(partDesc.tObjectDesc.ObjectTag, CHARTOWSTR(pPart->Attribute("GameObjectTag")).c_str());
        wcscpy_s(partDesc.tObjectDesc.ShaderTag, CHARTOWSTR(pPart->Attribute("ShaderTag")).c_str());
        wcscpy_s(partDesc.tObjectDesc.ModelTag, CHARTOWSTR(pPart->Attribute("ModelTag")).c_str());

        /* Socket */
        if (const char* socket = pPart->Attribute("Socket"))
            partDesc.SocketName = CHARTOWSTR(socket);

        _OutDesc.vecPartsDescs.push_back(partDesc);
        _OutDesc.tContainerDesc.iTotalPartsCnt++;

        /* Add_Prototye */
        const char* MeshPath = pPart->Attribute("MeshPath");
        _wstring RootPath = TEXT("../../Resources/Models");
        _OutMeshPath = RootPath + TEXT("/") + CHARTOWSTR(MeshPath);
        if (MeshPath)
        {
            Register_Prototype(_iInstanceLevel, partDesc.tObjectDesc.ModelTag, _OutMeshPath);
        }
    }

    return S_OK;
}

HRESULT CFileIO::Load_MapBinary(_uint _iFileIndex)
{
    _wstring wstrFullFileName =
        wstring(L"../../Resources/Data/dat/")
        + TEXT("MAP_LEVEL_") + std::to_wstring(_iFileIndex) + TEXT(".dat");

    /* First */
    HANDLE hFile = CreateFile(
        wstrFullFileName.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    DWORD dwFileSize = GetFileSize(hFile, nullptr);
    CloseHandle(hFile);

    const DWORD dwUnitSize = sizeof(CGameObject::OBJECT_DESC) + sizeof(CWorldObject::WORLD_DESC);
    if (dwFileSize == INVALID_FILE_SIZE || dwUnitSize == 0)
        return E_FAIL;

    _uint iCounter = dwFileSize / dwUnitSize;
    if (dwFileSize % dwUnitSize != 0)
    {
        MSG_BOX("FAILED TO READ : SIZE MISMATCH .dat");
        return S_OK;
    }

    m_vecInstanceObjectDescs.resize(iCounter);
    m_vecInstanceWorldDescs.resize(iCounter);

    /* Second */
    hFile = CreateFile(wstrFullFileName.c_str(),
        GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    for (_uint i = 0; i < iCounter; ++i)
    {
        ReadFile(hFile, &m_vecInstanceObjectDescs[i], sizeof(CGameObject::OBJECT_DESC), nullptr, nullptr);
        ReadFile(hFile, &m_vecInstanceWorldDescs[i], sizeof(CWorldObject::WORLD_DESC), nullptr, nullptr);
    }

    CloseHandle(hFile);
    return S_OK;
}

HRESULT CFileIO::Load_Full_Prototypes(_uint _iInstanceLevel)
{
    unordered_set<_wstring> registeredModels;
    _wstring RootPath = TEXT("../../Resources/Models");

    for (const auto& entry : filesystem::recursive_directory_iterator(RootPath))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != L".xml")
            continue;

        if (entry.path().wstring().find(L"_TAGS") == _wstring::npos)
            continue;

        EXPORT_TAGS tags;
        if (FAILED(Parse_XML_TAGS(entry.path().wstring(), tags)))
            continue;

        /* .mesh */
        if (registeredModels.find(tags.ModelTag) != registeredModels.end())
            continue;

        _wstring TAGS_XML_Path = entry.path().wstring(); // 미래를 위해 남겨놓기
        _wstring MeshPath = RootPath + TEXT("/") + tags.MeshPath;
        Register_Prototype(_iInstanceLevel, tags.ModelTag, MeshPath);

        registeredModels.insert(tags.ModelTag);
    }
    return S_OK;
}

HRESULT CFileIO::Load_Prototypes(_uint _iFileIndex, _uint _iInstanceLevel)
{
    Load_MapBinary(_iFileIndex);

    unordered_set<_wstring> requiredModels;
    _wstring RootPath = TEXT("../../Resources/Models");

    for (auto& Object : m_vecInstanceObjectDescs)
        requiredModels.insert(Object.ModelTag);

    for (auto& ModelTag : requiredModels)
    {
        _bool bSearch = { false };

        for (const auto& entry : filesystem::recursive_directory_iterator(RootPath))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != L".xml")
                continue;

            if (entry.path().wstring().find(L"_TAGS") == _wstring::npos)
                continue;

            EXPORT_TAGS tags;
            if (FAILED(Parse_XML_TAGS(entry.path().wstring(), tags)))
                continue;

            if (tags.ModelTag != ModelTag)
                continue;

            _wstring MeshPath = RootPath + TEXT("/") + tags.MeshPath;
            Register_Prototype(_iInstanceLevel, tags.ModelTag, MeshPath);

            bSearch = true;
            break;
        }
    }

    return S_OK;
}

HRESULT CFileIO::Parse_XML_TAGS(_wstring _XMLPath, EXPORT_TAGS& _outTags)
{
    tinyxml2::XMLDocument document;

    _string pathA = WSTRTOCHAR(_XMLPath);
    replace(pathA.begin(), pathA.end(), '\\', '/');

    if (document.LoadFile(pathA.c_str()) != tinyxml2::XML_SUCCESS)
        return E_FAIL;

    tinyxml2::XMLElement* pRoot =
        document.FirstChildElement("GameObject");

    if (!pRoot)
        return E_FAIL;

    auto ReadTag = [&](const char* _tag, _wstring& _out)
        {
            tinyxml2::XMLElement* pElem =
                pRoot->FirstChildElement(_tag);

            if (pElem && pElem->GetText())
                _out = STRTOWSTR(pElem->GetText());
        };

    _wstring wstrModelType;
    ReadTag("ObjectID", _outTags.ObjectID);
    ReadTag("LayerTag", _outTags.LayerTag);
    ReadTag("GameObjectTag", _outTags.GameObjectTag);
    ReadTag("ShaderTag", _outTags.ShaderTag);
    ReadTag("ModelTag", _outTags.ModelTag);
    ReadTag("MeshPath", _outTags.MeshPath);

    return S_OK;
}

HRESULT CFileIO::Save_AssetCache(_uint _iInstanceLevel)
{
    m_AssetCache.clear();

    _wstring CachePath = TEXT("../../Resources/Data/Cache/AssetCache.bin");

    /* 1. Prototype GameObjects */
    vector<_wstring> vecTags = {};
    m_pGameInstance->Get_PrototypeTag(_iInstanceLevel, vecTags);

    // ModelTag <-> MeshPath


    /* 2. Cache 저장 */
    Write_AssetCache(CachePath);

    MSG_BOX("SUCCESS TO SAVED : AssetCache.bin (Prototype 기준)");
    return S_OK;
}

HRESULT CFileIO::Register_EffectMeshes(_uint _iInstanceLevel)
{
    _wstring RootPath = TEXT("../../Resources/Models/Effects");

    for (const auto& entry : filesystem::recursive_directory_iterator(RootPath))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != L".mesh")
            continue;

        /* Mesh 파일 */
        _wstring meshFile = entry.path().filename().wstring();
        _wstring meshPath = entry.path().wstring();

        size_t pos = meshFile.find_last_of(L'.');
        if (pos != _wstring::npos)
            meshFile = meshFile.substr(0, pos);

        /* ModelTag 생성 */
        _wstring modelTag = TEXT("Prototype_Component_Model_Map_") + meshFile;

        Register_Prototype(_iInstanceLevel, modelTag, meshPath);
    }

    return S_OK;
}

HRESULT CFileIO::Register_EffectTextures(_uint _iInstanceLevel)
{
    _wstring RootPath = TEXT("../../Resources/Textures2D/Effects");

    for (const auto& entry : filesystem::recursive_directory_iterator(RootPath))
    {
        if (!entry.is_regular_file())
            continue;

        _wstring filePath = entry.path().wstring();
        _wstring ext = entry.path().extension().wstring();

        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

        /* 확장자 */
        if (ext != L".dds" && ext != L".png")
            continue;

        /* 파일 이름 */
        _wstring fileName = entry.path().stem().wstring();

        /* Prototype Tag */
        _wstring prototypeTag = L"Prototype_Component_Texture_" + fileName;
        _wstring texturePath = L"../../Resources/Textures2D/Effects/" + fileName + L".dds";

        /* Prototype 등록 */
        if (FAILED(m_pGameInstance->Add_Prototype(
            RESOURCE_LEVEL_STATIC,
            prototypeTag,
            CTexture::Create(m_pDevice, m_pDeviceContext,
                TEXT("../../Resources/Textures2D/Effects/Dissolve_00.dds"), 1)))) //TOOD : 수정해야함 위에 texturePath
        {
            continue;
        }

    }

    return S_OK;
}

HRESULT CFileIO::Save_EffectXML(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Textures)
{
    /* Path */
    wstring filePath = L"../../Resources/Data/Effect/" + _Param.EffectName + L"_Effect.xml";
    string path = WSTRTOCHAR(filePath);

    tinyxml2::XMLDocument doc;

    /* Root */
    tinyxml2::XMLElement* root = doc.NewElement("Effect");
    root->SetAttribute("Name", WSTRTOCHAR(_Param.EffectName).c_str());
    doc.InsertFirstChild(root);

    /* ========================= */
    /* Textures */
    /* ========================= */

    tinyxml2::XMLElement* texNode = doc.NewElement("Textures");

    auto AddTexture = [&](const char* name, const wstring& value)
        {
            tinyxml2::XMLElement* elem = doc.NewElement(name);
            elem->SetText(WSTRTOCHAR(value).c_str());
            texNode->InsertEndChild(elem);
        };

    AddTexture("Main", _Textures.MainTexture);
    AddTexture("Noise0", _Textures.Noise_0);
    AddTexture("Noise1", _Textures.Noise_1);
    AddTexture("Mask", _Textures.Mask);
    AddTexture("Distortion", _Textures.Distortion);
    AddTexture("Dissolve", _Textures.Dissolve);

    root->InsertEndChild(texNode);

    /* ========================= */
    /* Shader Params */
    /* ========================= */

    tinyxml2::XMLElement* shaderNode = doc.NewElement("ShaderParams");

    shaderNode->SetAttribute("Alpha", _Param.fAlpha);
    shaderNode->SetAttribute("Emissive", _Param.fEmissive);
    shaderNode->SetAttribute("Duration", _Param.fDuration);

    shaderNode->SetAttribute("UVScaleX", _Param.vUVScale.x);
    shaderNode->SetAttribute("UVScaleY", _Param.vUVScale.y);

    shaderNode->SetAttribute("UVSpeedX", _Param.vUVSpeed.x);
    shaderNode->SetAttribute("UVSpeedY", _Param.vUVSpeed.y);

    shaderNode->SetAttribute("Noise0ScaleX", _Param.vNoiseScale_0.x);
    shaderNode->SetAttribute("Noise0ScaleY", _Param.vNoiseScale_0.y);

    shaderNode->SetAttribute("Noise0SpeedX", _Param.vNoiseSpeed_0.x);
    shaderNode->SetAttribute("Noise0SpeedY", _Param.vNoiseSpeed_0.y);

    shaderNode->SetAttribute("Noise1ScaleX", _Param.vNoiseScale_1.x);
    shaderNode->SetAttribute("Noise1ScaleY", _Param.vNoiseScale_1.y);

    shaderNode->SetAttribute("Noise1SpeedX", _Param.vNoiseSpeed_1.x);
    shaderNode->SetAttribute("Noise1SpeedY", _Param.vNoiseSpeed_1.y);

    shaderNode->SetAttribute("Distortion", _Param.fDistortion);
    shaderNode->SetAttribute("DistortionDirX", _Param.vDistortionDir.x);
    shaderNode->SetAttribute("DistortionDirY", _Param.vDistortionDir.y);
    shaderNode->SetAttribute("DistortionSpeed", _Param.fDistortionSpeed);

    shaderNode->SetAttribute("Dissolve", _Param.fDissolve);
    shaderNode->SetAttribute("DissolveEdge", _Param.fDissolveEdge);

    shaderNode->SetAttribute("FrameX", _Param.iFrameX);
    shaderNode->SetAttribute("FrameY", _Param.iFrameY);
    shaderNode->SetAttribute("FrameSpeed", _Param.fFrameSpeed);

    /* Color */
    shaderNode->SetAttribute("ColorR", _Param.vColor.x);
    shaderNode->SetAttribute("ColorG", _Param.vColor.y);
    shaderNode->SetAttribute("ColorB", _Param.vColor.z);
    shaderNode->SetAttribute("ColorA", _Param.vColor.w);

    /* Dissolve Color */
    shaderNode->SetAttribute("DissolveColorR", _Param.vDissolveColor.x);
    shaderNode->SetAttribute("DissolveColorG", _Param.vDissolveColor.y);
    shaderNode->SetAttribute("DissolveColorB", _Param.vDissolveColor.z);
    shaderNode->SetAttribute("DissolveColorA", _Param.vDissolveColor.w);

    root->InsertEndChild(shaderNode);

    /* ========================= */
    /* Timeline */
    /* ========================= */

    tinyxml2::XMLElement* timelineNode = doc.NewElement("Timeline");

    timelineNode->SetAttribute("ScaleStartX", _Timeline.vScaleStart.x);
    timelineNode->SetAttribute("ScaleStartY", _Timeline.vScaleStart.y);
    timelineNode->SetAttribute("ScaleStartZ", _Timeline.vScaleStart.z);

    timelineNode->SetAttribute("ScaleEndX", _Timeline.vScaleEnd.x);
    timelineNode->SetAttribute("ScaleEndY", _Timeline.vScaleEnd.y);
    timelineNode->SetAttribute("ScaleEndZ", _Timeline.vScaleEnd.z);

    timelineNode->SetAttribute("RotationAxisX", _Timeline.vRotationAxis.x);
    timelineNode->SetAttribute("RotationAxisY", _Timeline.vRotationAxis.y);
    timelineNode->SetAttribute("RotationAxisZ", _Timeline.vRotationAxis.z);

    timelineNode->SetAttribute("RotationSpeed", _Timeline.fRotationSpeed);

    timelineNode->SetAttribute("AlphaStart", _Timeline.fAlphaStart);
    timelineNode->SetAttribute("AlphaEnd", _Timeline.fAlphaEnd);

    timelineNode->SetAttribute("DissolveStart", _Timeline.fDissolveStart);
    timelineNode->SetAttribute("DissolveEnd", _Timeline.fDissolveEnd);

    root->InsertEndChild(timelineNode);

    /* Save */
    doc.SaveFile(path.c_str());

    return S_OK;
}

HRESULT CFileIO::Load_EffectXML(_wstring _EffectName, SHADER_PARAM_DESC& _OutParam, SHADER_TIMELINE& _OutTimeline, SHADER_TEXTURE_DESC& _OutTextures)
{
    wstring filePath = L"../../Resources/Data/Effect/" + _EffectName + L"_Effect.xml";
    string path = WSTRTOCHAR(filePath);

    tinyxml2::XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
        return E_FAIL;

    tinyxml2::XMLElement* root = doc.FirstChildElement("Effect");
    if (!root)
        return E_FAIL;

    /* Textures */
    tinyxml2::XMLElement* texNode = root->FirstChildElement("Textures");

    auto GetTexture = [&](const char* name, wstring& out)
        {
            auto elem = texNode->FirstChildElement(name);
            if (elem && elem->GetText())
                out = CHARTOWSTR(elem->GetText());
        };

    GetTexture("Main", _OutTextures.MainTexture);
    GetTexture("Noise0", _OutTextures.Noise_0);
    GetTexture("Noise1", _OutTextures.Noise_1);
    GetTexture("Mask", _OutTextures.Mask);
    GetTexture("Distortion", _OutTextures.Distortion);
    GetTexture("Dissolve", _OutTextures.Dissolve);

    /* Shader Params */
    auto shaderNode = root->FirstChildElement("ShaderParams");

    shaderNode->QueryFloatAttribute("Alpha", &_OutParam.fAlpha);
    shaderNode->QueryFloatAttribute("Emissive", &_OutParam.fEmissive);
    shaderNode->QueryFloatAttribute("Duration", &_OutParam.fDuration);

    shaderNode->QueryFloatAttribute("UVScaleX", &_OutParam.vUVScale.x);
    shaderNode->QueryFloatAttribute("UVScaleY", &_OutParam.vUVScale.y);

    shaderNode->QueryFloatAttribute("UVSpeedX", &_OutParam.vUVSpeed.x);
    shaderNode->QueryFloatAttribute("UVSpeedY", &_OutParam.vUVSpeed.y);

    shaderNode->QueryFloatAttribute("Noise0ScaleX", &_OutParam.vNoiseScale_0.x);
    shaderNode->QueryFloatAttribute("Noise0ScaleY", &_OutParam.vNoiseScale_0.y);

    shaderNode->QueryFloatAttribute("Noise0SpeedX", &_OutParam.vNoiseSpeed_0.x);
    shaderNode->QueryFloatAttribute("Noise0SpeedY", &_OutParam.vNoiseSpeed_0.y);

    shaderNode->QueryFloatAttribute("Noise1ScaleX", &_OutParam.vNoiseScale_1.x);
    shaderNode->QueryFloatAttribute("Noise1ScaleY", &_OutParam.vNoiseScale_1.y);

    shaderNode->QueryFloatAttribute("Noise1SpeedX", &_OutParam.vNoiseSpeed_1.x);
    shaderNode->QueryFloatAttribute("Noise1SpeedY", &_OutParam.vNoiseSpeed_1.y);

    shaderNode->QueryFloatAttribute("Distortion", &_OutParam.fDistortion);
    shaderNode->QueryFloatAttribute("DistortionDirX", &_OutParam.vDistortionDir.x);
    shaderNode->QueryFloatAttribute("DistortionDirY", &_OutParam.vDistortionDir.y);
    shaderNode->QueryFloatAttribute("DistortionSpeed", &_OutParam.fDistortionSpeed);

    shaderNode->QueryFloatAttribute("Dissolve", &_OutParam.fDissolve);
    shaderNode->QueryFloatAttribute("DissolveEdge", &_OutParam.fDissolveEdge);

    /* Color */

    shaderNode->QueryFloatAttribute("ColorR", &_OutParam.vColor.x);
    shaderNode->QueryFloatAttribute("ColorG", &_OutParam.vColor.y);
    shaderNode->QueryFloatAttribute("ColorB", &_OutParam.vColor.z);
    shaderNode->QueryFloatAttribute("ColorA", &_OutParam.vColor.w);

    shaderNode->QueryFloatAttribute("DissolveColorR", &_OutParam.vDissolveColor.x);
    shaderNode->QueryFloatAttribute("DissolveColorG", &_OutParam.vDissolveColor.y);
    shaderNode->QueryFloatAttribute("DissolveColorB", &_OutParam.vDissolveColor.z);
    shaderNode->QueryFloatAttribute("DissolveColorA", &_OutParam.vDissolveColor.w);

    /* Timeline */
    auto timelineNode = root->FirstChildElement("Timeline");

    timelineNode->QueryFloatAttribute("ScaleStartX", &_OutTimeline.vScaleStart.x);
    timelineNode->QueryFloatAttribute("ScaleStartY", &_OutTimeline.vScaleStart.y);
    timelineNode->QueryFloatAttribute("ScaleStartZ", &_OutTimeline.vScaleStart.z);

    timelineNode->QueryFloatAttribute("ScaleEndX", &_OutTimeline.vScaleEnd.x);
    timelineNode->QueryFloatAttribute("ScaleEndY", &_OutTimeline.vScaleEnd.y);
    timelineNode->QueryFloatAttribute("ScaleEndZ", &_OutTimeline.vScaleEnd.z);

    timelineNode->QueryFloatAttribute("RotationAxisX", &_OutTimeline.vRotationAxis.x);
    timelineNode->QueryFloatAttribute("RotationAxisY", &_OutTimeline.vRotationAxis.y);
    timelineNode->QueryFloatAttribute("RotationAxisZ", &_OutTimeline.vRotationAxis.z);

    timelineNode->QueryFloatAttribute("RotationSpeed", &_OutTimeline.fRotationSpeed);

    timelineNode->QueryFloatAttribute("AlphaStart", &_OutTimeline.fAlphaStart);
    timelineNode->QueryFloatAttribute("AlphaEnd", &_OutTimeline.fAlphaEnd);

    timelineNode->QueryFloatAttribute("DissolveStart", &_OutTimeline.fDissolveStart);
    timelineNode->QueryFloatAttribute("DissolveEnd", &_OutTimeline.fDissolveEnd);

    return S_OK;
}

HRESULT CFileIO::Load_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex)
{
    string strFileName = "MAP_LIGHT_" + to_string(_iFileIndex) + ".dat";
    ifstream file("../../Resources/Data/dat/" + strFileName, ios::binary);

    if (!file.is_open())
        return E_FAIL;

    /* 기존 Light 초기화 */
    m_pGameInstance->Clear_Light();

    /* 1. Count */
    _uint iLightCount = 0;
    file.read(reinterpret_cast<char*>(&iLightCount), sizeof(_uint));

    if (iLightCount == 0)
    {
        file.close();
        return S_OK;
    }

    /* 2. Light 데이터 */
    for (_uint i = 0; i < iLightCount; ++i)
    {
        LIGHT_DESC desc = {};
        file.read(reinterpret_cast<char*>(&desc), sizeof(LIGHT_DESC));

        /* Engine Light 추가 */
        if (FAILED(m_pGameInstance->Add_Light(desc)))
        {
            file.close();
            return E_FAIL;
        }
    }

    file.close();

    return S_OK;
}

HRESULT CFileIO::Clone_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex)
{
    string strFileName = "MAP_LIGHT_" + to_string(_iFileIndex) + ".dat";
    ifstream file("../../Resources/Data/dat/" + strFileName, ios::binary);

    if (!file.is_open())
        return E_FAIL;
        
    /* Count */
    _uint iLightCount = 0;
    file.read(reinterpret_cast<char*>(&iLightCount), sizeof(_uint));

    if (iLightCount == 0)
    {
        file.close();
        return S_OK;
    }

    /* Light 생성 */
    for (_uint i = 0; i < iLightCount; ++i)
    {
        LIGHT_DESC desc = {};
        file.read(reinterpret_cast<char*>(&desc), sizeof(LIGHT_DESC));

        LIGHT_INIT_DESC tInitDesc = {};
        SET_DESC(tInitDesc.tObjectDesc.ObjectID, TEXT("Light"));
        tInitDesc.tLightDesc = desc;

        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            RESOURCE_LEVEL_STATIC,
            TEXT("Prototype_GameObject_MapObject_Light"),
            _iInstanceLevel,
            TEXT("Light"),
            &tInitDesc, nullptr)))
        {
            file.close();
            return E_FAIL;
        }

    }

    file.close();

    return S_OK;
}

HRESULT CFileIO::Register_Prototype(_uint _iInstanceLevel, _wstring _ModelTag, _wstring _MeshPath)
{
    if (_MeshPath.empty())
        return E_FAIL;

    /* Excep. */
    if (_ModelTag == TEXT("EXCEPTION"))
        return S_FALSE;

    _string meshPath = WSTRTOCHAR(_MeshPath);
    replace(meshPath.begin(), meshPath.end(), '\\', '/');

    /* Create Model */
    CModel* pModel = CModel::Create(
        m_pDevice,
        m_pDeviceContext,
        meshPath.c_str());

    if (!pModel)
        return E_FAIL;

    return Register_PreparedPrototype(_iInstanceLevel, _ModelTag, pModel);
}

/* CPU 준비 + GPU 생성 이 끝난 모델을 메인 스레드에서 프로토 타입 컨테이너에 등록 */
HRESULT CFileIO::Register_PreparedPrototype(_uint _iInstanceLevel, const _wstring& _ModelTag, CModel* _pModel)
{
    /* Static Batching : Field cell models only */
    const _bool bIsFieldModel =
        _ModelTag.rfind(
            TEXT("Prototype_Component_Model_Map_Field_"),
            0) == 0;

    if (ENABLE_FIELD_STATIC_BATCHING && bIsFieldModel)
    {
        HRESULT hrBatch = _pModel->Build_StaticBatches();

        if (FAILED(hrBatch))
        {
            Safe_Release(_pModel);
            return E_FAIL;
        }
    }

    HRESULT hr = m_pGameInstance->Add_Prototype(
        _iInstanceLevel,
        _ModelTag.c_str(),
        _pModel);

    if (FAILED(hr))
    {
        Safe_Release(_pModel);
        return S_OK;
    }

    return S_OK;
}

HRESULT CFileIO::Clone_Instance(CGameObject::OBJECT_DESC _tObjectDesc, CWorldObject::WORLD_DESC _tWorldDesc)
{
    CLONE_DESC tDesc = {};
    tDesc.tObjectDesc = _tObjectDesc;
    tDesc.tWorldDesc = _tWorldDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        _tObjectDesc.ObjectTag,
        _tObjectDesc.iLevel,
        _tObjectDesc.LayerTag,
        &tDesc, &pGameObject)))
        return E_FAIL;

    // TODO : deprecated
    // CTransform* pTransform = static_cast<CTransform*>(pGameObject->Get_Component(TEXT("Com_Transform")));
    // if (!pTransform)
    //     return E_FAIL;
    // pTransform->Set_State(STATE::RIGHT, XMVectorSet(_tWorldDesc.vRight.x, _tWorldDesc.vRight.y, _tWorldDesc.vRight.z, 0.f));
    // pTransform->Set_State(STATE::UP, XMVectorSet(_tWorldDesc.vUp.x, _tWorldDesc.vUp.y, _tWorldDesc.vUp.z, 0.f));
    // pTransform->Set_State(STATE::LOOK, XMVectorSet(_tWorldDesc.vLook.x, _tWorldDesc.vLook.y, _tWorldDesc.vLook.z, 0.f));
    // pTransform->Set_State(STATE::POSITION, XMVectorSet(_tWorldDesc.vPosition.x, _tWorldDesc.vPosition.y, _tWorldDesc.vPosition.z, 1.f));

    return S_OK;
}

HRESULT CFileIO::Clone_Instance_Container(_uint _iInstanceLevel, CContainerObject::CONTAINERR_INIT_DESC _ContainerTag, CGameObject** _ppOut)
{
    /* Container Object */
    EXPORT_PART_TAGS tags = {};
    tags.ObjectID = _ContainerTag.tObjectDesc.ObjectID;
    tags.GameObjectTag = _ContainerTag.tObjectDesc.ObjectTag;
    tags.LayerTag = TEXT("Layer_Actor");

    /* Each Level Set */
    _ContainerTag.tObjectDesc.iLevel = _iInstanceLevel;
    for (auto& Part : _ContainerTag.vecPartsDescs)
    {
        Part.tObjectDesc.iLevel = _iInstanceLevel;
    }

    /* Clone */
    CGameObject* pGameObject = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tags.GameObjectTag,
        _iInstanceLevel,
        tags.LayerTag,
        &_ContainerTag,
        &pGameObject)))
        return E_FAIL;

    if (_ppOut != nullptr)
        *_ppOut = pGameObject;

    return S_OK;
}

HRESULT CFileIO::Bake_AssetCache_FromXML(_wstring _RootBinPath, _wstring _OutBinPath)
{
    /* 일단 임시 */
    _RootBinPath = TEXT("../../Resources/Models");
    _OutBinPath = TEXT("../../Resources/Data/Cache/AssetCache.bin");

    m_AssetCache.clear();

    for (const auto& entry : filesystem::recursive_directory_iterator(_RootBinPath))
    {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();
        if (path.extension() != L".xml")
            continue;

        if (path.wstring().find(L"_TAGS") == _wstring::npos)
            continue;

        EXPORT_TAGS tags;
        if (FAILED(Parse_XML_TAGS(path.wstring(), tags)))
            continue;

        if (tags.ModelTag.empty() || tags.MeshPath.empty())
            continue;

        m_AssetCache[tags.ModelTag] = tags.MeshPath;
    }

    return Write_AssetCache(_OutBinPath);
}

HRESULT CFileIO::Load_AssetCache(_wstring _BinPath)
{
    m_AssetCache.clear();

    return Read_AssetCache(_BinPath);
}

HRESULT CFileIO::Write_AssetCache(_wstring _OutBinPath)
{
    filesystem::create_directories(filesystem::path(_OutBinPath).parent_path());

    _string OutPath = WSTRTOCHAR(_OutBinPath);
    replace(OutPath.begin(), OutPath.end(), '\\', '/');

    ofstream file(OutPath, ios::binary | ios::trunc);
    if (!file.is_open())
        return E_FAIL;

    _uint iCount = (_uint)m_AssetCache.size();
    file.write((_char*)&iCount, sizeof(_uint));

    for (auto& [ModelTag, MeshPath] : m_AssetCache)
    {
        _string _modelTag = WSTRTOCHAR(ModelTag);
        _string _pathTag = WSTRTOCHAR(MeshPath);

        _uint iModelTagLength = (_uint)_modelTag.size();
        _uint iPathTagLength = (_uint)_pathTag.size();

        file.write((_char*)&iModelTagLength, sizeof(_uint));
        file.write(_modelTag.c_str(), iModelTagLength);

        file.write((_char*)&iPathTagLength, sizeof(_uint));
        file.write(_pathTag.c_str(), iPathTagLength);
    }

    file.close();
    return S_OK;
}

HRESULT CFileIO::Read_AssetCache(_wstring _BinPath)
{
    HANDLE hFile = CreateFileW(
        _BinPath.c_str(),
        GENERIC_READ,
        0, nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    DWORD read = 0;
    _uint count = 0;
    ReadFile(hFile, &count, sizeof(_uint), &read, nullptr);

    for (_uint i = 0; i < count; ++i)
    {
        _uint lenTag = 0, lenPath = 0;
        ReadFile(hFile, &lenTag, sizeof(_uint), &read, nullptr);

        string tag(lenTag, '\0');
        ReadFile(hFile, tag.data(), lenTag, &read, nullptr);

        ReadFile(hFile, &lenPath, sizeof(_uint), &read, nullptr);
        string path(lenPath, '\0');
        ReadFile(hFile, path.data(), lenPath, &read, nullptr);

        m_AssetCache[STRTOWSTR(tag.c_str())] = STRTOWSTR(path.c_str());
    }

    CloseHandle(hFile);
    return S_OK;
}

_bool CFileIO::IsExistContainer(_wstring _ObjectID)
{
    _wstring path = TEXT("../../Resources/Models/Container/") + _ObjectID + TEXT("_CONTAINER.xml");
    return filesystem::exists(path);
}

HRESULT CFileIO::Execute_ModelCPU(void* _pContext)
{
    if (!_pContext)
        return E_INVALIDARG;

    MODEL_LOAD_CONTEXT* pContext = static_cast<MODEL_LOAD_CONTEXT*>(_pContext);
    if (!pContext->pModel || pContext->strMeshPath.empty())
        return E_INVALIDARG;

#ifdef _DEBUG
    pContext->dwWorkerThreadID = GetCurrentThreadId();
#endif // _DEBUG

    pContext->hrResult = pContext->pModel->Initialize_CPU(pContext->strMeshPath.c_str(), XMMatrixIdentity());

    return pContext->hrResult;
}

CFileIO* CFileIO::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CFileIO* pInstance = new CFileIO(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CFileIO");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CFileIO::Free()
{
    Safe_Release(m_pTasksSystem);   // Worker 가 참조할 수 있는 Device 와 GameInstance 보다 작업 시스템의 수명이 먼저 끝나야 함

	__super::Free();

    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
