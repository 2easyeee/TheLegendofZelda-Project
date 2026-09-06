#pragma once
#include "Client_Defines.h"
#include "WorldObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
class CSpear final : public CWorldObject
{
public:
	typedef struct tagSpearDesc
	{
		_float4x4 WorldMatrix;
	}SPEAR_DESC;

	typedef struct tagSpearInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		SPEAR_DESC tSpearDesc;
	}SPEAR_INIT_DESC;

private:
	CSpear(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CSpear(const CSpear& _Prototype);
	virtual ~CSpear() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Fly();
	void Fly_Target(_vector _vTargetPos);
	void Update_WorldMatrix(_float4x4 _WorldMatrix);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

	/* State */
	STATE_TIME m_tStickIntoTime = { false, 0.f, 5.f };
	_float m_fSpeed = { 0.f };
	_bool m_bFlyingFinished = { false };

	/* Physics */
	_float3 m_vVelocity = {};
	_float m_fGravity = -6.f; //-25.f;
	_bool m_bOnGround = { false };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);
	HRESULT Ready_Collider();
	void StickIntoGround(_vector _vPosition);

public:
	static CSpear* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END