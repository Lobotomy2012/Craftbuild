module;

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include <includes.hpp>
#include <thread>
#include <fstream>
#include <filesystem>
#include <mutex>

export module game.logger;

import misc.types;
import misc.format;
import game.core;
import game.thread;

using namespace godot;

namespace craftbuild {
    auto get_time = []() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        return now_tm;
    };

    auto get_info = []() {
        return format{} << "[" << get_time() << "] [" << ThreadRegistry::get_name(std::this_thread::get_id()) << "] ";
    };
}

export namespace craftbuild {
    struct LogQueue {
        inline static std::string file_queue;
        inline static std::mutex log_mutex;

        static none store(const std::string& log, const std::string& file_log) {
            if (craftbuild_debug) godot::UtilityFunctions::print(log.c_str());

            std::lock_guard<std::mutex> lock(log_mutex);
            file_queue += file_log + '\n';
        }

        static none flush() {
            std::string file_dump;

            {
                std::lock_guard<std::mutex> lock(log_mutex);
                file_dump.swap(file_queue);
            }

            if (file_dump.empty()) return;

            static auto log_file = []() {
                const auto time = get_time();
                const String real_path = ProjectSettings::get_singleton()->globalize_path(("user://game/logs/" + time2file_name(time) + ".txt").c_str());
                const std::string std_path = real_path.utf8().get_data();
                std::filesystem::create_directories(std::filesystem::path(std_path).parent_path());
                return std::ofstream(std_path);
            }();

            log_file << std::move(file_dump) << std::flush;
        }
    };

    enum class LogType : uint8 {
        NORMAL,
        VERBOSE,
        INFO,
        WARNING,
        ERROR
    };

    template <LogType LOG_TYPE>
    none log(const std::string& message) {}

    template <>
    none log<LogType::NORMAL>(const std::string& message) {
        const std::string info = get_info();
        const std::string current_log = format{} << info << message;
        std::string log;

        if (colored_log) log += format{} << "\033[97m" << info << " \033[37m" << message << "\033[0m";
        else log = current_log;

        LogQueue::store(log, current_log);
    }

    template <>
    none log<LogType::VERBOSE>(const std::string& message) {
        if (not log_verbose) return;

        const std::string info = get_info();
        const std::string current_log = format{} << info << "Verbose: " << message;
        std::string log;

        if (colored_log) log += format{} << "\033[90m" << info << "Verbose: " << message << "\033[0m";
        else log = current_log;

        LogQueue::store(log, current_log);
    }

    template <>
    none log<LogType::INFO>(const std::string& message) {
        const std::string info = get_info();
        const std::string current_log = format{} << info << "Info: " << message;
        std::string log;

        if (colored_log) log += format{} << "\033[96m" << info << "Info: \033[36m" << message << "\033[0m";
        else log = current_log;

        LogQueue::store(log, current_log);
    }

    template <>
    none log<LogType::WARNING>(const std::string& message) {
        const std::string info = get_info();
        const std::string current_log = format{} << info << "Warning: " << message;
        std::string log;

        if (colored_log) log += format{} << "\033[93m" << info << "Warning: \033[33m" << message << "\033[0m";
        else log = current_log;

        LogQueue::store(log, current_log);
    }

    template <>
    none log<LogType::ERROR>(const std::string& message) {
        const std::string info = get_info();
        const std::string current_log = format{} << info << "Error: " << message;
        std::string log;

        if (colored_log) log += format{} << "\033[91m" << info << "Error: \033[31m" << message << "\033[0m";
        else log = current_log;

        LogQueue::store(log, current_log);
    }
}