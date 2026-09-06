#include "Inspector_Animation.h"
#include "AssimpEditor.h"
#include "Model.h"
#include "StaticMapObject.h"

CInspector_Animation::CInspector_Animation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
	: CImGui_Object{ _pDevice, _pDeviceContext }
	, m_pAssimpEditor{ _pEditor }
{
}

HRESULT CInspector_Animation::Initialize()
{
	return S_OK;
}

void CInspector_Animation::Update(_float _fTimeDelta)
{
    Update_Animation(_fTimeDelta);
}

void CInspector_Animation::LateUpdate(_float _fTimeDelta)
{
}

HRESULT CInspector_Animation::Render()
{
	ImGui::Begin("Inspector (Animation)");
	Render_Animation();
	ImGui::End();

    ImGui::Begin("Inspector (Animation Sequence)");
    Render_AnimationSequence();
    ImGui::End();

	return S_OK;
}

void CInspector_Animation::Render_Animation()
{
    if (!m_pAssimpEditor)
        return;

    _uint animCount = m_pAssimpEditor->Get_AnimationTotalCount();
    if (animCount == 0)
    {
        ImGui::TextDisabled("No Animations");
        return;
    }

    /* Combo (deprecated) */
    vector<const char*> animNames;
    vector<_string> animIndexAndName;
    animNames.reserve(animCount);
    animIndexAndName.reserve(animCount);

    for (_uint i = 0; i < animCount; ++i)
    {
        const aiAnimation* anim = m_pAssimpEditor->Get_Animation(i);
        
        _string index = "[" + to_string(i) + "]";
        index += anim->mName.C_Str();

        animIndexAndName.push_back(index);
        animNames.push_back(animIndexAndName.back().c_str());
    }

    ImGui::SeparatorText("Animations");
    if (ImGui::Combo("##InspectorAnimation", &m_iSelectedAnim, animNames.data(), animCount))
    {
        iAnimComboIndex = m_iSelectedAnim;
    }
    if (ImGui::Button("Play"))
    {
        m_bSequencePlay = false;
        m_bSinglePlay = true;
        m_iSingleAnimIndex = iAnimComboIndex;

        CStaticMapObject* pGameObject =
            static_cast<CStaticMapObject*>(m_pAssimpEditor->Get_PreviewObject());

        if (!pGameObject) return;

        CModel* pModel =
            dynamic_cast<CModel*>(pGameObject->Get_Component(TEXT("Com_Model")));

        if (!pModel) return;

        pModel->Set_Animation(m_iSingleAnimIndex, m_bSingleLoop);
    }

    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        m_bSinglePlay = false;
    }

    ImGui::Checkbox("Loop (Single)", &m_bSingleLoop);
   
    /* Anim Info */
    const aiAnimation* currentAnim = m_pAssimpEditor->Get_Animation(iAnimComboIndex);
    if (currentAnim)
    {
        ImGui::SeparatorText("Channel");
        ImGui::Text("Duration        : %.2f", currentAnim->mDuration);
        ImGui::Text("Ticks Per Sec   : %.2f", currentAnim->mTicksPerSecond);
        ImGui::Text("Num Channels    : %d", currentAnim->mNumChannels);
    }
}

void CInspector_Animation::Render_AnimationSequence()
{
    /* Play & Stop */
    if (ImGui::Button("Play"))
    {
        if (!m_vecAnimSequence.empty())
        {
            m_bSequencePlay = true;
            m_iCurrentAnimIndex = 0;

            for (auto& Sequence : m_vecAnimSequence)
                Sequence.tStateTime.fAccTime = 0.f;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        m_bSequencePlay = false;

    /* Input Anim Sequence */
    ImGui::SeparatorText("Herer Add !");

    ImGui::PushItemWidth(80.f);
    ImGui::InputInt("Anim Index", &m_iAnimIndex);
    ImGui::PopItemWidth();
    ImGui::Checkbox("Loop", &m_bLoop);
    ImGui::PushItemWidth(80.f);
    ImGui::InputFloat("Duration", &m_fDuration);
    ImGui::PopItemWidth();

    m_iAnimIndex = max(0, m_iAnimIndex);
    m_fDuration = max(0.1f, m_fDuration);

    if (ImGui::Button("+", ImVec2(24, 24)))
    {
        ANIM_SEQ tAnimSeq = {};
        tAnimSeq.iAnimIndex = (_uint)m_iAnimIndex;
        tAnimSeq.bLoop = m_bLoop;
        tAnimSeq.tStateTime.fAccTime = 0.f;
        tAnimSeq.tStateTime.fAccDurationTime = m_fDuration;

        m_vecAnimSequence.push_back(tAnimSeq);
    }

    /* Show Anim Sequence */
    ImGui::SeparatorText("Animation Sequence");
    for (size_t i = 0; i < m_vecAnimSequence.size(); ++i)
    {
        auto& tAnimSequence = m_vecAnimSequence[i];
        ImGui::Text("[%zu] %s Anim=%d | Loop=%s | Duration=%.2f",
            i,
            Get_AnimationNameList(tAnimSequence.iAnimIndex),
            tAnimSequence.iAnimIndex,
            tAnimSequence.bLoop ? "YES" : "NO",
            tAnimSequence.tStateTime.fAccDurationTime
        );
    }
}

void CInspector_Animation::Update_Animation(_float _fTimeDelta)
{
    if (!m_pAssimpEditor)
        return;

    if (m_bSinglePlay)
        return;

    if (!m_bSequencePlay || m_vecAnimSequence.empty())
        return;
    
    if (m_iCurrentAnimIndex >= m_vecAnimSequence.size())
        m_iCurrentAnimIndex = 0;

    ANIM_SEQ& tAnimSeq = m_vecAnimSequence[m_iCurrentAnimIndex]; // 반드시 참조로 받기
    tAnimSeq.tStateTime.fAccTime += _fTimeDelta;
    if (tAnimSeq.tStateTime.fAccTime == _fTimeDelta)
    {
        CStaticMapObject* pGameObject = static_cast<CStaticMapObject*>(m_pAssimpEditor->Get_PreviewObject());
        CModel* pModel = dynamic_cast<CModel*>(pGameObject->Get_Component(TEXT("Com_Model")));
        if (!pModel)
            return;
        pModel->Set_Animation(tAnimSeq.iAnimIndex, tAnimSeq.bLoop);
    }

    if (tAnimSeq.tStateTime.fAccTime >= tAnimSeq.tStateTime.fAccDurationTime)
    {
        tAnimSeq.tStateTime.fAccTime = 0.f;
        m_iCurrentAnimIndex = (m_iCurrentAnimIndex + 1) % m_vecAnimSequence.size();
    }
}

const char* CInspector_Animation::Get_AnimationNameList(_uint _iAnimIndex)
{
    if (!m_pAssimpEditor)
        return "";

    aiAnimation* pAnim = m_pAssimpEditor->Get_Animation(_iAnimIndex);
    if (!pAnim)
        return "";

    return pAnim->mName.C_Str();
}

CInspector_Animation* CInspector_Animation::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CAssimpEditor* _pEditor)
{
	CInspector_Animation* pInstance = new CInspector_Animation(_pDevice, _pDeviceContext, _pEditor);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CInspector_Animation");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInspector_Animation::Free()
{
	__super::Free();
}
