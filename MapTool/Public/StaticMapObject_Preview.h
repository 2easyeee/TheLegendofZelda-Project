#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CStaticMapObject_Preview final : public CMapObject
{
private:
	CStaticMapObject_Preview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CStaticMapObject_Preview(const CStaticMapObject_Preview& _Prototype);
	virtual ~CStaticMapObject_Preview() = default;

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

private:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

public:
	static CStaticMapObject_Preview* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END