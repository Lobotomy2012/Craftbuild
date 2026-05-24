module;

#include <includes.hpp>
#include <ctime>

export module misc.format;

import misc.types;

export namespace craftbuild {
	inline std::string ptr_to_hex(const void* ptr) noexcept {
		uintptr_t value = (uintptr_t)ptr;

		if (value == 0) return "0x0";

		byte buffer[2 + sizeof(uintptr_t) * 2 + 1]; // "0x" + hex + null
		int i = sizeof(buffer) - 1;
		buffer[i--] = '\0';

		const byte* hex = "0123456789ABCDEF";

		while (value > 0) {
			buffer[i--] = hex[value & 0xF];
			value >>= 4;
		}

		buffer[i--] = 'x';
		buffer[i] = '0';

		return &buffer[i];
	}

	inline std::string time2str(const std::tm& tm, const bool with_date = false) {
		char buffer[20];
		if (with_date) {
			std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &tm);
		} else {
			std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
		}
		return std::string(buffer);
	}

	inline std::string time2file_name(const std::tm& tm) {
		char buffer[20];
		std::strftime(buffer, sizeof(buffer), "%Y.%m.%d %H-%M-%S", &tm);
		return std::string(buffer);
	}

	class format {
		std::string __buffer__;
	public:
		operator std::string() const noexcept {
			return __buffer__;
		}

		friend format&& operator<<(format&& f, const std::string& s) {
			f.__buffer__ += s;
			return std::move(f);
		}
		friend format&& operator<<(format&& f, const byte* s) {
			f.__buffer__ += std::string(s);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, int64 i) {
			f.__buffer__ += std::to_string(i);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, int i) {
			f.__buffer__ += std::to_string(i);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, uint64 i) {
			f.__buffer__ += std::to_string(i);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, unsigned int i) {
			f.__buffer__ += std::to_string(i);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, float64 i) {
			f.__buffer__ += std::to_string(i);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, bool b) {
			f.__buffer__ += b ? "true" : "false";
			return std::move(f);
		}
		friend format&& operator<<(format&& f, const void* v) {
			f.__buffer__ += ptr_to_hex(v);
			return std::move(f);
		}
		friend format&& operator<<(format&& f, const std::tm& tm) {
			f.__buffer__ += time2str(tm);
			return std::move(f);
		}
	};
}