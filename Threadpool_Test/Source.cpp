#include "Thread_Pool.h"
#include <cstdio>
#include <iostream>

void test_func(void* num)
{
	printf("Your number is: %d", *(int*)num);
	fflush(stdin);
}


int main()
{
	thread_pool test_pool(12);

	test_pool.thread_pool_start();

	int x = 1;
	thread_pool::thread_pool_job_t job1 = { .func = test_func, .args = &x };

	int y = 5;
	thread_pool::thread_pool_job_t job2 = { .func = test_func, .args = &y };

	test_pool.add_job(job1);
	test_pool.add_job(job2);


	std::string empty;

	std::cin >> empty;

	test_pool.thread_pool_stop(true);

	return 0;
}