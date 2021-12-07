#include <thread>

template <typename Task> void SplitToMultipleThreads(Task& task) {
	int threadCount = std::thread::hardware_concurrency();
	if (threadCount == 0)
		threadCount = 1;
	std::thread** threads = new std::thread*[threadCount];
	for (int i = 0; i < threadCount; i++)
		threads[i] = new std::thread(task, i, threadCount);
	for (int i = 0; i < threadCount; i++) {
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}
