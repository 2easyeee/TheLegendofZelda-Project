#pragma once
#include "Client_Defines.h"
#include "Level.h"
#include "EventManager.h"

NS_BEGIN(Engine)
class CCamera;
class CGameObject;
class CEventManager;
NS_END

NS_BEGIN(Client)
class CLevel_Dungeon final : public CLevel
{
private:
	typedef struct tagTreasureBox
	{
		class CMapObject_NonAnim* pBase = { nullptr };
		class CMapObject_Anim* pLid = { nullptr };
	}TREASUREBOX;

	typedef struct tagUIItem
	{
		CMapObject_NonAnim* pItem = { nullptr };
		STATE_TIME m_tLifTime = { false, 0.f, 2.f };
	}UI_ITEM;

private:
	CLevel_Dungeon(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLevel_Dungeon() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CCameraController* m_pCameraController = { nullptr };
	class CCamera* m_pCamera = { nullptr };
	class CGameObject* m_pPlayer = { nullptr };
	class CPlayer* m_pCastingPlayer = { nullptr };

	class CMonster_MasterStalfon* m_pMasterStalfon = { nullptr };
	class CMapObject_Anim* m_pEalgesTower = { nullptr };

	vector<class CMonster_StalKnightSword*> m_vecStalKnightSwords;
	class CMonster_StalKnightSword* m_pStalKnightSword_Crystal = { nullptr };
	vector<class CMonster_Togezo*> m_vecTogezos;
	vector<class CMonster_Bomber*> m_vecBombers;

	/* Item */
	_int m_iTreasureBoxCnt = 4;
	vector<TREASUREBOX> m_TreasureBoxes;
	_bool m_bBoxOpen_1 = { false };
	_bool m_bBoxOpen_2 = { false };

	/* Objects */
	class CMapObject_Anim* m_pBossDoor = { nullptr };
	STATE_TIME m_tBossDoorOpen = { false, 0.f, 1.5f };

	vector<class CMapObject_Anim*> m_vecDungeonDoors;
	_bool m_bDungeonDoorOpen = { false };

	class CCrystalSwitch* m_pCrystalSwitch = { nullptr };
	_bool m_bCrystalSwitchSpawn = { false };
	STATE_TIME m_tCrystalSwitchPuzzleClear;

	vector<UI_ITEM> m_vecUIItems; 
	
	/* Rupee UI */
	STATE_TIME m_tUIRupee = { false, 0.f, 0.f };
	_int m_iRemainRupee = { 0 };
	STATE_TIME m_tRupeeSFX;

	/* EaglesTower */
	STATE_TIME m_tTowerRotTime = { false, 0.f, 3.f };

	/* Stair */
	_bool m_bStairMoving = { false };
	_vector m_vStairTarget = XMVectorZero();

	/* Dodge */
	_int m_iPrevDodgePhase = { -1 };

private:
	HRESULT Ready_Layer_Background(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);
	HRESULT Ready_Player_And_Camera();
	HRESULT Ready_Trigger();
	HRESULT Ready_Layer_Monster();
	HRESULT Ready_Layer_Object();
	HRESULT Ready_Layer_Effect();
	HRESULT Ready_Layer_Light();
	HRESULT Ready_Layer_UI_Item();
	HRESULT Ready_Layer_UI_Item_Rupee();
	HRESULT Ready_PlayerDynamicLight();

	/* Event */
	HRESULT Update_Event();
	HRESULT Handle_GameEvent(CEventManager::GAME_EVENT& _tEvent);
	HRESULT Check_DungeonDoor();
	HRESULT Check_TreasureBox();
	HRESULT Check_CryStalSwitch();
	HRESULT Check_UIItem(_float _fTimeDelta);
	HRESULT Update_UIRupee(_float _fTimeDelta);
	HRESULT Update_BossDoor(_float _fTimeDelta);

	/* Tower */
	void Rotate_Tower(_float _fTimeDelta);

	/* Stair */
	void Update_StairMove(_float _fTimeDelta);

	/* Camera */
	void Check_CameraShake(_float _fTimeDelta);

	/* Dodge */
	HRESULT Check_PlayerDodge(_float _fTimeDelta);

public:
	static CLevel_Dungeon* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;
};
NS_END