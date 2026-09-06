#pragma once
#include "Client_Defines.h"
#include "WorldObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
class CAlbatoss_Feather final : public CWorldObject
{
public:
	typedef struct tagFeatherDesc
	{
		_float4x4 WorldMatrix;
	}FEATHER_DESC;

	typedef struct tagFeatherInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		FEATHER_DESC tFeatherDesc;
	}FEATHER_INIT_DESC;

private:
	CAlbatoss_Feather(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CAlbatoss_Feather(const CAlbatoss_Feather& _Prototype);
	virtual ~CAlbatoss_Feather() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Shoot_Target(_vector _vTargetPos);
	void Update_WorldMatrix(_float4x4 _WorldMatrix);

	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

private:
	/* Component */
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

	/* Feather */
	_bool m_bShoot = { false };
	_vector m_vDir = XMVectorZero();
	_float m_fSpeed = { 10.f };
	STATE_TIME m_tStickIntoTime = { false, 0.f, 2.5f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);
	HRESULT Ready_Collider();

	void Update_Shoot(_float _fTimeDelta);
	void Destroy();

public:
	static CAlbatoss_Feather* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END