#include "UI_Heart.h"
#include "GameInstance.h"

CUI_Heart::CUI_Heart(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_Heart::CUI_Heart(const CUI_Heart& _Prototype)
    : CUI_Rect(_Prototype)
{
}

HRESULT CUI_Heart::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Heart::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Heart::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Heart::Update(_float _fTimeDelta)
{
    Update_HP_State();
    CUI_Rect::Update(_fTimeDelta);
}

void CUI_Heart::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Heart::Render()
{
    /* Texture Index ¿¬µ¿ */
    m_iTextureIndex = ENUM_TO_UINT(m_eHP);

    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;

    return S_OK;
}

void CUI_Heart::Update_HP_State()
{
    if (m_fHPState >= 1.f)
        m_eHP = HP::FULL;
    else if (m_fHPState >= 0.5f)
        m_eHP = HP::HALF;
    else
        m_eHP = HP::EMPTY;
}

CUI_Heart* CUI_Heart::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Heart* pInstance = new CUI_Heart(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Heart");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Heart::Clone(void* _pArg)
{
    CUI_Heart* pInstance = new CUI_Heart(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Heart");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Heart::Free()
{
    CUI_Rect::Free();
}
