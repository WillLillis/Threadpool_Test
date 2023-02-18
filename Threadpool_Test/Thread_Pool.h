#if !defined(_THREAD_POOL_H_)
#define _THREAD_POOL_H_

#include <thread>
#include <queue>
#include <mutex>
#include <cstdint>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

class thread_pool {
public:
	typedef struct thread_pool_job_t {
		void (*func)(void*);
		void* args;
	}thread_pool_job_t;
	const uint_fast8_t max_threads;

	thread_pool() : max_threads(std::thread::hardware_concurrency())
	{
		job_avail = false;
		pool_cont = true;
	}

	thread_pool(uint_fast8_t num_threads) : max_threads(num_threads <= std::thread::hardware_concurrency()
		? num_threads : (uint_fast8_t)std::thread::hardware_concurrency())
	{
		job_avail = false;
		pool_cont = true;
	}

	void thread_pool_start()
	{
		for (uint_fast8_t i = 0; i < max_threads; i++) {
			worker_threads.push_back(std::thread(&(thread_pool::thread_start), this));
		}
	}

	// look into options for canceling threads
		// implementation-dependent, but at least there's windows and posix options to look into
	// if there are jobs in the queue that haven't been dispatched yet, this will cancel them
	void thread_pool_stop(bool allow_finish = false)
	{
		if (allow_finish) { // allow any jobs still in the queue to get executed
			while (true) {
				Sleep(0); // yield to other processes
				job_queue_lock.lock();
				if (job_queue.size() == 0) { // if the queue is empty we're done
					job_queue_lock.unlock();
					break;
				}
				job_queue_lock.unlock();
			}
		}

		pool_cont = false;
		for (uint_fast8_t i = 0; i < max_threads; i++) {
			if (worker_threads[i].joinable()) {
				worker_threads[i].join();
			}
		}
	}

	// add a job to the queue
	size_t add_job(thread_pool_job_t job)
	{
		job_queue_lock.lock();
		job_queue.push(job);
		size_t place = job_queue.size() - 1;
		job_avail = true;
		job_queue_lock.unlock();

		return place;
	}

private:
	std::vector<std::thread> worker_threads;
	std::queue<thread_pool_job_t> job_queue;
	std::mutex job_queue_lock;
	bool job_avail;
	std::mutex job_avail_lock;
	bool pool_cont;

	static void thread_start(thread_pool* ref)
	{
		while (ref->pool_cont)
		{
			Sleep(0); // yield to other processes
			if (!(ref->job_avail_lock.try_lock())) { // if we don't acquire the job notification lock
				continue;
			}
			if (ref->job_avail) { // if there's a job available to start
				ref->job_queue_lock.lock();
				thread_pool_job_t next_job = ref->job_queue.front(); // get the next job
				ref->job_queue.pop();
				ref->job_avail = ref->job_queue.size() > 0 ? true : false; // update the job ready flag
				ref->job_avail_lock.unlock();
				ref->job_queue_lock.unlock();
				next_job.func(next_job.args); // call the function from the job
			}
			else { // if there's no jobs
				ref->job_avail_lock.unlock();
			}
		}
	}

};


#endif // _THREAD_POOL_H_ include guard