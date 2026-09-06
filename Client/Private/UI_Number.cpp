#include "UI_Number.h"

CUI_Number::CUI_Number(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_Number::CUI_Number(const CUI_Number& _Prototype)
    : CUI_Rect(_Prototype)
{
}

HRESULT CUI_Number::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Number::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Number::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Number::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);
}

void CUI_Number::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Number::Render()
{
    /* Texture Index ¿¬µ¿ */
    m_iTextureIndex = m_iNumber;

    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;

    return S_OK;
}

CUI_Number* CUI_Number::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Number* pInstance = new CUI_Number(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Number");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Number::Clone(void* _pArg)
{
    CUI_Number* pInstance = new CUI_Number(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Number");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Number::Free()
{
    CUI_Rect::Free();
}
