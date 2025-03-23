#pragma once
#include <mutex>

class ThreadSafeInt {
public:
	int getValue();
	void setValue(int value);

	void inc();
	void dec();

private:
	std::mutex mutex;
	int value;
};