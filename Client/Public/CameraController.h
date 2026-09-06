#pragma once
#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CCamera;
class CTransform;
class CGameObject;
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CCameraController final : public CBase
{
public:
	enum class MODE { PLAYER, DUNGEON, EVENT, STATIC, ZOOM, SHOULDERVIEW, END };
	enum class LOCK_AXIS { NONE, HORIZONTAL, VERTICAL, END };

public:
	CCameraController();
	virtual ~CCameraController() = default;

public:
	HRESULT Initialize();
	void Update(_float _fTimeDelta);

public:
	void Set_Camera(class CCamera* _pCamera);
	void Set_Target(class CGameObject* _pTarget);
	void Set_Angle(_fvector _vAxis, _float _fAngle);
	void Set_LockAxis(LOCK_AXIS _eType);

	void Set_Mode(MODE _eMode);

	void Set_RoomCenter(_vector _vCenter);
	
	void Set_Event(_vector _vPos);
	void Set_Offset(_vector _vOffset);
	
	void Start_Shake(_float _fDuration, _float _fPower);
	
	void Set_StaticPos(_vector _vCamPos, _vector _vCamLook, _float _fSpeed);
	
	void Set_Zoom(_float _fDuration, _float _fPower, _float _fDelay = 0.f);

	/* ShoulderView (Freeze) */
	void Enter_ShoulderView();
	void Exit_ShoulderView();

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	/* Camera */
	MODE m_eMode = MODE::END;
	MODE m_ePreMode = MODE::END;

	/* Object */
	class CCamera* m_pCamera = { nullptr };
	class CTransform* m_pCamTransform = { nullptr };
	class CGameObject* m_pTarget = { nullptr };

	/* MODE::PLAYER */
	_float m_fFollowSpeed = { 5.f };
	_vector m_vOffset = XMVectorZero();
	LOCK_AXIS m_eLockAxis = LOCK_AXIS::NONE;

	/* MODE::DUNGEON */
	_vector m_vRoomCenter = XMVectorZero();
	_float m_fRoomMoveSpeed = { 20.f };
	_float m_iRoomSizeX = { 15.f };
	_float m_iRoomSizeZ = { 12.f };
	_int m_iCurrentRoomX = { -1 };
	_int m_iCurrentRoomZ = { -1 };
	_bool m_bRoomMoving = { false };
	_vector m_vCurCamPos = XMVectorZero();

	/* MODE::EVENT */
	_vector m_vEventPos = XMVectorZero();
	_vector m_vEventLook = XMVectorZero();
	_float m_fEventSpeed = { 3.f };

	/* MODE::SHAKE */
	STATE_TIME m_tShakeTime = { false, 0.f, 0.f };
	_float m_fShakePower = { 0.f };

	/* MODE::STATIC */
	_vector m_vCamPos = XMVectorZero();
	_vector m_vCamLook = XMVectorZero();
	_float m_fReturnSpeed = { 8.f };

	/* MODE::ZOOM */
	STATE_TIME m_tZoomTime = { false, 0.f, 3.f };
	STATE_TIME m_tDelayTime = { false, 0.f, 3.f };
	_vector m_vZoomStartPos = XMVectorZero();
	_float m_fShakePhase;

	/* MODE::SHOULDERVIEW */
	_vector m_vSmoothLook = XMVectorZero();
	_bool m_bShoulderFreeze = { false };
	_vector m_vFreezePos = XMVectorZero();
	_vector m_vFreezeLook = XMVectorZero();

private:
	void Update_Player(_float _fTimeDelta);
	void Update_Dungeon(_float _fTimeDelta);
	void Update_Event(_float _fTimeDelta);
	void Update_Static(_float _fTimeDelta);
	void Update_Zoom(_float _fTimeDelta);
	void Update_ShoulderView(_float _fTimeDelta);
	void Reset_Camera();

	void Apply_Shake(_vector& _vCamPos, _float _fTimeDelta);

public:
	static CCameraController* Create();
	virtual void Free() override;
};
NS_END