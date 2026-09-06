#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CPicking final : public CBase
{
private:
	CPicking(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CPicking() = default;

public:
	HRESULT Initialize(HWND _hWnd, _uint _iWinSizeX, _uint _iWinSizeY);
	void Update();
	void Transform_Picking_ToLocalSpace(const _float4x4* _pWorldMatrix);
	_bool isPicked_InLocalSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut);
	_bool isPicked_InWorldSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut, _float* _pOutDist);
	_bool Picking_PlaneY(_float _fY, _float3& _outPos);

	_bool isPicked_AABB(const _float3& vMin, const _float3& vMax, _float3* _pOutHitPos);

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	HWND m_hWnd = {};
	_uint m_iWinSizeX{}, m_iWinSizeY{};
	_float3 m_vWorldRayDir{}, m_vWorldRayPos{};
	_float3 m_vLocalRayDir{}, m_vLocalRayPos{};

 public:
	static CPicking* Create(HWND _hWnd, ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		_uint _iWinSizeX, _uint _iWinSizeY);
	virtual void Free() override;
};
NS_END