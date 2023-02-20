#include "Thread_Pool.h"
#include <cstdio>
#include <iostream>

void test_func(void* num)
{
	for (int i = 0; i < 100000; i++) {
		printf("Your number is: %d", *(int*)num);
	}
	
	fflush(stdin);
}


int main()
{
	srand(time(NULL));
	thread_pool test_pool(12);

	test_pool.thread_pool_start();

	/*int x = 1;
	thread_pool::thread_pool_job_t job1 = { .func = test_func, .args = &x };

	int y = 5;
	thread_pool::thread_pool_job_t job2 = { .func = test_func, .args = &y };

	test_pool.add_job(job1);
	test_pool.add_job(job2);*/

	thread_pool::thread_pool_job_t next_job;
	next_job.func = test_func;
	int nums[12];

	for (uint32_t i = 0; i < 12; i++) {
		nums[i] = rand() % 100;
		next_job.args = &nums[i];
		test_pool.add_job(next_job);
	}


	std::string empty;

	std::cin >> empty;

	test_pool.thread_pool_stop(true);

	return 0;
}