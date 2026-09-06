#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)
class CLadder final : public CGameObject
{
public:
	typedef struct tagLadderDesc
	{
		_vector vTopPos;
		_vector vBottomPos;
	}LADDER_DESC;

private:
	CLadder(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CLadder(const CLadder& _Prototype);
	virtual ~CLadder() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
	
	_float Get_TopY() const;
	_float Get_BottomY() const;

private:
	_vector m_vTopPos;
	_vector m_vBottomPos;

public:
	static CLadder* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END