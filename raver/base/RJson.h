#ifndef RJSON_H
#define RJSON_H

#include <cassert>
#include <cctype>
#include <iostream>
#include <string>

#include "noncopyable.h"

namespace rjson {

enum json_type {
  RJSON_STRING,
  RJSON_NUMBER,
  RJSON_OBJECT,
  RJSON_ARRAY,
  RJSON_FALSE,
  RJSON_TRUE,
  RJSON_NULL
};

using json_pair_t = struct json_pair;

using json_value_t = class JsonValue;

struct json_pair;

class JsonValue {
  friend class RJSON;

 public:
  JsonValue();
  ~JsonValue();

  json_type getType() const { return type_; }
  void setType(json_type type) { type_ = type; }
  double getNumber() const;
  void setNumber(double num) { num_ = num; }
  std::string* getString() const;
  void setString(std::string* str);
  json_pair_t* getPair() const;
  void setPair(json_pair_t* pair);
  json_value_t* getArray() const;
  size_t getArraySize() const { return arr_size_; }
  size_t getObjSize() const { return obj_size_; }
  JsonValue* getValueFromObject(const std::string& key);

 private:
  json_type type_;
  union {
    double num_;
    std::string* str_;
    struct {
      json_pair_t* pair_;
      size_t obj_size_;
    };
    struct {
      json_value_t* elem_;
      size_t arr_size_;
    };
  };
};

struct json_pair {
  json_pair() : value_() {}
  ~json_pair() = default;
  std::string* str_{nullptr};
  JsonValue value_;
};

enum parse_code {
  PARSE_OK,
  PARSE_INVALID_VALUE,
  PARSE_NOT_SINGULAR_VALUE,
  PARSE_NUMBER_TOO_BIG,
  PARSE_INVALID_ESCAPE_CHARCTER,
  PARSE_INVALID_UNICODE_HEX,
  PARSE_MISS_QUOTATION_MARK,
  PARSE_INVALID_STRING_CHAR,
  PARSE_INVALID_UNICODE_SURROGATE,
  PARSE_MISS_KEY,
  PARSE_MISS_COLON,
  PARSE_MISS_COMMA_OR_CURLY_BRACKET,
  PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

class RJSON : raver::noncopyable {
 public:
  explicit RJSON(std::string js);
  ~RJSON();

  parse_code parseJson();
  void setJsonText(const std::string& js);
  std::string generator();
  void parseCodeHandle(parse_code code);
  JsonValue* getValue() { return &value_; }

 private:
  parse_code parseValue(json_value_t* v);
  parse_code parseLiteral(json_value_t* v, const std::string& literal,
                          json_type type);
  parse_code parseNumber(json_value_t* v);
  parse_code parseString(json_value_t* v);
  parse_code parseStringRaw(char** str, size_t* len);
  parse_code parseObject(json_value_t* v);
  parse_code parseArray(json_value_t* v);

  void* pushJson(size_t sz);
  void* popJson(size_t sz);
  void stringifyValue(json_value_t* v);
  void stringifyString(const char* str, size_t len);
  void setString(json_value_t* v, const char* str, size_t len);
  void encodeUTF8(unsigned u);
  const char* parse4HexDigits(const char* p, unsigned* u);
  void cleanWhitespace() {
    while (isspace(*json_)) ++json_;
  }
  void eatChar(char ch) {
    assert(*json_ == ch);
    (void)ch;
    ++json_;
  }

 private:
  std::string json_text_;
  const char* json_;
  JsonValue value_;

  char* stack_;
  size_t top_;
  size_t size_;
};

}  // namespace rjson

std::string readFile(const char* filename);

#endif
