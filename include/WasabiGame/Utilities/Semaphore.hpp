#pragma once

#include <mutex>
#include <condition_variable>


namespace WasabiGame {

	class Semaphore {
		std::mutex m_mutex;
		std::condition_variable m_cv;
		uint32_t m_count;

	public:
		Semaphore(uint32_t count = 1) : m_count(count) {}

		void notify() {
			std::scoped_lock<decltype(m_mutex)> lock(m_mutex);
			m_count++;
			m_cv.notify_one();
		}

		void wait() {
			std::unique_lock<decltype(m_mutex)> lock(m_mutex);
			while (!m_count) {
				m_cv.wait(lock);
				m_count--;
			}
		}

		bool try_wait() {
			std::scoped_lock<decltype(m_mutex)> lock(m_mutex);
			if (m_count) {
				m_count--;
				return true;
			}
			return false;
		}
	};

};
