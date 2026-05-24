module;

#include <thread>
#include <includes.hpp>
#include <mutex>

export module game.thread;

import misc.types;

export namespace craftbuild {
	struct ThreadRegistry {
		inline static std::unordered_map<std::thread::id, std::string> threads;
		inline static std::mutex threads_mutex;

		static none register_thread(const std::string& thread_name) {
			std::lock_guard lock(threads_mutex);
			threads[std::this_thread::get_id()] = thread_name;
		}

		static std::string get_name(const std::thread::id& thread_id) {
			std::lock_guard lock(threads_mutex);
			auto it = threads.find(thread_id);
			if (it == threads.end()) return "Main Thread";
			return it->second;
		}
	};
}
