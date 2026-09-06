#include "Layer.h"

CLayer::CLayer()
{
}

void CLayer::Priority_Update(_float _fTimeDelta)
{
    for (auto& pGameObject : m_GameObjectList)
    {
        if (nullptr != pGameObject)
            pGameObject->Priority_Update(_fTimeDelta);
    }
}

void CLayer::Update(_float _fTimeDelta)
{
    for (auto& pGameObject : m_GameObjectList)
    {
        if (nullptr != pGameObject)
            pGameObject->Update(_fTimeDelta);
    }
}

void CLayer::Late_Update(_float _fTimeDelta)
{
    for (auto& pGameObject : m_GameObjectList)
    {
        if (nullptr != pGameObject)
            pGameObject->Late_Update(_fTimeDelta);
    }
}

HRESULT CLayer::Add_GameObject(CGameObject* _pGameObject)
{
    if (nullptr == _pGameObject)
        return E_FAIL;

    m_GameObjectList.push_back(_pGameObject);
    
    return S_OK;
}

HRESULT CLayer::Find_GameObject(const _wstring& _strObjectID, CGameObject** _ppOut)
{
    auto iter = find_if(m_GameObjectList.begin(), m_GameObjectList.end(), [&](CGameObject* pObj)
        {
            return pObj && pObj->Compare_ObjectID(_strObjectID);
        });

    if (iter == m_GameObjectList.end())
        return E_FAIL;

    *_ppOut = *iter;

    return S_OK;
}

HRESULT CLayer::Remove_GameObject(const _wstring& _wstrObjectID)
{
    if (m_GameObjectList.empty())
        return S_OK;

    m_GameObjectList.remove_if([&](CGameObject* pObject)
        {
            if (pObject->Compare_ObjectID(_wstrObjectID))
            {
                Safe_Release(pObject);
                return true;
            }
            return false;
        });

    return S_OK;
}

CComponent* CLayer::Get_Component(const _wstring& _strComponentTag, _uint _iIndex)
{
    auto iter = m_GameObjectList.begin();

    for (size_t i = 0; i < _iIndex; i++)
    {
        ++iter;
    }
    return (*iter)->Get_Component(_strComponentTag);
}

CLayer* CLayer::Create()
{
    return new CLayer();
}

void CLayer::Free()
{
    __super::Free();

    /* map release */
    for (auto& pGameObject : m_GameObjectList)
        Safe_Release(pGameObject);
    m_GameObjectList.clear();
}
