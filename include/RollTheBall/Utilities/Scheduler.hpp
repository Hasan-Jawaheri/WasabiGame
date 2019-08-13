#pragma once

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

namespace HBUtils {

	class SchedulerThread {
		friend class Scheduler;

	protected:
		class Scheduler* m_scheduler;
		bool m_isRunning;

	public:
		SchedulerThread() {
			m_isRunning = true;
			m_scheduler = nullptr;
		}

		void Stop() {
			m_isRunning = false;
		}

		bool IsRunning() {
			return m_isRunning;
		}

		virtual void Run() = 0;
	};

	class Scheduler {
		bool m_isRunning;
		std::mutex m_threadsLock;
		std::unordered_map<std::string, std::pair<std::thread*, SchedulerThread*>> m_threads;

		struct WorkUnit {
			std::function<void*(void)> perform;
			std::function<void(void*)> callback;
		};
		std::mutex m_workMutex;
		std::condition_variable m_workCondition;
		std::queue<WorkUnit> m_workQueue;

		class SchedulerWorker : public SchedulerThread {
		public:
			SchedulerWorker() {
			}

			virtual void Run() {
				while (m_isRunning) {
					WorkUnit w = m_scheduler->GetWork();
					void* result = w.perform();
					if (w.callback)
						w.callback(result);
				}
			}
		};

		WorkUnit GetWork() {
			std::unique_lock<std::mutex> ul(m_workMutex);
			m_workCondition.wait(ul, [this]() { return m_workQueue.size() > 0; });
			WorkUnit w = m_workQueue.front();
			m_workQueue.pop();
			ul.unlock();
			m_workCondition.notify_one();
			return w;
		}

	public:
		Scheduler() {
			m_isRunning = true;
		}

		void LaunchWorkers(uint32_t numWorkers) {
			for (uint32_t i = 0; i < numWorkers; i++)
				LaunchThread("worker-" + std::to_string(i), new SchedulerWorker());
		}

		void LaunchThread(std::string name, SchedulerThread* t) {
			t->m_scheduler = this;
			m_threadsLock.lock();
			std::thread* newThread = new std::thread([t]() {
				t->Run();
				});
			m_threads.insert(std::make_pair(name, std::make_pair(newThread, t)));
			m_threadsLock.unlock();
		}

		void LaunchThread(std::string name, std::function<void()> entryPoint) {
			class AnonymousThread : public SchedulerThread {
				std::function<void()> m_entryPoint;
			public:
				AnonymousThread(std::function<void()> entryPoint) : m_entryPoint(entryPoint) {}
				void Run() {
					m_entryPoint();
				}
			};
			LaunchThread(name, new AnonymousThread(entryPoint));
		}

		void StopThread(std::string name) {
			m_threadsLock.lock();
			auto it = m_threads.find(name);
			if (it != m_threads.end()) {
				it->second.second->Stop();
				it->second.first->join();
				delete it->second.first;
				delete it->second.second;
				m_threads.erase(it);
			}
			m_threadsLock.unlock();
		}

		template<typename ReturnType>
		void SubmitWork(std::function<ReturnType()> function, std::function<void(ReturnType)> callback = [](void*) {}) {
			WorkUnit work;
			work.perform = [function]() { return (void*)function(); };
			work.callback = [callback](void* ret) { callback((ReturnType)ret); };

			m_workMutex.lock();
			m_workQueue.push(work);
			m_workMutex.unlock();
			m_workCondition.notify_one();
		}

		template<typename ReturnType>
		void SubmitWorks(std::vector<std::function<ReturnType()>> functions, std::function<void(std::vector<ReturnType>)> callback = nullptr) {
			uint32_t numWorks = functions.size();
			ReturnType* results = new ReturnType[numWorks];
			std::mutex* resultsLock = new std::mutex;
			uint32_t* numWorksDone = new uint32_t(0);
			memset(results, 0, numWorks * sizeof(ReturnType));

			std::vector<WorkUnit> workUnits(numWorks);
			for (uint32_t i = 0; i < numWorks; i++) {
				workUnits[i].perform = [i, functions, results, numWorks, resultsLock, numWorksDone, callback]() {
					results[i] = functions[i]();
					resultsLock->lock();
					*numWorksDone = *numWorksDone + 1;
					bool isLastDone = *numWorksDone == numWorks;
					resultsLock->unlock();
					if (isLastDone) {
						std::vector<ReturnType> resultsVec(numWorks);
						for (uint32_t j = 0; j < numWorks; j++)
							resultsVec[j] = results[j];
						delete resultsLock;
						delete numWorksDone;
						delete[] results;
						if (callback)
							callback(resultsVec);
					}
					return nullptr;
				};
				workUnits[i].callback = [](void*) {};
			}

			m_workMutex.lock();
			for (uint32_t i = 0; i < numWorks; i++)
				m_workQueue.push(workUnits[i]);
			m_workMutex.unlock();
			m_workCondition.notify_all();
		}

		void Run() {
			while (m_isRunning) {
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}

			// cleanup
			m_threadsLock.lock();
			for (auto it = m_threads.begin(); it != m_threads.end(); it++) {
				it->second.second->Stop();
				it->second.first->join();
				delete it->second.first;
				delete it->second.second;
			}
			m_threads.clear();
			m_threadsLock.unlock();
		}

		void Stop() {
			m_isRunning = false;
		}
	};

};
