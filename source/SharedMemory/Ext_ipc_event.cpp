//#include "StdH.h"
#include "Ext_ipc_event.h"
#ifndef _WIN32
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#endif

#ifdef _WIN32

XSemaphore::XSemaphore(unsigned int initial_count, unsigned int max_count, const char *name, bool create_new)
: sem_(0), owned(create_new), name_(name)
{
	if (create_new)
		sem_ = CreateEvent(NULL, FALSE, TRUE, name);
	else
		sem_ = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
}

XSemaphore::~XSemaphore()
{
	if ((sem_ != INVALID_HANDLE_VALUE) && (sem_ != 0))
	{
		CloseHandle(sem_);
	}
}

int XSemaphore::P()
{
	DWORD ret = WaitForSingleObject(sem_, 30*1000);

	switch(ret)
	{
	case WAIT_OBJECT_0:
		return 0;
	default:
		// dirty code, set the event if timeout
		SetEvent(sem_);
		return -1;
	}
}

int XSemaphore::V()
{
	return SetEvent(sem_) ? 0 : -1;
}

void SleepInMilliseconds(int msec)
{
	Sleep(msec);
}

#else

namespace {
struct LocalEvent {
	std::mutex mtx;
	std::condition_variable cv;
	bool signaled;
	int refs;
};

std::mutex& EventMapMutex()
{
	static std::mutex g_mapMutex;
	return g_mapMutex;
}

std::unordered_map<std::string, std::shared_ptr<LocalEvent>>& EventMap()
{
	static std::unordered_map<std::string, std::shared_ptr<LocalEvent>> g_events;
	return g_events;
}
} // namespace

struct XSemaphore::XSemaphoreLocal
{
	std::string key;
	std::shared_ptr<LocalEvent> event;
};

XSemaphore::XSemaphore(unsigned int initial_count, unsigned int /*max_count*/, const char *name, bool create_new)
: sem_(0), owned(create_new), name_(name)
{
	if (name_ == 0)
		return;

	auto* local = new XSemaphoreLocal();
	local->key = name_;

	std::lock_guard<std::mutex> lock(EventMapMutex());
	auto& events = EventMap();
	auto it = events.find(local->key);
	if (it == events.end())
	{
		auto created = std::make_shared<LocalEvent>();
		created->signaled = (initial_count > 0);
		created->refs = 1;
		events.emplace(local->key, created);
		local->event = created;
	}
	else
	{
		it->second->refs += 1;
		local->event = it->second;
	}

	sem_ = local;
}

XSemaphore::~XSemaphore()
{
	if (sem_ == 0)
		return;

	std::string key = sem_->key;
	{
		std::lock_guard<std::mutex> lock(EventMapMutex());
		auto& events = EventMap();
		auto it = events.find(key);
		if (it != events.end())
		{
			it->second->refs -= 1;
			if (it->second->refs <= 0)
				events.erase(it);
		}
	}

	delete sem_;
	sem_ = 0;
}

int XSemaphore::P()
{
	if (sem_ == 0 || !sem_->event)
		return -1;

	std::unique_lock<std::mutex> lock(sem_->event->mtx);
	const auto timeout = std::chrono::seconds(30);
	const bool ok = sem_->event->cv.wait_for(lock, timeout, [&]() { return sem_->event->signaled; });
	if (!ok)
		return -1;

	sem_->event->signaled = false;
	return 0;
}

int XSemaphore::V()
{
	if (sem_ == 0 || !sem_->event)
		return -1;

	{
		std::lock_guard<std::mutex> lock(sem_->event->mtx);
		sem_->event->signaled = true;
	}
	sem_->event->cv.notify_one();
	return 0;
}

void SleepInMilliseconds(int msec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}
#endif
