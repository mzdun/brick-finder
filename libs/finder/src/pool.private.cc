// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module finder.pool;

import std;

using namespace std::chrono;

namespace finder {
	struct thread_queue {
		std::queue<callback> loading{}, processing{}, postprocessing{};

		bool empty() const noexcept { return loading.empty() && processing.empty() && postprocessing.empty(); }

		void push(task type, callback&& callback) {
			switch (type) {
				case task::loading:
					loading.push(std::move(callback));
					return;
				case task::processing:
					processing.push(std::move(callback));
					return;
				case task::postprocessing:
					postprocessing.push(std::move(callback));
					return;
			}
		}

		callback& front() {
			if (!postprocessing.empty()) {
				return postprocessing.front();
			}
			if (!processing.empty()) {
				return processing.front();
			}
			return loading.front();
		}

		void pop() {
			if (!postprocessing.empty()) {
				postprocessing.pop();
			} else if (!processing.empty()) {
				processing.pop();
			} else {
				return loading.pop();
			}
		}
	};

	class thread_pool {
	public:
		~thread_pool();
		void post(task type, callback&& refref);
		void run(unsigned int threads);

	private:
		void run_on_thread();
		callback get_task();
		void report_end(steady_clock::duration runtime);

		bool done() const noexcept { return given_out == returned && tasks.empty(); }

		mutable std::mutex mutex{};
		std::condition_variable cv{};
		thread_queue tasks{};
		std::size_t given_out{};
		std::size_t returned{};
		steady_clock::duration total_runtime{};
		steady_clock::duration threaded_runtime{};
	};

	thread_pool& instance() {
		static thread_pool pool{};
		return pool;
	}

	void post_task(task type, callback&& refref) { instance().post(type, std::move(refref)); }
	void run_tasks(unsigned int threads) { instance().run(threads); }

#define LOCK \
	std::lock_guard the { mutex }

	thread_pool::~thread_pool() {
		auto const threaded = duration_cast<milliseconds>(threaded_runtime);
		std::print("parallel: {}.{:03} s\n", threaded.count() / 1000, threaded.count() % 1000);

		auto const total = duration_cast<milliseconds>(total_runtime);
		std::print("total: {}.{:03} s\n", total.count() / 1000, total.count() % 1000);
	}

	void thread_pool::post(task type, callback&& refref) {
		{
			LOCK;
			tasks.push(type, std::move(refref));
		}
		cv.notify_one();
	}

	void thread_pool::run(unsigned int thread_count) {
		auto const then = steady_clock::now();

		{
			if (thread_count) --thread_count;
			std::vector<std::jthread> threads{};
			threads.reserve(thread_count);

			for (unsigned int index = 0; index < thread_count; ++index) {
				threads.push_back(std::jthread{[self = this] { self->run_on_thread(); }});
			}

			run_on_thread();
		}

		threaded_runtime += steady_clock::now() - then;
	}

	callback thread_pool::get_task() {
		std::unique_lock lock{mutex};

		callback task{};

		while (!done()) {
			if (!tasks.empty()) {
				task = std::move(tasks.front());
				tasks.pop();
				++given_out;
				break;
			}

			cv.wait(lock, [self = this] { return self->done(); });
		}

		return task;
	}

	void thread_pool::report_end(steady_clock::duration runtime) {
		LOCK;
		++returned;
		total_runtime += runtime;

		if (done()) {
			cv.notify_all();
		}
	}

	void thread_pool::run_on_thread() {
		while (true) {
			auto next_task = get_task();
			if (!next_task) {
				break;
			}

			auto const then = steady_clock::now();
			next_task();
			auto const runtime = steady_clock::now() - then;

			report_end(runtime);
		}
	}
}  // namespace finder
