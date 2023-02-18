#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <cstdint>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

// TODO:
	// transition over to using std::condition_variables to manage idle threads/ notify when a job is available
	// transition over to using templates so we can avoid using void pointers to pass in function arguments
		// get rid of thread_pool_job_t
	// replace bools and mutexes with std::atomic's


class thread_pool {
public:
	typedef struct thread_pool_job_t {
		void (*func)(void*);
		void* args;
	}thread_pool_job_t;
	const uint_fast8_t max_threads;

	thread_pool() : max_threads(1)
	{
		pool_cont = true;
	}

	thread_pool(uint_fast8_t num_threads) : max_threads(num_threads)
	{
		pool_cont = true;
	}

	void thread_pool_start()
	{
		for (uint32_t i = 0; i < max_threads; i++) {
			worker_threads.push_back(std::thread(&thread_pool::thread_start, this));
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
		for (uint32_t i = 0; i < max_threads; i++) {
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
		job_queue_lock.unlock();

		return place;
	}

private:
	std::vector<std::thread> worker_threads;
	std::queue<thread_pool_job_t> job_queue;
	std::mutex job_queue_lock;
	bool pool_cont;

	void thread_start()
	{
		while (pool_cont)
		{
			if (!(job_queue_lock.try_lock())) { // if we don't acquire the job notification lock
				continue;
			}
			if (job_queue.size() > 0) { // if there's a job available to start
				thread_pool_job_t next_job = job_queue.front(); // get the next job
				job_queue.pop();
				job_queue_lock.unlock();
				next_job.func(next_job.args); // call the function from the job
			} else {
				job_queue_lock.unlock();
			}
		}
	}

};