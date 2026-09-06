#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CTasksSystem final : public CBase
{
/* https://learn.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-queueuserworkitem */

public:
	/* pContext 는 작업이 완료될 때까지 유효해야 한다. */
	struct TASK_DESC
	{
		/* 함수반환형 / (함수포인터) / (함수매개변수) */
		HRESULT(*pFunction)(void*) = {nullptr};
		void* pContext = { nullptr };
	};

private:
	CTasksSystem();
	virtual ~CTasksSystem() = default;

public:
	HRESULT Initialize(_uint _iWorkerCount);
	HRESULT Submit(TASK_DESC& _tTaskDesc);
	HRESULT Wait_All();

private:
	static _uint APIENTRY Thread_Main(void* _pArg);
	void Worker_Loop();
	void Shutdown();

#ifdef _DEBUG
	HRESULT Debug_RunIndependentTaskTest();
#endif // 


private:
	static constexpr _uint MAX_WORKER_COUNT = 4;
	static constexpr _uint TASK_QUEUE_CAPACITY = 256; // 고정 용량의 장점

/* - 공유 상태 */
private:
	HANDLE m_hWorkers[MAX_WORKER_COUNT] = {};
	_uint m_iWorkerCount = 0;

	TASK_DESC m_tTasks[TASK_QUEUE_CAPACITY] = {};
	_uint m_iReadIndex = 0;	// 원형 버퍼 인덱스 (소비자가 다음 task 를 가져갈 위치)
	_uint m_iWriteIndex = 0; // 원형 버퍼 인덱스 (생산자가 다음 task 를 넣을 위치)
	_uint m_iQueuedTaskCount = 0; // 원형 버퍼 인덱스 (현재 버퍼 안에 있는 task 수)
	_uint m_iActiveTaskCount = 0; // Worker 가 꺼내 실행 중인 작업

	/* - 상호 배제 */
	CRITICAL_SECTION m_QueueCriticalSection = {}; // 생상자와 소비자가 공유하는 큐 상태를 보호한다.
	
	/* - 조건 대기 */
	CONDITION_VARIABLE m_WorkCondition = CONDITION_VARIABLE_INIT;
	CONDITION_VARIABLE m_IdleCondition = CONDITION_VARIABLE_INIT;

	/* Result */
	HRESULT m_hrFirstFailure = { S_OK };

	/* LifeTime */
	_bool m_isStopping = { false };
	_bool m_isInitialized = { false };

public:
	static CTasksSystem* Create(_uint _iWorkerCount);
	virtual void Free() override;
};
NS_END