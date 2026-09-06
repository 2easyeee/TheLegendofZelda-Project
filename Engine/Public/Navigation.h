#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		_int iCurrentCellIndex = { -1 };
	}NAVIGATION_DESC;

private:
	CNavigation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CNavigation(const CNavigation& _Prototype);
	virtual ~CNavigation() = default;
public:
	virtual HRESULT Initialize_Prototype(const _tchar* _pNavigationDataFiles);
	virtual HRESULT Initialize_Prototype(const _tchar* _pNavigationDataFiles,
		const _tchar* _pCellNeighbors);
	virtual HRESULT Initialize(void* _pArg) override;

	_bool IsMove(_fvector _vPosition);
	_vector SetUp_OnNavigation(_fvector _vWorldPos);

	void Add_Cell(_float3 _v0, _float3 _v1, _float3 _v2);
	void Remove_Cell(_int _iIndex);
	class CCell* Get_Cell(_int _iIndex);
	_int Get_CellCount() const;

	void Set_CurrentCell(_int _iIndex);
	_int Find_CurrentCell(_fvector _vPosition);

	void Undo();
	HRESULT Save(const _tchar* _pCellFile, const _tchar* _pNeighborFile);

	/* Nav Editor */
	void Set_SelctedCells(vector<_int> _vecCells);

private:
	vector<class CCell*> m_Cells;
	_int m_iCurrentCellIndex = { -1 };

	/* Nav Editor */
	vector<_int> m_vecSelectedCells;
	
public:
	HRESULT SetUp_Neighbors();
	HRESULT SetUp_Neighbors(const _tchar* _pCellNeighbors);

public:
	static CNavigation* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _tchar* _pNavigationDataFiles);
	static CNavigation* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _tchar* _pNavigationDataFiles, const _tchar* _pCellNeighbors);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;

private:
	class CShader* m_pShader = { nullptr };
#endif // _DEBUG
};
NS_END