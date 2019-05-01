#pragma once

#include <queue>
#include <mutex>

namespace Utils
{
	template<class T>
	class ThreadSafeQueue
	{
		public:
			ThreadSafeQueue(void)
			:	queue(),
				mutex()
			{}

			~ThreadSafeQueue(void) {}

			void Enqueue(T t)
			{
				std::lock_guard<std::mutex> lock(mutex);
				queue.push(t);
			}

			T Dequeue(void)
			{
				std::unique_lock<std::mutex> lock(mutex);
				if (queue.empty())
				{
					return 0;
				}
				T val = queue.front();
				queue.pop();
				return val;
			}

		private:
			std::queue<T> queue;
			mutable std::mutex mutex;
	};
}
