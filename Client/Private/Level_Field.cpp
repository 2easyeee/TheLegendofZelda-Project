#include "Level_Field.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "Camera_Static.h"
#include "CameraController.h"
#include "GameObject/Player.h"
#include "Trigger.h"
#include "GameObject/NPC.h"
#include "GameObject/MapObject_NonAnim.h"

CLevel_Field::CLevel_Field(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel{ _pDevice, _pDeviceContext }
{
}

HRESULT CLevel_Field::Initialize()
{
    m_pGameInstance->Set_InstancingEnabled(true);

    if (FAILED(Ready_Layer_Light()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Camera(TEXT("Camera"))))
        return E_FAIL;

    if (FAILED(Ready_Player_And_Camera()))
        return E_FAIL;

    if (FAILED(Ready_Trigger()))
        return E_FAIL;

    if (FAILED(Ready_Layer_NPC()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Shadow()))
        return E_FAIL;

    //if (FAILED(Ready_InstancingStressTest(100)))
    //    return E_FAIL;

    /* UI */
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INIT_ON;
    m_pGameInstance->Push_UIEVENT(tUIEvent);

    /* SFX */
    m_pGameInstance->Stop_AllSounds();
    m_pGameInstance->Play_BGM(L"BGM_Field_First.wav");

    return S_OK;
}

void CLevel_Field::Update(_float _fTimeDelta)
{
    /* Camera */
    if (m_pCameraController)
        m_pCameraController->Update(_fTimeDelta);

    /* Camera Clamp */
    Clamp_Camera();

    /* Event */
    Update_Event();
    Update_MoveToWarpHole(_fTimeDelta);
    IsGameClear(_fTimeDelta);
}

HRESULT CLevel_Field::Render()
{
    SetWindowText(g_hWnd, TEXT("Field"));

    return S_OK;
}

HRESULT CLevel_Field::Ready_InstancingStressTest(_uint iInstanceCount)
{
    if (0 == iInstanceCount)
        return S_OK;

    constexpr _uint iColumnCount = 25;
    constexpr _float fSpacing = 2.5f;
    const _float3 vStartPosition = { 20.f, 0.f, 5.f };

    for (_uint i = 0; i < iInstanceCount; ++i)
    {
        const _uint iColumn = i % iColumnCount;
        const _uint iRow = i / iColumnCount;

        CGameObject::OBJECT_DESC ObjectDesc = {};
        swprintf_s(ObjectDesc.ObjectID, TEXT("InstancingTree_%u"), i);
        ObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
        wcscpy_s(ObjectDesc.LayerTag, TEXT("Layer_InstancingStressTest"));
        wcscpy_s(ObjectDesc.ObjectTag, TEXT("Prototype_GameObject_MapObject_NonAnim"));
        wcscpy_s(ObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxMesh"));
        wcscpy_s(ObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_Tree2"));

        CMapObject::MAP_INIT_DESC InitDesc = {};
        InitDesc.tObjectDesc = ObjectDesc;
        InitDesc.tWorldDesc.vPosition = {
            vStartPosition.x + static_cast<_float>(iColumn) * fSpacing,
            vStartPosition.y,
            vStartPosition.z + static_cast<_float>(iRow) * fSpacing
        };
        InitDesc.tWorldDesc.vRight = { 1.f, 0.f, 0.f };
        InitDesc.tWorldDesc.vUp = { 0.f, 1.f, 0.f };
        InitDesc.tWorldDesc.vLook = { 0.f, 0.f, 1.f };

        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            ENUM_TO_UINT(LEVEL::STATIC),
            ObjectDesc.ObjectTag,
            ENUM_TO_UINT(LEVEL::FIELD),
            ObjectDesc.LayerTag,
            &InitDesc)))
        {
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT CLevel_Field::Ready_Layer_Camera(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Camera_Static"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::FIELD);
    SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Camera_Static"));

    CCamera::CAMERA_DESC CameraDesc = {};
    CameraDesc.vEye = _float3(12.f, 18.f, -11.078f);
    CameraDesc.vAt = _float3(12.f, 7.5f, 5.f);
    CameraDesc.fFovy = XMConvertToRadians(50.0f);
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
        ENUM_TO_UINT(LEVEL::FIELD),
        strLayerTag, &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pCamera = static_cast<CCamera*>(pGameObject);

    return S_OK;
}

HRESULT CLevel_Field::Ready_Player_And_Camera()
{
    /* Player */
    m_pPlayer = m_pGameInstance->Get_Player();
    static_cast<CPlayer*>(m_pPlayer)->Ready_Navigation(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Navigation_NONE")); // Before find Navigation cell
    //m_pGameInstance->Reset_Player_Position(31.f, 0.f, 6.f);  // MarinHouse
    m_pGameInstance->Reset_Player_Position(30.652f, 0.f, -2.58f); // Telephone

    /* Camera */
    m_pCameraController = CCameraController::Create();
    m_pCameraController->Set_Camera(m_pCamera);
    m_pCameraController->Set_Mode(CCameraController::MODE::PLAYER);
    m_pCameraController->Set_Angle(XMVectorSet(1.f, 0.f, 0.f, 0.f), 37.5f);

    /* Connet */
    m_pCameraController->Set_Target(m_pPlayer);
    m_pCastingPlayer = static_cast<CPlayer*>(m_pPlayer);

    /* Input Lock/UnLock */
    m_pCastingPlayer->Set_InputLock(false);

    /* Detail */
    m_pCastingPlayer->Set_InputMode(CPlayerInputController::INPUT_MODE::TOP);
    m_pCastingPlayer->Set_ViewMode(CPlayer::VIEW_MODE::TOP);

    CTransform* pTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));
    pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);

    return S_OK;
}

HRESULT CLevel_Field::Ready_Trigger()
{
    ///* Trigger (Martha's Bay) */
       //CTrigger::TRIGGER_DESC tDungeonTriggerDesc = {};
       //tDungeonTriggerDesc.eType = CTrigger::TRIGGERTYPE::LEVEL_CHANGE;
       //tDungeonTriggerDesc.iValue = ENUM_TO_UINT(LEVEL::MARTHAsBAY);
       //tDungeonTriggerDesc.vPosition = _float3(44.75f, 0.f, 10.75f);

       //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
       //    RESOURCE_LEVEL_STATIC,
       //    TEXT("Prototype_GameObject_Trigger"),
       //    ENUM_TO_UINT(LEVEL::FIELD),
       //    TEXT("Layer_Trigger"),
       //    &tDungeonTriggerDesc, nullptr)))
       //    return;

    /* Trigger (Martha's Bay) */
    CTrigger::TRIGGER_DESC tDungeonTriggerDesc = {};
    tDungeonTriggerDesc.eType = CTrigger::TRIGGERTYPE::LEVEL_CHANGE;
    tDungeonTriggerDesc.iValue = ENUM_TO_UINT(LEVEL::TELEPHONEBOX);
    tDungeonTriggerDesc.vPosition = _float3(30.652f, 0.f, -1.575f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::FIELD),
        TEXT("Layer_Trigger"),
        &tDungeonTriggerDesc, nullptr)))
        return S_OK;

    return S_OK;
}

HRESULT CLevel_Field::Ready_Layer_NPC()
{
    /* SmallBowWow */
    m_pSmallBowWow = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("SmallBowWow_0")));
    m_pSmallBowWow->Set_Animation("jump", true);

    /* Rabbit */
    m_pRabbit = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("Rabbit_0")));
    m_pRabbit->Set_Animation("talk", true);
    m_pRabbit = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("Rabbit_1")));
    m_pRabbit->Set_Animation("jumping", true);

    /* FlyingCucco */
    m_pFlyingCucco = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("FlyingCucco_0")));
    m_pFlyingCucco->Set_Animation("wait", true);

    /* Fox */
    m_pFox = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("Fox_0")));
    m_pFox->Set_Animation("listen", true);

    /* GrandmaUlrira */
    m_pGrandmaUlrira = static_cast<CNPC*>(m_pGameInstance->Find_Object(TEXT("GrandmaUlrira_0")));
    m_pGrandmaUlrira->Set_Animation("wait_broom", true);

    /* Butterfly */
    for (_int i = 0; i < 4; i++)
    {
        _wstring tag = L"Butterfly_" + to_wstring(i);

        m_pButterfly = static_cast<CNPC*>(m_pGameInstance->Find_Object(tag));
        m_pButterfly->Set_Animation("wait_move", true);
    }

    /* WarpHole */
    m_pWarpHole = static_cast<CMapObject_NonAnim*>(m_pGameInstance->Find_Object(TEXT("WarpHole_0")));
    m_pWarpHole->Set_Active(false);

    return S_OK;
}

HRESULT CLevel_Field::Ready_Layer_Light()
{
    m_pGameInstance->Load_LightBinary(ENUM_TO_UINT(LEVEL::FIELD), ENUM_TO_UINT(LEVEL::FIELD));

    return S_OK;
}

_bool CLevel_Field::IsGameClear(_float _fTimeDelta)
{
    if (m_tWarpTime.bActive)
    {
        m_tWarpTime.fAccTime += _fTimeDelta;
        if (m_tWarpTime.fAccTime >= m_tWarpTime.fAccDurationTime)
        {
            CEventManager::GAME_EVENT tLevelChange = {};
            tLevelChange.eType = CEventManager::GAME_EVENT_TYPE::LEVEL_CHANGE;
            tLevelChange.iValue = ENUM_TO_UINT(LEVEL::MARTHAsBAY);
            m_pGameInstance->Push_GameEVENT(tLevelChange);

            m_tWarpTime.bActive = false;
        }
    }

    return true;
}

HRESULT CLevel_Field::Update_Event()
{
    /* EventManager */
    CEventManager::GAME_EVENT tEvent = {};
    while (m_pGameInstance->Pop_GameEVENT(tEvent))
    {
        Handle_GameEvent(tEvent);
    }

    return S_OK;
}

HRESULT CLevel_Field::Handle_GameEvent(CEventManager::GAME_EVENT &_tEvent)
{
    switch (_tEvent.eType)
    {
    case CEventManager::GAME_EVENT_TYPE::LEVEL_CHANGE:
    {
        /* Input Lock/UnLock */
        m_pCastingPlayer->Set_InputLock(true);

        /* UI */
        CEventManager::UI_EVENT tUIEvent = {};
        tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::ALL_OFF;
        m_pGameInstance->Push_UIEVENT(tUIEvent);

        /* Collider */
        m_pGameInstance->Reset_Collider();
        m_pCastingPlayer->Collider_BodyEnable(false);

        /* Level */
        m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(LEVEL::LOADING),
            CLevel_Loading::Create(m_pDevice, m_pDeviceContext, (LEVEL)_tEvent.iValue));

        /* Clear Light */
        m_pGameInstance->Clear_Light();
    }
        break;
    case CEventManager::GAME_EVENT_TYPE::GRASS_CUT:
    {
        m_iGrassCount--;

        if (m_iGrassCount <= 0)
        {
            Spawn_Warp();
        }
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::ROOM_TRANSITION:
    {
        /* MoveTo */
        m_vWarpHoleCenter = _tEvent.vPos;
        m_vWarpHoleCenter = XMVectorSetW(m_vWarpHoleCenter, 1.f);
        m_tMoveTime.bActive = true;

        /* Walk Anim */
        m_pCastingPlayer->Set_TransitionInput(IState::TRANSITION_INPUT::MOVE_RUN);
    }
    break;
    }
    return S_OK;
}

void CLevel_Field::Spawn_Warp()
{
    if (m_pWarpHole)
        m_pWarpHole->Set_Active(true);

    /* Trigger (Player Warp) */
    CTrigger::TRIGGER_DESC tWarpAnimTriggerDesc = {};
    tWarpAnimTriggerDesc.eType = CTrigger::TRIGGERTYPE::ROOM_TRANSITION;
    tWarpAnimTriggerDesc.vPosition = _float3(44.75f, 0.f, 9.75f);
    tWarpAnimTriggerDesc.vTargetPosition = _float3(44.75f, 1.2f, 10.75f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::FIELD),
        TEXT("Layer_Trigger"),
        &tWarpAnimTriggerDesc, nullptr)))
        return;

    /* SFX */
    m_pGameInstance->Play_Sound(L"OBJ_Puzzle_Solved.mp3", SOUND::EFFECT);
}

void CLevel_Field::Update_MoveToWarpHole(_float _fTimeDelta)
{
    if (!m_tMoveTime.bActive)
        return;

    auto* pTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));

    pTransform->MoveTo(m_vWarpHoleCenter, _fTimeDelta, 0.1f);

    _vector vPos = pTransform->Get_State(STATE::POSITION);
    _vector vDist = XMVectorSetY(m_vWarpHoleCenter - vPos, 0.f);

    float fLen = XMVectorGetX(XMVector3Length(vDist));

    if (fLen < 0.15f)
    {
        m_tMoveTime.bActive = false;
        m_pCastingPlayer->Set_InputLock(false);

        /* Player */
        m_pCastingPlayer->Set_TransitionInput(IState::TRANSITION_INPUT::WARP_UP_REQUESTED);

        /* Warp Anim , LevelChange */
        m_tWarpTime.bActive = true;

        /* SFX */
        m_pGameInstance->Play_Sound(L"OBJ_WarpIn.wav", SOUND::EFFECT);
    }
}

void CLevel_Field::Clamp_Camera()
{
    auto pTransform = static_cast<CTransform*>(m_pCamera->Get_Component(TEXT("Com_Transform")));
    _vector vPos = pTransform->Get_State(STATE::POSITION);

    _float x = XMVectorGetX(vPos);
    _float y = XMVectorGetY(vPos);
    _float z = XMVectorGetZ(vPos);

    /* Field 영역 제한 */
    x = max(10.f, min(x, 60.f));
    z = max(-10.f, min(z, 40.f));

    pTransform->Set_State(STATE::POSITION, XMVectorSet(x, y, z, 1.f));
}

CLevel_Field* CLevel_Field::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CLevel_Field* pInstance = new CLevel_Field(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CLevel_Field");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLevel_Field::Free()
{
    __super::Free();

    m_pPlayer = nullptr;
    m_pCamera = nullptr;
    Safe_Release(m_pCameraController);
}
