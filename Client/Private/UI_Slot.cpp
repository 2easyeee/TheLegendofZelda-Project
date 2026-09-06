#include "UI_Slot.h"

CUI_Slot::CUI_Slot(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_Slot::CUI_Slot(const CUI_Slot& _Prototype)
    : CUI_Rect(_Prototype)
{
}

HRESULT CUI_Slot::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Slot::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Slot::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Slot::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);
}

void CUI_Slot::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Slot::Render()
{
    /* Texture Index ¿¬µ¿ */
    m_iTextureIndex = m_iEquipIndex;

    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;

    return S_OK;
}

void CUI_Slot::Equip(_uint _iIndex)
{
    m_iEquipIndex = _iIndex;
}

CUI_Slot* CUI_Slot::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Slot* pInstance = new CUI_Slot(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Slot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Slot::Clone(void* _pArg)
{
    CUI_Slot* pInstance = new CUI_Slot(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Slot");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Slot::Free()
{
    CUI_Rect::Free();
}
