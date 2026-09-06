#include "TasksSystem.h"
#include <process.h>

#ifdef _DEBUG
namespace 
{
    struct TASK_TEST_CONTEXT
    {
        /* Task 0 -> outputs[0], threadIDs[0] */
        _uint iInput = { 0 };                   // 입력값
        _uint* pOutput = { nullptr };           // 자신만의 결과 슬롯 주소
        DWORD* pWorkerThreadID = { nullptr };   // 자신만의 thread ID 기록 슬롯 주소
    };

    HRESULT Execute_IndependentTaskTest(void* _pContext)
    {
        if (!_pContext) return E_INVALIDARG;
        
        TASK_TEST_CONTEXT* pContext = static_cast<TASK_TEST_CONTEXT*>(_pContext);
        if (!pContext->pOutput || !pContext->pWorkerThreadID) return E_INVALIDARG;

        Sleep(10);

        *pContext->pOutput = pContext->iInput * pContext->iInput;
        *pContext->pWorkerThreadID = GetCurrentThreadId();

        return S_OK;
    }
}
#endif // _DEBUG

CTasksSystem::CTasksSystem()
{
}

HRESULT CTasksSystem::Initialize(_uint _iWorkerCount)
{
    if (m_isInitialized) return E_FAIL;
    if (0 == _iWorkerCount || (MAX_WORKER_COUNT < _iWorkerCount))
        return E_INVALIDARG;
    if (FALSE == InitializeCriticalSectionEx(&m_QueueCriticalSection, 0, 0))
        return HRESULT_FROM_WIN32(GetLastError());

    m_iWorkerCount = _iWorkerCount;
    m_iReadIndex = 0;
    m_iWriteIndex = 0;
    m_iQueuedTaskCount = 0;
    m_iActiveTaskCount = 0;
    
    m_hrFirstFailure = S_OK;

    m_isStopping = false;
    m_isInitialized = true;

    for (_uint i = 0; i < m_iWorkerCount; ++i)
    {
        m_hWorkers[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Thread_Main, this, 0, nullptr));
        if (m_hWorkers[i])
            continue;

        /* 일부 Workder 생성에 실패했다. 먼저 생성된 Worker 를 깨워 종료시킨다. */
        EnterCriticalSection(&m_QueueCriticalSection);
        m_isStopping = true;
        WakeAllConditionVariable(&m_WorkCondition);
        LeaveCriticalSection(&m_QueueCriticalSection);

        for (_uint j = 0; j < i; ++j)
        {
            if (!m_hWorkers[j])
                continue;

            WaitForSingleObject(m_hWorkers[j], INFINITE);
            CloseHandle(m_hWorkers[j]);
            m_hWorkers[j] = nullptr;
        }

        DeleteCriticalSection(&m_QueueCriticalSection);

        m_iWorkerCount = 0;
        m_isStopping = false;
        m_isInitialized = false;
        
        return E_FAIL;
    }

    return S_OK;
}

/* '생산자 작업 등록'
* 1. 함수 포인터 유효성 확인
* 2. 임계 영역 진입
* 3. 종료 상태 확인
* 4. 버퍼 포화 확인
* 5. WriteIndex 위치에 task 저장
* 6. WriteIndex 이동
* 7. QueuedCount 증가
* 8. Worker 하나 깨움
* 9. 임계영역 해제 */
HRESULT CTasksSystem::Submit(TASK_DESC& _tTaskDesc)
{
    if (false == m_isInitialized) return E_FAIL;
    if (!_tTaskDesc.pFunction)
        return E_INVALIDARG;

    EnterCriticalSection(&m_QueueCriticalSection);

    if (m_isStopping)
    {
        LeaveCriticalSection(&m_QueueCriticalSection);
        return E_FAIL;
    }

    if (TASK_QUEUE_CAPACITY == m_iQueuedTaskCount)
    {
        LeaveCriticalSection(&m_QueueCriticalSection);
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    m_tTasks[m_iWriteIndex] = _tTaskDesc;
    m_iWriteIndex = (m_iWriteIndex + 1) % TASK_QUEUE_CAPACITY;

    ++m_iQueuedTaskCount;

    /* 등록된 작업을 처리할 소비자 worker 하나를 깨운다 */
    WakeConditionVariable(&m_WorkCondition);
    LeaveCriticalSection(&m_QueueCriticalSection);

    return S_OK;
}

/* '전체 작업 완료 대기' 
이 함수는 worker task 내부에서 호출하지 않는다. */
HRESULT CTasksSystem::Wait_All()
{
    if (false == m_isInitialized) return E_FAIL;

    EnterCriticalSection(&m_QueueCriticalSection);

    while (0 < m_iQueuedTaskCount || 0 < m_iActiveTaskCount)
    {
        SleepConditionVariableCS(&m_IdleCondition, &m_QueueCriticalSection, INFINITE);
    }

    const HRESULT hrResult = m_hrFirstFailure;
    m_hrFirstFailure = S_OK;

    LeaveCriticalSection(&m_QueueCriticalSection);

    return hrResult;
}

/* https://learn.microsoft.com/en-us/windows/win32/procthread/thread-pooling */
/* Worker 는 매 작업마다 만들지 않는다.
* 프로그램 / 시스템 초기화 시 생성 -> 여러 task 에 재사용 -> 종료 시 한 번 회수 */
/* APIENTRY : __stdcall 호출 규약 */
_uint APIENTRY CTasksSystem::Thread_Main(void* _pArg)
{
    if (!_pArg) return 1;

    CTasksSystem* pTaskSystem = static_cast<CTasksSystem*>(_pArg);
    pTaskSystem->Worker_Loop();

    return 0;
}

/* '소비자 대기 및 실행'
* 1. 임계영역 진입
* 2. 큐가 비었고 종료 요청도 없으면 조건 변수 대기
* 3. 큐에서 task 하나 꺼냄
* 4. QueuedCount 감소
* 5. ActiveCount 증가
* 6. 임계영역 해제
* 7. task 실행
* 8. 임계영역 재진입
* 9. ActiveCount 감소
* 10. 전체 완료 조건이면 대기 thread 깨움
* 11. 임계영역 해제 */
void CTasksSystem::Worker_Loop()
{
    while (true)
    {
        TASK_DESC tTaskDesc = {};
        
        EnterCriticalSection(&m_QueueCriticalSection);

        /* 유한 버퍼가 비어있고 종료 요청도 없다면, CPU 를 반복 사용하지 않고 조건 변수에서 대기한다 */
        while (0 == m_iQueuedTaskCount && false == m_isStopping)
        {
            SleepConditionVariableCS(&m_WorkCondition, &m_QueueCriticalSection, INFINITE);
        }

        /* 종료 요청 이후에도 이미 생산된 작업은 처리한다. 버퍼가 비었을 때 worker loop 를 종료한다. */
        if (m_isStopping && (0 == m_iQueuedTaskCount))
        {
            LeaveCriticalSection(&m_QueueCriticalSection);
            break;
        }

        tTaskDesc = m_tTasks[m_iReadIndex];
        m_iReadIndex = (m_iReadIndex + 1) % TASK_QUEUE_CAPACITY;

        --m_iQueuedTaskCount;
        ++m_iActiveTaskCount;

        LeaveCriticalSection(&m_QueueCriticalSection);

        /* 실제 task 는 임계영역 밖에서 실행한다. 이를 통해 여러 소비자 worker 가 병렬 실행된다. */
        HRESULT hrTask = E_FAIL;

        if (tTaskDesc.pFunction)
        {
            hrTask = tTaskDesc.pFunction(tTaskDesc.pContext);
        }

        EnterCriticalSection(&m_QueueCriticalSection);

        if (FAILED(hrTask) && SUCCEEDED(m_hrFirstFailure))
        {
            m_hrFirstFailure = hrTask;
        }

        --m_iActiveTaskCount;

        if ((0 == m_iQueuedTaskCount) && (0 == m_iActiveTaskCount))
        {
            WakeAllConditionVariable(&m_IdleCondition);
        }

        LeaveCriticalSection(&m_QueueCriticalSection);
    }
}

/* '종료' 'drain shutdown' 
* 1. 종료 상태 설정
* 2. 신규 task 등록 차단
* 3. 잠든 모든 worker 깨움
* 4. 이미 등록된 task 는 계속 처리
* 5. 큐가 비면 worker 종료
* 6. WaitForSingleOjbect 로 종료 대기
* 7. thread handle 닫기
* 8. 임계영역 제거 */
void CTasksSystem::Shutdown()
{
    if (false == m_isInitialized) return;

    EnterCriticalSection(&m_QueueCriticalSection);

    m_isStopping = true;

    WakeAllConditionVariable(&m_WorkCondition);
    LeaveCriticalSection(&m_QueueCriticalSection);

    for (_uint i = 0; i < m_iWorkerCount; ++i)
    {
        if (!m_hWorkers[i])
            continue;

        WaitForSingleObject(m_hWorkers[i], INFINITE);
        CloseHandle(m_hWorkers[i]);
        m_hWorkers[i] = nullptr;
    }

    DeleteCriticalSection(&m_QueueCriticalSection);

    m_iWorkerCount = 0;

    m_iReadIndex = 0;
    m_iWriteIndex = 0;
    m_iQueuedTaskCount = 0;
    m_iActiveTaskCount = 0;
    
    m_hrFirstFailure = S_OK;

    m_isStopping = false;
    m_isInitialized = false;
}

#ifdef _DEBUG
HRESULT CTasksSystem::Debug_RunIndependentTaskTest()
{
    static constexpr _uint TEST_TASK_COUNT = 8;

    TASK_TEST_CONTEXT tContexts[TEST_TASK_COUNT] = {};
    TASK_DESC tTaskDesc[TEST_TASK_COUNT] = {};

    _uint outputs[TEST_TASK_COUNT] = {};
    DWORD workerThreadIDs[TEST_TASK_COUNT] = {};

    for (_uint i = 0; i < TEST_TASK_COUNT; ++i)
    {
        tContexts[i].iInput = i + 1;
        tContexts[i].pOutput = &outputs[i];
        tContexts[i].pWorkerThreadID = &workerThreadIDs[i];

        tTaskDesc[i].pFunction = Execute_IndependentTaskTest;
        tTaskDesc[i].pContext = &tContexts[i];

        const HRESULT hrSubmit = Submit(tTaskDesc[i]);
        if (FAILED(hrSubmit))
            return hrSubmit;
    }

    const HRESULT hrWait = Wait_All();
    if (FAILED(hrWait))
        return hrWait;

    for (_uint i = 0; i < TEST_TASK_COUNT; ++i)
    {
        const _uint iExpected = (i + 1) * (i + 1);
        if (iExpected != outputs[i])
            return E_FAIL;
    }

    for (_uint i = 0; i < TEST_TASK_COUNT; ++i)
    {
        wchar_t szLog[128] = {};
        swprintf_s(szLog, _countof(szLog),
            L"[TasksSystem Test] " L"Task=%u, Input=%u, Output=%u," L"WorkerThreadID=%lu\n",
            i, tContexts[i].iInput, outputs[i], workerThreadIDs[i]);
        OutputDebugStringW(szLog);
    }

    OutputDebugStringW(L"[TasksSystem Test] " L"All independent tasks completed.\n");

    return S_OK;
}
#endif // _DEBUG

CTasksSystem* CTasksSystem::Create(_uint _iWorkerCount)
{
    CTasksSystem* pInstance = new CTasksSystem();
    if (FAILED(pInstance->Initialize(_iWorkerCount)))
    {
        MSG_BOX("FAILED TO CREATED : CTaskSystem");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTasksSystem::Free()
{
    Shutdown();

    __super::Free();
}
