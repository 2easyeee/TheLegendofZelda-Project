#pragma once
#include "ImGui_Object.h"
#include "EffectEditor.h"
#include "ShaderPreview.h"

NS_BEGIN(MapTool)
class CInspector_Shader final : public CImGui_Object
{
public:
	typedef struct tagComboDesc
	{
		_uint iSelectedIndex = 0;
		_wstring wstrSelectedTag;
		bool bIsSelected = { false };
	}COMBO_DESC;

private:
	CInspector_Shader(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CInspector_Shader() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	/* Preview */
	class CShaderPreview* m_pShaderPreviewObject = { nullptr };

	/* Texture Type */
	CShaderPreview::TEXTURE_SLOT m_eSelectedSlot = CShaderPreview::TEXTURE_SLOT::MAIN;

	/* Mesh List */
	vector<_wstring> m_MeshTags;
	COMBO_DESC m_tComboDesc_Mesh;

	_char EffectName[128] = "";

private:
	/* Render */
	HRESULT Render_Shader_Param();
	HRESULT Render_Save();
	HRESULT Render_Load();
	
	/* Texture */
	HRESULT Load_Texture(const wchar_t* _Path);

	/* ShaderPreview Object */
	HRESULT Create_ShaderPreviewObject();

	/* Load All Effect Meshes */
	void Load_Mesh_List();

private:
	void Render_ImGui_Combo(const _char* _strLabel, vector<_wstring> _vecTags, COMBO_DESC& _tComboDesc, _wstring& _outTag);
	void Filter_Tags(const vector<_wstring>& _vecTags, const _wstring& _prefix, vector<_wstring>& _output);

public:
	static CInspector_Shader* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	void Free() override;
};
NS_END