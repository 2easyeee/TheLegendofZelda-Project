#pragma once
#include "Client_Defines.h"
#include "WorldObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CTransform;
class CCollider;
NS_END

NS_BEGIN(Client)
class CBoomerang final : public CWorldObject
{
public:
	enum class BOOM_STATE { ATTACHED, FLY, RETURN, END };

public:
	typedef struct tagBoomerangDesc
	{
		_float4x4 WorldMatrix;
	}BOOMERANG_DESC;

	typedef struct tagBoomerangInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		BOOMERANG_DESC tBoomerangDesc;
	}BOOMERANG_INIT_DESC;

private:
	CBoomerang(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CBoomerang(const CBoomerang& _Prototype);
	virtual ~CBoomerang() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Throw(_float3 _vDir);
	void Set_ReturnTargetMatrix(_float4x4 _Matrix);
	void Update_WorldMatrix(_float4x4 _WorldMatrix);
	_bool IsAttached() { return m_eBoomState == BOOM_STATE::ATTACHED; }
	void Set_HomingTarget(_vector _vTargetPos);


	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

	/* State */
	BOOM_STATE m_eBoomState = BOOM_STATE::ATTACHED;
	STATE_TIME m_BoomTime = { false, 0.f, 0.75f };

	/* Physics */
	_float3 m_vDir = { 0.f, 0.f, 0.f };
	_float m_fSpeed = { 10.f };
	_float3 m_vStartPos = {};
	_float m_fSpinAngle = {};

	/* Return */
	_float4x4 m_ReturnTargetMatrix = {};
	_bool m_bHasTarget = { false };

	/* Homing */
	_vector m_vHomingTarget = {};
	_bool m_bUseHoming = { false };
	_float m_fHomingStrengh = { 2.5f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);
	HRESULT Ready_Collider();

public:
	static CBoomerang* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END