#pragma once
#include "MapTool_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CVIBuffer_Plane;
class CModel;
class CShader;
class CTexture;
NS_END

/* For. Only Shader Effect Render */
NS_BEGIN(MapTool)
class CShaderPreview final : public CMapObject
{
public:
	enum class TEXTURE_SLOT { MAIN, NOISE_0, NOISE_1, MASK, DISTORTION, DISSOLVE, END };

private:
	CShaderPreview(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CShaderPreview(const CShaderPreview& _Prototype);
	virtual ~CShaderPreview() = default;

public:
	/* Param */
	SHADER_PARAM_DESC m_tShaderDesc;
	SHADER_TEXTURE_DESC m_tTextures;

	/* Timeline */
	_bool m_bUseTimeline = { false };
	SHADER_TIMELINE m_tTimeline = {};
	STATE_TIME m_tTime = { false, 0.f, 0.f };

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_Texture(TEXTURE_SLOT _eSlot, ID3D11ShaderResourceView* _SRV, _wstring _Path);
	ID3D11ShaderResourceView* Get_Texture(TEXTURE_SLOT _eSlot);
	ID3D11ShaderResourceView*& Get_TextureSlot(TEXTURE_SLOT _eSlot);

	/* Change Model */
	HRESULT Change_Model(_wstring _ModelTag);
	void Apply_Effect(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Texture);

private:
	/* Component */
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	/* Texture Type */
	TEXTURE_SLOT m_eSelectedSlot = TEXTURE_SLOT::MAIN;

	/* Param */
	ID3D11ShaderResourceView* m_pMainTexture = { nullptr };
	ID3D11ShaderResourceView* m_pNoiseTexture_0 = { nullptr };
	ID3D11ShaderResourceView* m_pNoiseTexture_1 = { nullptr };
	ID3D11ShaderResourceView* m_pMaskTexture = { nullptr };
	ID3D11ShaderResourceView* m_pDistortionTexture = { nullptr };
	ID3D11ShaderResourceView* m_pDissolveTexture = { nullptr };

	/* Reset */
	SHADER_PARAM_DESC m_tShaderDescDefault;
	_float3 m_vDefaultScale = { 1.f,1.f,1.f };

private:
	HRESULT Ready_Components(OBJECT_DESC* _pTags);
	HRESULT Render_Binding();
	HRESULT Load_Texture_FromPath(TEXTURE_SLOT slot, const std::wstring& path);

public:
	static CShaderPreview* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END