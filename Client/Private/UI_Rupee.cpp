#include "UI_Rupee.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "UI_Image.h"
#include "UI_Number.h"

CUI_Rupee::CUI_Rupee()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CUI_Rupee::Initialize_Rupee(_uint _iRupee)
{
    m_iRupee = _iRupee;

    m_tLayout.fIconGap = 10.f;
    m_tLayout.fNumberGap = 22.f;
    m_tLayout.vStartPosition = _float2(g_iWinSizeX * 0.85f, g_iWinSizeY * 0.2f);
    m_tLayout.vEndPosition = _float2(g_iWinSizeX * 0.94f, g_iWinSizeY * 0.2f);

    /* Create Element */
    Create_Rupee_Image(_float2(m_tLayout.vStartPosition.x, m_tLayout.vStartPosition.y), 0.6f);
    for (size_t i = 0; i < 4; i++)
    {
        Create_Rupee_Number(_float2(m_tLayout.vStartPosition.x + m_tLayout.fNumberGap, m_tLayout.vStartPosition.y), 0.5f);
    }

    m_bUpdatalbe = false;
}

void CUI_Rupee::Add_Rupee(_uint _iRupee)
{
    m_iRupee += _iRupee;

    if (m_iRupee >= m_iMaxRupee)
        m_iRupee = m_iMaxRupee;
}

void CUI_Rupee::Spend_Rupee(_uint _iRupee)
{
    if (_iRupee >= m_iRupee)
    {
        m_iRupee = 0;
        return;
    }

    m_iRupee -= _iRupee;
}

_uint CUI_Rupee::Get_Rupee() const
{
    return m_iRupee;
}

void CUI_Rupee::Hide_Rupee()
{
    m_bUpdatalbe = true;
    m_pRupeeImage->Set_Active(false);
    for (auto& Number : m_vecNumber)
        Number->Set_Active(false);
}

void CUI_Rupee::PopUp_Rupee()
{
    m_bUpdatalbe = true;
    m_pRupeeImage->Set_Active(true);
    for (auto& Number : m_vecNumber)
        Number->Set_Active(true);

    /* Visible */
    m_tVisibleTime.bActive = true;
    m_tVisibleTime.fAccTime = 0.f;
}

void CUI_Rupee::AlwaysOn_Rupee()
{
}

void CUI_Rupee::Update(_float _fDeltaTime)
{
    /* Visible */
    if (!m_tVisibleTime.bActive)
        return;
    m_tVisibleTime.fAccTime += _fDeltaTime;
    if (m_tVisibleTime.fAccTime >= m_tVisibleTime.fAccDurationTime)
    {
        Hide_Rupee();
    }

    /* Update Rupee */
    if (m_vecNumber.empty())
        return;
    
    _int iNumbers[4];
    iNumbers[0] = m_iRupee / 1000;
    iNumbers[1] = (m_iRupee / 100) % 10;
    iNumbers[2] = (m_iRupee / 10) % 10;
    iNumbers[3] = m_iRupee % 10;

    if (m_bUpdatalbe)
        Update_Layout(iNumbers);
}

HRESULT CUI_Rupee::Create_Rupee_Image(_float2 _fPosition, _float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Heart"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Heart"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Rupee"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = _fPosition.x;
    tUIDesc.fY = _fPosition.y;
    tUIDesc.fSizeX = 64.f * _fScale;
    tUIDesc.fSizeY = 64.f * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pRupeeImage = static_cast<CUI_Image*>(pGameObject);

    return S_OK;
}

HRESULT CUI_Rupee::Create_Rupee_Number(_float2 _fPosition, _float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Heart"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Number"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Heart"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Number"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = _fPosition.x;
    tUIDesc.fY = _fPosition.y;
    tUIDesc.fSizeX = 100.f * _fScale;
    tUIDesc.fSizeY = 100.f * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    if (pGameObject)
    {
        auto pNumber = dynamic_cast<CUI_Number*>(pGameObject);
        m_vecNumber.push_back(pNumber);
    }

    return S_OK;
}

void CUI_Rupee::Update_Layout(const _int* _iNumbers)
{
    _int iFirstIndex = { 0 };
    while (iFirstIndex < 3 && _iNumbers[iFirstIndex] == 0)
    {
        ++iFirstIndex;
    }

    /* Set Number */
    _float fFixedX = m_tLayout.vEndPosition.x;
    for (_int i = m_vecNumber.size() - 1; i >= 0; --i)
    {
        m_vecNumber[i]->Set_Number(_iNumbers[i]);
        
        _float2 vPosition = _float2(fFixedX, m_tLayout.vEndPosition.y);
        m_vecNumber[i]->Set_UIPosition(vPosition.x, vPosition.y);
        m_vecNumber[i]->Apply_UITransform();
        fFixedX -= m_tLayout.fNumberGap;
    }
    
    /* ZERO InActive */
    for (_int i = 0; i < iFirstIndex; ++i)
        m_vecNumber[i]->Set_Active(false);

    /* Rupee Icon */
    _int iVisibleCount = (4 - iFirstIndex);
    _float fIconPositionX = m_tLayout.vEndPosition.x
        - (iVisibleCount * m_tLayout.fNumberGap)
        - m_tLayout.fIconGap;
    m_pRupeeImage->Set_UIPosition(fIconPositionX, (m_tLayout.vEndPosition.y));
    m_pRupeeImage->Apply_UITransform();

    /* Flag OFF */
    m_bUpdatalbe = false;
}

CUI_Rupee* CUI_Rupee::Create()
{
    return new CUI_Rupee();
}

void CUI_Rupee::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
