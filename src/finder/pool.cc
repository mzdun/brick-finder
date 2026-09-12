// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module finder.pool;

import std;

export namespace finder {
	struct callback {
	public:
		callback() = default;
		callback(callback const&) = delete;
		callback& operator=(callback const&) = delete;
		callback(callback&&) noexcept = default;
		callback& operator=(callback&&) noexcept = default;

		template <std::invocable<> T>
		callback(T&& handler) : cb{std::make_unique<impl<T>>(std::move(handler))} {}

		explicit operator bool() const noexcept { return !!cb; }
		void operator()() { cb->call(); }

	private:
		struct interface {
			virtual ~interface() = default;
			virtual void call() = 0;
		};

		template <std::invocable<> T>
		struct impl : interface {
			T held;
			impl(T&& held) : held{std::move(held)} {}
			void call() override { held(); }
		};

		std::unique_ptr<interface> cb{};
	};

	enum class task {
		loading,
		processing,
		postprocessing,
	};

	void post_task(task type, callback&& refref);
	void run_tasks(unsigned int threads = std::thread::hardware_concurrency());
}  // namespace finder
