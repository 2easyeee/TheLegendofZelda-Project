#pragma once
#include "WorldObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CCamera abstract : public CWorldObject
{
public:
	typedef struct tagCameraDesc : public CTransform::TRANSFORM_DESC
	{
		_float3 vEye;
		_float3 vAt;
		_float fFovy;
		_float fNear, fFar;
		_float fSensor;
	}CAMERA_DESC;

	typedef struct tagCameraInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		CAMERA_DESC tCameraDesc;
	}CAMERA_INIT_DESC;

protected:
	CCamera(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CCamera(const CCamera& _Prototype);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	_float m_fFovy = {};
	_float m_fNear = {};
	_float m_fFar = {};
	_float m_fAspect = {};
	_float m_fMouseSensor = {};

protected:
	void Update_PipeLines();

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END