#include "Level_Loading.h"

#include "GameInstance.h"
#include "Loader.h"
#include "Level_Logo.h"
#include "Level_Credit.h"
#include "Level_MarinHouse.h"
#include "Level_SandBox.h"
#include "Level_Field.h"
#include "Level_MARTHAsBAY.h"
#include "Level_Dungeon.h"
#include "Level_Tower.h"
#include "Level_TelephoneBox.h"
#include "GameObject.h"
#include "UIObject.h"
#include "UI_Transition.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel { _pDevice, _pDeviceContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL _eNextLevel)
{
    m_eNextLevel = _eNextLevel;

    if (FAILED(Ready_Layer_Background(TEXT("Layer_Background"))))
        return E_FAIL;
    if (FAILED(Ready_Layer_Egg(TEXT("Layer_Egg"))))
        return E_FAIL;
    if (FAILED(Ready_Layer_Spinner(TEXT("Layer_Spinner"))))
        return E_FAIL;
    if (FAILED(Ready_Layer_Spicy(TEXT("Layer_Spicy"))))
        return E_FAIL;

    return S_OK;
}

void CLevel_Loading::Update(_float _fTimeDelta)
{
    m_tMinDisplayTime.fAccTime += _fTimeDelta;

    if (m_tMinDisplayTime.bActive)
        return;

    if (!m_bLoaderStarted)
    {
        /* Loading Scene */
        m_pLoader = CLoader::Create(m_pDevice, m_pDeviceContext, m_eNextLevel);
        if (nullptr == m_pLoader)
            return;
        m_bLoaderStarted = true;
        return;
    }

    if (!m_pLoader->isFinished())
        return;

    if (m_tMinDisplayTime.fAccTime < m_tMinDisplayTime.fAccDurationTime)
        return;

    if (!m_pGameInstance->Get_DIKeyDown(DIK_RETURN))
        return;

    CLevel* pNewLevel = { nullptr };

    switch (m_eNextLevel)
    {
    case Client::LEVEL::LOGO:
        pNewLevel = CLevel_Logo::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::MARINHOUSE:
        pNewLevel = CLevel_MarinHouse::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::FIELD:
        pNewLevel = CLevel_Field::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::CREDIT:
        pNewLevel = CLevel_Credit::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::SANDBOX:
        pNewLevel = CLevel_SandBox::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::MARTHAsBAY:
        pNewLevel = CLevel_MARTHAsBAY::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::DUNGEON:
        pNewLevel = CLevel_Dungeon::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::TOWER:
        pNewLevel = CLevel_Tower::Create(m_pDevice, m_pDeviceContext);
        break;
    case Client::LEVEL::TELEPHONEBOX:
        pNewLevel = CLevel_TelephoneBox::Create(m_pDevice, m_pDeviceContext);
        break;
    }

    if (nullptr == pNewLevel)
        return;

    if (FAILED(m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eNextLevel), pNewLevel)))
        return;
}

HRESULT CLevel_Loading::Render()
{
    m_pLoader->Output();

    return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_Background(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Loading_Background"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Background"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Logo_Background_Black"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.5f;
    tUIDesc.fY = g_iWinSizeY * 0.5f;
    tUIDesc.fSizeX = g_iWinSizeX;
    tUIDesc.fSizeY = g_iWinSizeY;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::BACKGROUND;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::NONBLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::LOADING),
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_Spinner(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Loading_Spinner"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Spinner"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Background"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Loading_Spinner"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.95f;
    tUIDesc.fY = g_iWinSizeY * 0.9f;
    tUIDesc.fSizeX = 200.f * 0.45f;
    tUIDesc.fSizeY = 200.f * 0.45f;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::LOADING),
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_Egg(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Loading_Spinner"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Spinner"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Loading_Egg"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.95f;
    tUIDesc.fY = g_iWinSizeY * 0.9f;
    tUIDesc.fSizeX = 48.f * 0.7f;
    tUIDesc.fSizeY = 48.f * 0.7f;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::LOADING),
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_Spicy(const _wstring& strLayerTag)
{
    //CGameObject::OBJECT_DESC tObjectDesc = {};
    //wcscpy_s(tObjectDesc.ObjectID, TEXT("Loading_Spinner"));
    //tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    //wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Transition"));
    //wcscpy_s(tObjectDesc.LayerTag, TEXT("Background"));
    //wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    //wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Loading_Spicy"));
    //wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    ////wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    //CUIObject::CUIObject::UI_DESC tUIDesc = {};
    //tUIDesc.fX = g_iWinSizeX * 0.5f;
    //tUIDesc.fY = g_iWinSizeY * 0.f;
    //tUIDesc.fSizeX = 1280.f * 0.001f;
    //tUIDesc.fSizeY = 1280.f * 0.001f;
    //tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    //tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    //tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    //CUIObject::UI_INIT_DESC tInitDesc = {};
    //tInitDesc.tObjectDesc = tObjectDesc;
    //tInitDesc.tuiDesc = tUIDesc;

    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
    //    tObjectDesc.iLevel,
    //    tObjectDesc.ObjectTag,
    //    tObjectDesc.iLevel,
    //    strLayerTag, &tInitDesc, &m_pTransition_Spicy)))
    //    return E_FAIL;

    //m_pTransition_Spicy->Set_Active(false); // TODO : 일단 임시로 막아두기
    
    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, LEVEL _eNextLevel)
{
    CLevel_Loading* pInstance = new CLevel_Loading(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize(_eNextLevel)))
    {
        MSG_BOX("FAILED TO CREATED : CLevel_Loading");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();

    Safe_Release(m_pLoader);
}
