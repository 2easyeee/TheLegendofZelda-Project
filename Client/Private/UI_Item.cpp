#include "UI_Item.h"

CUI_Item::CUI_Item(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_Item::CUI_Item(const CUI_Item& _Prototype)
    : CUI_Rect(_Prototype)
{
}

HRESULT CUI_Item::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Item::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Item::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Item::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);
}

void CUI_Item::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Item::Render()
{
    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;

    return S_OK;
}

void CUI_Item::Add_Item(_uint _iCount)
{
    m_iCount += _iCount;
}

void CUI_Item::Spend_Item(_uint _iCount)
{
    m_iCount -= _iCount;

    if (m_iCount <= 0)
        m_iCount = 0;
}

CUI_Item* CUI_Item::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Item* pInstance = new CUI_Item(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Item");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Item::Clone(void* _pArg)
{
    CUI_Item* pInstance = new CUI_Item(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Item");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Item::Free()
{
    CUI_Rect::Free();
}
