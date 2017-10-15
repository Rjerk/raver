#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "RJson.h"

using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::string;

string readFile(const char* filename)
{
	std::ifstream t(string{filename});
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

namespace detail {

bool isdigit_1to9(char ch)
{
	return ch != '0' && isdigit(ch);
}

constexpr size_t STACK_INIT_SIZE = 100;

void freeValue(rjson::JsonValue* v)
{
    using namespace rjson;
	assert(v != NULL);
	switch (v->getType()) {
        case RJSON_STRING:
			delete v->getString();
			break;
		case RJSON_OBJECT:
			for (size_t i = 0; i < v->getObjSize(); ++i) {
				delete v->getPair()[i].str_;
				freeValue(&v->getPair()[i].value_);
			}
			delete [] v->getPair();
            break;
		case RJSON_ARRAY:
			for (size_t i = 0; i < v->getArraySize(); ++i) {
				freeValue(&(v->getArray()[i]));
			}
			delete [] v->getArray();
			break;
		default:
			break;
	}
	v->setType(RJSON_NULL);
}

}

namespace rjson {

JsonValue::JsonValue()
{
    type_ = RJSON_NULL;
}

JsonValue::~JsonValue()
{
}

double JsonValue::getNumber() const
{
    assert(type_ == RJSON_NUMBER);
    return num_;
}

std::string* JsonValue::getString() const
{
    assert(type_ == RJSON_STRING);
    return str_;
}

void JsonValue::setString(string* str)
{
    str_ = str;
}

json_pair_t* JsonValue::getPair() const
{
    assert(type_ == RJSON_OBJECT);
    return pair_;
}

void JsonValue::setPair(json_pair_t* pair)
{
    pair_ = pair;
}

json_value_t* JsonValue::getArray() const
{
    return elem_;
}

JsonValue* JsonValue::getValueFromObject(const string& key)
{
    assert(type_ == RJSON_OBJECT && obj_size_ != 0 && key.size() != 0);
    for (size_t i = 0; i < obj_size_; ++i) {
        if (*(pair_[i].str_) == key) {
            return &(pair_[i].value_);
        }
    }
    return nullptr;
}

RJSON::RJSON(const string& json)
	: json_text_(json), json_(json_text_.data()), value_(), stack_(new char[detail::STACK_INIT_SIZE]),
	  top_(0), size_(detail::STACK_INIT_SIZE)
{
}

RJSON::~RJSON()
{
    detail::freeValue(&value_);
	delete [] stack_;
}

parse_code RJSON::parseJson()
{
	cleanWhitespace();
	parse_code code;
	if ((code = parseValue(&value_)) == PARSE_OK) {
		cleanWhitespace();
		if (*json_ != '\0') {
			code = PARSE_NOT_SINGULAR_VALUE;
		}
	}
    return code;
}

void RJSON::setJsonText(const string& js)
{
    json_text_.assign(js);
    json_ = json_text_.data();
    detail::freeValue(&value_);
    delete [] stack_;
    stack_ = new char[detail::STACK_INIT_SIZE];
    top_ = 0;
    size_ = detail::STACK_INIT_SIZE;
}

parse_code RJSON::parseValue(json_value_t* v)
{
	switch (*json_) {
		case 't':  return parseLiteral(v, "true", RJSON_TRUE);
		case 'f':  return parseLiteral(v, "false", RJSON_FALSE);
		case 'n':  return parseLiteral(v, "null", RJSON_NULL);
		case '"': return parseString(v);
		case '{':  return parseObject(v);
		case '[':  return parseArray(v);
		case '\0': return PARSE_OK;
		default:   return parseNumber(v);
	}
	return PARSE_INVALID_VALUE;
}

parse_code RJSON::parseLiteral(json_value_t* v, const string& literal, const json_type type)
{
	eatChar(literal[0]);
	size_t i = 0;
	for (; literal[i+1]; ++i) {
		if (json_[i] != literal[i+1]) {
			return PARSE_INVALID_VALUE;
		}
	}
	json_ += i;
	v->type_ = type;
	return PARSE_OK;
}

parse_code RJSON::parseNumber(json_value_t* v)
{
	const char* p = json_;

	if (*p == '-') {
		++p;
	}

	if (*p == '0') {
		++p;
	} else {
		if (!detail::isdigit_1to9(*p)) {
			return PARSE_INVALID_VALUE;
		}
		while (isdigit(*p)) {
			++p;
		}
	}

	if (*p == '.') {
		++p;
		if (!isdigit(*p)) {
			return PARSE_INVALID_VALUE;
		}
		while (isdigit(*p)) {
			++p;
		}
	}

	if (*p == 'e' || *p == 'E') {
		++p;
		if (*p == '+' || *p == '-') {
			++p;
		}
		if (!isdigit(*p)) {
			return PARSE_INVALID_VALUE;
		}
		while (isdigit(*p)) {
			++p;
		}
	}

	errno = 0;
	double num = std::strtod(json_, NULL);
	if ((errno == ERANGE) || (num == HUGE_VAL) || (num == -HUGE_VAL)) {
		return PARSE_NUMBER_TOO_BIG;
	}

    v->type_ = RJSON_NUMBER;
    v->num_ = num;
	json_ = p;
	return PARSE_OK;
}

parse_code RJSON::parseString(json_value_t* v)
{
	parse_code ret;
	char* s;
	size_t len;
	if ((ret = parseStringRaw(&s, &len)) == PARSE_OK) {
		setString(v, s, len);
	}
	return ret;
}

parse_code RJSON::parseStringRaw(char** str, size_t* len)
{
	eatChar('\"');
	const char* p = json_;
	size_t pos = top_;
	for ( ; ; ) {
		char ch = *p++;
		switch (ch) {
			case '\"':
				*len = top_ - pos;
				*str = static_cast<char*>(popJson (*len));
				//*str[*len] = '\0';
				json_ = p;
				return PARSE_OK;
			case '\\': {
				switch (*p++) {
					case '\"': *(static_cast<char*>(pushJson(sizeof(ch)))) = '\"'; break;
					case '\\': *(static_cast<char*>(pushJson(sizeof(ch)))) = '\\'; break;
					case '/':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '/';  break;
					case 'b':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '\b'; break;
					case 'f':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '\f'; break;
					case 'n':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '\n'; break;
					case 'r':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '\r'; break;
					case 't':  *(static_cast<char*>(pushJson(sizeof(ch)))) = '\t'; break;
					case 'u': {
						unsigned u, u2;

						if (!(p = parse4HexDigits(p, &u))) {
							top_ = pos;
							return PARSE_INVALID_UNICODE_HEX;
						}
						// if the first codepoint is high surrogate, then we should handle low surrogate.
						if (0xD800 <= u && u <= 0xDBFF) {
							if (*p++ != '\\') {
								top_ = pos;
								return PARSE_INVALID_UNICODE_SURROGATE;
							}
							if (*p++ != 'u') {
								top_ = pos;
								return PARSE_INVALID_UNICODE_SURROGATE;
							}
							if (!(p = parse4HexDigits(p, &u2))) {
								top_ = pos;
								return PARSE_INVALID_UNICODE_HEX;
							}
							if (u2 < 0xDC00 || u2 > 0xDFFF) {
								top_ = pos;
								return PARSE_INVALID_UNICODE_SURROGATE;
							}
							u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x1000;
						}
						encodeUTF8(u);
						break;
					}
					default:
						top_ = pos;
						return PARSE_INVALID_ESCAPE_CHARCTER;
				}
				break;
			}
			case '\0':
				top_ = pos;
				return PARSE_MISS_QUOTATION_MARK;
			default:
				if (static_cast<unsigned char>(ch) < 0x20) {
					top_ = pos;
					return PARSE_INVALID_STRING_CHAR;
				}
				*(static_cast<char*>(pushJson(sizeof(ch)))) = ch;
		}
	}
}

void RJSON::setString(json_value_t* v, const char* str, size_t len)
{
	assert(v != nullptr && (str != NULL || len == 0));
    detail::freeValue(v);
    v->str_ = new string(str, len);
    v->type_ = RJSON_STRING;
}

void* RJSON::pushJson(size_t sz)
{
	assert(sz > 0);
	while (sz + top_ >= size_) {
		size_ += (size_ >> 1);
	}

	char* tmp = new char[size_];
	memcpy(tmp, stack_, top_);
    delete [] stack_;
	stack_ = tmp;
	tmp = nullptr;

	auto t = top_;
	top_ += sz;
	return stack_ + t;
}

void* RJSON::popJson(size_t sz)
{
	assert(sz <= top_);
	return stack_ + (top_ -= sz);
}

const char* RJSON::parse4HexDigits(const char* p, unsigned* u)
{
	std::stringstream ss;
	string hex4;
	*u = 0;
	for (int i = 0; i < 4; ++i) {
		if (isxdigit(*p)) {
			hex4 += *p++;
		} else {
			return nullptr;
		}
	}
	ss << std::hex << hex4;
	ss >> *u;
	return p;
}

parse_code RJSON::parseObject(json_value_t* v)
{
	eatChar('{');
	cleanWhitespace();

	if (*json_ == '}') {
		++json_;
        v->type_ = RJSON_OBJECT;
		v->pair_ = nullptr;
		v->obj_size_ = 0;
		return PARSE_OK;
	}

	json_pair_t pair;
	size_t sz = 0;
	parse_code ret = PARSE_INVALID_VALUE;

	while (true) {
        v->type_ = RJSON_NULL;
		pair.value_.type_ = RJSON_NULL;

		// parse string
		if (*json_ != '\"') {
			ret = PARSE_MISS_KEY;
			break;
		}

		char* str = nullptr;
		size_t len;
		if ((ret = parseStringRaw(&str, &len)) != PARSE_OK) {
			break;
		}

		pair.str_ = new string(str, len);
		str = nullptr;

		cleanWhitespace();
		if (*json_ != ':') {
			ret = PARSE_MISS_COLON;
			break;
		}
		++json_;
		cleanWhitespace();

		// parse value
		if ((ret = parseValue(&pair.value_)) != PARSE_OK) {
			break;
		}
		memcpy(pushJson(sizeof(json_pair_t)), &pair, sizeof(json_pair_t));
		pair.str_ = nullptr;
		++sz;

		cleanWhitespace();
		if (*json_ == ',') {
			++json_;
			cleanWhitespace();
		} else if (*json_ == '}') {
			++json_;
            v->type_ = RJSON_OBJECT;
			v->obj_size_ = sz;
			size_t sumsize = sizeof(json_pair_t) * sz;
			memcpy(v->pair_ = new json_pair_t[sz], popJson(sumsize), sumsize);
			return PARSE_OK;
		} else {
			ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}

    delete pair.str_;
	for (size_t i = 0; i < sz; ++i) {
		auto p = static_cast<json_pair_t*>(popJson(sizeof(json_pair_t))); (void) p;
		delete p->str_;
        detail::freeValue(&(p->value_));
	}

    v->type_ = RJSON_NULL;
	return ret;
}

parse_code RJSON::parseArray(json_value_t* v)
{
	eatChar('[');
	cleanWhitespace();

	if (*json_ == ']') {
		++json_;
        v->type_ = RJSON_ARRAY;
		v->pair_ = nullptr;
		v->arr_size_ = 0;
		return PARSE_OK;
	}

	size_t sz = 0;
	parse_code ret = PARSE_INVALID_VALUE;

	while (true) {
		json_value_t value;

		if ((ret = parseValue(&value)) != PARSE_OK) {
			break;
		}
		memcpy(pushJson(sizeof(json_value_t)), &value, sizeof(json_value_t));
		++sz;

		cleanWhitespace();
		if (*json_ == ',') {
			++json_;
			cleanWhitespace();
		} else if (*json_ == ']') {
			++json_;
            v->type_ = RJSON_ARRAY;
			v->arr_size_ = sz;
			auto sumsz = sz * sizeof(json_value_t);
			memcpy(v->elem_ = new json_value_t[sz], popJson(sumsz), sumsz);
			return PARSE_OK;
		} else {
			ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}


	for (size_t i = 0; i < sz; ++i) {
        detail::freeValue(static_cast<json_value_t*>(popJson(sizeof(json_value_t))));
	}

    v->type_ = RJSON_NULL;
	return ret;
}

void RJSON::parseCodeHandle(parse_code code)
{
	switch (code) {
		case PARSE_OK:
			cout << "Parse ok." << endl; break;
		case PARSE_INVALID_VALUE:
			cout << "Invalid value."; break;
		case PARSE_MISS_COLON:
			cout << "Miss colon in object." << endl; break;
		case PARSE_MISS_COMMA_OR_SQUARE_BRACKET:
			cout << "Miss comma or square in array." << endl; break;
		case PARSE_MISS_COMMA_OR_CURLY_BRACKET:
			cout << "Miss comma or curly bracket in object." << endl; break;
		case PARSE_MISS_KEY:
			cout << "Miss key in object." << endl; break;
		case PARSE_NOT_SINGULAR_VALUE:
			cout << "Not singular value." << endl; break;
		case PARSE_NUMBER_TOO_BIG:
			cout << "Number is too big." << endl; break;
		case PARSE_INVALID_ESCAPE_CHARCTER:
			cout << "Invalid escaped character." << endl; break;
		case PARSE_INVALID_UNICODE_HEX:
			cout << "Invalid unicode hexadecimal digits." << endl; break;
		case PARSE_MISS_QUOTATION_MARK:
			cout << "Miss quotation mark in string." << endl; break;
		case PARSE_INVALID_STRING_CHAR:
			cout << "Invalid char in string." << endl; break;
		case PARSE_INVALID_UNICODE_SURROGATE:
			cout << "Invalid unicode surrogate in string." << endl; break;
		default:
			cout << "Unexpected error." << endl;
	}
}

void RJSON::stringifyValue(json_value_t* v)
{
	switch (v->type_) {
		case RJSON_NULL: memcpy(pushJson(4), "null", 4); break;
		case RJSON_FALSE: memcpy(pushJson(5), "false", 5); break;
		case RJSON_TRUE: memcpy(pushJson(4), "true", 4); break;
		case RJSON_NUMBER:
			// %g print the number with as many as digits as needed for precision,
			// preferring exponetial syntax when the numbers are small or huge (1e-5 rather than 0.00005)
			// and skipping any trailing zeroes (1 rather than 1.0000).
			top_ -= (32 - sprintf(static_cast<char*>(pushJson(32)), "%.17g", v->getNumber()));
			break;
		case RJSON_STRING:
			stringifyString(v->getString()->data(), v->getString()->size());
			break;
		case RJSON_OBJECT:
			memcpy(pushJson(sizeof('{')), "{", sizeof('{'));
			for (size_t i = 0; i < v->obj_size_; ++i) {
				if (i > 0) {
					memcpy(pushJson(sizeof(',')), ",", sizeof(','));
				}
				stringifyString(v->pair_[i].str_->data(), v->pair_[i].str_->size());
				memcpy(pushJson(sizeof(':')), ":", sizeof(':'));
				stringifyValue(&v->pair_[i].value_);
			}
			memcpy(pushJson(sizeof('}')), "}", sizeof('}'));
			break;
		case RJSON_ARRAY:
			memcpy(pushJson(sizeof('[')), "[", sizeof('['));
			for (size_t i = 0; i < v->arr_size_; ++i) {
				if (i > 0) {
					memcpy(pushJson(sizeof(',')), ",", sizeof(','));
				}
				stringifyValue(&v->elem_[i]);
			}
			memcpy(pushJson(sizeof(']')), "]", sizeof(']'));
			break;
		default:
			assert(0 && "Invalid type");
	}
}

void RJSON::stringifyString(const char* str, size_t len)
{
	assert(str != nullptr);
	const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	size_t size;
	char* head = nullptr;
	char* p = nullptr;
	p = head = static_cast<char*>(pushJson(size = len * 6 + 2)); // if there's len unicode characters. 2 is for quotation marks.
	*p++ = '"';
	for (size_t i = 0; i < len; ++i) {
		auto ch = static_cast<unsigned char>(str[i]);
		switch (ch) {
			case '\"': *p++ = '\\'; *p++ = '\"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '/':  *p++ = '\\'; *p++ = '/';  break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
            	if (ch < 0x20) {
            		*p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
            		*p++ = hex_digits[ch >> 4];
            		*p++ = hex_digits[ch & 15];
				} else {
					*p++ = str[i];
				}
		}
	}
	*p++ = '"';
	top_ -= (size - (p - head));
}

string RJSON::generator()
{
	assert(stack_);
	assert(value_.type_ == RJSON_OBJECT || value_.type_ == RJSON_ARRAY);
	stringifyValue(&value_);
	auto sz = top_;
	return string(static_cast<char*>(popJson(top_)), sz);
}

void RJSON::encodeUTF8(unsigned u)
{
	if (u <= 0x7F) { // 0xxxxxxx
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(u & 0xFF);
	} else if (u <= 0x7FF) { // 110xxxxx 10xxxxxx
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0xC0 | ((u >> 6) & 0xFF));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ( u       & 0x3F));
	} else if (u <= 0xFFFF) { // 1110xxxx 10xxxxxx 10xxxxxx
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0xE0 | ((u >> 12) & 0xFF));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ((u >>  6) & 0x3F));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ( u        & 0x3F));
	} else {
		assert(u <= 0x10FFFF); // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0xF0 | ((u >> 18) & 0xFF));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ((u >> 12) & 0x3F));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ((u >>  6) & 0x3F));
		*(static_cast<char*>(pushJson(sizeof(char)))) = static_cast<char>(0x80 | ( u        & 0x3F));
	}
}

}
