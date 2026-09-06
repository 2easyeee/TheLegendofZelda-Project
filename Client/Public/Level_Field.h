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
class CLevel_Field final : public CLevel
{
private:
	CLevel_Field(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLevel_Field() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CCameraController* m_pCameraController = { nullptr };
	class CCamera* m_pCamera = { nullptr };
	class CGameObject* m_pPlayer = { nullptr };
	class CPlayer* m_pCastingPlayer = { nullptr };

	class CNPC* m_pSmallBowWow = { nullptr };
	class CNPC* m_pRabbit = { nullptr };
	class CNPC* m_pFlyingCucco = { nullptr };
	class CNPC* m_pButterfly = { nullptr };
	class CNPC* m_pFox = { nullptr };
	class CNPC* m_pGrandmaUlrira = { nullptr };

	class CGameObject* m_pWarpHole = { nullptr };

	_int m_iGrassCount = { 30 };

	/* MoveTo WarpHole */
	STATE_TIME m_tMoveTime = { false, 0.f, 0.f };
	_vector m_vWarpHoleCenter;
	
	/* Warp */
	STATE_TIME m_tWarpTime = { false, 0.f, 5.3f };

private:
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);
	HRESULT Ready_Player_And_Camera();
	HRESULT Ready_Trigger();
	HRESULT Ready_Layer_NPC();
	HRESULT Ready_Layer_Light();
	HRESULT Ready_InstancingStressTest(_uint iInstanceCount);

	_bool IsGameClear(_float _fTimeDelta);
	HRESULT Update_Event();
	HRESULT Handle_GameEvent(CEventManager::GAME_EVENT &_tEvent);

	void Spawn_Warp();
	void Update_MoveToWarpHole(_float _fTimeDelta);
	
	void Clamp_Camera();

public:
	static CLevel_Field* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;
};
NS_END
