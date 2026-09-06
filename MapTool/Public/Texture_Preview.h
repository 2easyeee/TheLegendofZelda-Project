#pragma once
#include "MapTool_Defines.h"

NS_BEGIN(MapTool)
class CTexture_Preview
{
public:
	CTexture_Preview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	~CTexture_Preview() { Free(); }

public:
	ID3D11ShaderResourceView* Get_SRV(const _string& _filePath);
	void Clear_SRV();

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviecContext = { nullptr };

	map<_string, ID3D11ShaderResourceView*> m_SRVs;

private:
	ID3D11ShaderResourceView* Load_Texture(const _string& _filePath);

public:
	static CTexture_Preview* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free();
};
NS_END