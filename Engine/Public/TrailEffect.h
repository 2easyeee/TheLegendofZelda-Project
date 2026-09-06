#pragma once
#include "MapObject.h"

NS_BEGIN(Engine)
class CVIBuffer_Trail;
class CShader;
class CTexture;
NS_END

NS_BEGIN(Engine)
class ENGINE_DLL CTrailEffect : public CMapObject
{
public:
	enum class TRAIL_EFFECT_TYPE { SLASH, CHARGESHOT, END };

	typedef struct tagEffectInitDesc
	{
		OBJECT_DESC tObjectDesc;
		WORLD_DESC tWorldDesc;
		_float fWidth = 0.5f;
		_uint iMaxPoint = 32;
		_float fTrailDuration = { 0.3f };
		const _float4x4* pParentMatrix = { nullptr };
		const _float4x4* pBaseSocketMatrix = { nullptr };
		const _float4x4* pTipSocketMatrix = { nullptr };
	}TRAIL_INIT_DESC;

public:
	CTrailEffect(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CTrailEffect(const CTrailEffect& _Prototype);
	virtual ~CTrailEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Add_Point(_vector _vPos);
	void Add_TipBasePoint(_vector _vTip, _vector _vBase);

protected:
	/* Timeline */
	_bool m_bUseTimeline = { false };
	SHADER_TIMELINE m_tTimeline = {};
	STATE_TIME m_tTime = { true, 0.f, 0.3f };
	_bool m_bDelete = { false };
	_float m_fRandValue = { 0.f };

	/* Param */
	SHADER_PARAM_DESC m_tShaderDesc;

	/* Transform */
	_float m_fVSScale = { 1.f };

	/* Trail */
	const _float4x4* m_pParentMatrix = { nullptr };
	const _float4x4* m_pBaseSocketMatrix = { nullptr };
	const _float4x4* m_pTipSocketMatrix = { nullptr };

	vector<_vector> m_TrailTips;
	vector<_vector> m_TrailBases;

protected:
	void Set_TextureSlots(_uint main = 0, _uint distortion = 0);
	void Set_EffectPassIndex(TRAIL_EFFECT_TYPE _eEffectType);

private:
	/* Component */
	CVIBuffer_Trail* m_pVIBufferCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pMainTexture = { nullptr };
	CTexture* m_pDistortionTexture = { nullptr };

	/* Trail */
	vector<_vector>  m_TrailPoints;
	_float m_fWidth = 0.5f;
	_uint  m_iMaxPoint = 32;

	/* Teture Num */
	_uint m_iMainTexSlot = { 0 };
	_uint m_iDistortionSlot = { 0 };

	/* Pass */
	_uint m_iEffectPassIndex = ENUM_TO_UINT(TRAIL_EFFECT_TYPE::SLASH);

private:
	HRESULT Ready_Components(OBJECT_DESC* _pTags);

public:
	static CTrailEffect* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;

};
NS_END
