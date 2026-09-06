#include "MapEditor.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Layer.h"
#include "Transform.h"
#include "MapObject.h"
#include "ImGui_Manager.h"
#include "AssimpEditor.h"
#include "Camera_Free.h"
#include "Tool_FileIO.h"
#include "Tool_ChangeLevel.h"
#include "Tool_Edit.h"
#include "Tool_Placement.h"
#include "Tool_Light.h"

CMapEditor::CMapEditor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CLevel { _pDevice, _pDeviceContext }
{
}

HRESULT CMapEditor::Initialize(CImGui_Manager* _pImGuiManager)
{
	/* ImGui */
	if (!m_pImGuiManager)
		m_pImGuiManager = _pImGuiManager;

	/* Register ImGui Object */
	m_pImGuiManager->Register_ImGui_Objcet(TEXT("Tool_FileIO"), CTool_FileIO::Create(m_pDevice, m_pDeviceContext));
	m_pImGuiManager->Register_ImGui_Objcet(TEXT("Tool_Edit"), CTool_Edit::Create(m_pDevice, m_pDeviceContext, this));
	m_pImGuiManager->Register_ImGui_Objcet(TEXT("Tool_Placement"), CTool_Placement::Create(m_pDevice, m_pDeviceContext, this));
	m_pImGuiManager->Register_ImGui_Objcet(TEXT("Tool_Light"), CTool_Light::Create(m_pDevice, m_pDeviceContext, this));
	
	/* Add_GameObject */
 	if (FAILED(Ready_Layer_Camera(TEXT("Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Terrain(TEXT("Terrain"))))
		return E_FAIL;
	
    return S_OK;
}

void CMapEditor::Update(_float _fTimeDelta)
{
	/* Light */
	Add_Lights();
}

HRESULT CMapEditor::Render()
{
	SetWindowText(g_hWnd, TEXT("MAP EDITOR"));

	ImGui::Begin("Hierarchy");
	m_pImGuiSelected = Render_ImGui_Hierarchy();
	ImGui::End();

	ImGui::Begin("Inspector");
	Render_ImGui_Inspector();
	ImGui::End();

    return S_OK;
}

_bool CMapEditor::IsPreviewUpdated()
{
	if (m_bIsPreviewUpdate)
	{
		m_bIsPreviewUpdate = false;
		return true;
	}
	return false;
}

CGameObject* CMapEditor::Render_ImGui_Hierarchy()
{
	CGameObject* pLastSelected = m_pImGuiSelected;
	CGameObject* pSelected = { nullptr };
	_uint iIndex = 0;

	/* LEVEL */ 
	for (size_t i = 0; i < ENUM_TO_UINT(LEVEL::END); ++i)
	{
		/* Layer */
		for (auto& Pair : m_pGameInstance->Get_Layers()[i])
		{
			auto Layer = Pair.second;
			if (nullptr == Layer)
				continue;

			string strLayerTag = string().assign(Pair.first.begin(), Pair.first.end());
			if (strLayerTag == "")
				strLayerTag = "Layer_Default";

			/* Group Hierarchy by Layer Tags */
			if (ImGui::TreeNode(strLayerTag.c_str()))
			{
				/* GameObject */
				for (auto* pGameObject : Pair.second->Get_GameObjects())
				{
					if (nullptr == pGameObject)
						continue;
					
					ImGui::PushID(pGameObject); 
					ImGuiTreeNodeFlags eFlag = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

					if (pGameObject == pLastSelected)
						eFlag |= ImGuiTreeNodeFlags_Selected;

					_wstring wstrObjectID = pGameObject->Get_ObjectID();
					string strDisplayName = string().assign(wstrObjectID.begin(), wstrObjectID.end());
					string strLabel = strDisplayName + "##" + to_string(iIndex);

					if (ImGui::TreeNodeEx(strLabel.c_str(), eFlag))
					{
						if (ImGui::IsItemClicked())
							pSelected = pGameObject;
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		}
	}

	if (!pSelected)
		pSelected = pLastSelected;

	m_pImGuiSelected = pSelected;
	return pSelected;
}

HRESULT CMapEditor::Render_ImGui_Inspector()
{
	if (!m_pImGuiSelected)
		return S_OK;

	/* Object 별*/
	_wstring wstrObjectID = m_pImGuiSelected->Get_ObjectID();
	string strDisplayName = string().assign(wstrObjectID.begin(), wstrObjectID.end());
	if (ImGui::CollapsingHeader(strDisplayName.c_str()))
	{
		//ImGui::InputText();
	}

	/* Component 별 */
	string strLabel = "Com_Transform";
	if (ImGui::CollapsingHeader(strLabel.c_str()))
		Render_ImGui_Transform();

	return S_OK;
}

HRESULT CMapEditor::Render_ImGui_Transform()
{
	CTransform* pTransform = static_cast<CTransform*>(m_pImGuiSelected->Get_Component(TEXT("Com_Transform")));

	/* 저장용 */
	_float3 vPosition = {};
	_float3 vScale = {};
	_float4 vRotationQuat = {};

	_float4x4 worldMatrix = *pTransform->Get_WorldMatrixPtr();
	_matrix world = XMLoadFloat4x4(&worldMatrix);

	/* 연산용 */
	_vector justDecomposeScale;
	_vector justDecomposeRotationQuat;
	_vector justDecomposePosition;

	if (XMMatrixDecompose(&justDecomposeScale, &justDecomposeRotationQuat, &justDecomposePosition, world))
	{
		/* 연산용 -> 메모리용 */
		XMStoreFloat3(&vScale, justDecomposeScale);
		XMStoreFloat4(&vRotationQuat, justDecomposeRotationQuat);
		XMStoreFloat3(&vPosition, justDecomposePosition);
	}

	/* Edit "Position" */
	if (ImGui::DragFloat3("Position (Slide)", &vPosition.x, 0.05f))
	{
		/* 저장 -> 연산 */
		_vector Position = XMLoadFloat3(&vPosition);
		
		/* Set "W" */
		Position = XMVectorSetW(Position, 1.f);

		/* 연산용으로 전달 */
		pTransform->Set_State(STATE::POSITION, Position);
	}

	/* Edit "Scale" */
	if (ImGui::DragFloat3("Scale", &vScale.x, 0.01f))
	{
		/* 최소값 보정 */
		vScale.x = max(0.001f, vScale.x);
		vScale.y = max(0.001f, vScale.y);
		vScale.z = max(0.001f, vScale.z);

		/* 저장 -> 연산 */
		_vector Scale = XMLoadFloat3(&vScale);

		/* 기존 축 */
		_vector vRight = pTransform->Get_State(STATE::RIGHT);
		_vector vUp = pTransform->Get_State(STATE::UP);
		_vector vLook = pTransform->Get_State(STATE::LOOK);

		/* 정규화 */
		vRight = XMVector3Normalize(vRight);
		vUp = XMVector3Normalize(vUp);
		vLook = XMVector3Normalize(vLook);

		/* 축 * 스케일 */
		pTransform->Set_State(STATE::RIGHT, XMVectorScale(vRight, XMVectorGetX(Scale)));
		pTransform->Set_State(STATE::UP, XMVectorScale(vUp, XMVectorGetY(Scale)));
		pTransform->Set_State(STATE::LOOK, XMVectorScale(vLook, XMVectorGetZ(Scale)));
	}

	/* Edit "Rotation" */
	_float3 vEuler = pTransform->GetEular(vRotationQuat);
	if (ImGui::DragFloat3("Rotation (Slide)", &vEuler.x, 0.1f))
		pTransform->Update_ImGui_Inspector_Rotation(vEuler);

	return S_OK;
}

_matrix CMapEditor::PreTransformMatrix(const _float3& _scale, const _float3& _rotation, const _float3& _translation)
{
	return XMMatrixScaling(_scale.x, _scale.y, _scale.z)
		* XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(_rotation.x),
			XMConvertToRadians(_rotation.y),
			XMConvertToRadians(_rotation.z))
		* XMMatrixTranslation(_translation.x, _translation.y, _translation.z);
}

HRESULT CMapEditor::Ready_Layer_Terrain(const _wstring& strLayerTag)
{
	CGameObject::OBJECT_DESC tObjectDesc = {};
	SET_DESC(tObjectDesc.ObjectID, TEXT("Terrain"));
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
	SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
	SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Monster"));
	SET_DESC(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Terrain"));
	SET_DESC(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxNorTex"));
	SET_DESC(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_Terrain"));

	CMapObject::MAP_INIT_DESC tInitDesc = {};
	tInitDesc.tObjectDesc = tObjectDesc;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		tObjectDesc.iLevel,
		TEXT("Prototype_GameObject_Terrain"),
		ENUM_TO_UINT(LEVEL::MAPEDITOR),
		strLayerTag, &tInitDesc)))
		return E_FAIL;

    return S_OK;
}

HRESULT CMapEditor::Ready_Layer_Camera(const _wstring& strLayerTag)
{
	CGameObject::OBJECT_DESC tObjectDesc = {};
	SET_DESC(tObjectDesc.ObjectID, TEXT("Camera_Free"));
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
	SET_DESC(tObjectDesc.LayerTag, strLayerTag.c_str());
	SET_DESC(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_Camera_Free"));

	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.vEye = _float3(0.f, 10.f, -8.f);
	CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
	CameraDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = 90.0f;
	CameraDesc.fSensor = 0.07f;

	CCamera::CAMERA_INIT_DESC tInitDesc = {};
	tInitDesc.tObjectDesc = tObjectDesc;
	tInitDesc.tCameraDesc = CameraDesc;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		tObjectDesc.iLevel,
		tObjectDesc.ObjectTag,
		ENUM_TO_UINT(LEVEL::MAPEDITOR),
		strLayerTag, &tInitDesc)))
		return E_FAIL;

	return S_OK;
}

void CMapEditor::Add_Lights()
{
	auto pLayers = m_pGameInstance->Get_Layers()[ENUM_TO_UINT(LEVEL::MAPEDITOR)];

	m_pGameInstance->Clear_Light();

	/* Layer */
	for (auto& Pair : pLayers)
	{
		const _wstring& wstrLayerTag = Pair.first;

		if (wstrLayerTag.rfind(TEXT("Light"), 0) != 0)
			continue;

		/* GameObjects */
		CLayer* pLayer = Pair.second;
		if (!pLayer)
		{
			MSG_BOX("FAILED TO BRING : Layer 하위 GameObjects");
			continue;
		}

		for (auto& pGameObject : pLayer->Get_GameObjects())
		{
			if (!pGameObject)
				continue;

			CMapObject_Light* pLightObject = dynamic_cast<CMapObject_Light*>(pGameObject);
			if (!pLightObject)
				continue;

			m_pGameInstance->Add_Light(pLightObject->Get_LightDesc());
		}
	}
}

CMapEditor* CMapEditor::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CImGui_Manager* _pImGuiManager)
{
	CMapEditor* pInstance = new CMapEditor(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize(_pImGuiManager)))
	{
		MSG_BOX("FAILED TO CREATED : CMapEditor");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMapEditor::Free()
{
	__super::Free();
}
