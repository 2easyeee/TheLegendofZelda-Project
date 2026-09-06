#include "UI_Transition.h"

CUI_Transition::CUI_Transition(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_Transition::CUI_Transition(const CUI_Transition& _Prototype)
    : CUI_Rect(_Prototype)
{
}

HRESULT CUI_Transition::Initialize_Prototype()
{
    if (FAILED(CUI_Rect::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Transition::Initialize(void* _pArg)
{
    if (FAILED(CUI_Rect::Initialize(_pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Transition::Priority_Update(_float _fTimeDelta)
{
    CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_Transition::Update(_float _fTimeDelta)
{
    CUI_Rect::Update(_fTimeDelta);

    switch (m_eUITransition)
    {
    case Client::CUI_Transition::UI_TRANSITION::NONE:
        break;
    case Client::CUI_Transition::UI_TRANSITION::APPEAR:
    {
        m_pTransformCom->Set_Scale(0.1f, 0.1f, 1.f);
        m_eUITransition = UI_TRANSITION::EXPAND;
    }
        break;
    case Client::CUI_Transition::UI_TRANSITION::EXPAND:
    {
        m_fElaspedTime += _fTimeDelta;
        _float fNormalize = m_fElaspedTime / m_fExpandDuration;
        if (fNormalize > 1.f)
            fNormalize = 1.f;

        // Ease In
        _float eased = fNormalize * fNormalize;
        //_float eased = fNormalize * fNormalize * (3.f - (2.f * fNormalize));

        _float fScale = m_fStartScale + (eased * (m_fTargetScale - m_fStartScale));
        m_pTransformCom->Set_Scale(fScale, fScale, 1.f);
        if (fNormalize >= 1.f)
        {
            m_fElaspedTime = 0.f;
            m_eUITransition = UI_TRANSITION::HOLD;
        }
    }
        break;
    case Client::CUI_Transition::UI_TRANSITION::HOLD:
    {
        m_fHoldTime += _fTimeDelta;
        if (m_fHoldTime >= m_fTargetTime)
        {
            m_fHoldTime = 0.f;
            m_eUITransition = UI_TRANSITION::END;
        }
    }
        break;
    case Client::CUI_Transition::UI_TRANSITION::FADEIN:
    {
        m_fFadeElaspedTime += _fTimeDelta;
        float fNormalize = m_fFadeElaspedTime / m_fFadeDuration;
        fNormalize = min(fNormalize, 1.f);

        // ease out
        float eased = 1.f - ((1.f - fNormalize) * (1.f - fNormalize));

        Set_Alpha(eased);

        if (fNormalize >= 1.f)
            m_eUITransition = UI_TRANSITION::END;
    }
        break;
    case Client::CUI_Transition::UI_TRANSITION::FADEOUT:
    {
        m_fFadeElaspedTime += _fTimeDelta;
        float fNormalize = m_fFadeElaspedTime / m_fFadeDuration;
        fNormalize = min(fNormalize, 1.f);

        float eased = 1.f - ((1.f - fNormalize) * (1.f - fNormalize));
        Set_Alpha(1.f - eased);

        if (fNormalize >= 1.f)
            m_eUITransition = UI_TRANSITION::END;
    }
        break;
    case Client::CUI_Transition::UI_TRANSITION::END:
    {
        m_fFadeElaspedTime = 0.f;
        m_bActive = false;
        m_eUITransition = UI_TRANSITION::NONE;
    }
        break;
    }
}

void CUI_Transition::Late_Update(_float _fTimeDelta)
{
    CUI_Rect::Late_Update(_fTimeDelta);
}

HRESULT CUI_Transition::Render()
{
    if (!m_bActive)
        return S_OK;

    if (FAILED(CUI_Rect::Render()))
        return E_FAIL;
    return S_OK;
}

CUI_Transition* CUI_Transition::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CUI_Transition* pInstance = new CUI_Transition(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("FAILED TO CREATED : CUI_Transition");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_Transition::Clone(void* _pArg)
{
    CUI_Transition* pInstance = new CUI_Transition(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CUI_Transition");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_Transition::Free()
{
    CUI_Rect::Free();
}
