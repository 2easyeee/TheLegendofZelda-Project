#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CTexture(const CTexture& _Prototype);
	virtual ~CTexture() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* _pTextureFilePath, _uint _iNumTextures);
	virtual HRESULT Initialize(void* _pArg);

	HRESULT Bind_ShaderResourceView(class CShader* _pShader, 
		const _char* _pConstantName, 
		_uint _iTextureIndex);

	_uint Get_TextureTotalCnt() { return m_iNumTexture; }

private:
	_uint m_iNumTexture = {};
	vector<ID3D11ShaderResourceView*> m_vecTextures;

public:
	static CTexture* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		const _tchar* _pTextureFilePath, _uint _iNumTextures);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END