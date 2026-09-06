#pragma once
#include "ImGui_Object.h"

NS_BEGIN(MapTool)
class CInspector_Animation final : public CImGui_Object
{
public:
	typedef struct tagAnimSequence
	{
		_uint iAnimIndex;
		_bool bLoop;
		STATE_TIME tStateTime;
	}ANIM_SEQ;

private:
	CInspector_Animation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, class CAssimpEditor* _pEditor);
	virtual ~CInspector_Animation() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void LateUpdate(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CAssimpEditor* m_pAssimpEditor = { nullptr };

	/* Anim Info */
	_int iAnimComboIndex = { 0 };

	/* Anim Sequence */
	vector<ANIM_SEQ> m_vecAnimSequence;
	_int m_iCurrentAnimIndex = { 0 };
	_bool m_bSequencePlay = { false };

	_int m_iAnimIndex = { 0 };
	_bool m_bLoop = { false };
	_float m_fDuration = { 1.f };

	/* Anim Single */
	_bool m_bSinglePlay = { false };
	_uint m_iSingleAnimIndex = { 0 };
	_bool m_bSingleLoop = { true };
	_int m_iSelectedAnim = { 0 };

private:
	void Render_Animation();
	void Render_AnimationSequence();
	void Update_Animation(_float _fTimeDelta);

private:
	const char* Get_AnimationNameList(_uint _iAnimIndex);

public:
	static CInspector_Animation* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, 
		class CAssimpEditor* _pEditor);
	void Free() override;
};
NS_END