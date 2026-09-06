#include "EditorApplication.h"
#include "GameInstance.h"
#include "MapEditor.h"
#include "AssimpEditor.h"
#include "PartsBuilder.h"
#include "NavigationEditor.h"
#include "EffectEditor.h"
#include "ImGui_Manager.h"
#include "Terrain.h"
#include "Camera_Free.h"
#include "StaticMapObject_Preview.h"
#include "GameObject/MapObject_Anim.h"
#include "GameObject/MapObject_NonAnim.h"
#include "Grid.h"
#include "Shader.h"
#include "ImGui_Manager.h"
#include "Tool_ChangeLevel.h"
#include "GameObject/Player.h"
#include "GameObject/Body_Player.h"
#include "GameObject/Weapon_Player.h"
#include "GameObject/Accessory_Player.h"
#include "GameObject/Boomerang.h"
#include "GameObject/Monster.h"
#include "GameObject/Body_Monster.h"
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
#include "GameObject/Monster_MoriblinSword.h"
#include "GameObject/Monster_Albatoss.h"
#include "GameObject/NPC.h"
#include "GameObject/Monster_BuzzBlob.h"
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
#include "ShaderPreview.h"
#include "GameObject/MapObject_Light.h"
#include "GameObject/Fire_01.h"
#include "GameObject/Fire_02.h"
#include "GameObject/Fire_03.h"
#include "GameObject/Grass_Cut.h"
#include "GameObject/DungeonFire.h"

CEditorApplication::CEditorApplication()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CEditorApplication::Initialize()
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

	/* Loading */
	Loading_Resources();

	/* 2. ImGui */
	m_pImGuiManager = CImGui_Manager::Create(g_hWnd, m_pDevice, m_pDeviceContext);
	if (nullptr == m_pImGuiManager)
		return E_FAIL;
	
	/* Light */
	if (FAILED(Ready_Layer_Light()))
		return E_FAIL;

	/* 3. Set Init Level */
	if (FAILED(Start_Level(LEVEL::EFFECTEDITOR)))
		return E_FAIL;

	return S_OK;
}

void CEditorApplication::Update(_float _fTimeDelta)
{
	/* Check Change Level */
	Change_Level();

	/* Updates */
	m_pImGuiManager->Update(_fTimeDelta);
	m_pGameInstance->Update_Engine(_fTimeDelta);
	m_pImGuiManager->Late_Update(_fTimeDelta);
}

HRESULT CEditorApplication::Render()
{
	if (FAILED(m_pGameInstance->Begin_Draw(_float4(0.f, 0.f, 1.f, 1.f))))
		return E_FAIL;

	if (FAILED(m_pImGuiManager->Render_Begin()))
		return E_FAIL;

	if (FAILED(m_pImGuiManager->Render()))
		return E_FAIL;

	if (FAILED(Render_Change_Level()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Draw()))
		return E_FAIL;

	if (FAILED(m_pImGuiManager->Render_End()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->End_Draw()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEditorApplication::Start_Level(LEVEL _eStartLevel)
{
	switch (_eStartLevel)
	{
	case MapTool::LEVEL::MAPEDITOR:
		m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(_eStartLevel),
			CMapEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::ASSIMPEDITOR:
		m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(_eStartLevel),
			CAssimpEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::PARTSBUILDER:
		m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(_eStartLevel),
			CPartsBuilder::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::NAVIGATIONEDITOR:
		m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(_eStartLevel),
			CNavigationEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::EFFECTEDITOR:
		m_pGameInstance->PrepareStartLevel(ENUM_TO_UINT(_eStartLevel),
			CEffectEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	}

	return S_OK;
}

HRESULT CEditorApplication::Loading_Resources()
{
#pragma region Shaders
	/* Shader */
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
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"),
			VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"),
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
	/* For.Prototype_Component_Shader_VtxMesh_Preview */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_Preview"),
		CShader::Create(m_pDevice, m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Preview.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements))))
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
	/* For.Prototype_Component_Shader_VtxLine */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxLine"),
		CShader::Create(m_pDevice, m_pDeviceContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxLine.hlsl"),
			VTXLINE::Elements,
			VTXLINE::iNumElements))))
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
#pragma endregion

#pragma region Models
	/* Model */
	/* For.Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Terrain/Height.bmp")))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Line */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Line"),
		CVIBuffer_Line::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Sphere */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Sphere"),
		CVIBuffer_Sphere::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Plane */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Plane"),
		CVIBuffer_Plane::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_Component_VIBuffer_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
#pragma endregion

#pragma region Mesh
	/* Texture */
	/* For.Prototype_Component_Texture_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Terrain/Tile0.dds"), 1))))
		return E_FAIL;
	///* For.Prototype_Component_Model_Map_swordslash_00out */
	//if (FAILED(m_pGameInstance->Add_Prototype(
	//	RESOURCE_LEVEL_STATIC,
	//	TEXT("Prototype_Component_Model_Map_swordslash_00out"),
	//	CModel::Create(
	//		m_pDevice,
	//		m_pDeviceContext,
	//		"../../Resources/Models/Effects/Player_SwordSlash/swordslash_00out.mesh"))))
	//	return E_FAIL;
	/* For.Prototype_Component_Model_Map_Grass */
	if (FAILED(m_pGameInstance->Add_Prototype(
		RESOURCE_LEVEL_STATIC,
		TEXT("Prototype_Component_Model_Map_Grass"),
		CModel::Create(
			m_pDevice,
			m_pDeviceContext,
			"../../Resources/Models/Effects/Obj_Grass/Grass.mesh"))))
		return E_FAIL;

#pragma endregion

#pragma region Texture
	/* .xml */
	/* For.Prototype_Component_Texture_Map_Water */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_Component_Texture_Map_Water"),
		CTexture::Create(m_pDevice, m_pDeviceContext,
			TEXT("../../Resources/Textures2D/Effects/Noise_%d.png"), 3))))
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
			TEXT("../../Resources/Textures2D/Effects/Noise_%d.png"), 4))))
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
#pragma endregion

#pragma region Prototype
	/* Prototype */
	/* For.Prototype_GameObject_Background */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Camera*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MapObject_NonAnim*/
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
	/* For.Prototype_GameObject_Actor_Boomerang */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Boomerang"),
		CBoomerang::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_StaticMapObject_Preview*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_StaticMapObject_Preview"),
		CStaticMapObject_Preview::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_Grid*/
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Grid"),
		CGrid::Create(m_pDevice, m_pDeviceContext))))
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
	/* For.Prototype_GameObject_Actor_Weapon_Monster */
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
	/* For.Prototype_GameObject_Actor_Monster_Bomber */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_Monster_Bomber"),
		CMonster_Bomber::Create(m_pDevice, m_pDeviceContext))))
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
	/* For.Prototype_GameObject_Actor_NPC */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Actor_NPC"),
		CNPC::Create(m_pDevice, m_pDeviceContext))))
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
	/* For.Prototype_GameObject_Map_ShaderPreview */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Map_ShaderPreview"),
		CShaderPreview::Create(m_pDevice, m_pDeviceContext))))
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
	/* For.Prototype_GameObject_Effect_Grass_Cut */
	if (FAILED(m_pGameInstance->Add_Prototype(
		ENUM_TO_UINT(LEVEL::STATIC),
		TEXT("Prototype_GameObject_Effect_Grass_Cut"),
		CGrass_Cut::Create(m_pDevice, m_pDeviceContext))))
		return E_FAIL;
#pragma endregion

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
#pragma endregion

	/* 로딩이 완료되었습니다. */

	return S_OK;
}

HRESULT CEditorApplication::Render_Change_Level()
{
	ImGui::Begin("Switch Level");
	if (ImGui::Button("Assimp Editor"))
	{
		m_bLevelChangeEnable = true;
		m_eLevel = LEVEL::ASSIMPEDITOR;
	}
	ImGui::SameLine();
	if (ImGui::Button("Map Editor"))
	{
		m_bLevelChangeEnable = true;
		m_eLevel = LEVEL::MAPEDITOR;
	}
	ImGui::SameLine();
	if (ImGui::Button("Parts Builder"))
	{
		m_bLevelChangeEnable = true;
		m_eLevel = LEVEL::PARTSBUILDER;
	}
	ImGui::SameLine();
	if (ImGui::Button("Nav Editor"))
	{
		m_bLevelChangeEnable = true;
		m_eLevel = LEVEL::NAVIGATIONEDITOR;
	}
	ImGui::SameLine();
	if (ImGui::Button("Effect Editor"))
	{
		m_bLevelChangeEnable = true;
		m_eLevel = LEVEL::EFFECTEDITOR;
	}
	ImGui::End();

	return S_OK;
}

void CEditorApplication::Change_Level()
{
	if (!m_bLevelChangeEnable)
		return;

	/* Clear All ImGui Objects*/
	m_pImGuiManager->Clear();

	/* Change Level */
	switch (m_eLevel)
	{
	case MapTool::LEVEL::MAPEDITOR:
		m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eLevel),
			CMapEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::ASSIMPEDITOR:
		m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eLevel),
			CAssimpEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::PARTSBUILDER:
		m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eLevel),
			CPartsBuilder::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::NAVIGATIONEDITOR:
		m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eLevel),
			CNavigationEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	case MapTool::LEVEL::EFFECTEDITOR:
		m_pGameInstance->RequestToChangeLevel(ENUM_TO_UINT(m_eLevel),
			CEffectEditor::Create(m_pDevice, m_pDeviceContext, m_pImGuiManager));
		break;
	}

	m_bLevelChangeEnable = false;
}

HRESULT CEditorApplication::Ready_Layer_Light()
{
	LIGHT_DESC		LightDesc{};

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

CEditorApplication* CEditorApplication::Create()
{
	CEditorApplication* pInstance = new CEditorApplication();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CEditorApplication");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditorApplication::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pDeviceContext);

	Safe_Release(m_pImGuiManager);
	m_pGameInstance->Release_Engine();
	Safe_Release(m_pGameInstance);
}
