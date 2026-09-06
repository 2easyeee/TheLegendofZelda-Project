#include "Inspector_NavMesh.h"
#include "GameInstance.h"
#include "NavHolder.h"
#include "Cell.h"

CInspector_NavMesh::CInspector_NavMesh(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CImGui_Object{ _pDevice, _pDeviceContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
    , m_iFileIndex_Load(-1)
    , m_iFileIndex_Save(-1)
{
    Safe_AddRef(m_pGameInstance);
}


HRESULT CInspector_NavMesh::Initialize()
{
    if (FAILED(Init_NavFile()))
        return E_FAIL;

    if (FAILED(Load_Resources()))
        return E_FAIL;

    return S_OK;
}

void CInspector_NavMesh::Update(_float fTimeDelta)
{
    Update_Delete_Nav();

    Handle_MouseInput();
    Picking_Cell();
    Undo();
}

void CInspector_NavMesh::LateUpdate(_float fTimeDelta)
{
}

HRESULT CInspector_NavMesh::Render()
{
    ImGui::Begin("Navigation Editor");
    if (FAILED(Render_Create_Cell()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("FileIO");
    if (FAILED(Render_FileIO()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("Delete");
    if (FAILED(Render_Delete_Cell()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("Height");
    if (FAILED(Render_Height()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("Point Edit");
    if (FAILED(Render_Position()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("Multi Move");
    if (FAILED(Render_Position_Multi()))
        return E_FAIL;
    ImGui::End();

    ImGui::Begin("Nav Info");
    if (FAILED(Render_Cell_Info()))
        return E_FAIL;
    ImGui::End();

    return S_OK;
}

void CInspector_NavMesh::Update_Delete_Nav()
{
    if (m_bDeleteNavHolder)
    {
        m_pGameInstance->Reserve_DeleteObject(m_pCurrentNavHolder);
        m_pCurrentNavHolder = nullptr;
        m_bDeleteNavHolder = false;
    }
}

HRESULT CInspector_NavMesh::Render_FileIO()
{
    if (ImGui::BeginTable("FileIOTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 140.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Save Nav */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Save Level");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(30);
        ImGui::InputInt("##SaveLevel", &m_iFileIndex_Save, 0);
        ImGui::SameLine();
        if (ImGui::Button("Click!##Save"))
        {
            int iResult = MessageBox(nullptr, L"저장 잘못누른거 아니지 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
            if (iResult == IDOK)
            {
                Save_Binary();
            }
        }

        /* Load Nav */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Load Level");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(30);
        ImGui::InputInt("##LoadLevel", &m_iFileIndex_Load, 0);
        ImGui::SameLine();
        if (ImGui::Button("Click!##Load"))
        {
            int iResult = MessageBox(nullptr, L"로드하기 전에 저장 했나 ???", L"Last Chance.", MB_OKCANCEL | MB_ICONQUESTION);
            if (iResult == IDOK)
            {
                MakeNavigationHolder();
            }
        }

        /* Load Map */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Map Level");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(30);
        ImGui::InputInt("##MapLevel", &m_iFileIndex_Map, 0);
        ImGui::SameLine();
        if (ImGui::Button("Click!##Map"))
        {
            Load_Resources();
        }

        ImGui::EndTable();
    }
    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Cell_Info()
{
    if (!m_pNavCom)
    {
        ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), "No Nav.");
        return S_OK;
    }

    for (int i = 0; i < m_pNavCom->Get_CellCount(); ++i)
    {
        CCell* pCell = m_pNavCom->Get_Cell(i);
        if (!pCell)
            continue;

        _bool bSelected =
            find(m_vecSelectedCells.begin(),
                m_vecSelectedCells.end(), i) != m_vecSelectedCells.end();

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            (bSelected ? ImGuiTreeNodeFlags_Selected : 0);

        _bool bOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "Cell %d", i);

        if (ImGui::IsItemClicked())
        {
            if (ImGui::GetIO().KeyShift)
            {
                if (!bSelected)
                {
                    m_vecSelectedCells.push_back(i);
                }
                else
                {
                    m_vecSelectedCells.erase(
                        remove(m_vecSelectedCells.begin(), m_vecSelectedCells.end(), i),
                        m_vecSelectedCells.end());
                }
            }
            else
            {
                m_vecSelectedCells.clear();
                m_vecSelectedCells.push_back(i);
            }
        }

        if (bOpen)
        {
            _float3 A, B, C;

            XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
            XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
            XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

            ImGui::Text("A: %.2f %.2f %.2f", A.x, A.y, A.z);
            ImGui::Text("B: %.2f %.2f %.2f", B.x, B.y, B.z);
            ImGui::Text("C: %.2f %.2f %.2f", C.x, C.y, C.z);

            ImGui::TreePop();
        }
    }

    /* Set Nav */
    if (!m_vecSelectedCells.empty())
        m_pNavCom->Set_SelctedCells(m_vecSelectedCells);

    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Create_Cell()
{
    if (!m_pNavCom)
    {
        ImGui::TextDisabled("No Nav.");
        return S_OK;
    }

    if (ImGui::BeginTable("CreateCellTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 140.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Clear Temp */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Clear");

        ImGui::TableSetColumnIndex(1);
        if (ImGui::Button("Clear All"))
            m_vecTempPoints.clear();

        /* Cell Count */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Cell Count");

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", m_pNavCom->Get_CellCount());

        /* Temp Points Count */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Temp Points");

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d / 3", (int)m_vecTempPoints.size());

        ImGui::EndTable();
    }
    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Delete_Cell()
{
    if (ImGui::BeginTable("DeleteCellTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 140.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Name */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Cell Index");

        ImGui::TableSetColumnIndex(1);
        if (m_iSelectedCell != -1)
        {
            ImGui::Text("Cell %d", m_iSelectedCell);
        }

        /* Clear Temp */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Delete");

        ImGui::TableSetColumnIndex(1);
        if (m_iSelectedCell != -1)
        {
            if (ImGui::Button("Delete"))
            {
                m_pNavCom->Remove_Cell(m_iSelectedCell);
                m_iSelectedCell = -1;
            }
        }

        ImGui::EndTable();
    }
    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Height()
{
    if (!m_pNavCom)
        return S_OK;

    if (ImGui::BeginTable("NavHeightTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("[TAG]", ImGuiTableColumnFlags_WidthFixed, 160.f);
        ImGui::TableSetupColumn("[VALUE]", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        /* Height (New Cell) */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Height (New Cell)");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::DragFloat("##HeightNew", &m_fHeightOffset, 0.1f, -10.f, 10.f);

       /* Height (Selected) */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Height (Selected)");

        ImGui::TableSetColumnIndex(1);

        if (m_iSelectedCell == -1)
        {
            ImGui::TextDisabled("No Selected");
        }
        else
        {
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::DragFloat("##HeightSelected", &m_fSelectedHeightOffset, 0.1f, -10.f, 10.f))
            {
                _float fOffset = m_fSelectedHeightOffset - m_fPrevSelectedHeightOffset;

                Apply_SelectedCell_Height(fOffset);

                m_fPrevSelectedHeightOffset = m_fSelectedHeightOffset;
            }

            if (!ImGui::IsItemActive())
            {
                m_fSelectedHeightOffset = 0.f;
                m_fPrevSelectedHeightOffset = 0.f;
            }
        }

        /* Slope X */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Slope X");

        ImGui::TableSetColumnIndex(1);

        if (m_iSelectedCell == -1)
        {
            ImGui::TextDisabled("No Selected");
        }
        else
        {
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::DragFloat("##SlopeX", &m_fSlopeX, 0.01f, -XM_PI, XM_PI))
            {
                _float fOffset = m_fSlopeX - m_fPrevSlopeX;

                Apply_SelectedCell_Slope(fOffset, true);

                m_fPrevSlopeX = m_fSlopeX;
            }
        }

        /* Slope Z */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Slope Z");

        ImGui::TableSetColumnIndex(1);

        if (m_iSelectedCell == -1)
        {
            ImGui::TextDisabled("No Selected");
        }
        else
        {
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (ImGui::DragFloat("##SlopeZ", &m_fSlopeZ, 0.01f, -XM_PI, XM_PI))
            {
                _float fOffset = m_fSlopeZ - m_fPrevSlopeZ;

                Apply_SelectedCell_Slope(fOffset, false);

                m_fPrevSlopeZ = m_fSlopeZ;
            }
        }

        ImGui::EndTable();
    }

    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Position()
{
    if (m_iSelectedCell != -1)
    {
        CCell* pCell = m_pNavCom->Get_Cell(m_iSelectedCell);
        if (pCell)
        {
            _float3 A, B, C;

            XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
            XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
            XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

            _bool bChanged = false;

            if (ImGui::DragFloat3("A", &A.x, 0.05f)) bChanged = true;
            if (ImGui::DragFloat3("B", &B.x, 0.05f)) bChanged = true;
            if (ImGui::DragFloat3("C", &C.x, 0.05f)) bChanged = true;

            if (bChanged)
            {
                pCell->Set_Point(CCell::POINT::A, XMLoadFloat3(&A));
                pCell->Set_Point(CCell::POINT::B, XMLoadFloat3(&B));
                pCell->Set_Point(CCell::POINT::C, XMLoadFloat3(&C));

                m_pNavCom->SetUp_Neighbors();
            }
        }
    }
    return S_OK;
}

HRESULT CInspector_NavMesh::Render_Position_Multi()
{
    static _float3 moveOffset = { 0.f, 0.f, 0.f };

    if (ImGui::DragFloat3("Offset", &moveOffset.x, 0.1f))
    {
        Move_SelectedCells(moveOffset);
        moveOffset = { 0.f, 0.f, 0.f };
    }

    return S_OK;
}

void CInspector_NavMesh::Handle_MouseInput()
{
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (ImGui::IsMouseClicked(0))
    {
        OutputDebugString(TEXT("Mouse Clicked\n"));

        _float3 hitPos;

        if (m_pGameInstance->Picking_PlaneY(0.f, hitPos))
        {
            /* Alt : Snap OFF / Ctrl : 0.1f*/
            ImGuiIO& io = ImGui::GetIO();

            _float snapSize = 1.f;
            if (io.KeyAlt)
                snapSize = 0.f;
            else if (io.KeyCtrl)
                snapSize = 1.5f;

            if (snapSize > 0.f)
                Snap_Translate(hitPos, _float3(snapSize, 0.f, snapSize));

            /* Height(y) */
            hitPos.y += m_fHeightOffset;

            m_vecTempPoints.push_back(hitPos);
        }

        if (m_vecTempPoints.size() == 3)
        {
            m_pNavCom->Add_Cell(m_vecTempPoints[0], m_vecTempPoints[1], m_vecTempPoints[2]);

            m_vecTempPoints.clear();
        }
            
    }
}

void CInspector_NavMesh::Picking_Cell()
{
    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::GetIO().WantCaptureMouse)
        return;

    _float minDist = FLT_MAX;
    m_iSelectedCell = -1;

    for (int i = 0; i < m_pNavCom->Get_CellCount(); ++i)
    {
        CCell* pCell = m_pNavCom->Get_Cell(i);
        if (!pCell)
            continue;

        _float3 A, B, C;

        XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
        XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
        XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

        _float fDist;
        if (m_pGameInstance->isPicked_InWorldSpace(&A, &B, &C, nullptr, &fDist))
        {
            if (fDist < minDist)
            {
                minDist = fDist;
                m_iSelectedCell = i;
                break;
            }

        }
    }

    /* Picking Multi Selected (Shift) */
    if (m_iSelectedCell != -1)
    {
        m_vecSelectedCells.clear();
        m_vecSelectedCells.push_back(m_iSelectedCell);
    }
}

void CInspector_NavMesh::Save_Binary()
{
    _wstring NavFileName = TEXT("NAV_LEVEL_") + to_wstring(m_iFileIndex_Save) + TEXT(".dat");
    _wstring NavNeighborFileName = TEXT("NAV_NEIGHBOR_LEVEL_") + to_wstring(m_iFileIndex_Save) + TEXT(".dat");
    _wstring BasePath = TEXT("../../Resources/Data/Navigation/");

    if (m_pNavCom)
    {
        m_pNavCom->Save(
            (BasePath + NavFileName).c_str(),
            (BasePath + NavNeighborFileName).c_str());
    }
}

void CInspector_NavMesh::Undo()
{
    /* Undo : Ctrl + Z*/
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z))
    {
        m_pNavCom->Undo();
    }
}

HRESULT CInspector_NavMesh::Init_NavFile()
{
    /* Index == -1 로 기본 생성 파일 */
    if (FAILED(MakeNavigationHolder()))
        return E_FAIL;

    Save_Binary();

    return S_OK;
}

void CInspector_NavMesh::Apply_SelectedCell_Height(_float _fOffset)
{
    if (!m_pNavCom || m_iSelectedCell == -1)
        return;

    auto* pCell = m_pNavCom->Get_Cell(m_iSelectedCell);
    if (!pCell)
        return;

    _float3 A, B, C;

    XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
    XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
    XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

    A.y += _fOffset;
    B.y += _fOffset;
    C.y += _fOffset;
    
    pCell->Set_Point(CCell::POINT::A, XMLoadFloat3(&A));
    pCell->Set_Point(CCell::POINT::B, XMLoadFloat3(&B));
    pCell->Set_Point(CCell::POINT::C, XMLoadFloat3(&C));

    /* Reconnet */
    m_pNavCom->SetUp_Neighbors();
}

void CInspector_NavMesh::Apply_SelectedCell_Slope(_float _fOffset, _bool _bActive)
{
    if (!m_pNavCom || m_iSelectedCell == -1)
        return;

    auto* pCell = m_pNavCom->Get_Cell(m_iSelectedCell);
    if (!pCell)
        return;

    _float3 A, B, C;
    XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
    XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
    XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

    _vector vA = XMLoadFloat3(&A);
    _vector vB = XMLoadFloat3(&B);
    _vector vC = XMLoadFloat3(&C);

    _vector center = (vA + vB + vC) / 3.f;

    _vector axis = _bActive ?
        XMVectorSet(1, 0, 0, 0) :
        XMVectorSet(0, 0, 1, 0);

    _matrix rot = XMMatrixRotationAxis(axis, _fOffset);

    vA = XMVector3TransformCoord(vA - center, rot) + center;
    vB = XMVector3TransformCoord(vB - center, rot) + center;
    vC = XMVector3TransformCoord(vC - center, rot) + center;

    pCell->Set_Point(CCell::POINT::A, vA);
    pCell->Set_Point(CCell::POINT::B, vB);
    pCell->Set_Point(CCell::POINT::C, vC);

    m_pNavCom->SetUp_Neighbors();
}

void CInspector_NavMesh::Move_SelectedCells(_float3 _fOffset)
{
    for (int index : m_vecSelectedCells)
    {
        CCell* pCell = m_pNavCom->Get_Cell(index);
        if (!pCell)
            continue;

        _float3 A, B, C;

        XMStoreFloat3(&A, pCell->Get_Point(CCell::POINT::A));
        XMStoreFloat3(&B, pCell->Get_Point(CCell::POINT::B));
        XMStoreFloat3(&C, pCell->Get_Point(CCell::POINT::C));

        A.x += _fOffset.x; A.y += _fOffset.y; A.z += _fOffset.z;
        B.x += _fOffset.x; B.y += _fOffset.y; B.z += _fOffset.z;
        C.x += _fOffset.x; C.y += _fOffset.y; C.z += _fOffset.z;

        pCell->Set_Point(CCell::POINT::A, XMLoadFloat3(&A));
        pCell->Set_Point(CCell::POINT::B, XMLoadFloat3(&B));
        pCell->Set_Point(CCell::POINT::C, XMLoadFloat3(&C));
    }

    m_pNavCom->SetUp_Neighbors();
}

HRESULT CInspector_NavMesh::Load_Resources()
{
    /* Load All XML TAGS */
    m_pGameInstance->Load_ResourceAndInstances(ENUM_TO_UINT(LEVEL::NAVIGATIONEDITOR), m_iFileIndex_Map);

    return S_OK;
}

HRESULT CInspector_NavMesh::MakeNavigationHolder()
{
    _wstring NavFileName = TEXT("NAV_LEVEL_") + to_wstring(m_iFileIndex_Load) + TEXT(".dat");
    _wstring NavNeighborFileName = TEXT("NAV_NEIGHBOR_LEVEL_") + to_wstring(m_iFileIndex_Load) + TEXT(".dat");
    _wstring BasePath = TEXT("../../Resources/Data/Navigation/");

    _wstring protoNavName = TEXT("Prototype_Component_Navigation_") + to_wstring(m_iFileIndex_Load);
    _wstring protoHolderName = TEXT("Prototype_GameObject_Map_NavHolder_") + to_wstring(m_iFileIndex_Load);

    if (m_pCurrentNavHolder)
    {
        m_bDeleteNavHolder = true;
        return S_OK;
    }

    /* Navigation */
#pragma region NAVIGATON
    /* For.Prototype_Component_Navigation */
    CNavigation* pNavigation = CNavigation::Create(
        m_pDevice, m_pDeviceContext,
        (BasePath + NavFileName).c_str(),
        (BasePath + NavNeighborFileName).c_str());

    HRESULT hr = m_pGameInstance->Add_Prototype(
        RESOURCE_LEVEL_STATIC,
        protoNavName.c_str(),
        pNavigation);

    if (FAILED(hr))
    {
        Safe_Release(pNavigation);
    }
#pragma endregion

    /* For.Prototype_GameObject_Map_NavHolder */
    if (FAILED(m_pGameInstance->Add_Prototype(
        RESOURCE_LEVEL_STATIC,
        protoHolderName.c_str(),
        CNavHolder::Create(m_pDevice, m_pDeviceContext))))
        return S_FALSE;

    CGameObject* pGameObject = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        RESOURCE_LEVEL_STATIC,
        protoHolderName.c_str(),
        ENUM_TO_UINT(LEVEL::NAVIGATIONEDITOR),
        TEXT("Layer_Map"),
        &protoNavName, &pGameObject)))
        return E_FAIL;

    m_pCurrentNavHolder = static_cast<CNavHolder*>(pGameObject);
    m_pNavCom = static_cast<CNavigation*>(m_pCurrentNavHolder->Get_Component(TEXT("Com_Navigation")));

     return S_OK;
}

_float3 CInspector_NavMesh::Snap_RoundVector3(_float3& v3, _float3 vStep)
{
    _float fRoundX = (vStep.x != 0.f) ? roundf(v3.x / vStep.x) * vStep.x : v3.x;
    _float fRoundY = (vStep.y != 0.f) ? roundf(v3.y / vStep.y) * vStep.y : v3.y;
    _float fRoundZ = (vStep.z != 0.f) ? roundf(v3.z / vStep.z) * vStep.z : v3.z;

    fRoundX = roundf(fRoundX * 1000.f) / 1000.f;
    fRoundY = roundf(fRoundY * 1000.f) / 1000.f;
    fRoundZ = roundf(fRoundZ * 1000.f) / 1000.f;

    return _float3(fRoundX, fRoundY, fRoundZ);
}

void CInspector_NavMesh::Snap_Translate(_float3& vPosition, _float3 vStep)
{
    vPosition = Snap_RoundVector3(vPosition, vStep);
}

CInspector_NavMesh* CInspector_NavMesh::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
    CInspector_NavMesh* pInstance = new CInspector_NavMesh(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CInspector_NavMesh");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CInspector_NavMesh::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}
