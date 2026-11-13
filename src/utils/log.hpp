#pragma once
#include <windows.h>
#include <fstream>
#include <string>
#include <format>
#include <mutex>

namespace utils {

    class Logger {
    public:
        static inline std::mutex s_logMutex;

        static void LogRaw(const std::string& line) {
            std::lock_guard lock(s_logMutex);
            std::ofstream f("C:\\Users\\nedo\\Desktop\\lixxuid_debug.log", std::ios::app);
            if (f.is_open()) {
                f << line << "\n";
                f.flush();
            }
            OutputDebugStringA((line + "\n").c_str());
        }

        static void Attach(std::string_view title) {
            std::ofstream f("C:\\Users\\nedo\\Desktop\\lixxuid_debug.log", std::ios::trunc);
            if (f.is_open()) {
                f << "=== LIXXUID DEBUG LOG STARTED ===\n";
            }
        }

        static void Detach() {}

        template <typename... Args>
        static void Info(std::format_string<Args...> fmt, Args&&... args) {
            LogRaw("[+] " + std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void Error(std::format_string<Args...> fmt, Args&&... args) {
            LogRaw("[-] " + std::format(fmt, std::forward<Args>(args)...));
        }
    };

} // namespace utils
