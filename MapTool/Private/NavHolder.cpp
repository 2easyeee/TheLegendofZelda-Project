#include "NavHolder.h"
#include "GameInstance.h"

CNavHolder::CNavHolder(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CMapObject{ _pDevice, _pDeviceContext }
{
}

CNavHolder::CNavHolder(const CNavHolder& _Prototype)
    : CMapObject(_Prototype)
{
}

HRESULT CNavHolder::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CNavHolder::Initialize(void* _pArg)
{
    MAP_INIT_DESC tMapInitDesc = {};

    /* 1. Map Object */
    if (FAILED(CMapObject::Initialize(&tMapInitDesc)))
        return E_FAIL;

    /* Navigation */
    _wstring* comTag = static_cast<_wstring*>(_pArg);
    if (FAILED(Ready_Components(*comTag)))
        return E_FAIL;

    return S_OK;
}

void CNavHolder::Priority_Update(_float _fTimeDelta)
{
}

void CNavHolder::Update(_float _fTimeDelta)
{
    /* Navigation */
    if (m_pNavCom)
        m_pTransformCom->Set_State(STATE::POSITION, m_pNavCom->SetUp_OnNavigation(m_pTransformCom->Get_State(STATE::POSITION)));
}

void CNavHolder::Late_Update(_float _fTimeDelta)
{
    if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
        return;
}

HRESULT CNavHolder::Render()
{
#ifdef _DEBUG
    if (m_pNavCom) m_pNavCom->Render();
#endif // _DEBUG
    return S_OK;
}

HRESULT CNavHolder::Ready_Components(_wstring _ComTag)
{
    CNavigation::NAVIGATION_DESC tNavDesc = {};
    tNavDesc.iCurrentCellIndex = 1;

    if (FAILED(Add_Component(
        RESOURCE_LEVEL_STATIC,
        _ComTag.c_str(),
        TEXT("Com_Navigation"),
        reinterpret_cast<CComponent**>(&m_pNavCom),
        &tNavDesc)))
        return E_FAIL;

    return S_OK;
}

CNavHolder* CNavHolder::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CNavHolder* pInstance = new CNavHolder(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CNavHolder");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CNavHolder::Clone(void* _pArg)
{
    CNavHolder* pInstance = new CNavHolder(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CNavHolder");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNavHolder::Free()
{
    __super::Free();

    Safe_Release(m_pNavCom);
}
