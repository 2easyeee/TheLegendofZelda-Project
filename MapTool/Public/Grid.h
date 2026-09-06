#pragma once
#include "MapTool_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Line;
NS_END

NS_BEGIN(MapTool)
class CGrid final : public CGameObject
{
private:
	CGrid(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CGrid(const CGrid& _Prototype);
	virtual ~CGrid() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	vector<VTXLINE> m_Vertices;
	ID3D11Buffer* m_pVB = { nullptr };

	_float m_fGridSize = {1.f};
	_int m_iHalfCount = {20};
	_float m_fGridY = {0.f};

	_float3 m_vLastCenter = { FLT_MAX, FLT_MAX, FLT_MAX };

	class CShader* m_pShaderCom = { nullptr };
	class CVIBuffer_Line* m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();

public:
	static CGrid* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg);
	virtual void Free() override;
};
NS_END