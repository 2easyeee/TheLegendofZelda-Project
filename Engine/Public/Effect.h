#pragma once
#include "MapObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CVIBuffer_Plane;
NS_END

NS_BEGIN(Engine)
class ENGINE_DLL CEffect : public CMapObject
{
public:
	enum class EFFECT_TYPE { DEFAULT, FIRE_01, FIRE_02, 
		GRASS, STAR, PURPLE_SMOKE, DUST_SMOKE, END };

public:
	typedef struct tagEffectDesc
	{
		_wstring MainTextureTag;
		_wstring NoiseTag_0;
		_wstring NoiseTag_1;
		_wstring MaskTag;
		_wstring DistortionTag;
		_wstring DissolveTag;
	}EFFECT_DESC;

	typedef struct tagEffectInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		EFFECT_DESC tEffectDesc;
		_float fScale;
	}EFFECT_INIT_DESC;

public:
	CEffect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CEffect(const CEffect& _Prototype);
	virtual ~CEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_Dir(_vector _vDir);

protected:
	/* Component */
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	/* Ex. */
	CVIBuffer_Plane* m_pVIBufferCom = { nullptr };

	/* Timeline */
	_bool m_bUseTimeline = { false };
	SHADER_TIMELINE m_tTimeline = {};
	STATE_TIME m_tTime = { true, 0.f, 5.f };
	_bool m_bDelete = { false };
	_float m_fRandValue = { 0.f };

	/* Param */
	SHADER_PARAM_DESC m_tShaderDesc;

	/* Transform */
	_float m_fVSScale = { 1.f };

	/* Pass */
	_uint m_iEffectPassIndex = ENUM_TO_UINT(EFFECT_TYPE::DEFAULT);

protected:
	HRESULT Render_Binding();

	void Set_TextureSlots(_uint main = 0, _uint noise0 = 0, _uint noise1 = 0, _uint mask = 0, _uint distort = 0, _uint dissolve = 0);
	void Set_EffectPassIndex(EFFECT_TYPE _eEffectType);

private:
	/* Component */
	CTexture* m_pMainTexture = { nullptr };
	CTexture* m_pNoiseTexture_0 = { nullptr };
	CTexture* m_pNoiseTexture_1 = { nullptr };
	CTexture* m_pMaskTexture = { nullptr };
	CTexture* m_pDistortionTexture = { nullptr };
	CTexture* m_pDissolveTexture = { nullptr };

	/* Teture Num */
	_uint m_iMainTexSlot		= { 0 };
	_uint m_iNoise0Slot			= { 0 };
	_uint m_iNoise1Slot			= { 0 };
	_uint m_iMaskSlot			= { 0 };
	_uint m_iDistortionSlot		= { 0 };
	_uint m_iDissolveSlot		= { 0 };

	/* Param */
	SHADER_TEXTURE_DESC m_tTextures;
	
private:
	HRESULT Ready_Components(EFFECT_INIT_DESC* _pTags);

public:
	static CEffect* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END