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
class CLevel_MarinHouse final : public CLevel
{
private:
	CLevel_MarinHouse(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLevel_MarinHouse() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CCameraController* m_pCameraController = { nullptr };
	class CCamera* m_pCamera = { nullptr };
	class CGameObject* m_pPlayer = { nullptr };
	class CPlayer* m_pCastingPlayer = { nullptr };

	class CNPC* m_pMarin = { nullptr };
	class CNPC* m_pTarin = { nullptr };

	class CMapObject_Anim* m_pBed = { nullptr };

	unordered_map<_wstring, CGameObject*> m_ActorMap;

private:
	HRESULT Ready_Layer_Camera(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Background(const _wstring& strLayerTag);
	HRESULT Ready_Layer_NPC();
	HRESULT Ready_Actor_Table();

	HRESULT Ready_Player_And_Camera();
	HRESULT Ready_Trigger();
	HRESULT Ready_Layer_Light();
	virtual HRESULT Ready_Layer_Shadow() override;

	HRESULT Update_Event();
	HRESULT Handle_GameEvent(CEventManager::GAME_EVENT& _tEvent);
	HRESULT Handle_CameraEvent(CEventManager::CAMERA_EVENT& _tEvent);

	void Check_SleepEndEvent(_float _fTimeDelta);

	void Start_DialogueCamera(_wstring _Actor);

public:
	static CLevel_MarinHouse* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual void Free() override;
};
NS_END