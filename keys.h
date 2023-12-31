#pragma once

#include <string>
#include <cstring>
#include <stdexcept>

namespace ami {
  /// @brief Handler of keys
  class key {
	private:
		char code1 = -1, code2 = -1, code3 = -1, code4 = -1;
	public:
		/// @brief Default constructor. All codes is -1.
		key(): code1(-1), code2(-1), code3(-1), code4(-1) {} 
		/// @brief Constructor by separate key codes
		/// @param s1 First byte
		/// @param s2 Second byte
		/// @param s3 Third byte
		/// @param s4 Fourth byte
		key(char s1, char s2, char s3, char s4)
			: code1(s1), code2(s2), code3(s3), code4(s4) {}
		/// @brief Constructor by c-string or initial list ({ -1, -1, -1, -1}).
		/// @param chars c-string or initial list long from 1 to 4.\
		/// @throws std::invalid_argument
		key(const char* chars) {
			size_t length = std::strlen(chars);
			if(length == 0 || length > 4)
				throw std::invalid_argument("'chars' can't be empty or longer then 4 characters");
			
			this->code1 = length > 0 ? chars[0] : -1;
			this->code2 = length > 1 ? chars[1] : -1;
			this->code3 = length > 2 ? chars[2] : -1;
			this->code4 = length > 3 ? chars[3] : -1;
		}

		/// @return First byte of key
		char get_code1() const { return code1; }
		/// @return Second byte of key
		char get_code2() const { return code2; }
		/// @return Third byte of key
		char get_code3() const { return code3; }
		/// @return Fourth byte of key
		char get_code4() const { return code4; }
		/// @return byte sequence as string
		std::string get_string() const {
			std::string res = "";
			if(code1 && code1 != -1) res += code1;
			if(code2 && code2 != -1) res += code2;
			if(code3 && code3 != -1) res += code3;
			if(code4 && code4 != -1) res += code4;
			return res;
		}

		/// @return true if all bytes of `other` are same with `this`
		bool operator==(const key& other) const {
			return this->code1 == other.code1 && this->code2 == other.code2
				  && this->code3 == other.code3 && this->code4 == other.code4;
		} 
		/// @return true if any bytes of `other` are not sane with `this`
		bool operator!=(const key& other) const {
			return !this->operator==(other);
		}
		/// @return true if any bytes are not equal to -1
		operator bool() const {
			return this->code1 != -1 || this->code2 != -1
					|| this->code3 != -1 || this->code4 != -1;
		}
	};
}