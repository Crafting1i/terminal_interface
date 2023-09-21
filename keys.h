#include <string>
#include <cstring>
#include <stdexcept>

namespace keys {
  class key {
	private:
		char code1 = -1, code2 = -1, code3 = -1, code4 = -1;
	public:
		key(): code1(-1), code2(-1), code3(-1), code4(-1) {} 
		key(char s1, char s2, char s3, char s4)
			: code1(s1), code2(s2), code3(s3), code4(s4) {}
		key(const char* chars) {
			size_t length = std::strlen(chars);
			if(length == 0 || length > 4)
				throw std::invalid_argument("'chars' can't be empty or longer then 4 characters");
			
			this->code1 = length > 0 ? chars[0] : -1;
			this->code2 = length > 1 ? chars[1] : -1;
			this->code3 = length > 2 ? chars[2] : -1;
			this->code4 = length > 3 ? chars[3] : -1;
		}

		char get_code1() const { return code1; }
		char get_code2() const { return code2; }
		char get_code3() const { return code3; }
		char get_code4() const { return code4; }
		std::string get_string() const {
			return std::string({ code1, code2, code3, code4 });
		}

		bool operator==(const key& other) const {
			return this->code1 == other.code1 && this->code2 == other.code2
				  && this->code3 == other.code3 && this->code4 == other.code4;
		}
		operator bool() const {
			return this->code1 != -1 || this->code2 != -1
					|| this->code3 != -1 || this->code4 != -1;
		}
	};
}