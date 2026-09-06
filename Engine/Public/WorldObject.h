#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CWorldObject abstract : public CGameObject
{
public:
	typedef struct tagWorldObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_float3		vRight = {1.f, 0.f, 0.f};
		_float3		vUp = { 0.f, 1.f, 0.f };
		_float3		vLook = { 0.f, 0.f, 1.f };
		_float3		vPosition = { 0.f, 0.f, 0.f };
		_float3		vColliderCenter = { 0.f, 0.f, 0.f };
		_float3		vColliderExtents = { 1.f, 1.f, 1.f };
	}WORLD_DESC;

protected:
	CWorldObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CWorldObject(const CWorldObject& _Prototype);
	virtual ~CWorldObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

	WORLD_DESC Get_WorldDesc() const { return m_WorldDesc; }

private:
	WORLD_DESC m_WorldDesc = {};

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END