#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
class CColorToggleBlock final : public CMapObject
{
public:
	enum class BLOCK_COLOR { ORANGE, BLUE, END };
	enum class ACTIVE_BLOCK { ORANGE, BLUE, END };

public:
	static ACTIVE_BLOCK s_eState;
	
private:
	CColorToggleBlock(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CColorToggleBlock(const CColorToggleBlock& _Prototype);
	virtual ~CColorToggleBlock() = default;

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

	/* Animation */
	void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false);

private:
	/* Component */
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	/* Collider */
	class CCollider* m_pCollider = { nullptr };

	/* ColorToggleBlock */
	BLOCK_COLOR m_eColor;
	_bool m_bPrevBlockActive = { false };

private:
	HRESULT Ready_Components(OBJECT_DESC* _pTags);
	HRESULT Ready_Collider();

public:
	static CColorToggleBlock* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END