#include "ObjectManager.h"

#include "GameInstance.h"
#include "Layer.h"

CObjectManager::CObjectManager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CObjectManager::Initialize(_uint _iTotalLevelCnt)
{
    m_Layers = new map<const _wstring, CLayer*>[_iTotalLevelCnt];

    m_iTotalLevelCnt = _iTotalLevelCnt;

    return S_OK;
}

void CObjectManager::Priority_Update(_float _fTimeDelta)
{
    for (size_t i = 0; i < m_iTotalLevelCnt; i++)
    {
        for (auto& Pair : m_Layers[i])
            Pair.second->Priority_Update(_fTimeDelta);
    }
}

void CObjectManager::Update(_float _fTimeDelta)
{
    for (size_t i = 0; i < m_iTotalLevelCnt; i++)
    {
        for (auto& Pair : m_Layers[i])
            Pair.second->Update(_fTimeDelta);
    }
}

void CObjectManager::Late_Update(_float _fTimeDelta)
{
    for (size_t i = 0; i < m_iTotalLevelCnt; i++)
    {
        for (auto& Pair : m_Layers[i])
            Pair.second->Late_Update(_fTimeDelta);
    }
}

void CObjectManager::Clear(_uint _iLevelIndex)
{
    if (_iLevelIndex >= m_iTotalLevelCnt)
        return;

    for (auto& Pair : m_Layers[_iLevelIndex])
        Safe_Release(Pair.second);

    m_Layers[_iLevelIndex].clear();
}

HRESULT CObjectManager::Add_GameObject_ToLayer(_uint _iPrototypeLevelIndex, const _wstring& _strPrototypeTag, _uint _iLayerLevelIndex, const _wstring& _strLayerTag, void* _pArg, CGameObject** ppOut)
{
    if (_iLayerLevelIndex >= m_iTotalLevelCnt)
        return E_FAIL;

    CGameObject* pGameObject = dynamic_cast<CGameObject*>(
        m_pGameInstance->Clone_Prototype(
            PROTOTYPE::GAMEOBJECT,
            _iPrototypeLevelIndex,
            _strPrototypeTag,
            _pArg));
    if (nullptr == pGameObject)
        return E_FAIL;

    if (ppOut != nullptr)
        *ppOut = pGameObject;

    CLayer* pLayer = Find_Layer(_iLayerLevelIndex, _strLayerTag);
    if (nullptr == pLayer)
    {
        pLayer = CLayer::Create();
        pLayer->Add_GameObject(pGameObject);
        m_Layers[_iLayerLevelIndex].emplace(_strLayerTag, pLayer);
    }
    else
    {
        pLayer->Add_GameObject(pGameObject);
    }


    /* Register Table */
    if (pGameObject)
    {
        m_ObjectTable[pGameObject->Get_ObjectID()] = pGameObject;
    }

    return S_OK;
}

HRESULT CObjectManager::Reserve_DeleteObject(CGameObject* _pGameObject)
{
    if (nullptr == _pGameObject)
        return S_OK;

    /* Return if duplicate */
    if (std::find(m_vecDeleteObjects.begin(), m_vecDeleteObjects.end(), _pGameObject) != m_vecDeleteObjects.end())
        return S_OK;

    m_vecDeleteObjects.push_back(_pGameObject);

    /* Remove Table */
    if (_pGameObject)
    {
        m_ObjectTable.erase(_pGameObject->Get_ObjectID());
    }

    return S_OK;
}

CGameObject* CObjectManager::Find_Object(const _wstring& _strObjectID)
{
    auto iter = m_ObjectTable.find(_strObjectID);

    if (iter == m_ObjectTable.end())
        return nullptr;

    return iter->second;
}

void CObjectManager::Delete_Update()
{
    if (m_vecDeleteObjects.empty())
        return;

    for (auto& pObject : m_vecDeleteObjects)
    {
        Delete_GameObject_ToLayer(pObject);
    }
    m_vecDeleteObjects.clear();
}

HRESULT CObjectManager::Delete_GameObject_ToLayer(CGameObject* _pTarget)
{
    if (!_pTarget)
        return E_FAIL;

    _wstring wstrSavedObjectID;

    try
    {
        wstrSavedObjectID = _pTarget->Get_ObjectID();
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }

    if (wstrSavedObjectID.empty() || wstrSavedObjectID.length() > 256)
        return E_FAIL;

    /* 0. Level º° */
    for (_uint i = 0; i < m_iTotalLevelCnt; i++)
    {
        /* 1. LayerTag º° */
        for (auto& pLayer : m_Layers[i])
        {
            pLayer.second->Remove_GameObject(wstrSavedObjectID);
        }
    }

    return S_OK;
}

CComponent* CObjectManager::Get_Component(_uint _iLevelIndex, const _wstring& _strLayerTag, const _wstring& _strComponentTag, _uint _iIndex)
{
    CLayer* pLayer = Find_Layer(_iLevelIndex, _strLayerTag);
    if (nullptr == pLayer)
        return nullptr;

    return pLayer->Get_Component(_strComponentTag, _iIndex);
}

CLayer* CObjectManager::Find_Layer(_uint _iLevelIndex, const _wstring& _strLayerTag)
{
    if (_iLevelIndex >= m_iTotalLevelCnt)
        return nullptr;

    auto iter = m_Layers[_iLevelIndex].find(_strLayerTag);
    if (iter == m_Layers[_iLevelIndex].end())
        return nullptr;

    return iter->second;
}

CObjectManager* CObjectManager::Create(_uint _iTotalLevelCnt)
{
    CObjectManager* pInstance = new CObjectManager();
    if (FAILED(pInstance->Initialize(_iTotalLevelCnt)))
    {
        MSG_BOX("FAILED TO CREATED : CObjectManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CObjectManager::Free()
{
    __super::Free();

    /* map[] release */
    for (size_t i = 0; i < m_iTotalLevelCnt; ++i)
    {
        for (auto& Pair : m_Layers[i])
            Safe_Release(Pair.second);
        m_Layers[i].clear();
    }
    Safe_Delete_Array(m_Layers);

    Safe_Release(m_pGameInstance);
}
