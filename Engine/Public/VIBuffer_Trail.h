#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)
class ENGINE_DLL CVIBuffer_Trail final : public CVIBuffer
{
private:
	CVIBuffer_Trail(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CVIBuffer_Trail(const CVIBuffer_Trail& Prototype);
	virtual ~CVIBuffer_Trail() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual HRESULT Render() override;

	void Update_Trail(const vector<_vector>& _tip, const vector<_vector>& _base);
	void Set_MaxPoint(_uint _iMaxPoint);

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	vector<VTXTRAIL> m_Vertices;
	vector<_ushort>  m_Indices;

	_uint m_iMaxPoint = 0;

	_uint m_iCurrentIndexCount = { 0 };

private:
	_vector CatmullRom(_vector p0, _vector p1, _vector p2, _vector p3, float t);

public:
	static CVIBuffer_Trail* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END