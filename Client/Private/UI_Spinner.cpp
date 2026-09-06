#include "UI_Spinner.h"

CUI_Spinner::CUI_Spinner(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect { _pDevice, _pDeviceContext }
{
}

CUI_Spinner::CUI_Spinner(const CUI_Spinner& _Prototype)
    : CUI_Rect (_Prototype)
{
}

HRESULT CUI_Spinner::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Spinner::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    m_fRotSpeed = 100.f;

    return S_OK;
}

void CUI_Spinner::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Spinner::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);

    m_fAngle += m_fRotSpeed * _fTimeDelta;
    if (m_fAngle > 360.f)
        m_fAngle = 0.f;

    _vector vAxix = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    m_pTransformCom->Rotation(vAxix, -m_fAngle);
}

void CUI_Spinner::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Spinner::Render()
{
    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;
    return S_OK;
}

CUI_Spinner* CUI_Spinner::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Spinner* pInstance = new CUI_Spinner(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Spinner");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Spinner::Clone(void* _pArg)
{
    CUI_Spinner* pInstance = new CUI_Spinner(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Spinner");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Spinner::Free()
{
    CUI_Rect::Free();
}
