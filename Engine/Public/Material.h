#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CMaterial final : public CBase
{
public:
	enum class DEFAULT_SRV { WHITE, BLACK, END };
	typedef struct tagMaterialDesc
	{
		_string name;
		_float4 vAmbient;
		_float4 vDiffuse;
		_float4 vSpecular;
		_float4 vEmissive;
		_string diffuseFile;
		_string	AOFile;
		_string normalFile;
		_string	metallicFile;
		_string	roughnessFile;
	}MATERIALDESC;

private:
	CMaterial(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CMaterial() = default;

public:
	HRESULT Initialize(tinyxml2::XMLElement* _pNode, const _char* _pMaterialXMLPath);
	HRESULT Bind_Material(class CShader* _pShader, const _char* _pConstantName,
		MATERIAL _eMaterialType, _uint _iTextureIndex);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	vector<ID3D11ShaderResourceView*> m_vecMaterials[static_cast<_uint>(MATERIAL::END)];

	ID3D11ShaderResourceView* m_pDefaultSRV_White = { nullptr };
	ID3D11ShaderResourceView* m_pDefaultSRV_Black = { nullptr };

private:
	MATERIAL Get_MaterialTypeFromTag(const char* tag);

	MATERIALDESC m_tMaterialDesc = {};

private:
	HRESULT Create_DefaultTexture(DEFAULT_SRV _eSRVColor);
	ID3D11ShaderResourceView* Get_SRV(MATERIAL _eType, _uint _iIndex);

public:
	static CMaterial* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, 
		tinyxml2::XMLElement* _pNode, const _char* _pMaterialXMLPath);
	virtual void Free() override;
};
NS_END