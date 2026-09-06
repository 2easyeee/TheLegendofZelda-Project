#include "Tool_Placement.h"
#include "GameInstance.h"
#include "Layer.h"
#include "MapEditor.h"
#include "MapObject.h"
#include "GameObject/MapObject_Light.h"

CTool_Placement::CTool_Placement(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CImGui_Object{ _pDevice, _pDeviceContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CTool_Placement::Initialize(CMapEditor* _pMapEditor)
{
	if (!_pMapEditor)
		return E_FAIL;

	m_pMapEditor = _pMapEditor;

	return S_OK;
}

void CTool_Placement::Update(_float fTimeDelta)
{
	/* Picking */
	Picking_Enable();

	/* Preview GameObject */
	Update_Place_Preview();

	/* Snap */
	Snap_Enable();
}

void CTool_Placement::LateUpdate(_float fTimeDelta)
{
}

HRESULT CTool_Placement::Render()
{
	/* PICKING */
	ImGui::Begin("Picking Option");
	Render_Picking_Button();
	ImGui::End();

	/* SNAP */
	ImGui::Begin("Snap Option");
	Render_Snap_Button();
	ImGui::End();

	return E_NOTIMPL;
}

HRESULT CTool_Placement::Render_Picking_Button()
{
	// TODO : 선택 여부로 Outline
	ImGui::RadioButton("None", (int*)&m_ePickingState, (int)PICKING_STATE::NONE);
	ImGui::SameLine();
	ImGui::RadioButton("Select", (int*)&m_ePickingState, (int)PICKING_STATE::SELECT);
	ImGui::SameLine();
	ImGui::RadioButton("Place", (int*)&m_ePickingState, (int)PICKING_STATE::PLACE);
	if (m_ePickingState == PICKING_STATE::PLACE)
	{
		ImGui::Separator();
		ImGui::Text("FLOOR (Y): ");

		ImGui::SameLine();
		_float fOffset = 1.f;
		if (ImGui::Button("-"))
			m_YFloor -= fOffset;
		ImGui::SameLine();
		ImGui::Text(" %.0f ", m_YFloor);
		ImGui::SameLine();
		if (ImGui::Button("+"))
			m_YFloor += fOffset;
	}

	return S_OK;
}

HRESULT CTool_Placement::Render_Snap_Button()
{
	ImGui::Checkbox("Grid Snap", &m_bSnapEnabled);

	if (m_bSnapEnabled)
	{
		ImGui::Separator();
		ImGui::Text("Snap Step");
		ImGui::DragFloat3("##SnapStep", &m_vSnapStep.x, 0.1f, 0.1f, 10.f);
	}

	return S_OK;
}

void CTool_Placement::Picking_Enable()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
	{
		switch (m_ePickingState)
		{
		case PICKING_STATE::NONE:
			break;
		case PICKING_STATE::SELECT:
			Update_Picking();
			break;
		case PICKING_STATE::PLACE:
		{
			_float3 vPlacePosition = {};
			if (m_pGameInstance->Picking_PlaneY(m_YFloor, vPlacePosition))
			{
				/* Snap E*/
				if (m_bSnapEnabled)
				{
					Snap_Translate(vPlacePosition, m_vSnapStep);
				}

				m_pMapEditor->Set_ImGui_PickedPosition(vPlacePosition);
				Clone_GameObject();
			}
			break;
		}
		}
	}
}

void CTool_Placement::Update_Picking()
{
	_float fMinDist = FLT_MAX;
	CGameObject* pPicked = { nullptr };

	/* LEVEL */
	for (size_t i = 0; i < ENUM_TO_UINT(LEVEL::END); ++i)
	{
		/* Layer */
		for (auto& Pair : m_pGameInstance->Get_Layers()[i])
		{
			auto Layer = Pair.second;
			if (nullptr == Layer)
				continue;

			/* GameObject */
			for (auto* pGameObject : Pair.second->Get_GameObjects())
			{
				if (nullptr == pGameObject)
					continue;

				CTransform* pTransform = static_cast<CTransform*>(pGameObject->Get_Component(TEXT("Com_Transform")));
				m_pGameInstance->Transform_Picking_ToLocalSpace(pTransform->Get_WorldMatrixPtr());

				CModel* pModel = dynamic_cast<CModel*>(pGameObject->Get_Component(TEXT("Com_Model")));
				if (!pModel)
					continue;

				//_float3 vOutHitPos = {};
				//if (!m_pGameInstance->isPicked_AABB(pModel->Get_AABB_Min(), pModel->Get_AABB_Max(), &vOutHitPos))
				//	continue;

				for (auto* pMesh : pModel->Get_Meshes())
				{
					const auto& vertices = pMesh->Get_Vertices();
					const auto& indices = pMesh->Get_Indices();

					if (indices.size() < 3)
						continue;

					for (size_t i = 0; i < indices.size(); i += 3)
					{
						_float3 vHit = {};

						if (m_pGameInstance->isPicked_InLocalSpace(
							&vertices[indices[i + 0]].vPosition,
							&vertices[indices[i + 1]].vPosition,
							&vertices[indices[i + 2]].vPosition,
							&vHit))
						{
							_float fDist =
								XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vHit)));

							if (fDist < fMinDist)
							{
								fMinDist = fDist;
								pPicked = pGameObject;

								if (fMinDist < 0.001f)
									break;
							}
						}
					}

				}

			}
		}
	}

	m_pMapEditor->Set_ImGui_SelectedObject(pPicked);
}

void CTool_Placement::Snap_Enable()
{
	CGameObject* pImGuiSelected = m_pMapEditor->Get_ImGui_SelectedObject();
	if (!pImGuiSelected)
		return;

	if (pImGuiSelected && m_bSnapEnabled)
	{
		CTransform* pTransform = static_cast<CTransform*>(pImGuiSelected->Get_Component(TEXT("Com_Transform")));
		_float3 vPosition = {};
		XMStoreFloat3(&vPosition, pTransform->Get_State(STATE::POSITION));

		Snap_Translate(vPosition, m_vSnapStep);

		_vector Position = XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 1.f);
		pTransform->Set_State(STATE::POSITION, Position);
	}
}

_float3 CTool_Placement::Snap_RoundVector3(_float3& v3, _float3 vStep)
{
	_float fRoundX = roundf(v3.x / vStep.x) * vStep.x;
	_float fRoundY = roundf(v3.y / vStep.y) * vStep.y;
	_float fRoundZ = roundf(v3.z / vStep.z) * vStep.z;

	return _float3(fRoundX, fRoundY, fRoundZ);
}

void CTool_Placement::Snap_Translate(_float3& vPosition, _float3 vStep)
{
	vPosition = Snap_RoundVector3(vPosition, vStep);
}

void CTool_Placement::Update_Place_Preview()
{
	if (m_ePickingState != PICKING_STATE::PLACE)
	{
		if (m_pPreviewObject)
		{
			m_pGameInstance->Reserve_DeleteObject(m_pPreviewObject);
			m_pPreviewObject = nullptr;
		}
		return;
	}

	_float3 vPreivewPos = {};
	if (!m_pGameInstance->Picking_PlaneY(m_YFloor, vPreivewPos))
		return;

	if (m_bSnapEnabled)
		Snap_Translate(vPreivewPos, m_vSnapStep);

	if (!m_pPreviewObject)
	{
		Clone_Preview(vPreivewPos);
		return;
	}

	CTransform* pTransform =
		static_cast<CTransform*>(m_pPreviewObject->Get_Component(TEXT("Com_Transform")));

	if (!pTransform)
		return;
	_vector vPos = XMVectorSet(vPreivewPos.x, vPreivewPos.y, vPreivewPos.z, 1.f);
	pTransform->Set_State(STATE::POSITION, vPos);

	/* Picking::Place 상태가 아니면 삭제 처리 || 갱신 시 삭제 처리*/
	if ((m_pMapEditor->IsPreviewUpdated()))
	{
		if (m_pPreviewObject)
		{
			m_pGameInstance->Reserve_DeleteObject(m_pPreviewObject);
			m_pPreviewObject = nullptr;
		}
		return;
	}
}

void CTool_Placement::Clone_Preview(_float3 _vPreviewPos)
{
	EXPORT_TAGS tags = m_pMapEditor->Get_ImGui_PreviewGameObject();

	CGameObject::OBJECT_DESC tObjectDesc = {};
	wcscpy_s(tObjectDesc.ObjectID, TEXT("Preview"));
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MAPEDITOR);
	wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_StaticMapObject_Preview"));
	wcscpy_s(tObjectDesc.LayerTag, TEXT("Preview"));
	wcscpy_s(tObjectDesc.ModelTag, tags.ModelTag.c_str());
	wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_Preview"));

	CMapObject::MAP_INIT_DESC tInitDesc = {};
	tInitDesc.tObjectDesc = tObjectDesc;
	tInitDesc.tWorldDesc.vPosition = _vPreviewPos;

	CGameObject* pGameObject = { nullptr };
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		RESOURCE_LEVEL_STATIC,
		tObjectDesc.ObjectTag,
		ENUM_TO_UINT(LEVEL::MAPEDITOR),
		tObjectDesc.LayerTag,
		&tInitDesc, &pGameObject)))
		return;

	m_pPreviewObject = pGameObject;
}

void CTool_Placement::Clone_GameObject()
{
	EXPORT_TAGS tags = m_pMapEditor->Get_ImGui_PreviewGameObject();

	CGameObject::OBJECT_DESC tObjectDesc = {};
	_wstring newID = Make_UniqueID(tags.ObjectID);
	wcscpy_s(tObjectDesc.ObjectID, newID.c_str());
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::MAPEDITOR);
	wcscpy_s(tObjectDesc.ObjectTag, tags.GameObjectTag.c_str());
	wcscpy_s(tObjectDesc.LayerTag, tags.LayerTag.c_str());
	wcscpy_s(tObjectDesc.ShaderTag, tags.ShaderTag.c_str());
	wcscpy_s(tObjectDesc.ModelTag, tags.ModelTag.c_str());

	if (_wstring(tObjectDesc.LayerTag) == TEXT("Light"))
	{
		CMapObject_Light::LIGHT_INIT_DESC tInitDesc = {};
		tInitDesc.tObjectDesc = tObjectDesc;
		tInitDesc.tWorldDesc.vPosition =
			m_pMapEditor->Get_ImGui_PickedPosition();
		tInitDesc.tLightDesc.eType = LIGHT::POINT;
		tInitDesc.tLightDesc.vPosition = {
			tInitDesc.tWorldDesc.vPosition.x,
			tInitDesc.tWorldDesc.vPosition.y,
			tInitDesc.tWorldDesc.vPosition.z,
			1.f
		};
		tInitDesc.tLightDesc.vDirection = { 0.f, -1.f, 0.f, 0.f };

		tInitDesc.tLightDesc.vDiffuse = { 1.f,1.f,1.f,1.f };
		tInitDesc.tLightDesc.vAmbient = { 0.2f,0.2f,0.2f,1.f };
		tInitDesc.tLightDesc.vSpecular = { 1.f,1.f,1.f,1.f };

		tInitDesc.tLightDesc.fRange = 5.f;

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
			RESOURCE_LEVEL_STATIC,
			tObjectDesc.ObjectTag,
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			tObjectDesc.LayerTag,
			&tInitDesc, nullptr)))
			return;
	}
	else
	{
		CMapObject::MAP_INIT_DESC tInitDesc = {};
		tInitDesc.tObjectDesc = tObjectDesc;
		tInitDesc.tWorldDesc.vPosition =
			m_pMapEditor->Get_ImGui_PickedPosition();

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
			RESOURCE_LEVEL_STATIC,
			tObjectDesc.ObjectTag,
			ENUM_TO_UINT(LEVEL::MAPEDITOR),
			tObjectDesc.LayerTag,
			&tInitDesc, nullptr)))
			return;
	}
}

_wstring CTool_Placement::Make_UniqueID(_wstring& _ID)
{
	_uint& iIndex = m_ObjectsID[_ID]; 	// 참조!!!

	_wstring wstrNewID = _ID + TEXT("_") + to_wstring(iIndex);
	++iIndex;

	return wstrNewID;
}

CTool_Placement* CTool_Placement::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, CMapEditor* _pMapEditor)
{
	CTool_Placement* pInstance = new CTool_Placement(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize(_pMapEditor)))
	{
		MSG_BOX("FAILED TO CREATED : CTool_Placement");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTool_Placement::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
