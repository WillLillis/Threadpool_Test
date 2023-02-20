#include "Thread_Pool.h"
#include <cstdio>
#include <iostream>

HANDLE window_handle;

void test_func(void* num)
{
	for (int i = 0; i < 100000; i++) {
		printf("Your number is: %d", *(int*)num);
		SetConsoleTextAttribute(window_handle, (WORD) * (int*)num);
	}
}


int main()
{
	srand(time(NULL));
	window_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD color_attr;

	thread_pool test_pool(12);

	test_pool.thread_pool_start();

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