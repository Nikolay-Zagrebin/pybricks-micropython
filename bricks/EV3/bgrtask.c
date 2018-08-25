#include "bgrtask.h"

// This file is based on the tutorial: http://2net.co.uk/tutorial/periodic_threads

struct periodic_info {
	int timer_fd;
	unsigned long long wakeups_missed;
};

// Configure timer at specified interval
static int configure_timer_thread(unsigned int period_ms, struct periodic_info *info)
{
	struct itimerspec itval;

	int fd = timerfd_create(CLOCK_MONOTONIC, 0);
	info->wakeups_missed = 0;
	info->timer_fd = fd;
	if (fd == -1){
        return fd;
    }

	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = period_ms * 1000000;
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = period_ms * 1000000;

	return timerfd_settime(fd, 0, &itval, NULL);
}

// Wait for timer to complete period
static void wait_period(struct periodic_info *info)
{
	unsigned long long missed;
	int ret;

	// Wait for the next timer event. If we have missed any the number is written to "missed".
	ret = read(info->timer_fd, &missed, sizeof(missed));
	if (ret == -1) {
		perror("read timer");
		return;
	}

	info->wakeups_missed += missed;
}

// The background thread that keeps firing the task handler
static void *task_caller(void *arg)
{
	struct periodic_info info;

	configure_timer_thread(10, &info);
	while (1) {
        handler();
		wait_period(&info);
	}
	return NULL;
}

// Start the background thread from main
void start_background_tasks(){
	pthread_t task_caller_thread;
	pthread_create(&task_caller_thread, NULL, task_caller, NULL);
}
