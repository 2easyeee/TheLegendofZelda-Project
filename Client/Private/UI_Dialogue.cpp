#include "UI_Dialogue.h"
#include "GameInstance.h"
#include "UIObject.h"
#include "UI_Image.h"

CUI_Dialogue::CUI_Dialogue()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CUI_Dialogue::Initialize_Dialogue()
{
    Create_MessageBox_Image(TEXT("Prototype_Component_Texture_UI_MessageFrame"), TEXT("UI_MessageBox"), 
        _float2(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.85f), 1.2f, m_pMessageBox);
    Create_MessageBox_Image(TEXT("Prototype_Component_Texture_UI_ChoiceMessageBox"), TEXT("UI_ChoiceMessageBox"),
        _float2(g_iWinSizeX * 0.9f, g_iWinSizeY * 0.8f), 0.39f, m_pChoiceMessageBox_A);
    Create_MessageBox_Image(TEXT("Prototype_Component_Texture_UI_ChoiceMessageBox"), TEXT("UI_ChoiceMessageBox"),
        _float2(g_iWinSizeX * 0.9f, g_iWinSizeY * 0.9f), 0.39f, m_pChoiceMessageBox_B);
    Create_MessageBox_Image(TEXT("Prototype_Component_Texture_UI_GetMessageFrame"), TEXT("UI_GetMessageBox"),
        _float2(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.85f), 1.2f, m_pGetMessageBox);

    Create_MessageBox_Image(TEXT("Prototype_Component_Texture_UI_MessageNext"), TEXT("UI_MessageNext"),
        _float2(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.95f), 1.f, m_pMessageNext, 72.f, 54.f);
    
}
void CUI_Dialogue::Update(_float _fDeltaTime)
{
    m_fNextAnimTime += _fDeltaTime;

    if (m_pMessageNext && m_pMessageNext->IsActive())
    {
        _float baseY = g_iWinSizeY * 0.95f;
        _float offset = sinf(m_fNextAnimTime * 5.f) * 5.f;

        m_pMessageNext->Set_UIPosition(g_iWinSizeX * 0.5f, baseY + offset);
        m_pMessageNext->Apply_UITransform();
    }
}

void CUI_Dialogue::Hide_AllMessageBox()
{
    m_pMessageBox->Set_Active(false);
    m_pChoiceMessageBox_A->Set_Active(false);
    m_pChoiceMessageBox_B->Set_Active(false);
    m_pGetMessageBox->Set_Active(false);
    m_pMessageNext->Set_Active(false);
}

void CUI_Dialogue::Hide_ChoiceMessageBox()
{
    m_pChoiceMessageBox_A->Set_Active(false);
    m_pChoiceMessageBox_B->Set_Active(false);
}

void CUI_Dialogue::PopUp_MessageBox()
{
    m_pMessageBox->Set_Active(true);
}

void CUI_Dialogue::PopUp_GetMessageBox()
{
    m_pGetMessageBox->Set_Active(true);
}

void CUI_Dialogue::PopUp_ChoiceMessageBox()
{
    m_pChoiceMessageBox_A->Set_Active(true);
    m_pChoiceMessageBox_B->Set_Active(true);
}

void CUI_Dialogue::PopUp_MessageNext()
{
    m_pMessageNext->Set_Active(true);
}

HRESULT CUI_Dialogue::Create_MessageBox_Image(_wstring _wstrTextureTag, _wstring _wstrObjectID, _float2 _vPosition, _float _fScale, CUI_Image*& _out, _float _fSizeX, _float _fSizeY)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, _wstrObjectID.c_str());
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Dialogue"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, _wstrTextureTag.c_str());
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = _vPosition.x;
    tUIDesc.fY = _vPosition.y;
    tUIDesc.fSizeX = _fSizeX * _fScale;
    tUIDesc.fSizeY = _fSizeY * _fScale;
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

    _out = dynamic_cast<CUI_Image*>(pGameObject);

    return S_OK;
}

CUI_Dialogue* CUI_Dialogue::Create()
{
	return new CUI_Dialogue();
}

void CUI_Dialogue::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
