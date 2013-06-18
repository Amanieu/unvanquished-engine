//@@COPYRIGHT@@

// Various synchronization primitives

#ifdef __SSE__
#include <xmmintrin.h>
#endif

namespace thread {

// Pause for use in spinloops. On hyperthreaded CPUs, this yields to the other
// hardware thread. For all other cases, it is simply a no-op.
inline void spin_pause()
{
#ifdef __SSE__
	_mm_pause();
#endif
}

// Spinlock with same interface as std::mutex
class spinlock: boost::noncopyable {
public:
	void lock()
	{
		while (!try_lock()) {
			// If direct locking fails then spin using load() only
			// before trying again. This saves bus traffic since the
			// spinlock is already in the cache.
			while (locked.load(std::memory_order_relaxed))
				spin_pause();
		}
	}

	bool try_lock()
	{
		bool expected = false;
		return locked.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed);
	}

	void unlock()
	{
		locked.store(false, std::memory_order_release);
	}

private:
	std::atomic<bool> locked{false};
};

// Semaphore wrapper which avoids system calls when it can
class semaphore: boost::noncopyable {
public:
	explicit semaphore(int initial = 0)
		: sem_count{initial}
	{
#ifdef _WIN32
		sem_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
#else
		sem_init(&sem_handle, 0, 0);
#endif
	}

	~semaphore()
	{
#ifdef _WIN32
		CloseHandle(sem_handle);
#else
		sem_destroy(&sem_handle);
#endif
	}

	void wait()
	{
		if (sem_count.fetch_sub(1, std::memory_order_relaxed) > 0) {
			std::atomic_thread_fence(std::memory_order_acquire);
			return;
		}

#ifdef _WIN32
		WaitForSingleObject(sem_handle, INFINITE);
#else
		while (sem_wait(&sem_handle) != 0) {}
#endif
	}

	bool try_wait()
	{
		int my_sem_count = sem_count.load(std::memory_order_relaxed);

		do {
			if (my_sem_count <= 0)
				return false;
		} while (sem_count.compare_exchange_weak(my_sem_count, my_sem_count - 1, std::memory_order_acquire, std::memory_order_relaxed));

		return true;
	}

	void post(int count = 1)
	{
		int my_sem_count = sem_count.fetch_add(count, std::memory_order_relaxed);
		if (my_sem_count >= 0) {
			std::atomic_thread_fence(std::memory_order_release);
			return;
		}

#ifdef _WIN32
		ReleaseSemaphore(sem_handle, std::min(count, -my_sem_count), NULL);
#else
		for (int i = 0; i < std::min(count, -my_sem_count); i++)
			sem_post(&sem_handle);
#endif
	}

private:
	std::atomic<int> sem_count;
#ifdef _WIN32
	HANDLE sem_handle;
#else
	sem_t sem_handle;
#endif
};

}
