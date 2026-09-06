#include "PrototypeManager.h"

#include "GameObject.h"
#include "Component.h"

CPrototypeManager::CPrototypeManager()
{
}

HRESULT CPrototypeManager::Initailize(_uint _iTotalLevelCnt)
{
    m_Prototypes = new map<const _wstring, class CBase*>[_iTotalLevelCnt];
    
    m_iTotalLevelCnt = _iTotalLevelCnt;

    return S_OK;
}

void CPrototypeManager::Clear(_uint _iLevelIndex)
{
    if (_iLevelIndex >= m_iTotalLevelCnt)
        return;

    for (auto& Pair : m_Prototypes[_iLevelIndex])
        Safe_Release(Pair.second);

    m_Prototypes[_iLevelIndex].clear();
}

HRESULT CPrototypeManager::Add_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag, CBase* _pPrototype)
{
    if (nullptr != Find_Prototype(_iLevelIndex, _strPrototypeTag))
        return E_FAIL;

    m_Prototypes[_iLevelIndex].emplace(_strPrototypeTag, _pPrototype);

    return S_OK;
}

CBase* CPrototypeManager::Clone_Prototype(PROTOTYPE _ePrototype, _uint _iLevelIndex, const _wstring& _strPrototypeTag, void* _pArg)
{
    CBase* pPrototpye = Find_Prototype(_iLevelIndex, _strPrototypeTag);
    if (nullptr == pPrototpye)
        return nullptr;

    CBase* pCloneObject = { nullptr };

    switch (_ePrototype)
    {
    case Engine::PROTOTYPE::GAMEOBJECT:
    {
        pCloneObject = static_cast<CGameObject*>(pPrototpye)->Clone(_pArg);
        break;
    }
    case Engine::PROTOTYPE::COMPONENT:
    {
        pCloneObject = static_cast<CComponent*>(pPrototpye)->Clone(_pArg);
        break;
    }
    }
    if (nullptr == pCloneObject)
        return nullptr;
    
    return pCloneObject;
}

void CPrototypeManager::Get_PrototypeTag(_uint _iLevelIndex, vector<_wstring>& _outTags) const
{
    _outTags.clear();

    //if (_iLevelIndex >= m_Prototypes->size())
    //    return;

    _outTags.reserve(m_Prototypes[_iLevelIndex].size());

    for (const auto& Pair : m_Prototypes[_iLevelIndex])
        _outTags.push_back(Pair.first);
}

CBase* CPrototypeManager::Find_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag)
{
    if (_iLevelIndex >= m_iTotalLevelCnt)
        return nullptr;

    auto iter = m_Prototypes[_iLevelIndex].find(_strPrototypeTag);
    if (iter == m_Prototypes[_iLevelIndex].end())
        return nullptr;

    return iter->second;
}

CPrototypeManager* CPrototypeManager::Create(_uint _iTotalLevelCnt)
{
    CPrototypeManager* pInstance = new CPrototypeManager();
    if (FAILED(pInstance->Initailize(_iTotalLevelCnt)))
    {
        MSG_BOX("FAILED TO CREATED : CPrototypeManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CPrototypeManager::Free()
{
    __super::Free();

    /* map[] release */
    for (size_t i = 0; i < m_iTotalLevelCnt; ++i)
    {
        for (auto& Pair : m_Prototypes[i])
            Safe_Release(Pair.second);
        m_Prototypes[i].clear();
    }
    Safe_Delete_Array(m_Prototypes);
}
