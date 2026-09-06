#include "Level_Dungeon.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "Camera_Static.h"
#include "CameraController.h"
#include "GameObject/Player.h"
#include "Trigger.h"
#include "UIObject.h"
#include "GameObject/MapObject_Anim.h"
#include "GameObject/Monster_MasterStalfon.h"
#include "GameObject/Monster_StalKnightSword.h"
#include "GameObject/MapObject_Anim.h"
#include "GameObject/MapObject_NonAnim.h"
#include "GameObject/Monster_Bomber.h"
#include "GameObject/Monster_Togezo.h"
#include "GameObject/CrystalSwitch.h"

CLevel_Dungeon::CLevel_Dungeon(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CLevel{ _pDevice, _pDeviceContext }
{
}

HRESULT CLevel_Dungeon::Initialize()
{
    if (FAILED(Ready_Layer_Light()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Background(TEXT("Background"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_Camera(TEXT("Camera"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_Effect()))
        return E_FAIL;

    if (FAILED(Ready_Player_And_Camera()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Monster()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Object()))
        return E_FAIL;

    if (FAILED(Ready_Layer_UI_Item()))
        return E_FAIL;

    if (FAILED(Ready_Layer_UI_Item_Rupee()))
        return E_FAIL;

    if (FAILED(Ready_Trigger()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Shadow()))
        return E_FAIL;

    /* UI */
    CEventManager::UI_EVENT tUIEvent = {};
    tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INIT_ON;
    m_pGameInstance->Push_UIEVENT(tUIEvent);

    /* BGM */
    m_pGameInstance->Stop_AllSounds();
    m_pGameInstance->Play_BGM(L"BGM_Dungeon_KeyCarven.wav");
    m_pGameInstance->Play_Sound(L"Ambience_Dungeon_Cave.wav", SOUND::EFFECT, 0.8f);

    /* Light */
    if (FAILED(Ready_PlayerDynamicLight()))
        return E_FAIL;

    return S_OK;
}
void CLevel_Dungeon::Update(_float _fTimeDelta)
{
    /* Camera */
    if (m_pCameraController)
        m_pCameraController->Update(_fTimeDelta);

    /* EFFECT : Camera */
    Check_CameraShake(_fTimeDelta);

    /* Stair Move */
    Update_StairMove(_fTimeDelta);

    /* Event */
    Update_Event();
    Rotate_Tower(_fTimeDelta);
    Check_TreasureBox();
    Check_CryStalSwitch();
    Check_DungeonDoor();
    Check_UIItem(_fTimeDelta);
    Update_UIRupee(_fTimeDelta);
    Update_BossDoor(_fTimeDelta);
    Check_PlayerDodge(_fTimeDelta);
}

HRESULT CLevel_Dungeon::Render()
{
    SetWindowText(g_hWnd, TEXT("Dungeon 입니다."));

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_Background(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("Dungeon_Background"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::DUNGEON);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_WorldBackground"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("Background"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Logo_Background_White"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxCaveVoid"));
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

HRESULT CLevel_Dungeon::Ready_Layer_Camera(const _wstring& strLayerTag)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    SET_DESC(tObjectDesc.ObjectID, TEXT("Camera_Static"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::DUNGEON);
    SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
    SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Camera_Static"));

    CCamera::CAMERA_DESC CameraDesc = {};
    CameraDesc.vEye = _float3(0.f, 20.f, -10.f);
    CameraDesc.vAt = _float3(0.f, 0.5f, 0.f);
    CameraDesc.fFovy = XMConvertToRadians(41.5f);
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
        ENUM_TO_UINT(LEVEL::DUNGEON),
        strLayerTag, &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pCamera = static_cast<CCamera*>(pGameObject);

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Player_And_Camera()
{
    /* Player */
    m_pPlayer = m_pGameInstance->Get_Player();
    static_cast<CPlayer*>(m_pPlayer)->Ready_Navigation(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_Component_Navigation_Dungeon_PLAYER"));
    m_pGameInstance->Reset_Player_Position(15.f, 0.f, -2.f);
    //m_pGameInstance->Reset_Player_Position(89.973f, 0.f, 0.1f); //  For. Boss Test (Dungeon)
    static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")))->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180.f);

    /* Camera */
    m_pCameraController = CCameraController::Create();
    m_pCameraController->Set_Camera(m_pCamera);
    m_pCameraController->Set_Mode(CCameraController::MODE::DUNGEON);
    m_pCameraController->Set_Target(m_pPlayer);
    m_pCameraController->Set_Offset(XMVectorSet(0.f, 14.f, 0.f, 0.f));
    m_pCameraController->Set_Angle(XMVectorSet(1.f, 0.f, 0.f, 0.f), 72.f);

    /* Connet */
    m_pCastingPlayer = static_cast<CPlayer*>(m_pPlayer);

    /* Input Lock/UnLock */
    m_pCastingPlayer->Set_InputLock(false);

    /* Detail */
    m_pCastingPlayer->Set_InputMode(CPlayerInputController::INPUT_MODE::TOP);
    m_pCastingPlayer->Set_ViewMode(CPlayer::VIEW_MODE::TOP);

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Trigger()
{
    /* Trigger (1st Down) */
    CTrigger::TRIGGER_DESC tTriggerDesc = {};
    tTriggerDesc.eType = CTrigger::TRIGGERTYPE::ROOM_TRANSITION;
    tTriggerDesc.vPosition = _float3(17.263f, 0.f, 48.713f);
    tTriggerDesc.vTargetPosition = _float3(89.5f, 0.f, -5.2f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::DUNGEON),
        TEXT("Layer_Trigger"),
        &tTriggerDesc, nullptr)))
        return E_FAIL;

    /* Trigger (Boss Spawn) */
    CTrigger::TRIGGER_DESC tTriggerDesc_Cutscene_Start = {};
    tTriggerDesc_Cutscene_Start.eType = CTrigger::TRIGGERTYPE::CUTSCENE_START;
    tTriggerDesc_Cutscene_Start.vPosition = _float3(89.973f, 0.f, 4.2f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::DUNGEON),
        TEXT("Layer_Trigger"),
        &tTriggerDesc_Cutscene_Start, nullptr)))
        return E_FAIL;

    /* Trigger (Boss Spawn) */
    CTrigger::TRIGGER_DESC tTriggerDesc_BossSpawn = {};
    tTriggerDesc_BossSpawn.eType = CTrigger::TRIGGERTYPE::SPAWN_BOSS;
    tTriggerDesc_BossSpawn.vPosition = _float3(89.973f, 0.f, 9.1f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::DUNGEON),
        TEXT("Layer_Trigger"),
        &tTriggerDesc_BossSpawn, nullptr)))
        return E_FAIL;

    /* Trigger (Tower Spawn) */
    CTrigger::TRIGGER_DESC tTriggerDesc_TowerSpawn = {};
    tTriggerDesc_TowerSpawn.eType = CTrigger::TRIGGERTYPE::SPAWN_TOWER;
    tTriggerDesc_TowerSpawn.vPosition = _float3(90.f, 0.f, 21.5f);
    //tTriggerDesc.vPosition = _float3(90.299f, 1.5f, 26.72f); // Tower position

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::DUNGEON),
        TEXT("Layer_Trigger"),
        &tTriggerDesc_TowerSpawn, nullptr)))
        return E_FAIL;

    /* Trigger (Level_Change) */
    CTrigger::TRIGGER_DESC tTriggerDesc_LevelChange = {};
    tTriggerDesc_LevelChange.eType = CTrigger::TRIGGERTYPE::LEVEL_CHANGE;
    tTriggerDesc_LevelChange.iValue = ENUM_TO_UINT(LEVEL::TOWER);
    tTriggerDesc_LevelChange.vPosition = _float3(90.f, 0.f, 23.f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        TEXT("Prototype_GameObject_Trigger"),
        ENUM_TO_UINT(LEVEL::DUNGEON),
        TEXT("Layer_Trigger"),
        &tTriggerDesc_LevelChange, nullptr)))
        return E_FAIL;

    /* Trigger (TreasureBox Open) */
    for (size_t i = 0; i < m_TreasureBoxes.size(); ++i)
    {
        auto* pBase = m_TreasureBoxes[i].pBase;
        if (!pBase)
            continue;

        auto* pTransform = static_cast<CTransform*>(pBase->Get_Component(TEXT("Com_Transform")));

        _float3 vPos;
        XMStoreFloat3(&vPos, pTransform->Get_State(STATE::POSITION));

        CTrigger::TRIGGER_DESC tDesc = {};
        tDesc.eType = CTrigger::TRIGGERTYPE::BOX_OPEN;
        tDesc.vPosition = vPos;
        tDesc.vScale = _float3(1.f, 0.5f, 1.f);
        tDesc.iValue = static_cast<int>(i);

        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            RESOURCE_LEVEL_STATIC,
            TEXT("Prototype_GameObject_Trigger"),
            ENUM_TO_UINT(LEVEL::DUNGEON),
            TEXT("Layer_Trigger"),
            &tDesc, nullptr)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_Monster()
{
    /* EaglesTower */
    m_pEalgesTower = static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("EaglesTowerout_1")));
    m_pEalgesTower->Set_Active(false);

    /* (Boss) MasterStalfon */
    m_pMasterStalfon = static_cast<CMonster_MasterStalfon*>(m_pGameInstance->Find_Object(TEXT("MasterStalfon_0")));
    m_pMasterStalfon->Set_Active(false);
    static_cast<CTransform*>(m_pMasterStalfon->Get_Component(TEXT("Com_Transform")))->Set_State(STATE::POSITION, XMVectorSet(90.f, 0.f, 12.f, 1.f));
    static_cast<CTransform*>(m_pMasterStalfon->Get_Component(TEXT("Com_Transform")))->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);

    /* StalKnightSword */
    m_vecStalKnightSwords.push_back(static_cast<CMonster_StalKnightSword*>(m_pGameInstance->Find_Object(TEXT("StalKnightSword_0"))));
    m_pStalKnightSword_Crystal = static_cast<CMonster_StalKnightSword*>(m_pGameInstance->Find_Object(TEXT("StalKnightSword_1")));

    /* Togezo */
    m_vecTogezos.push_back(static_cast<CMonster_Togezo*>(m_pGameInstance->Find_Object(TEXT("Togezo_0"))));
    m_vecTogezos.push_back(static_cast<CMonster_Togezo*>(m_pGameInstance->Find_Object(TEXT("Togezo_1"))));
    static_cast<CTransform*>(m_vecTogezos[0]->Get_Component(TEXT("Com_Transform")))->Set_State(STATE::POSITION, XMVectorSet(16.8f, 0.f, 25.9f, 1.f));
    static_cast<CTransform*>(m_vecTogezos[1]->Get_Component(TEXT("Com_Transform")))->Set_State(STATE::POSITION, XMVectorSet(16.8f, 0.f, 24.1f, 1.f));

    /* BomberRed */
    m_vecBombers.push_back(static_cast<CMonster_Bomber*>(m_pGameInstance->Find_Object(TEXT("BomberRed_0"))));
    m_vecBombers.push_back(static_cast<CMonster_Bomber*>(m_pGameInstance->Find_Object(TEXT("BomberRed_1"))));
    m_vecBombers.push_back(static_cast<CMonster_Bomber*>(m_pGameInstance->Find_Object(TEXT("BomberRed_2"))));
    m_vecBombers.push_back(static_cast<CMonster_Bomber*>(m_pGameInstance->Find_Object(TEXT("BomberRed_3"))));

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_Object()
{
    /* TreasureBox */
    for (_int i = 0; i <= m_iTreasureBoxCnt; ++i)
    {
        TREASUREBOX tBox;
        _wstring BaseName = L"Treasure_Base_" + to_wstring(i);
        _wstring LidName = L"Treasure_Lid_" + to_wstring(i);

        tBox.pBase = static_cast<CMapObject_NonAnim*>(m_pGameInstance->Find_Object(BaseName.c_str()));
        tBox.pLid = static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(LidName.c_str()));

        tBox.pLid->Set_Animation("not_open", true);

        tBox.pBase->Set_RenderActive(false);
        tBox.pLid->Set_RenderActive(false);

        m_TreasureBoxes.push_back(tBox);
    }

    /* Excep. */
    m_TreasureBoxes[4].pBase->Set_RenderActive(true);
    m_TreasureBoxes[4].pLid->Set_RenderActive(true);

    /* DungeonDoor */
    m_vecDungeonDoors.push_back(static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("DungeonDoor_0"))));
    m_vecDungeonDoors.push_back(static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("DungeonDoor_1"))));
    for (auto DungeonDoor : m_vecDungeonDoors)
    {
        DungeonDoor->Set_Animation("close2", false);
    }

    /* BossDoor */
    m_pBossDoor = static_cast<CMapObject_Anim*>(m_pGameInstance->Find_Object(TEXT("BossDoor_0")));
    m_pBossDoor->Set_Animation("remove_key", false);

    /* CrystalSwitch */
    m_pCrystalSwitch = static_cast<CCrystalSwitch*>(m_pGameInstance->Find_Object(TEXT("CrystalSwitchout_0")));
    m_pCrystalSwitch->Set_Active(false);

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_Effect()
{
    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_Light()
{
    m_pGameInstance->Load_LightBinary(ENUM_TO_UINT(LEVEL::DUNGEON), ENUM_TO_UINT(LEVEL::DUNGEON));
    m_pGameInstance->Scale_AllPointLightIntensity(3.f);

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_UI_Item()
{
    {
        CGameObject::OBJECT_DESC tObjectDesc_DungeonKey = {};
        wcscpy_s(tObjectDesc_DungeonKey.ObjectID, TEXT("DungeonKey"));
        tObjectDesc_DungeonKey.iLevel = RESOURCE_LEVEL_STATIC;
        wcscpy_s(tObjectDesc_DungeonKey.ObjectTag, TEXT("Prototype_GameObject_MapObject_NonAnim"));
        wcscpy_s(tObjectDesc_DungeonKey.LayerTag, TEXT("Layer_UI_Object"));
        wcscpy_s(tObjectDesc_DungeonKey.ShaderTag, TEXT("Prototype_Component_Shader_VtxMesh"));
        wcscpy_s(tObjectDesc_DungeonKey.ModelTag, TEXT("Prototype_Component_Model_Map_SmallKey"));

        CMapObject::MAP_INIT_DESC tInitDesc_DungeonKey = {};
        tInitDesc_DungeonKey.tObjectDesc = tObjectDesc_DungeonKey;

        CGameObject* pGameObject = { nullptr };
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            RESOURCE_LEVEL_STATIC,
            tObjectDesc_DungeonKey.ObjectTag,
            tObjectDesc_DungeonKey.iLevel,
            tObjectDesc_DungeonKey.LayerTag,
            &tInitDesc_DungeonKey, &pGameObject)))
            return E_FAIL;
        UI_ITEM tItem = { static_cast<CMapObject_NonAnim*>(pGameObject), { false, 0.f, 2.f }};
        m_vecUIItems.push_back(tItem);
        tItem.pItem->Set_Active(false);
    }

    {
        CGameObject::OBJECT_DESC tObjectDesc_Rupee = {};
        wcscpy_s(tObjectDesc_Rupee.ObjectID, TEXT("Rupee"));
        tObjectDesc_Rupee.iLevel = RESOURCE_LEVEL_STATIC;
        wcscpy_s(tObjectDesc_Rupee.ObjectTag, TEXT("Prototype_GameObject_MapObject_NonAnim"));
        wcscpy_s(tObjectDesc_Rupee.LayerTag, TEXT("Layer_UI_Object"));
        wcscpy_s(tObjectDesc_Rupee.ShaderTag, TEXT("Prototype_Component_Shader_VtxMesh"));
        wcscpy_s(tObjectDesc_Rupee.ModelTag, TEXT("Prototype_Component_Model_Map_RupeeGold"));

        CMapObject::MAP_INIT_DESC tInitDesc_Rupee = {};
        tInitDesc_Rupee.tObjectDesc = tObjectDesc_Rupee;

        CGameObject* pGameObject = { nullptr };
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            RESOURCE_LEVEL_STATIC,
            tObjectDesc_Rupee.ObjectTag,
            tObjectDesc_Rupee.iLevel,
            tObjectDesc_Rupee.LayerTag,
            &tInitDesc_Rupee, &pGameObject)))
            return E_FAIL;
        UI_ITEM tItem = { static_cast<CMapObject_NonAnim*>(pGameObject), { false, 0.f, 2.f } };
        m_vecUIItems.push_back(tItem);
        tItem.pItem->Set_Active(false);

        /* SFX */
        m_tRupeeSFX.Start(0.1f);
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_Layer_UI_Item_Rupee()
{
    {
        CGameObject::OBJECT_DESC tObjectDesc = {};
        wcscpy_s(tObjectDesc.ObjectID, TEXT("BossKey"));
        tObjectDesc.iLevel = RESOURCE_LEVEL_STATIC;
        wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_MapObject_NonAnim"));
        wcscpy_s(tObjectDesc.LayerTag, TEXT("Layer_UI_Object"));
        wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxMesh"));
        wcscpy_s(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_BossKey"));

        CMapObject::MAP_INIT_DESC tInitDesc = {};
        tInitDesc.tObjectDesc = tObjectDesc;

        CGameObject* pGameObject = { nullptr };
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            RESOURCE_LEVEL_STATIC,
            tObjectDesc.ObjectTag,
            tObjectDesc.iLevel,
            tObjectDesc.LayerTag,
            &tInitDesc, &pGameObject)))
            return E_FAIL;
        UI_ITEM tItem = { static_cast<CMapObject_NonAnim*>(pGameObject), { false, 0.f, 2.f } };
        m_vecUIItems.push_back(tItem);
        tItem.pItem->Set_Active(false);
    }
    return S_OK;
}

HRESULT CLevel_Dungeon::Ready_PlayerDynamicLight()
{
    /* Player Dynamic Light */
    LIGHT_DESC m_tPlayerLight = {};
    m_tPlayerLight.eType = LIGHT::POINT;
    m_tPlayerLight.vDiffuse = { 0.77f, 0.956f, 0.253f, 1.f };
    m_tPlayerLight.vAmbient = { 0.2f, 0.2f, 0.2f, 1.f };
    m_tPlayerLight.vSpecular = { 1.f, 1.f, 1.f, 1.f };
    m_tPlayerLight.fRange = 2.4f;
    m_tPlayerLight.fIntensity = 3.84f;
    m_tPlayerLight.eUsage = LIGHT_USAGE::DYNAMIC;
    CTransform* pTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));
    m_pGameInstance->Add_Light(m_tPlayerLight, pTransform, XMVectorSet(0.f, 1.5f, 0.f, 1.f));

    return S_OK;
}

HRESULT CLevel_Dungeon::Update_Event()
{
    /* EventManager */
    CEventManager::GAME_EVENT tEvent = {};
    while (m_pGameInstance->Pop_GameEVENT(tEvent))
    {
        Handle_GameEvent(tEvent);
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Handle_GameEvent(CEventManager::GAME_EVENT& _tEvent)
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

        /* LEVEL */
        m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(LEVEL::LOADING),
            CLevel_Loading::Create(m_pDevice, m_pDeviceContext, (LEVEL)_tEvent.iValue));

        /* Clear Light */
        m_pGameInstance->Clear_Light();
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::ROOM_TRANSITION:
    {
        /* Stair */
        _float3 vPos;
        XMStoreFloat3(&vPos, _tEvent.vPos);
        m_pGameInstance->Reset_Player_Position(vPos.x, vPos.y, vPos.z);
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::STAIR:
    {
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::CUTSCENE_START:
    {
        /* bosskey */
        //m_pCastingPlayer->Set_InputLock(true);
        m_pCastingPlayer->Reset_Animation_TrackPosition();
        m_pCastingPlayer->Set_TransitionInput(IState::TRANSITION_INPUT::DOOR_OPEN_REQUESTED);
        m_pBossDoor->Set_Animation("open", false);
        m_pCameraController->Start_Shake(0.1f, 0.1f);
        m_tBossDoorOpen.bActive = true;

        /* SFX */
        m_pGameInstance->Stop_Sound(SOUND::BGM);
        m_pGameInstance->Play_Sound(L"OBJ_Door_Key_Edit.m4a", SOUND::EFFECT, 2.f);
        m_pGameInstance->Play_Sound(L"OBJ_Door_Open.wav", SOUND::EFFECT, 3.f);
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::SPAWN_BOSS:
    {
        /* Player */
        m_pCastingPlayer->Set_InputLock(false);

        /* Boss */
        m_pMasterStalfon->Set_Active(true);

        m_pCameraController->Set_Mode(CCameraController::MODE::ZOOM);
        auto* pBossTransform = static_cast<CTransform*>(
            m_pMasterStalfon->Get_Component(TEXT("Com_Transform")));
        m_pCameraController->Set_Event(pBossTransform->Get_State(STATE::POSITION));
        m_pCameraController->Set_Zoom(3.9f, 3.f);
        m_pCameraController->Start_Shake(2.f, 0.15f);

        /* SFX */
        m_pGameInstance->Play_BGM(L"BGM_MasterStalfon_Short.wav", 1.f, true);
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::SPAWN_TOWER:
    {
        /* Light */
        m_pGameInstance->Clear_Light();
        m_pGameInstance->Load_LightBinary(ENUM_TO_UINT(LEVEL::DUNGEON), 888);

        /* Input Lock/UnLock */
        m_pCastingPlayer->Set_InputLock(true);

        /* Spawn Tower Object */
        m_pEalgesTower->Set_Active(true);

        m_tTowerRotTime.bActive = true;
        m_tTowerRotTime.fAccTime = 0.f;

        auto* pCamTransform = static_cast<CTransform*>(
            m_pCamera->Get_Component(TEXT("Com_Transform")));
        _vector vOffset = XMVectorSet(0.f, -3.5f, -4.2f, 1.f);
        _vector vFinal = pCamTransform->Get_State(STATE::POSITION) + vOffset;
        vFinal = XMVectorSetW(vFinal, 1.f);

        m_pCameraController->Set_Mode(CCameraController::MODE::STATIC);
        m_pCameraController->Set_StaticPos(vFinal,
            XMVectorSet(0.f, 0.5f, 0.f, 1.f), 5.f);
        m_pCameraController->Set_Angle(XMVectorSet(1.f, 0.f, 0.f, 0.f), 50.f);

        /* SFX */
        m_pGameInstance->Play_Sound(L"OBJ_Door_Open.wav", SOUND::EFFECT, 2.f);
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::BOX_OPEN:
    {
        _int index = _tEvent.iValue;
        auto* pTreasureTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));

        if (index >= 0 && index < m_TreasureBoxes.size())
        {
            m_TreasureBoxes[index].pLid->Set_Animation("open", false);
            m_pCastingPlayer->Set_TransitionInput(IState::TRANSITION_INPUT::GET_ITEM_REQUESTED);

            m_pCameraController->Set_Mode(CCameraController::MODE::ZOOM);
        }

        /* UI_Item */
        m_vecUIItems.front().pItem->Set_Active(true);
        m_vecUIItems.front().m_tLifTime.bActive = true;
        auto pPlayerTrans = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")))->Get_State(STATE::POSITION);
        _vector vItemOffset = XMVectorSet(0.f, 2.5f, 1.f, 0.f);
        auto pDungeonKeyTrans = static_cast<CTransform*>(m_vecUIItems.front().pItem->Get_Component(TEXT("Com_Transform")));
        pDungeonKeyTrans->Set_State(STATE::POSITION, pPlayerTrans + vItemOffset);

        /* SFX */
        m_pGameInstance->Play_Sound(L"Player_Item_Get.mp3", SOUND::EFFECT, 1.f);
        
        /* Light */
        LIGHT_DESC m_tPlayerLight = {};
        m_tPlayerLight.eType = LIGHT::POINT;
        m_tPlayerLight.vDiffuse = { 1.f, 1.f, 1.f, 1.f };
        m_tPlayerLight.vAmbient = { 0.2f, 0.2f, 0.2f, 1.f };
        m_tPlayerLight.vSpecular = { 1.f, 1.f, 1.f, 1.f };
        m_tPlayerLight.fRange = 5.f;
        m_tPlayerLight.fIntensity = 3.f;
        m_tPlayerLight.eUsage = LIGHT_USAGE::DYNAMIC;
        CTransform* pTransform = static_cast<CTransform*>(pDungeonKeyTrans);
        m_pGameInstance->Add_Light(m_tPlayerLight, pTransform, XMVectorSet(0.f, 2.5f, -0.2f, 1.f));

        switch (_tEvent.iValue)
        {
        case 1:
        {
            /* Camera */
            _vector vOffst = XMVectorSet(15.f, 2.f, 0.f, 0.f);
            m_pCameraController->Set_Event(pTreasureTransform->Get_State(STATE::POSITION) + vOffst);
            m_pCameraController->Set_Zoom(1.2f, 1.f, 2.5f);

            /* Load Dialogue */
            CGameInstance::GetInstance()->Load_XMLFile(L"../../Resources/Data/Dialogue/01_Get_Item_SmallKey.xml");
            CGameInstance::GetInstance()->Start_Dialogue(L"item");
        }
        break;
        case 2:
        {
            /* Camera */
            _vector vOffst = XMVectorSet(15.f, 2.f, 0.f, 0.f);
            m_pCameraController->Set_Event(pTreasureTransform->Get_State(STATE::POSITION) + vOffst);
            m_pCameraController->Set_Zoom(1.2f, 1.f, 2.5f);

            /* Load Dialogue */
            CGameInstance::GetInstance()->Load_XMLFile(L"../../Resources/Data/Dialogue/03_Get_Item_Rupee_100.xml");
            CGameInstance::GetInstance()->Start_Dialogue(L"item");

            /* UI */
            m_iRemainRupee = 1000;
        }
        break;
        case 4:
        {
            /* Camera */
            _vector vOffst = XMVectorSet(0.f, 7.f, 0.f, 0.f);
            m_pCameraController->Set_Event(pTreasureTransform->Get_State(STATE::POSITION) + vOffst);
            m_pCameraController->Set_Zoom(1.2f, 1.f, 2.5f);

            /* Load Dialogue */
            CGameInstance::GetInstance()->Load_XMLFile(L"../../Resources/Data/Dialogue/02_Get_Item_BossKey.xml");
            CGameInstance::GetInstance()->Start_Dialogue(L"item");
        }
        break;
        }
    }
    break;
    case CEventManager::GAME_EVENT_TYPE::PLAYER_DAMAGE:
    {
        if (m_pCastingPlayer)
            m_pCastingPlayer->Take_Damage(_tEvent.fValue1, _tEvent.vPos);
    }
    break;
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Check_DungeonDoor()
{
    /* Check */
    _int iTogezoInActive = 0;
    for (auto Togezo : m_vecTogezos)
    {
        if (!Togezo->IsActive())
            iTogezoInActive++;
    }

    /* Condition */
    if (!m_bDungeonDoorOpen && iTogezoInActive >= 2)
    {
        /* Door */
        for (auto DungeonDoor : m_vecDungeonDoors)
        {
            /* SFX */
            m_pGameInstance->Play_Sound(L"OBJ_Door_Open.wav", SOUND::EFFECT, 3.f);

            /* Door */
            DungeonDoor->Set_Animation("open_wait1", false);
            m_pCameraController->Start_Shake(0.1f, 0.1f);

            /* Effect */
            auto pTransform = static_cast<CTransform*>(
                DungeonDoor->Get_Component(TEXT("Com_Transform")));

            _vector basePos = pTransform->Get_State(STATE::POSITION);

            _int iCount = 8;
            _float fSpacing = 0.15f;

            for (_int i = 0; i < iCount; ++i)
            {
                CEffectManager::EFFECT_EVENT tEvent{};
                tEvent.eType = CEffectManager::EFFECT_EVENT_TYPE::SPAWN_EFFECT;
                tEvent.EffectName = TEXT("Dust_Smoke");
                
                _float fOffset = (i - iCount * 0.5f) * fSpacing;
                fOffset += ((rand() % 100) / 100.f - 0.5f) * 0.05f;

                _float fY = ((rand() % 100) / 100.f) * 0.08f;
                _float fZ = ((rand() % 100) / 100.f - 0.5f) * 0.1f;

                _vector vPos = basePos + XMVectorSet(fOffset, fY, fZ, 0.f);
                tEvent.vPosition = vPos;

                _vector vDir = XMVectorSet(
                    ((rand() % 100) / 100.f - 0.5f) * 0.3f,
                    1.f,
                    ((rand() % 100) / 100.f - 0.5f) * 0.3f,
                    0.f
                );
                tEvent.vDirection = XMVector3Normalize(vDir);

                CGameInstance::GetInstance()->Push_EffectEVENT(tEvent);
            }
        }
        m_bDungeonDoorOpen = true;
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Check_TreasureBox()
{
    /* Check */
    _int iStalKnightInActive = 0;
    for (auto StalKnight : m_vecStalKnightSwords)
    {
        if (!StalKnight->IsActive())
            iStalKnightInActive++;
    }

    _int iBomberInActive = 0;
    for (auto Bomber : m_vecBombers)
    {
        if (!Bomber->IsActive())
            iBomberInActive++;
    }

    /* 조건 */
    if (!m_bBoxOpen_1 && iStalKnightInActive >= 1 && iBomberInActive >= 2)
    {
        m_TreasureBoxes[1].pBase->Set_RenderActive(true);
        m_TreasureBoxes[1].pLid->Set_RenderActive(true);
        m_bBoxOpen_1 = true;

        /* EFFECT */
        

        /* SFX */
        m_pGameInstance->Play_Sound(L"OBJ_Treasure_Appear.wav", SOUND::EFFECT);
        m_pGameInstance->Play_Sound(L"ENEMY_Bomber_Delete.wav", SOUND::EFFECT);
    }
    if (!m_bBoxOpen_2 && iBomberInActive >= 4)
    {
        /* TreasureBox */
        m_TreasureBoxes[2].pBase->Set_RenderActive(true);
        m_TreasureBoxes[2].pLid->Set_RenderActive(true);
        m_bBoxOpen_2 = true;

        /* SFX */
        m_pGameInstance->Play_Sound(L"OBJ_Treasure_Appear.wav", SOUND::EFFECT);
        m_pGameInstance->Play_Sound(L"ENEMY_Bomber_Delete.wav", SOUND::EFFECT);
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Check_CryStalSwitch()
{
    if (!m_bCrystalSwitchSpawn && !m_pStalKnightSword_Crystal->IsActive())
    {
        m_pCrystalSwitch->Set_Active(true);
        
        /* Position */
        auto pStalKnightSwordTrans = static_cast<CTransform*>(m_pStalKnightSword_Crystal->Get_Component(TEXT("Com_Transform")));
        auto pCrystalSwitch = static_cast<CTransform*>(m_pCrystalSwitch->Get_Component(TEXT("Com_Transform")));
        pCrystalSwitch->Set_State(STATE::POSITION, pStalKnightSwordTrans->Get_State(STATE::POSITION));

        m_bCrystalSwitchSpawn = true;

        /* SFX */
        m_pGameInstance->Play_Sound(L"OBJ_Treasure_Appear.wav", SOUND::EFFECT);
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Check_UIItem(_float _fTimeDelta)
{
    for (auto& Item : m_vecUIItems)
    {
        if (!Item.m_tLifTime.bActive)
            continue;

        Item.m_tLifTime.fAccTime += _fTimeDelta;

        /* Y 둥둥 */
        auto pTransform = static_cast<CTransform*>(Item.pItem->Get_Component(TEXT("Com_Transform")));
        _vector vPos = pTransform->Get_State(STATE::POSITION);
        vPos += XMVectorSet(0.f, _fTimeDelta * 0.5f, 0.f, 0.f);

        /* Rotation + Scale */
        _float t = Item.m_tLifTime.fAccTime;
        _float duration = Item.m_tLifTime.fAccDurationTime;
        _float ratio = t / duration;
        ratio = min(ratio, 1.f);
        _float speedFactor = powf(1.f - ratio, 3.f);
        _float maxSpeed = 3000.f;
        _float currentSpeed = maxSpeed * speedFactor;
        pTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), _fTimeDelta * currentSpeed);

        if (ratio >= 0.95f)
        {
            pTransform->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);
        }

        if (ratio >= 0.9f)
        {
            _float burst = 2.f + sinf((ratio - 0.9f) * 50.f) * 0.3f;
            pTransform->Set_Scale(burst, burst, burst);
            m_pCameraController->Start_Shake(0.1f, 0.1f);
        }

        if (Item.m_tLifTime.fAccTime >= Item.m_tLifTime.fAccDurationTime)
        {
            Item.m_tLifTime.bActive = false;
            m_pGameInstance->Reserve_DeleteObject(Item.pItem);
            m_vecUIItems.erase(m_vecUIItems.begin());

            /* Light */
            m_pGameInstance->Pop_Back_Lights(1);
        }
    }
    return S_OK;
}

HRESULT CLevel_Dungeon::Update_UIRupee(_float _fTimeDelta)
{
    /* UI Rupee Effect */
    static _float fAccRupee = 0.f;
    _float fSpeed = 400.f;

    if (m_iRemainRupee > 0)
    {
        /* SFX */
        if (m_tRupeeSFX.Tick_Loop(_fTimeDelta))
        {
            m_pGameInstance->Play_Sound(L"OBJ_Rupee.wav", SOUND::EFFECT);
        }

        /* Rupee */
        _float fAdd = fSpeed * _fTimeDelta;
        fAccRupee += fAdd;

        _int iAdd = (_int)fAccRupee;

        if (iAdd > 0)
        {
            fAccRupee -= iAdd;

            iAdd = min(iAdd, m_iRemainRupee);

            CEventManager::UI_EVENT tUIEvent = {};
            tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::RUPEE_ADD;
            tUIEvent.iValue = iAdd;

            m_pGameInstance->Push_UIEVENT(tUIEvent);

            m_pCameraController->Start_Shake(0.05f, 0.025f);

            m_iRemainRupee -= iAdd;
        }

        if (m_iRemainRupee == 0)
        {
            m_pCameraController->Start_Shake(0.2f, 0.3f);
        }
    }

    return S_OK;
}

HRESULT CLevel_Dungeon::Update_BossDoor(_float _fTimeDelta)
{
    if (m_tBossDoorOpen.bActive)
    {
        auto pBossDoorTrans = static_cast<CTransform*>(m_pBossDoor->Get_Component(TEXT("Com_Transform")));
        m_tBossDoorOpen.fAccTime += _fTimeDelta;

        _float fRatio = m_tBossDoorOpen.fAccTime / m_tBossDoorOpen.fAccDurationTime;
        if (fRatio >= 0.5f)
        {
            pBossDoorTrans->Go_Up(_fTimeDelta * 30.f);
        }

        if (m_tBossDoorOpen.fAccTime >= m_tBossDoorOpen.fAccDurationTime)
        {
            /* After Boss Door Open */
            m_pBossDoor->Set_Active(false);
            m_tBossDoorOpen.bActive = false;
        }
    }

    return S_OK;
}

void CLevel_Dungeon::Rotate_Tower(_float _fTimeDelta)
{
    if (m_tTowerRotTime.bActive)
    {
        m_tTowerRotTime.fAccTime += _fTimeDelta;

        auto* pTransform = static_cast<CTransform*>(
            m_pEalgesTower->Get_Component(TEXT("Com_Transform")));
        pTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), _fTimeDelta * 60.f);

        if (m_tTowerRotTime.fAccTime >= m_tTowerRotTime.fAccDurationTime)
        {
            m_tTowerRotTime.bActive = false;

            /* Input Lock/UnLock */
            m_pCastingPlayer->Set_InputLock(false);
        }
    }
}

void CLevel_Dungeon::Update_StairMove(_float _fTimeDelta)
{
    if (!m_bStairMoving)
        return;

    auto* pTransform = static_cast<CTransform*>(m_pPlayer->Get_Component(TEXT("Com_Transform")));

    pTransform->MoveTo(m_vStairTarget, _fTimeDelta, 0.1f);

    _vector vPos = pTransform->Get_State(STATE::POSITION);
    _vector vDist = XMVectorSetY(m_vStairTarget - vPos, 0.f);

    float fLen = XMVectorGetX(XMVector3Length(vDist));

    if (fLen < 0.15f)
    {
        m_bStairMoving = false;
        m_pCastingPlayer->Set_InputLock(false);
    }
}

void CLevel_Dungeon::Check_CameraShake(_float _fTimeDelta)
{
    if (m_pCastingPlayer)
    {
        if (m_pCastingPlayer->Notify_CameraShake())
        {
            m_pGameInstance->Start_HitStop(0.05f, 0.05f);
            m_pCameraController->Start_Shake(0.1f, 0.15f);
        }
    }

    if (m_pCrystalSwitch)
    {
        if (m_pCrystalSwitch->Check_CameraShake())
        {
            m_pCameraController->Start_Shake(0.1f, 0.1f);
        }
    }

    if (m_pMasterStalfon)
    {
        if (m_pMasterStalfon->Check_CameraShake())
        {
            m_pCameraController->Start_Shake(0.1f, 0.5f);
        }
    }
}

HRESULT CLevel_Dungeon::Check_PlayerDodge(_float _fTimeDelta)
{
    _int iDodgePhase = m_pCastingPlayer->Get_DodgePhase();

    if (iDodgePhase != m_iPrevDodgePhase)
    {
        if (iDodgePhase == 0)
        {
            //// 카메라 숄더뷰
            m_pCameraController->Enter_ShoulderView();
        }
        else if (iDodgePhase == 1)
        {
            m_pCameraController->Set_Mode(CCameraController::MODE::SHOULDERVIEW);
        }
        else if (iDodgePhase == 2)
        {
        }
        else if (iDodgePhase == 3)
        {

            // 카메라 원상태로 복구
            m_pCameraController->Exit_ShoulderView();

            m_pCameraController->Set_Mode(CCameraController::MODE::DUNGEON);
            m_pCameraController->Set_Target(m_pPlayer);
            m_pCameraController->Set_Offset(XMVectorSet(0.f, 14.f, 0.f, 0.f));
            m_pCameraController->Set_Angle(XMVectorSet(1.f, 0.f, 0.f, 0.f), 72.f);
        }
        
        m_iPrevDodgePhase = iDodgePhase;
    }

    return S_OK;
}

CLevel_Dungeon* CLevel_Dungeon::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CLevel_Dungeon* pInstance = new CLevel_Dungeon(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CLevel_Dungeon");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLevel_Dungeon::Free()
{
    __super::Free();

    m_pPlayer = nullptr;
    m_pCamera = nullptr;
    Safe_Release(m_pCameraController);
}
