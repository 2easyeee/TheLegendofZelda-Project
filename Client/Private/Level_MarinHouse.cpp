#include "Level_MarinHouse.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "GameObject.h"
#include "Camera_Static.h"
#include "UIObject.h"
#include "UIManager.h"
#include "CameraController.h"
#include "GameObject/Player.h"
#include "Trigger.h"
#include "GameObject/NPC.h"
#include "GameObject/MapObject_Anim.h"

CLevel_MarinHouse::CLevel_MarinHouse(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel { _pDevice, _pDeviceContext }
{
}

HRESULT CLevel_MarinHouse::Initialize()
{
    if (FAILED(Ready_Layer_Light()))
        return E_FAIL;
        
    if (FAILED(Ready_Layer_Background(TEXT("Background"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_Camera(TEXT("Camera"))))
        return E_FAIL;

    if (FAILED(Ready_Player_And_Camera()))
        return E_FAIL;

    if (FAILED(Ready_Trigger()))
        return E_FAIL;

    if (FAILED(Ready_Layer_NPC()))
        return E_FAIL;

    if (FAILED(Ready_Actor_Table()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Shadow()))
        return E_FAIL;

    /* SFX */
    m_pGameInstance->Play_BGM(L"BGM_MarinTarinHouse.wav");
    m_pGameInstance->Play_Sound(L"Ambience_Field_Village.wav", SOUND::EFFECT, 0.8f);

    /* UI */
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INIT_ON;
    m_pGameInstance->Push_UIEVENT(tUIEvent);

    return S_OK;
}

void CLevel_MarinHouse::Update(_float _fTimeDelta)
{
    /* Camera */
    if (m_pCameraController)
        m_pCameraController->Update(_fTimeDelta);

    /* Event */
    Update_Event();
    Check_SleepEndEvent(_fTimeDelta);
}

HRESULT CLevel_MarinHouse::Render()
{
    SetWindowText(g_hWnd, TEXT("MARINTARIN HOUSE"));

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Layer_Camera(const _wstring& strLayerTag)
{   
    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Camera_Static"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MARINHOUSE);
    SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Camera_Static"));

    CCamera::CAMERA_DESC CameraDesc = {};
    CameraDesc.vEye = _float3(0.f, 11.f, -10.f);
    CameraDesc.vAt = _float3(0.f, 0.5f, 0.f);
    CameraDesc.fFovy = XMConvertToRadians(42.0f);
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 1000.f;
    CameraDesc.fSpeedPerSec = 10.f;
    CameraDesc.fRotationPerSec = 90.0f;
    CameraDesc.fSensor = 0.07f;
    
    CCamera::CAMERA_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tCameraDesc = CameraDesc;

    CGameObject* pGameObject = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        tObjectDesc.ObjectTag,
        ENUM_TO_UINT(LEVEL::MARINHOUSE), 
        strLayerTag, &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pCamera = static_cast<CCamera*>(pGameObject);

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Layer_Background(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("MarinHouse_Background"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MARINHOUSE);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_WorldBackground"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Background"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Logo_Background_White"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxGradient"));
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
        tObjectDesc.iLevel,
        strLayerTag, &tInitDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Layer_NPC()
{
    /* Marin */
    m_pMarin = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("Marin_0")));
    m_pMarin->Set_Animation("wait", true);

    /* Tarin */
    m_pTarin = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("Tarin_0")));
    m_pTarin->Set_Animation("eat", true);

    /* Bed */
    m_pBed = static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("Bed_0")));
    m_pBed->Set_Animation("ftn_idle", true);
    m_pBed = static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("Bed_1")));
    m_pBed->Set_Animation("ftn_ev_wakeup_lp", true);

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Actor_Table()
{
    m_ActorMap[TEXT("마린")] = m_pMarin;
    m_ActorMap[TEXT("타린")] = m_pTarin;
    m_ActorMap[TEXT("링크")] = m_pPlayer;
    m_ActorMap[TEXT("system")] = m_pPlayer;
   
    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Player_And_Camera()
{
    /* Player */
    m_pPlayer = m_pGameInstance->Get_Player();
    static_cast<CPlayer*>(m_pPlayer)->Ready_Navigation(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Navigation_NONE")); // Before find Navigation cell
    m_pGameInstance->Reset_Player_Position(-0.5f, 0.f, 1.f);

    CTransform* pTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));
    pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), -90.f);

    /* Camera */
    m_pCameraController = CCameraController::Create();
    m_pCameraController->Set_Camera(m_pCamera);
    m_pCameraController->Set_Mode(CCameraController::MODE::PLAYER);

    /* Casting */
    m_pCastingPlayer = static_cast<CPlayer*>(m_pPlayer);
    m_pCastingPlayer->Set_TransitionInput(IState::TRANSITION_INPUT::SLEEP_REQUESTED);

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Trigger()
{
    CTrigger::TRIGGER_DESC tTriggerDesc = {};
    tTriggerDesc.eType = CTrigger::TRIGGERTYPE::LEVEL_CHANGE;
    tTriggerDesc.iValue = ENUM_TO_UINT(LEVEL::FIELD);
    tTriggerDesc.vPosition = _float3(-0.035f, 0.f, -4.2f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::MARINHOUSE),
        TEXT("Layer_Trigger"),
        &tTriggerDesc, nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Layer_Light()
{
    m_pGameInstance->Load_LightBinary(ENUM_TO_UINT(LEVEL::MARINHOUSE), ENUM_TO_UINT(LEVEL::MARINHOUSE));

    return S_OK;
}

HRESULT CLevel_MarinHouse::Ready_Layer_Shadow()
{
    SHADOW_LIGHT_DESC ShadowLightDesc = {};

    /* Marin House Shadow Light */
    ShadowLightDesc.vEye = _float4(-8.f, 14.f, -10.f, 1.f);
    ShadowLightDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
    ShadowLightDesc.fFovy = XMConvertToRadians(70.f);
    ShadowLightDesc.fNear = 0.1f;
    ShadowLightDesc.fFar = 100.f;
    ShadowLightDesc.fAspect =
        static_cast<_float>(g_iMaxWidth) /
        static_cast<_float>(g_iMaxHeight);

    if (FAILED(m_pGameInstance->Add_ShadowLight(ShadowLightDesc)))
        return E_FAIL;

    return S_OK;
}
HRESULT CLevel_MarinHouse::Update_Event()
{
    /* EventManager */
    CEventManager::GAME_EVENT tEvent = {};
    while (m_pGameInstance->Pop_GameEVENT(tEvent))
    {
        Handle_GameEvent(tEvent);
    }

    /* EventManager (Camera) */
    CEventManager::CAMERA_EVENT tCameraEvent = {};
    while (m_pGameInstance->Pop_CameraEVENT(tCameraEvent))
    {
        Handle_CameraEvent(tCameraEvent);
    }

    return S_OK;
}

HRESULT CLevel_MarinHouse::Handle_GameEvent(CEventManager::GAME_EVENT& _tEvent)
{
    switch (_tEvent.eType)
    {
    case CEventManager::GAME_EVENT_TYPE::LEVEL_CHANGE:
    {
        /* UI */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::ALL_OFF;
        m_pGameInstance->Push_UIEVENT(tUIEvent);

        /* Collider */
        m_pGameInstance->Reset_Collider();

        m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(LEVEL::LOADING),
            CLevel_Loading::Create(m_pDevice, m_pDeviceContext, LEVEL::FIELD));

        /* Clear Light */
        m_pGameInstance->Clear_Light();
    }
    break;
    }

    return S_OK;
}

HRESULT CLevel_MarinHouse::Handle_CameraEvent(CEventManager::CAMERA_EVENT& _tEvent)
{
    switch (_tEvent.eType)
    {
    case CEventManager::CAMERA_EVENT_TYPE::DIALOGUE:
    {
        Start_DialogueCamera(_tEvent.wstrValue);
        m_pMarin->Set_Animation("talk", true);
        m_pTarin->Set_Animation("sit_talk", true);
    }
    break;
    case CEventManager::CAMERA_EVENT_TYPE::DIALOGUE_END:
    {
        m_pCameraController->Set_Mode(CCameraController::MODE::STATIC);
        m_pCameraController->Set_StaticPos(XMVectorSet(0.f, 11.f, -10.f, 1.f), XMVectorSet(0.f, 0.5f, 0.f, 1.f), 5.f);

        /* UI */
        //CEventManager::UI_EVENT tUIEvent = {};
        //tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INIT_ON;

        //m_pGameInstance->Push_UIEVENT(tUIEvent);
    }
    break;
    }

    return S_OK;
}

void CLevel_MarinHouse::Check_SleepEndEvent(_float _fTimeDelta)
{
    if (m_pCastingPlayer)
    {
        if (m_pCastingPlayer->Notify_WakeUpEvent())
        {
            m_pBed->Set_Animation("ftn_ev_outbed", false);
        }

        if (m_pCastingPlayer->Notify_SleepEndEvent())
        {
            /* Load Dialogue */
            m_pGameInstance->Load_XMLFile(L"../../Resources/Data/Dialogue/00_MarinTarinHouse.xml");
            m_pGameInstance->Start_Dialogue(L"intro_01");
        }
    }
}

void CLevel_MarinHouse::Start_DialogueCamera(_wstring _Actor)
{
    if (!m_pPlayer)
        return;

    auto iter = m_ActorMap.find(_Actor);
    if (iter == m_ActorMap.end())
        return;

    CGameObject* pTarget = iter->second;
    if (!pTarget)
        return;

    /* Transform */
    CTransform* pPlayerTr =
        static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));
    CTransform* pTargetTr =
        static_cast<CTransform*>(pTarget->Get_Component(TEXT("Com_Transform")));

    if (!pPlayerTr || !pTargetTr)
        return;

    _vector vPlayerPos = pPlayerTr->Get_State(STATE::POSITION);
    _vector vTargetPos = pTargetTr->Get_State(STATE::POSITION);

    _vector vMid = (vPlayerPos + vTargetPos) * 0.5f;

    _vector vOffset = XMVectorSet(0.f, 6.f, -6.f, 0.f);
    _vector vCamPos = vMid + vOffset;

    m_pCameraController->Set_Event(vCamPos); //, vMid
    m_pCameraController->Set_Mode(CCameraController::MODE::EVENT);
}

CLevel_MarinHouse* CLevel_MarinHouse::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CLevel_MarinHouse* pInstance = new CLevel_MarinHouse(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CLevel_MarinHouse");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLevel_MarinHouse::Free()
{
    __super::Free();

    m_pCastingPlayer = nullptr;
    m_pMarin = nullptr;
    m_pPlayer = nullptr;
    m_pCamera = nullptr;
    Safe_Release(m_pCameraController);
}
