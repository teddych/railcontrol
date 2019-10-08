/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

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
