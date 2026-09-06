#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
class CRupee final : public CMapObject
{
public:
	enum class RUPEE_TYPE { GREEN, RED, PURPLE, GOLD, END };

private:
	CRupee(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CRupee(const CRupee& _Prototype);
	virtual ~CRupee() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

	/* Rupee */
	void Set_Type(RUPEE_TYPE _eType);
	_int Get_Value() const;

	void Set_Burst(_vector _vVelocity);
	void Set_Billboard(_bool _bActive);

private:
	/* Component */
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	/* Collider */
	class CCollider* m_pCollider = { nullptr };

	/* Rupee */
	RUPEE_TYPE m_eType = RUPEE_TYPE::GREEN;
	_int m_iValue = { 1 };

	/* BillBoard */
	_bool m_bBillboard = { false };

	/* EFFECT */
	_bool m_bSpawnBounce = { true };
	_float m_fBaseHeight = { 0.f };
	_float m_fSpawnVelocity = { 4.f };
	_float m_fGravity = { -9.8f };
	_float m_fFloatTime = 0.f;

	/* EFFECT(Burst Mode) */
	_bool m_bBurstMode = { false };
	_vector m_vVelocity = XMVectorZero();
	
	/* Delay */
	STATE_TIME m_tDelayTime = { false, 0.f, 1.f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _pTags);
	HRESULT Update_ModelCom(_wstring _pModelTag);
	HRESULT Ready_Collider();

	void UI_Event();
	void Destroy();

	/* Billboard */
	void Update_Billboard(_float _fTimeDelta);

	/* EFFECT */
	void Bounce(_float _fTimeDelta);
	void Float(_float _fTimeDelta);
	void Rotate(_float _fTimeDelta);

	/* EFFECT (BurstMode) */
	void Update_Burst(_float _fTimeDelta);

public:
	static CRupee* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END