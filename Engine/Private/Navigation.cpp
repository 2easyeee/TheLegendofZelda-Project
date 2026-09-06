#include "Navigation.h"
#include "GameInstance.h"
#include "Cell.h"

#ifdef _DEBUG
#include "Shader.h"
#endif // _DEBUG


CNavigation::CNavigation(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CComponent { _pDevice, _pDeviceContext }
{
}

CNavigation::CNavigation(const CNavigation& _Prototype)
    : CComponent (_Prototype)
    , m_Cells { _Prototype.m_Cells }
#ifdef _DEBUG
    , m_pShader { _Prototype.m_pShader }
#endif // _DEBUG

{
    for (auto& pCell : m_Cells)
        Safe_AddRef(pCell);

#ifdef _DEBUG
    Safe_AddRef(m_pShader);
#endif // _DEBUG

}

HRESULT CNavigation::Initialize_Prototype(const _tchar* _pNavigationDataFiles)
{
    for (auto& pCell : m_Cells)
        Safe_Release(pCell);
    m_Cells.clear();

    _ulong dwByte = {};
    HANDLE hFile = CreateFile(_pNavigationDataFiles, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    while (true)
    {
        _float3 vPoints[3] = {};
        
        ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);
        if (0 == dwByte)
            break;

        CCell* pCell = CCell::Create(m_pDevice, m_pDeviceContext, vPoints, m_Cells.size());
        if (nullptr == pCell)
            return E_FAIL;

        m_Cells.push_back(pCell);
    }
    CloseHandle(hFile);

    if (FAILED(SetUp_Neighbors()))
        return E_FAIL;

#ifdef _DEBUG
    m_pShader = CShader::Create(
        m_pDevice, m_pDeviceContext,
        TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"),
        VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif // _DEBUG

    return S_OK;
}

HRESULT CNavigation::Initialize_Prototype(const _tchar* _pNavigationDataFiles, const _tchar* _pCellNeighbors)
{
    for (auto& pCell : m_Cells)
        Safe_Release(pCell);
    m_Cells.clear();

    _ulong dwByte = {};
    HANDLE hFile = CreateFile(_pNavigationDataFiles, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    while (true)
    {
        _float3 vPoints[3] = {};

        ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);
        if (0 == dwByte)
            break;

        CCell* pCell = CCell::Create(m_pDevice, m_pDeviceContext, vPoints, m_Cells.size());
        if (nullptr == pCell)
            return E_FAIL;

        m_Cells.push_back(pCell);
    }
    CloseHandle(hFile);

    if (FAILED(SetUp_Neighbors(_pCellNeighbors)))
        return E_FAIL;

#ifdef _DEBUG
    m_pShader = CShader::Create(
        m_pDevice, m_pDeviceContext,
        TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"),
        VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif // _DEBUG

    return S_OK;
}

HRESULT CNavigation::Initialize(void* _pArg)
{
    NAVIGATION_DESC* pDesc = static_cast<NAVIGATION_DESC*>(_pArg);

    if (m_Cells.empty())
        m_iCurrentCellIndex = -1;
    else
        m_iCurrentCellIndex = pDesc->iCurrentCellIndex;

    return S_OK;
}

_bool CNavigation::IsMove(_fvector _vPosition)
{
    if (m_Cells.empty())
        return true;
    if (m_iCurrentCellIndex < 0 || m_iCurrentCellIndex >= m_Cells.size())
        return false;

    _int iNeighborIndex = { -1 };
    if (true == m_Cells[m_iCurrentCellIndex]->IsIn(_vPosition, &iNeighborIndex))
    {
        return true;
    }
    else
    {
        if (-1 == iNeighborIndex)
        {
            /* 나간 방향에 이웃이 없다면 */
            return false;
        }
        else
        {
            /* 나간 방향에 이웃이 있다면 */
            while (true)
            {
                if (-1 == iNeighborIndex)
                    return false;

                if (true == m_Cells[iNeighborIndex]->IsIn(_vPosition, &iNeighborIndex))
                    break;
            }

            m_iCurrentCellIndex = iNeighborIndex;
            return true;
        }
    }
}

_vector CNavigation::SetUp_OnNavigation(_fvector _vWorldPos)
{
    if (m_Cells.empty())
        return _vWorldPos;
    if (m_iCurrentCellIndex < 0 || m_iCurrentCellIndex >= m_Cells.size())
        return _vWorldPos;

    _vector vPositon = _vWorldPos;
    vPositon = XMVectorSetY(vPositon,
        m_Cells[m_iCurrentCellIndex]->Compute_Height(vPositon));

    return vPositon;
}

void CNavigation::Add_Cell(_float3 _v0, _float3 _v1, _float3 _v2)
{
    /* CW 정렬 */
    _vector A = XMLoadFloat3(&_v0);
    _vector B = XMLoadFloat3(&_v1);
    _vector C = XMLoadFloat3(&_v2);

    _vector AB = B - A;
    _vector AC = C - A;

    _vector cross = XMVector3Cross(AB, AC);

    /* Excep. 면적 너무 작으면 */
    _float areaSq = XMVectorGetX(XMVector3LengthSq(cross));
    if (areaSq < 0.0001f)
    {
        OutputDebugString(L"Degenerate triangle blocked.\n");
        wchar_t buffer[256];
        swprintf_s(buffer,
            L"A(%.3f %.3f %.3f) B(%.3f %.3f %.3f) C(%.3f %.3f %.3f)\n",
            _v0.x, _v0.y, _v0.z,
            _v1.x, _v1.y, _v1.z,
            _v2.x, _v2.y, _v2.z);

        OutputDebugString(buffer);
        return;
    }

    if (XMVectorGetY(cross) < 0.f)
        swap(_v1, _v2);

    /* Add Cell */
    _float3 points[3] = { _v0, _v1, _v2 };

    CCell* pCell = CCell::Create(m_pDevice, m_pDeviceContext, points, m_Cells.size());

    m_Cells.push_back(pCell);

    SetUp_Neighbors();
}

void CNavigation::Remove_Cell(_int _iIndex)
{
    if (_iIndex < 0 || _iIndex >= m_Cells.size())
        return;

    Safe_Release(m_Cells[_iIndex]);
    m_Cells.erase(m_Cells.begin() + _iIndex);

    /* Index 재정렬 */
    for (_int i = 0; i < m_Cells.size(); ++i)
    {
        m_Cells[i]->Set_Index(i);
    }

    /* 이웃 */
    SetUp_Neighbors();
}

CCell* CNavigation::Get_Cell(_int _iIndex)
{
    if (_iIndex < 0 || _iIndex >= m_Cells.size())
        return nullptr;

    return m_Cells[_iIndex];
}

_int CNavigation::Get_CellCount() const
{
    return static_cast<_int>(m_Cells.size());
}

void CNavigation::Set_CurrentCell(_int _iIndex)
{
    m_iCurrentCellIndex = _iIndex;
}

_int CNavigation::Find_CurrentCell(_fvector _vPosition)
{
    for (_int i = 0; i < m_Cells.size(); ++i)
    {
        _int dummy = -1;
        if (m_Cells[i]->IsIn(_vPosition, &dummy))
            return i;
    }
    return -1;
}

void CNavigation::Undo()
{
    if (m_Cells.empty())
        return;

    Remove_Cell(static_cast<_int>(m_Cells.size() - 1));
}

HRESULT CNavigation::Save(const _tchar* _pCellFile, const _tchar* _pNeighborFile)
{
    /* Cell */
    HANDLE hCellFile = CreateFile(_pCellFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hCellFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    DWORD dwByte = 0;
    for (auto& pCell : m_Cells)
    {
        if (!pCell)
            continue;

        _float3 points[3];

        XMStoreFloat3(&points[0], pCell->Get_Point(CCell::POINT::A));
        XMStoreFloat3(&points[1], pCell->Get_Point(CCell::POINT::B));
        XMStoreFloat3(&points[2], pCell->Get_Point(CCell::POINT::C));

        WriteFile(hCellFile, points, sizeof(points), &dwByte, nullptr);
    }
    CloseHandle(hCellFile);

    /* Neighbor */
    HANDLE hNeighborFile = CreateFile(_pNeighborFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hNeighborFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    for (auto& pCell : m_Cells)
    {
        if (!pCell)
            continue;

        WriteFile(hNeighborFile,
            pCell->Get_NeighborIndices(),
            sizeof(_int) * 3,
            &dwByte,
            nullptr);
    }

    CloseHandle(hNeighborFile);

    MSG_BOX("Navigation Saved!");

    return S_OK;
}

void CNavigation::Set_SelctedCells(vector<_int> _vecCells)
{
    m_vecSelectedCells = _vecCells;
}

HRESULT CNavigation::SetUp_Neighbors()
{
    for (auto& pCell : m_Cells)
    {
        _int empty[3] = { -1, -1, -1 };
        pCell->Set_Neighbor(empty);
    }

    for (size_t i = 0; i < m_Cells.size(); ++i)
    {
        for (size_t j = i + 1; j < m_Cells.size(); ++j) {
            CCell* pA = m_Cells[i];
            CCell* pB = m_Cells[j];

            if (pA->Get_SharedPointCount(pB) == 2)
            {
                pA->Link_Neighbor(pB);
                pB->Link_Neighbor(pA);
            }


            int shared = pA->Get_SharedPointCount(pB);

            if (shared > 0)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, L"Cell %d & %d shared: %d\n", i, j, shared);
                OutputDebugString(buffer);
            }

            for (int l = 0; l < 3; ++l)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, L"Cell %d Line %d -> Neighbor %d\n",
                    i, l, pA->Get_NeighborIndices()[l]);
                OutputDebugString(buffer);
            }


        }
    }

    return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors(const _tchar* _pCellNeighbors)
{
    _ulong dwByte = {};
    HANDLE hFile = CreateFile(_pCellNeighbors, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    for (auto& pCell : m_Cells)
    {
        _int iNeighbors[3] = {};
        ReadFile(hFile, iNeighbors, sizeof(_int) * 3, &dwByte, nullptr);
        pCell->Set_Neighbor(iNeighbors);
    }
    CloseHandle(hFile);

    return S_OK;
}

CNavigation* CNavigation::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pNavigationDataFiles)
{
    CNavigation* pInstance = new CNavigation(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_pNavigationDataFiles)))
    {
        MSG_BOX("FAILED TO CREATED : CNavigation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CNavigation* CNavigation::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pNavigationDataFiles, const _tchar* _pCellNeighbors)
{
    CNavigation* pInstance = new CNavigation(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_pNavigationDataFiles, _pCellNeighbors)))
    {
        MSG_BOX("FAILED TO CREATED : CNavigation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CNavigation::Clone(void* _pArg)
{
    CNavigation* pInstance = new CNavigation(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("FAILED TO CLONED : CNavigation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNavigation::Free()
{
    __super::Free();

    for (auto& pCell : m_Cells)
        Safe_Release(pCell);
    m_Cells.clear();

#ifdef _DEBUG
    Safe_Release(m_pShader);
#endif // _DEBUG

}

#ifdef _DEBUG
HRESULT CNavigation::Render()
{
    _float4x4 WorldMatrix = {};
    XMStoreFloat4x4(&WorldMatrix, XMMatrixTranslation(0.f, 0.5f, 0.f)); // 살짝 띄우기

    if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform(D3DTS::PROJ))))
        return E_FAIL;

    _float4 vColor = {};
    for (_int i = 0; i < m_Cells.size(); ++i)
    {
        CCell* pCell = m_Cells[i];
        if (!pCell)
            continue;

        _bool bSelected =
            find(m_vecSelectedCells.begin(), m_vecSelectedCells.end(), i) != m_vecSelectedCells.end();

        _float4 vColor;

        if (bSelected)
        {
            vColor = _float4(1.f, 0.f, 0.f, 1.f);
            if (FAILED(m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
                return E_FAIL;
            m_pShader->Begin(1);
            pCell->Render(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 3);

            continue;
        }

        _int iNeighborCount = 0;
        for (_int j = 0; j < 3; ++j)
        {
            if (pCell->Get_NeighborIndices()[j] != -1)
                iNeighborCount++;
        }

        if (iNeighborCount == 3)
            vColor = _float4(0.f, 1.f, 0.f, 1.f);      // 굿 (초록)
        else if (iNeighborCount == 2)
            vColor = _float4(1.f, 1.f, 0.f, 1.f);      // 경계 (노랑)
        else
            vColor = _float4(1.f, 0.f, 1.f, 1.f);      // 이상 (보라)

        if (FAILED(m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4))))
            return E_FAIL;
        m_pShader->Begin(0);
        pCell->Render(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 4);
    }
    return S_OK;
}
#endif // _DEBUG