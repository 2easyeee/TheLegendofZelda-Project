#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CPartObject abstract : public CGameObject
{
public:
	typedef struct tagPartDesc
	{
		const _float4x4* pParentMatrix;
		const _uint* pPartentSTATE = { nullptr };
		const _float4x4* pSocketMatrix = { nullptr }; // for Weapon..
	}PART_DESC;

	typedef struct tagPartCreateDesc
	{
		PART ePartType;
		OBJECT_DESC tObjectDesc;
		PART_DESC tPartDesc;
		_wstring SocketName; // for. where you want to attach bone Name..
	}PART_CREATE_DESC;

protected:
	CPartObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CPartObject(const CPartObject& _Prototype);
	virtual ~CPartObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	const _float4x4* m_pParentMatrix = { nullptr };
	_float4x4 m_CombinedWorldMatrix = {};

protected:
	HRESULT Update_WorldMatrix(_fmatrix _childWorldMatrix);
	HRESULT Bind_WorldMatrix(class CShader* _pShader, const _char* _pConstantName);

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END