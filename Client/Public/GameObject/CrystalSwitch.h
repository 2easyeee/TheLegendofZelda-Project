#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
class CCrystalSwitch final : public CMapObject
{
private:
	CCrystalSwitch(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CCrystalSwitch(const CCrystalSwitch& _Prototype);
	virtual ~CCrystalSwitch() = default;

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

	/* EFFECT */
	_bool Check_CameraShake();

private:
	/* Component */
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	/* Collider */
	class CCollider* m_pCollider = { nullptr };

	/* HIT */
	STATE_TIME m_tHitTime = { false, 0.f, 2.f };
	_bool m_bRequestShake = { false };

	/* Once Clear */
	_bool m_bClearPuzzleOnce = { false };

private:
	HRESULT Ready_Components(OBJECT_DESC* _pTags);
	HRESULT Ready_Collider();

public:
	static CCrystalSwitch* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END