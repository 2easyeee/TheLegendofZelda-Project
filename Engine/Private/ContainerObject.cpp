#include "ContainerObject.h"
#include "GameInstance.h"

CContainerObject::CContainerObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CGameObject { _pDevice, _pDeviceContext }
{
}

CContainerObject::CContainerObject(const CContainerObject& _Prototype)
    : CGameObject (_Prototype)
{
}

HRESULT CContainerObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CContainerObject::Initialize(void* _pArg)
{
    /* 1. */
    CONTAINERR_INIT_DESC* pInitDesc = static_cast<CONTAINERR_INIT_DESC*>(_pArg);
    if (&pInitDesc->tObjectDesc)
    {
        if (FAILED(CGameObject::Initialize(&pInitDesc->tObjectDesc)))
            return E_FAIL;
        if (FAILED(m_pTransformCom->Initialize(nullptr)))
            return E_FAIL;
    }

    /* 2. Container Object Desc */
    if (&pInitDesc->tContainerDesc)
    {
        m_iTotalPartsCnt = pInitDesc->tContainerDesc.iTotalPartsCnt;
        m_vecPartObjects.resize(m_iTotalPartsCnt);
    }

    return S_OK;
}

void CContainerObject::Priority_Update(_float _fTimeDelta)
{
    for (auto& pPartObject : m_vecPartObjects)
    {
        if (nullptr != pPartObject)
            pPartObject->Priority_Update(_fTimeDelta);
    }
}

void CContainerObject::Update(_float _fTimeDelta)
{
    for (auto& pPartObject : m_vecPartObjects)
    {
        if (nullptr != pPartObject)
            pPartObject->Update(_fTimeDelta);
    }
}

void CContainerObject::Late_Update(_float _fTimeDelta)
{
    for (auto& pPartObject : m_vecPartObjects)
    {
        if (nullptr != pPartObject)
            pPartObject->Late_Update(_fTimeDelta);
    }
}

HRESULT CContainerObject::Render()
{
    return S_OK;
}

HRESULT CContainerObject::Add_PartObject(_uint _iPartObjIndex, _uint _iPrototypeLevelIndex, const _wstring& _strPrototypeTag, void* _pArg)
{
    /* 1. Add PartObject */
    CPartObject* pPartObject = static_cast<CPartObject*>(
        m_pGameInstance->Clone_Prototype(
            PROTOTYPE::GAMEOBJECT, _iPrototypeLevelIndex,
            _strPrototypeTag, _pArg));
    if (nullptr == pPartObject)
        return E_FAIL;

    m_vecPartObjects[_iPartObjIndex] = pPartObject;

    return S_OK;
}

void CContainerObject::Free()
{
    __super::Free();

    /* vector release */
    for (auto& pPartObject : m_vecPartObjects)
        Safe_Release(pPartObject);
    m_vecPartObjects.clear();
}
