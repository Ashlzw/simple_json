#pragma warning(disable:4996)
#include <cmath>
#include <cerrno>
#include <sstream>
#include <iomanip>

#include "simplejson.h"

const json_value * json_value::get_json_value_by_name(const std::string & name)
{
	assert(_type == json_type::JSON_OBJECT);
	simple_vector<char> tmp(name.c_str(), name.c_str() + name.size());
	for (size_t i = 0; i < o.size(); ++i)
		if (tmp == o[i].get_name())
			return &o[i].get_value();
	return nullptr;
}

parse_ret_type json_parse(json_value * const v, const std::string & json)
{
	assert(!json.empty());
	json_context context(json);
	parse_ret_type ret;
	v->set_json_type(json_type::JSON_NULL);
	json_parse_whitespace(context);
	if ((ret = json_parse_value(v, context)) == parse_ret_type::JSON_PARSE_OK)
	{
		json_parse_whitespace(context);
		char *p = context.get_pointer();
		if (*p != '\0')
		{
			ret = parse_ret_type::JSON_PARSE_ROOT_NOT_SINGULAR;
			v->set_json_type(json_type::JSON_NULL);
		}
	}
	
	return ret;
}

parse_ret_type json_parse_value(json_value * const v, json_context & ctx)
{
	switch (*ctx.get_pointer())
	{
	case'n': return json_parse_literal(v, ctx, "null", json_type::JSON_NULL);
	case't': return json_parse_literal(v, ctx, "true", json_type::JSON_TRUE);
	case'f': return json_parse_literal(v, ctx, "false", json_type::JSON_FALSE);
	case'-':
	case'0':
	case'1':
	case'2':
	case'3':
	case'4':
	case'5':
	case'6':
	case'7':
	case'8':
	case'9': return json_parse_number(v, ctx);
	case'"': return json_parse_string(v, ctx);
	case'[': return json_parse_array(v, ctx);
	case'{': return json_parse_object(v, ctx);
	default: return parse_ret_type::JSON_PARSE_INVALID;
	}
}

inline
void json_parse_whitespace(json_context &ctx) { if (*ctx.get_pointer() == ' ' || *ctx.get_pointer() == '\t' || *ctx.get_pointer() == '\n' || *ctx.get_pointer() == '\r') ++ctx; }

parse_ret_type json_parse_literal(json_value * const v, json_context & ctx, const std::string & literal, const json_type type)
{
	for (size_t i = 0; i < literal.size(); ++i)
		if (ctx[i] != literal[i])
			return parse_ret_type::JSON_PARSE_INVALID;

	ctx += literal.size();
	v->set_json_type(type);

	return parse_ret_type::JSON_PARSE_OK;
}

parse_ret_type json_parse_number(json_value * const v, json_context & ctx)
{
	const char *p = ctx.get_pointer();
	if (*p == '-') ++p;
	
	if (*p == '0') ++p;
	else
	{
		if (!(*p <= '9' && *p >= '1')) return parse_ret_type::JSON_PARSE_INVALID;
		while (*p <= '9' && *p >= '0') ++p;
	}

	if (*p == '.')
	{
		++p;
		if (!(*p <= '9' && *p >= '0')) return parse_ret_type::JSON_PARSE_INVALID;
		++p;
		while (*p <= '9' && *p >= '0') ++p;
	}
	
	if (*p == 'e' || *p == 'E')
	{
		++p;
		if (*p == '+' || *p == '-') ++p;
		if (!(*p <= '9' && *p >= '0')) return parse_ret_type::JSON_PARSE_INVALID;
		++p;
		while (*p <= '9' && *p >= '0') ++p;
	}

	v->set_json_type(json_type::JSON_NUMBER);
	v->set_json_number(std::strtod(ctx.get_pointer(), nullptr));
	errno = 0;
	if (errno == ERANGE && (v->get_json_number() == HUGE_VAL || v->get_json_number() == -HUGE_VAL))
		return parse_ret_type::JSON_PARSE_NUMBER_TOO_BIG;

	ctx += p - ctx.get_pointer();

	return parse_ret_type::JSON_PARSE_OK;
}

const char * json_chk_hex(const char * p, unsigned *u)
{
	*u = 0;
	for (size_t i = 0; i < 4; ++i)
	{
		*u <<= 4;
		const char ch = *p++;
		if (ch <= '9' && ch >= '0') *u |= (ch - '0');
		else if (ch <= 'F' && ch >= 'A') *u |= (ch - 'A' + 10);
		else if (ch <= 'f' && ch >= 'a') *u |= (ch - 'a' + 10);
		else return nullptr;
	}
	return p;
}

void json_encode_utf8(simple_vector<char> * const ctr, unsigned u)
{
	if (u <= 0x7f)
		ctr->push_back(u & 0xff);
	else if (u <= 0x7ff)
	{
		ctr->push_back(0xc0 | (u >> 6) & 0xff);
		ctr->push_back(0x80 | (u) & 0x3f);
	}
	else if (u <= 0xffff)
	{
		ctr->push_back(0xe0 | (u >> 12) & 0xff);
		ctr->push_back(0x80 | (u >> 6) & 0x3f);
		ctr->push_back(0x80 | u & 0x3f);
	}
	else if (u <= 0x10ffff)
	{
		ctr->push_back(0xf0 | (u >> 18) & 0xff);
		ctr->push_back(0x80 | (u >> 12) & 0x3f);
		ctr->push_back(0x80 | (u >> 6) & 0x3f);
		ctr->push_back(0x80 | u & 0x3f);
	}
}

parse_ret_type json_parse_string_raw(simple_vector<char> * const ctr, json_context &ctx)
{
	const char *p = ctx.get_pointer();
	unsigned u = 0;
	if (*p++ != '\"') return parse_ret_type::JSON_PARSE_INVALID;
	while (true)
	{
		const char ch = *p++;
		switch (ch)
		{
		case'\"':
			ctx += p - ctx.get_pointer();
			return parse_ret_type::JSON_PARSE_OK;
		case '\\':
			switch (*p++)
			{
			case'\"': ctr->push_back('\"'); break;
			case'\\': ctr->push_back('\\'); break;
			case'/': ctr->push_back('/'); break;
			case'b': ctr->push_back('\b'); break;
			case'f': ctr->push_back('\f'); break;
			case'n': ctr->push_back('\n'); break;
			case'r': ctr->push_back('\r'); break;
			case't': ctr->push_back('\t'); break;
			case'u':
				if (!(p = json_chk_hex(p, &u)))
					return parse_ret_type::JSON_PARSE_INVALID_UNICODE_HEX;
				if (u >= 0xD800 && u <= 0xDBFF)
				{
					if (*p++ != '\\')
						return parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE;
					if (*p++ != 'u')
						return parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE;
					unsigned u2 = 0;
					if (!(p = json_chk_hex(p, &u2)))
						return parse_ret_type::JSON_PARSE_INVALID_UNICODE_HEX;
					if (!(u2 <= 0xdfff && u2 >= 0xdc00))
						return parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE;
					u = (((u - 0xd800) << 10) | (u2 - 0xdc00)) + 0x10000;
				}
				json_encode_utf8(ctr, u);
				break;
			default:
				return parse_ret_type::JSON_PARSE_INVALID_STRING_ESCAPE;
			}
			break;
		default:
			if (ch < 0x20)
				return parse_ret_type::JSON_PARSE_INVALID_STRING_CHAR;
			ctr->push_back(ch);
		}
	}
}

parse_ret_type json_parse_string(json_value * const v, json_context & ctx)
{
	parse_ret_type ret;
	simple_vector<char> ctr;
	if ((ret = json_parse_string_raw(&ctr, ctx)) != parse_ret_type::JSON_PARSE_OK)
		return ret;

	v->set_json_type(json_type::JSON_STRING);
	v->set_json_string(ctr);

	return ret;
}

parse_ret_type json_parse_array(json_value * const v, json_context & ctx)
{
	parse_ret_type ret;
	const char *p = ctx.get_pointer();
	if (*ctx.get_pointer() != '[') return parse_ret_type::JSON_PARSE_INVALID;
	++ctx;
	json_parse_whitespace(ctx);
	if (*ctx.get_pointer() == ']')
	{
		++ctx;
		v->set_json_type(json_type::JSON_ARRAY);
		return parse_ret_type::JSON_PARSE_OK;
	}
	while (true)
	{
		json_value e = json_value();
		if ((ret = json_parse_value(&e, ctx)) != parse_ret_type::JSON_PARSE_OK)
			return ret;
		v->push_json_array_elem(e);
		json_parse_whitespace(ctx);
		if (*ctx.get_pointer() == ',')
		{
			++ctx;
			json_parse_whitespace(ctx);
		}
		else if (*ctx.get_pointer() == ']')
		{
			++ctx;
			v->set_json_type(json_type::JSON_ARRAY);
			return parse_ret_type::JSON_PARSE_OK;
		}
		else
			return parse_ret_type::JSON_PARSE_INVALID_ARRAY_CHAR;
	}
}

parse_ret_type json_parse_object(json_value * const v, json_context & ctx)
{
	parse_ret_type ret;
	if (*ctx.get_pointer() != '{') return parse_ret_type::JSON_PARSE_INVALID;
	++ctx;
	json_parse_whitespace(ctx);
	if (*ctx.get_pointer() == '}')
	{
		++ctx;
		v->set_json_type(json_type::JSON_OBJECT);
		return parse_ret_type::JSON_PARSE_OK;
	}
	while (true)
	{
		simple_vector<char> name;
		if ((ret = json_parse_string_raw(&name, ctx)) != parse_ret_type::JSON_PARSE_OK)
			return ret;
		json_parse_whitespace(ctx);
		if (*ctx.get_pointer() != ':') return parse_ret_type::JSON_PARSE_INVALID_SEPARATOR;
		++ctx;
		json_parse_whitespace(ctx);
		json_value value;
		if ((ret = json_parse_value(&value, ctx)) != parse_ret_type::JSON_PARSE_OK)
			return ret;
		json_obj<> obj;
		obj.set_name(name);
		obj.set_value(value);
		v->push_json_object(obj);
		json_parse_whitespace(ctx);
		if (*ctx.get_pointer() == ',')
		{
			++ctx;
			json_parse_whitespace(ctx);
		}
		else if (*ctx.get_pointer() == '}')
		{
			++ctx;
			v->set_json_type(json_type::JSON_OBJECT);
			return parse_ret_type::JSON_PARSE_OK;
		}
		else
			return parse_ret_type::JSON_PARSE_INVALID_OBJECT_CHAR;
	}
}

void json_generator_string(const simple_vector<char> &str, simple_vector<char> &buf)
{
	buf.push_back('"');
	for (size_t i = 0; i < str.size(); ++i)
	{
		switch (str[i])
		{
		case'\"': buf.push_back('\\');buf.push_back('\"');break;
		case'\\': buf.push_back('\\');buf.push_back('\\');break;
		case'/': buf.push_back('\\');buf.push_back('/');break;
		case'\b': buf.push_back('\\');buf.push_back('\b');break;
		case'\f': buf.push_back('\\');buf.push_back('\f');break;
		case'\n': buf.push_back('\\');buf.push_back('\n');break;
		case'\r': buf.push_back('\\');buf.push_back('\r');break;
		case'\t': buf.push_back('\\');buf.push_back('\t');break;
		default:
			if (str[i] < 0x20)
			{
				std::stringstream ss;
				ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << str[i];
				std::string tmp = ss.str();
				for (size_t i = 0; i < tmp.size(); ++i)
					buf.push_back(tmp[i]);
			}
			else
				buf.push_back(str[i]);
			break;
		}
	}
	buf.push_back('"');
}

void json_generator(const json_value * const v, simple_vector<char> &buf)
{
	switch (v->get_json_type())
	{
	case json_type::JSON_NULL:
		buf.push_back('n');buf.push_back('u');buf.push_back('l');buf.push_back('l'); break;
	case json_type::JSON_TRUE:
		buf.push_back('t');buf.push_back('r');buf.push_back('u');buf.push_back('e'); break;
	case json_type::JSON_FALSE:
		buf.push_back('f');buf.push_back('a');buf.push_back('l');buf.push_back('s');buf.push_back('e'); break;
	case json_type::JSON_NUMBER:
	{
		std::stringstream ss;
		ss << v->get_json_number();
		std::string tmp = ss.str();
		for (size_t i = 0; i < tmp.size(); ++i)
			buf.push_back(tmp[i]);
	}
	break;
	case json_type::JSON_STRING: json_generator_string(v->get_json_string(), buf); break;
	case json_type::JSON_ARRAY: 
		buf.push_back('[');
		for (size_t i = 0; i < v->get_json_array().size(); ++i)
		{
			if (i > 0)
				buf.push_back(',');
			json_generator(&v->get_json_array_elem(i), buf);
		}
		buf.push_back(']');
		break;
	case json_type::JSON_OBJECT:
		buf.push_back('{');
		for (size_t i = 0; i < v->get_json_object().size(); ++i)
		{
			if (i > 0)
				buf.push_back(',');
			json_generator_string(v->get_json_object_arr_elem(i).get_name(), buf);
			buf.push_back(':');
			json_generator(&(v->get_json_object_arr_elem(i).get_value()), buf);
		}
		buf.push_back('}');
		break;
	default:
		break;
	}
}

