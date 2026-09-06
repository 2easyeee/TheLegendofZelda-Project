#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CAccessory_Player final : public CPartObject
{
private:
	CAccessory_Player(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CAccessory_Player(const CAccessory_Player& _Prototype);
	virtual ~CAccessory_Player() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	const _uint* m_pParentState = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

public:
	static CAccessory_Player* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END