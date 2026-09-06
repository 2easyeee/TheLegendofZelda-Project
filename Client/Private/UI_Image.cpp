#include "UI_Image.h"

CUI_Image::CUI_Image(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect { _pDevice, _pDeviceContext }
{
}

CUI_Image::CUI_Image(const CUI_Image& _Prototype)
    : CUI_Rect (_Prototype)
{
}

HRESULT CUI_Image::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Image::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Image::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Image::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);
}

void CUI_Image::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Image::Render()
{
    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;

    return S_OK;
}

CUI_Image* CUI_Image::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Image* pInstance = new CUI_Image(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Image");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Image::Clone(void* _pArg)
{
    CUI_Image* pInstance = new CUI_Image(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Image");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Image::Free()
{
    CUI_Rect::Free();
}
