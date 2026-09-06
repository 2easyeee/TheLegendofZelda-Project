#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Engine)
class ENGINE_DLL CBody abstract : public CPartObject
{
protected:
	CBody(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CBody(const CBody& _Prototype);
	virtual ~CBody() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

	const _float4x4* Get_SocketMatrixPtr(const _char* _pBoneName);

public:
	/* Animation */
	void Set_Animation(_string _AnimName, _bool _bLoop = true, _bool _isForce = false);
	void Reset_Animation_TrackPosition();
	void Set_AnimationSpeedMulti(_float _fAnimSpeedMulti);
	_bool IsAnimFinished() const;
	_float Get_PlayRatio() const;

	/* Shader Effect */
	void Start_HitFlash();
	void Start_Dissolve();

protected:
	/* Component */
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	class CTexture* m_pTextureCom = { nullptr };

	/* EFFECT (Shader) */
	_float m_fHitStrength = { 0.f };
	STATE_TIME m_tHitTime = { false, 0.f, 0.2f };
	STATE_TIME m_tDissovleTime = { false, 0.f, 3.f };

protected:
	HRESULT Ready_Components(OBJECT_DESC* _Desc);

	/* Shader Effect */
	void Update_ShaderEffect(_float _fTimeDelta);
	HRESULT Render_Binding();
	void Start_DeleteVFX();

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END