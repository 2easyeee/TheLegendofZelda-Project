#include "MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "UIManager.h"
#include "UI_Transition.h"
#include "UI_Heart.h"
#include "UI_Image.h"
#include "UI_Number.h"
#include "UI_Slot.h"
#include "UI_Item.h"
#include "UI_Spinner.h"
#include "UI_WorldBackground.h"
#include "GameObject/MapObject_Anim.h"
#include "GameObject/MapObject_NonAnim.h"
#include "GameObject/Player.h"
#include "GameObject/Body_Player.h"
#include "GameObject/Weapon_Player.h"
#include "Camera_Dynamic.h"
#include "Camera_Static.h"
#include "GameObject/Body_Monster.h"
#include "GameObject/Monster.h"
#include "GameObject/Weapon_Monster.h"
#include "GameObject/Monster_MasterStalfon.h"
#include "GameObject/Monster_Togezo.h"
#include "GameObject/Monster_Gohma.h"
#include "GameObject/MoriblinSpear.h"
#include "GameObject/Body_MoriblinSpear.h"
#include "GameObject/Weapon_MoriblinSpear.h"
#include "GameObject/Spear.h"
#include "GameObject/Water.h"
#include "GameObject/Cloud.h"
#include "GameObject/Accessory_Player.h"
#include "GameObject/Boomerang.h"
#include "GameObject/Monster_MoriblinSword.h"
#include "GameObject/Monster_Albatoss.h"
#include "Trigger.h"
#include "GameObject/NPC.h"
#include "GameObject/Monster_BuzzBlob.h"
#include "Ladder.h"
#include "GameObject/Albatoss_Feather.h"
#include "GameObject/MapObject_Collision.h"
#include "GameObject/Monster_Bomber.h"
#include "GameObject/Grass.h"
#include "GameObject/Rupee.h"
#include "GameObject/CrystalSwitch.h"
#include "GameObject/ColorToggleBlock.h"
#include "GameObject/MasterStalfon_ShadowIndicator.h"
#include "GameObject/MapObject_Boundary.h"
#include "GameObject/Monster_StalKnightSword.h"
#include "GameObject/Snow.h"
#include "GameObject/Explosion.h"
#include "GameObject/SwordSlash.h"
#include "GameObject/MapObject_Light.h"
#include "GameObject/Fire_01.h"
#include "GameObject/Fire_02.h"
#include "GameObject/Grass_Cut.h"
#include "GameObject/Star.h"
#include "GameObject/PurpleSmoke.h"
#include "TrailEffect.h"
#include "GameObject/DustSmoke.h"
#include "GameObject/Fire_03.h"
#include "GameObject/DungeonFire.h"

CMainApp::CMainApp()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CMainApp::Initialize()
{
	/* 1. Engine 초기화 */
	ENGINE_DESC EngineDesc = {};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eMode = WINMODE::WIN;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.iTotalLevelCnt = ENUM_TO_UINT(LEVEL::END);

	if (FAILED(m_pGameInstance->Initialize_Engine(EngineDesc, &m_pDevice, &m_pDeviceContext)))
		return E_FAIL;

	/* 2-0. Set STATIC Resources */
	Loading_For_Level_Static();

	/* 2-1. Set Init Level */
 	if (FAILED(Start_Level(LEVEL::LOGO)))
		return E_FAIL;

	/* 3. UIManager 생성 */
	if (FAILED(Register_UIManager()))
		return E_FAIL;

	/* Font */
	if (FAILED(m_pGameInstance->Add_Fonts(m_pDevice, m_pDeviceContext,
		TEXT("Quicksand-Medium"),
		TEXT("../../Resources/Fonts/korean_bold_italic.spritefont"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Fonts(m_pDevice, m_pDeviceContext,
		TEXT("Quicksand-Bold"),
		TEXT("../../Resources/Fonts/korean_extrabold.spritefont"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Fonts(m_pDevice, m_pDeviceContext,
		TEXT("Cafe24Ssurround_24"),
		TEXT("../../Resources/Fonts/Cafe24Ssurround_24.spritefont"))))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(_float _fTimeDelta)
{
	m_pGameInstance->Update_Engine(_fTimeDelta);

	m_pUIManager->Update(_fTimeDelta);
}

HRESULT CMainApp::Render()
{
	if (FAILED(m_pGameInstance->Begin_Draw(_float4(0.f, 0.f, 1.f, 1.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Draw()))
		return E_FAIL;

#ifdef _DEBUG
	Show_FPS(m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default")));
#endif // _DEBUG

	if (FAILED(m_pGameInstance->End_Draw()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Start_Level(LEVEL _eStartLevel)
{
	m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pDeviceContext, _eStartLevel));

	return S_OK;
}

HRESULT CMainApp::Register_UIManager()
{
	/* UI_Manager */
	m_pUIManager = CUIManager::Create();

	return S_OK;
}

HRESULT CMainApp::Loading_For_Level_Static()
{
	/* Texture */
#pragma region TEXTURE
	/* For.Prototype_Component_Texture_Loading_Spicy */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Loading_Spicy"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			//TEXT("../../Resources/Textures2D/UI/Loading_Spicy.png"), 1))))
			TEXT("../../Resources/Textures2D/UI/triforce.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Loading_Background_Fade */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Logo_Background_Black_Fade"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ScreenFade_00.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_HP_Heart_0~2 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_HP"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Heart_%d.dds"), 3))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Rupee */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Rupee"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Rupee.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Number */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Number"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Number_%d.dds"), 10))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Inventory_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Inventory_Background"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Inventory_Empty.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Inventory_ItemSlot */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Inventory_ItemSlot"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ItemSlot_%d.dds"), 3))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Inventory_ItemSlot_Cursor */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Inventory_ItemSlot_Cursor"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ItemSlot_Cursor.dds"), 1))))
		return E_FAIL;
	{
		/* For.Prototype_Component_Texture_UI_Inventory_ItemBow */
		if (FAILED(m_pGameInstance->Add_Prototype(
			ENUM_TO_UINT(LEVEL::STATIC),
			TEXT("Prototype_Component_Texture_UI_Inventory_ItemBow"),
			CTexture::Create(m_pDevice, m_pDeviceContext,
				TEXT("../../Resources/Textures2D/UI/ItemSword.png"), 1))))
			return E_FAIL;
		/* For.Prototype_Component_Texture_UI_Inventory_ItemOcarina */
		if (FAILED(m_pGameInstance->Add_Prototype(
			ENUM_TO_UINT(LEVEL::STATIC),
			TEXT("Prototype_Component_Texture_UI_Inventory_ItemOcarina"),
			CTexture::Create(m_pDevice, m_pDeviceContext,
				TEXT("../../Resources/Textures2D/UI/ItemShield.dds"), 1))))
			return E_FAIL;
		/* For.Prototype_Component_Texture_UI_Inventory_ItemRocsFeather */
		if (FAILED(m_pGameInstance->Add_Prototype(
			ENUM_TO_UINT(LEVEL::STATIC),
			TEXT("Prototype_Component_Texture_UI_Inventory_ItemRocsFeather"),
			CTexture::Create(m_pDevice, m_pDeviceContext,
				TEXT("../../Resources/Textures2D/UI/ItemBoomerang.png"), 1))))
			return E_FAIL;
	}
	/* For.Prototype_Component_Texture_UI_Inventory_EquipSlot_X */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Inventory_EquipSlot_X"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/EquipSlot_2.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_Inventory_EquipSlot_Y */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_Inventory_EquipSlot_Y"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/EquipSlot_3.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_GetMessageFrame */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_GetMessageFrame"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/GetMessageFrame.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_MessageFrame */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_MessageFrame"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/MessageFrame.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_MessageChoice */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_ChoiceMessageBox"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ChoiceMessageBox.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_UI_MessageNext */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_UI_MessageNext"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/MessageNext.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Map_Water */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Map_Water"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Noise_%d.png"), 5))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Map_Cloud */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Map_Cloud"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Cloud_%d.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Map_Shadow */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Map_Shadow"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Shadow_%d.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Loading_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Logo_Background_Black"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/ScreenFade_00.dds"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Loading_Spinner */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Loading_Spinner"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Loading_Spinner_0.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Loading_Egg */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Loading_Egg"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/UI/Loading_Egg.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Snow"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Snow.png"), 1))))
		return E_FAIL;
	/* Effect (Common) */
	/* For.Prototype_Component_Texture_Dissolve */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Dissolve"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Dissolve_%d.png"), 4))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Distortion */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Distortion"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Distortion_%d.png"), 2))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Glow */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Glow"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Glow_%d.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Grad */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Grad"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Grad_%d.png"), 2))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Noise */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Noise"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Noise_%d.png"), 5))))
		return E_FAIL;
	/* Effect */
	/* For.Prototype_Component_Texture_Fire */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Fire"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Fire/Fire_%d.png"), 5))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_GrassCut */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_GrassCut"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Grass/Grass_%d.png"), 2))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Star */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Star"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Star/Star_%d.png"), 3))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Purple_Smoke */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Purple_Smoke"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Purple_Smoke/Purple_Smoke_%d.png"), 1))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Slash */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Slash"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Player/Slash_%d.png"), 4))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Dust_Smoke */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Dust_Smoke"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Smoke/Smoke_%d.png"), 4))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Dust_Smoke_nml */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Dust_Smoke_nml"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Smoke/Smoke_nml_%d.png"), 1))))
		return E_FAIL;
#pragma endregion
	/* Model */
#pragma region MODEL
	/* For.Prototype_Component_Model_Map_RupeeGreen */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_RupeeGreen"),
		CModel::Create(
		m_pDevice, m_pDeviceContext,
		"../../Resources/Models/NonAnim/Obj_Rupee_Green/RupeeGreen.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_RupeePurple */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_RupeePurple"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_Rupee_Purple/RupeePurple.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_RupeeRed */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_RupeeRed"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_Rupee_Red/RupeeRed.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_RupeeGold */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_RupeeGold"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_Rupee_Gold/RupeeGold.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_BossKey */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_BossKey"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/Skinned/Item_BossKey/BossKey.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_SmallKey */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_SmallKey"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/Skinned/Item_SmallKey/SmallKey.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_Ocarina */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_Ocarina"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_Ocarina/Ocarina.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_Shield */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_Shield"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_Shield/Shield.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_SwordLv1 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_SwordLv1"),
		CModel::Create(
			m_pDevice, m_pDeviceContext,
			"../../Resources/Models/NonAnim/Obj_SwordLv1/SwordLv1.mesh"))))
		return E_FAIL;
	/* For.Prototype_Component_Model_Map_Tree2 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Model_Map_Tree2"),
		CModel::Create(
			m_pDevice,
			m_pDeviceContext,
			"../../Resources/Models/NonAnim/Map_Tree/Tree2.mesh"))))
		return E_FAIL;
	/* Effect Mesh */
	m_pGameInstance->Register_EffectMeshes(ENUM_TO_UINT(LEVEL::STATIC));
#pragma endregion
	/* 객체 원형 */
#pragma region OBJECT
	/* For.Prototype_Component_VIBuffer_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Plane */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Plane"),
		CVIBuffer_Plane::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;	
	/* For.Prototype_Component_VIBuffer_Sphere */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Sphere"),
		CVIBuffer_Sphere::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Trail */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Trail"),
		CVIBuffer_Trail::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_Transition */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Transition"),
		CUI_Transition::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_Heart */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Heart"),
		CUI_Heart::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_Number */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Number"),
		CUI_Number::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_Image */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Image"),
		CUI_Image::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Slot */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Slot"),
		CUI_Slot::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Item */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Item"),
		CUI_Item::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_Spinner */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Spinner"),
		CUI_Spinner::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Camera_Dynamic */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Camera_Dynamic"),
		CCamera_Dynamic::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Camera_Static */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Camera_Static"),
		CCamera_Static::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_NonAim*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_MapObject_NonAnim"),
		CMapObject_NonAnim::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_Anim*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_MapObject_Anim"),
		CMapObject_Anim::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_Collision*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_MapObject_Collision"),
		CMapObject_Collision::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_Boundary*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_MapObject_Boundary"),
		CMapObject_Boundary::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_UI_WorldBackground */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_WorldBackground"),
		CUI_WorldBackground::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Player"),
		CPlayer::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Body_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Body_Player"),
		CBody_Player::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_Player"),
		CWeapon_Player::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Accessory_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Accessory_Player"),
		CAccessory_Player::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster"),
		CMonster::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Body_Monster */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Body_Monster"),
		CBody_Monster::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_Monster"),
		CWeapon_Monster::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_MasterStalfon */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_MasterStalfon"),
		CMonster_MasterStalfon::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_MoriblinSword */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_MoriblinSword"),
		CMonster_MoriblinSword::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_Albatoss */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_Albatoss"),
		CMonster_Albatoss::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_Togezo */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_Togezo"),
		CMonster_Togezo::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_Gohma */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_Gohma"),
		CMonster_Gohma::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_BuzzBlob */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_BuzzBlob"),
		CMonster_BuzzBlob::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Moriblin */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Moriblin"),
		CMoriblinSpear::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Body_Moriblin */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Body_Moriblin"),
		CBody_MoriblinSpear::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon_Moriblin */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_Moriblin"),
		CWeapon_MoriblinSpear::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon_Spear */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_Spear"),
		CSpear::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon_Boomerang */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_Boomerang"),
		CBoomerang::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Weapon_AlbatossFeather */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Weapon_AlbatossFeather"),
		CAlbatoss_Feather::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Water */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Water"),
		CWater::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Cloud */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Cloud"),
		CCloud::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_DungeonFire */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_DungeonFire"),
		CDungeonFire::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Trigger */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Trigger"),
		CTrigger::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_NPC */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_NPC"),
		CNPC::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Ladder */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Ladder"),
		CLadder::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_Bomber */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_Bomber"),
		CMonster_Bomber::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Grass */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Grass"),
		CGrass::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Rupee */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Rupee"),
		CRupee::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_CrystalSwitch */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_CrystalSwitch"),
		CCrystalSwitch::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_ColorToggleBlock */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_ColorToggleBlock"),
		CColorToggleBlock::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_MasterStalfon_ShadowIndicator */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_MasterStalfon_ShadowIndicator"),
		CMasterStalfon_ShadowIndicator::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Actor_Monster_StalKnightSword */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_StalKnightSword"),
		CMonster_StalKnightSword::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Snow"),
		CSnow::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Map_Explosion */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_Explosion"),
		CExplosion::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_SwordSlash */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_SwordSlash"),
		CSwordSlash::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_Light */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_MapObject_Light"),
		CMapObject_Light::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Fire_01 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Fire_01"),
		CFire_01::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Fire_02 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Fire_02"),
		CFire_02::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Fire_03 */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Fire_03"),
		CFire_03::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Grass_Cut */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Grass_Cut"),
		CGrass_Cut::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Star */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Star"),
		CStar::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Purple_Smoke */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Purple_Smoke"),
		CPurpleSmoke::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_TrailEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_TrailEffect"),
		CTrailEffect::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Effect_Dust_Smoke */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Dust_Smoke"),
		CDustSmoke::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
#pragma endregion
	/* Shader */
#pragma region SHADER
	/* For.Prototype_Component_Shader_VtxPosTex */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxMesh_NormalFlip"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMesh_NormalFlip.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxMesh_Instance */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxMesh_Instance"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"),
			VTXMESH_INSTANCE::Elements,
			VTXMESH_INSTANCE::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxAnimMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"),
			VTXNORTEX::Elements,
			VTXNORTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxGradient */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxGradient"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxGradient.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
		/* For.Prototype_Component_Shader_VtxCaveVoid */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxCaveVoid"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxCaveVoid.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxWater */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxWater"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxWater.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxCloud */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxCloud"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxCloud.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxFire */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxFire"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxFire.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_Cell */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_Cell"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"),
			VTXPOS::Elements,
			VTXPOS::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxParticleRect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxParticleRect"),
		CShader::Create(
			m_pDevice, 
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxParticleRect.hlsl"), 
			VTX_PARTICLE_RECT::Elements, 
			VTX_PARTICLE_RECT::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxParticlePoint */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxParticlePoint"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxParticlePoint.hlsl"),
			VTX_PARTICLE_POINT::Elements,
			VTX_PARTICLE_POINT::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxEffect"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffect.hlsl"),
			VTXPOSTEX::Elements,
			VTXPOSTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxMeshEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxMeshEffect"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshEffect.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxTrailEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxTrailEffect"),
		CShader::Create(
			m_pDevice,
			m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxTrailEffect.hlsl"),
			VTXTRAIL::Elements,
			VTXTRAIL::iNumElements))))
		return E_FAIL;
#pragma endregion
	/* Collider */
#pragma region COLLIDER
	/* For.Prototype_Component_Collider_AABB */
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_AABB"),
		CCollider::Create(m_pDevice, m_pDeviceContext, 
			COLLIDER::AABB))))
		return E_FAIL;
	/* For.Prototype_Component_Collider_OBB */
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_OBB"),
		CCollider::Create(m_pDevice, m_pDeviceContext,
			COLLIDER::OBB))))
		return E_FAIL;
	/* For.Prototype_Component_Collider_SPHERE */
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Collider_SPHERE"),
		CCollider::Create(m_pDevice, m_pDeviceContext,
			COLLIDER::SPHERE))))
		return E_FAIL;
#pragma endregion
	/* Navigation */
#pragma region NAVIGATON
	/* For.Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_NONE"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_-1.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_-1.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_FIELD"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_6.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_6.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_MARTHAsBAY_PLAYER"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_7_PLAYER.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_7_PLAYER.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_MARTHAsBAY_MONSTER"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_7_MONSTER.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_7_MONSTER.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Dungeon_MONSTER"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_8_MONSTER.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_8_MONSTER.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Dungeon_MONSTER_BOSS"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_8_MONSTER_BOSS.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_8_MONSTER_BOSS.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Dungeon_PLAYER"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_8_PLAYER.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_8_PLAYER.dat")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Navigation_Tower_Player"),
		CNavigation::Create(
			m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Data/Navigation/NAV_LEVEL_9.dat"),
			TEXT("../../Resources/Data/Navigation/NAV_NEIGHBOR_LEVEL_9.dat")))))
		return E_FAIL;
#pragma endregion
	/* Player */
#pragma region PLAYER(ONCE)
	{
		CContainerObject::CONTAINERR_INIT_DESC tContainerInitDesc = {};
		m_pGameInstance->Load_ContainerXML(
			RESOURCE_LEVEL_STATIC,
			TEXT("Player"),
			tContainerInitDesc, TEXT(""));

		CGameObject* pGameObject = { nullptr };
		m_pGameInstance->Clone_Instance_Container(RESOURCE_LEVEL_STATIC, tContainerInitDesc, &pGameObject);
		m_pGameInstance->Set_Player(pGameObject);
	}
#pragma endregion
	/* Particle */
#pragma region PARTICLE
	/* For.Prototype_Component_VIBuffer_Particle_Snow */
	CVIBuffer_Particle_Rect::PARTICLE_RECT_DESC SnowDesc = {};
	SnowDesc.iNumInstance = 3000;
	SnowDesc.vCenter = _float3(0.f, 0.f, 0.f);
	SnowDesc.vRange = _float3(150.f, 0.5f, 150.f);
	SnowDesc.vSize = _float2(0.1f, 0.3f);
	SnowDesc.vSpeed = _float2(1.f, 5.f);
	SnowDesc.vLifeTime = _float2(2.f, 4.f);
	SnowDesc.vColorMin = _float4(1.f, 0.75f, 0.8f, 1.f);
	SnowDesc.bIsLoop = true;
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Particle_Snow"),
		CVIBuffer_Particle_Rect::Create(
			m_pDevice, 
			m_pDeviceContext, 
			&SnowDesc))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_Particle_Explosion */
	CVIBuffer_Particle_Point::PARTICLE_POINT_DESC tExplosionDesc = {};
	tExplosionDesc.iNumInstance = 300;
	tExplosionDesc.vCenter = _float3(0.f, 0.f, 0.f);
	tExplosionDesc.vRange = _float3(0.5f, 0.5f, 0.5f);
	tExplosionDesc.vSize = _float2(0.1f, 0.3f);
	tExplosionDesc.vSpeed = _float2(1.f, 5.f);
	tExplosionDesc.vLifeTime = _float2(2.f, 4.f);
	tExplosionDesc.bIsLoop = true;
	tExplosionDesc.vPivot = tExplosionDesc.vCenter;
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Particle_Explosion"),
		CVIBuffer_Particle_Rect::Create(
			m_pDevice,
			m_pDeviceContext,
			&tExplosionDesc))))
		return E_FAIL;

#pragma endregion


	{
		CEffect::EFFECT_INIT_DESC tInitDesc = {};

		CGameObject::OBJECT_DESC tObjectDesc = {};
		tObjectDesc.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc.ObjectID, TEXT("Slash"));
		SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Effect_SwordSlash"));
		SET_DESC(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc.ModelTag, TEXT("Prototype_Component_Model_Map_swordslash_00out"));

		tInitDesc.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc.tObjectDesc = tObjectDesc;

		m_pGameInstance->Register_EffectEvent(TEXT("swordslash_00out"), tInitDesc);
	}

	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Fire_01 = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Fire_01 = {};
		tObjectDesc_tInitDesc_Fire_01.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Fire_01.ObjectID, TEXT("Fire_01"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_01.ObjectTag, TEXT("Prototype_GameObject_Effect_Fire_01"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_01.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_01.ModelTag, TEXT("Prototype_Component_Model_Map_Fire_01"));

		tInitDesc_Fire_01.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Fire");
		tInitDesc_Fire_01.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_01.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_01.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Fire_01.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Fire_01.tObjectDesc = tObjectDesc_tInitDesc_Fire_01;

		m_pGameInstance->Register_EffectEvent(TEXT("Fire_01"), tInitDesc_Fire_01);

		CEffect::EFFECT_INIT_DESC tInitDesc_Fire_02 = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Fire_02 = {};
		tObjectDesc_tInitDesc_Fire_02.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Fire_02.ObjectID, TEXT("Fire_02"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_02.ObjectTag, TEXT("Prototype_GameObject_Effect_Fire_02"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_02.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_02.ModelTag, TEXT("Prototype_Component_Model_Map_Fire_01"));

		tInitDesc_Fire_02.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Fire");
		tInitDesc_Fire_02.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_02.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_02.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Fire_02.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Fire_02.tObjectDesc = tObjectDesc_tInitDesc_Fire_02;

		m_pGameInstance->Register_EffectEvent(TEXT("Fire_02"), tInitDesc_Fire_02);
	}

	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Grass = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Grass = {};
		tObjectDesc_tInitDesc_Grass.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Grass.ObjectID, TEXT("Grass_Cut"));
		SET_DESC(tObjectDesc_tInitDesc_Grass.ObjectTag, TEXT("Prototype_GameObject_Effect_Grass_Cut"));
		SET_DESC(tObjectDesc_tInitDesc_Grass.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Grass.ModelTag, TEXT("Prototype_Component_Model_Map_Grass"));

		tInitDesc_Grass.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_GrassCut");
		tInitDesc_Grass.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Grass.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Grass.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Grass.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Grass.tObjectDesc = tObjectDesc_tInitDesc_Grass;

		m_pGameInstance->Register_EffectEvent(TEXT("Grass_Cut"), tInitDesc_Grass);
	}	
	
	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Star = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Star = {};
		tObjectDesc_tInitDesc_Star.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Star.ObjectID, TEXT("Grass_Cut"));
		SET_DESC(tObjectDesc_tInitDesc_Star.ObjectTag, TEXT("Prototype_GameObject_Effect_Star"));
		SET_DESC(tObjectDesc_tInitDesc_Star.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Star.ModelTag, TEXT("Prototype_Component_Model_Map_Star"));

		tInitDesc_Star.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Star");
		tInitDesc_Star.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Star.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Star.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Star.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Star.tObjectDesc = tObjectDesc_tInitDesc_Star;

		m_pGameInstance->Register_EffectEvent(TEXT("Star"), tInitDesc_Star);
	}
	
	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Purple_Smoke = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Purple_Smoke = {};
		tObjectDesc_tInitDesc_Purple_Smoke.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Purple_Smoke.ObjectID, TEXT("Purple_Smoke"));
		SET_DESC(tObjectDesc_tInitDesc_Purple_Smoke.ObjectTag, TEXT("Prototype_GameObject_Effect_Purple_Smoke"));
		SET_DESC(tObjectDesc_tInitDesc_Purple_Smoke.ShaderTag, TEXT("Prototype_Component_Shader_VtxMeshEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Purple_Smoke.ModelTag, TEXT("Prototype_Component_Model_Map_Plane"));

		tInitDesc_Purple_Smoke.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Purple_Smoke");
		tInitDesc_Purple_Smoke.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Purple_Smoke.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Purple_Smoke.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Purple_Smoke.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Purple_Smoke.tObjectDesc = tObjectDesc_tInitDesc_Purple_Smoke;

		m_pGameInstance->Register_EffectEvent(TEXT("Plane"), tInitDesc_Purple_Smoke);
	}	

	{
		CTrailEffect::TRAIL_INIT_DESC tInitDesc_Trail = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Trail = {};
		tObjectDesc_tInitDesc_Trail.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Trail.ObjectID, TEXT("Trail"));
		SET_DESC(tObjectDesc_tInitDesc_Trail.ObjectTag, TEXT("Prototype_GameObject_Effect_TrailEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Trail.ShaderTag, TEXT("Prototype_Component_Shader_VtxTrailEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Trail.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Trail"));
		
		tInitDesc_Trail.tObjectDesc = tObjectDesc_tInitDesc_Trail;
		tInitDesc_Trail.fWidth = 1.f;
		tInitDesc_Trail.iMaxPoint = 128;
		m_pGameInstance->Register_EffectEvent(TEXT("Trail"), tInitDesc_Trail);
	}

	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Dust_Smoke = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Dust_Smoke = {};
		tObjectDesc_tInitDesc_Dust_Smoke.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Dust_Smoke.ObjectID, TEXT("Dust_Smoke"));
		SET_DESC(tObjectDesc_tInitDesc_Dust_Smoke.ObjectTag, TEXT("Prototype_GameObject_Effect_Dust_Smoke"));
		SET_DESC(tObjectDesc_tInitDesc_Dust_Smoke.ShaderTag, TEXT("Prototype_Component_Shader_VtxEffect"));
		//SET_DESC(tObjectDesc_tInitDesc_Dust_Smoke.ModelTag, TEXT("Prototype_Component_Model_Map_Plane"));
		SET_DESC(tObjectDesc_tInitDesc_Dust_Smoke.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Plane"));

		tInitDesc_Dust_Smoke.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Dust_Smoke");
		tInitDesc_Dust_Smoke.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Dust_Smoke.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Dust_Smoke.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Dust_Smoke.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Dust_Smoke.tObjectDesc = tObjectDesc_tInitDesc_Dust_Smoke;

		m_pGameInstance->Register_EffectEvent(TEXT("Dust_Smoke"), tInitDesc_Dust_Smoke);
	}

	{
		CEffect::EFFECT_INIT_DESC tInitDesc_Fire_03 = {};

		CGameObject::OBJECT_DESC tObjectDesc_tInitDesc_Fire_03 = {};
		tObjectDesc_tInitDesc_Fire_03.iLevel = RESOURCE_LEVEL_STATIC;
		SET_DESC(tObjectDesc_tInitDesc_Fire_03.ObjectID, TEXT("Fire_03"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_03.ObjectTag, TEXT("Prototype_GameObject_Effect_Fire_03"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_03.ShaderTag, TEXT("Prototype_Component_Shader_VtxEffect"));
		SET_DESC(tObjectDesc_tInitDesc_Fire_03.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Plane"));

		tInitDesc_Fire_03.tEffectDesc.MainTextureTag = TEXT("Prototype_Component_Texture_Fire");
		tInitDesc_Fire_03.tEffectDesc.NoiseTag_0 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_03.tEffectDesc.NoiseTag_1 = TEXT("Prototype_Component_Texture_Noise");
		tInitDesc_Fire_03.tEffectDesc.DistortionTag = TEXT("Prototype_Component_Texture_Distortion");
		tInitDesc_Fire_03.tEffectDesc.DissolveTag = TEXT("Prototype_Component_Texture_Dissolve");
		tInitDesc_Fire_03.tObjectDesc = tObjectDesc_tInitDesc_Fire_03;

		m_pGameInstance->Register_EffectEvent(TEXT("Fire_03"), tInitDesc_Fire_03);
	}

	/* Done */
	return S_OK;
}

#ifdef  _DEBUG
void CMainApp::Show_FPS(_float _fTimeDelta)
{
	m_fTimeAcc += _fTimeDelta;
	++m_iRenderCnt;

	/* 2s */
	if (m_fTimeAcc >= 1.f)
	{
		_float fFPS = m_iRenderCnt / m_fTimeAcc;

		TCHAR szDebug[64];
		swprintf_s(szDebug, TEXT("[FPS] %.1f\n"), fFPS);
		OutputDebugString(szDebug);

		// 초기화
		m_fTimeAcc = 0.f;
		m_iRenderCnt = 0;
	}
}
#endif //  _DEBUG

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CMainApp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pDeviceContext);

	Safe_Release(m_pUIManager);
	m_pGameInstance->Release_Engine();
	Safe_Release(m_pGameInstance);
}
