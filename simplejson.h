#pragma once
#ifndef SIMPLEJSON_H
#define SIMPLEJSON_H

#include <iostream>
#include <string>

#include <cassert>
#include <cstring>

#include "simple_vector.h"
#include "name_value.h"

enum class json_type 
{
	JSON_NULL, JSON_TRUE, JSON_FALSE, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT
};

enum class parse_ret_type
{
	JSON_PARSE_OK = 0,
	JSON_PARSE_INVALID,
	JSON_PARSE_ROOT_NOT_SINGULAR,
	JSON_PARSE_NUMBER_TOO_BIG,
	JSON_PARSE_INVALID_STRING_ESCAPE,
	JSON_PARSE_INVALID_STRING_CHAR,
	JSON_PARSE_INVALID_UNICODE_HEX,
	JSON_PARSE_INVALID_UNICODE_SURROGATE,
	JSON_PARSE_INVALID_ARRAY_CHAR,
	JSON_PARSE_INVALID_OBJECT_CHAR,
	JSON_PARSE_INVALID_SEPARATOR
};

class json_context
{
public:
	json_context(const std::string &json) : _json(0) { _json = new char[json.size() + 1]; _tmp = _json; std::copy(json.begin(), json.end(), _json); _json[json.size()] = '\0'; }
	json_context(const json_context &rhs) = delete;

	json_context &operator=(const json_context &rhs) = delete;
	~json_context() { delete[] _tmp; }

	const char *const cbegin() { return _tmp; }
	const char *const cend() { return _json; }
	char *get_pointer() { return _json; }
	char operator[](size_t index) { return _json[index]; }

	void operator++() { ++_json; }
	void operator++(int) { _json++; }
	void operator+=(int len) { _json += len; };

private:
	char *_tmp;
	char *_json;
};

class json_value
{
public:
	json_value() {}
	json_value(const std::string &json) { }

	const bool get_json_boolean() const { assert(_type == json_type::JSON_TRUE || _type == json_type::JSON_FALSE); _type == json_type::JSON_TRUE ? true : false; }
	const double get_json_number() const { assert(_type == json_type::JSON_NUMBER); return n; }
	const simple_vector<char> &get_json_string() const { assert(_type == json_type::JSON_STRING); return s; }
	const simple_vector<json_value> &get_json_array() const { assert(_type == json_type::JSON_ARRAY); return a; }
	const simple_vector<json_obj<>> &get_json_object() const { assert(_type == json_type::JSON_OBJECT); return o; }
	const json_value & get_json_array_elem(size_t i) const { return a[i]; }
	const json_obj<> & get_json_object_arr_elem(size_t i) const { return o[i]; }
	const json_value * get_json_value_by_name(const std::string &name);
	const json_type get_json_type() const { return _type; }

	void set_json_type(json_type type) { _type = type; }
	void set_json_number(double num) { n = num; }
	void set_json_string(const simple_vector<char> &str) { s = str; }
	void set_json_array(const simple_vector<json_value> &arr) { a = arr; }
	void set_json_object_arr(const simple_vector<json_obj<>> &objs) { o = objs; }

	void push_json_char(const char ch) { s.push_back(ch); }
	void push_json_array_elem(const json_value &v) { a.push_back(v); }
	void push_json_object(const json_obj<> &obj) { o.push_back(obj); }

private:
	simple_vector<json_obj<>> o;
	simple_vector<json_value> a;
	simple_vector<char> s;
	double n;
	json_type _type;
};

parse_ret_type json_parse(json_value *const v, const std::string &json);
parse_ret_type json_parse_value(json_value *const v, json_context &ctx);
void json_parse_whitespace(json_context &ctx);
parse_ret_type json_parse_literal(json_value * const v, json_context &ctx, const std::string &literal, const json_type type);
parse_ret_type json_parse_number(json_value *const v, json_context &ctx);
const char *json_chk_hex(const char *p, unsigned *u);
void json_encode_utf8(simple_vector<char> * const ctr, unsigned u);
parse_ret_type json_parse_string_raw(simple_vector<char> *ctr, json_context &ctx);
parse_ret_type json_parse_string(json_value *const v, json_context &ctx);
parse_ret_type json_parse_array(json_value *const v, json_context &ctx);
parse_ret_type json_parse_object(json_value *const v, json_context &ctx);
void json_generator(const json_value * const v, simple_vector<char> &buf);
#endif // !1