#pragma warning(disable:4996)
#include <iostream>
#include <iomanip>
#include <cstdio>

#include "simplejson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual) \
	do \
	{ \
		test_count++; \
		if (equality) \
			test_pass++; \
		else \
		{ \
			std::clog << __FILE__ << ":" << __LINE__ << " expect: " << expect << " actual: " << actual << std::endl; \
			main_ret = 1; \
		} \
	} while (0)

#define EXPECT_EQ_RET_TYPE(expect, actual)	EXPECT_EQ_BASE(expect == actual, static_cast<int>(expect), static_cast<int>(actual))

#define EXPECT_EQ_VALUE_TYPE(expect, actual) EXPECT_EQ_BASE(expect == actual, static_cast<int>(expect), static_cast<int>(actual))

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(expect == actual, expect, actual)

#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(simple_vector<char>(expect, expect + (sizeof(expect) - 1)) == actual, expect, actual)

#define TEST_NUMBER(n, json) \
	do \
	{ \
		json_value v; \
		v.set_json_type(json_type::JSON_TRUE); \
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, json)); \
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_type()); \
		EXPECT_EQ_DOUBLE(n, v.get_json_number()); \
	} while (0)

#define TEST_STRING(str, json) \
	do \
	{ \
		json_value v; \
		v.set_json_type(json_type::JSON_TRUE); \
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, json)); \
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_type()); \
		EXPECT_EQ_STRING(str, v.get_json_string()); \
	} while (0)

#define TEST_STRING_WITH_NULL(str, json) \
	do \
	{ \
		json_value v; \
		v.set_json_type(json_type::JSON_TRUE); \
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, json)); \
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_type()); \
		const char ch = v.get_json_string()[0]; \
		EXPECT_EQ_BASE(std::memcmp(str, &ch, sizeof(str) - 1), str, v.get_json_string()); \
	} while (0)

#define TEST_ERROR(error, json) \
	do \
	{ \
		json_value v; \
		v.set_json_type(json_type::JSON_TRUE); \
		EXPECT_EQ_RET_TYPE(error, json_parse(&v, json)); \
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NULL, v.get_json_type()); \
	} while (0)

static void test_parse_null()
{
	json_value v;
	v.set_json_type(json_type::JSON_FALSE);
	EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "null"));
	EXPECT_EQ_VALUE_TYPE(json_type::JSON_NULL, v.get_json_type());
}

static void test_parse_true()
{
	json_value v;
	v.set_json_type(json_type::JSON_NULL);
	EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "true"));
	EXPECT_EQ_VALUE_TYPE(json_type::JSON_TRUE, v.get_json_type());
}

static void test_parse_false()
{
	json_value v;
	v.set_json_type(json_type::JSON_NULL);
	EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "false"));
	EXPECT_EQ_VALUE_TYPE(json_type::JSON_FALSE, v.get_json_type());
}

static void test_parse_number()
{
	TEST_NUMBER(0, "0");
	TEST_NUMBER(-0, "-0");
	TEST_NUMBER(1, "1");
	TEST_NUMBER(-1, "-1");
	TEST_NUMBER(0.99, "0.99");
	TEST_NUMBER(1.99, "1.99");
	TEST_NUMBER(3.1415, "3.1415");

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	TEST_NUMBER(1.0000000000000004, "1.0000000000000004");
	TEST_NUMBER(2, "2");
	TEST_NUMBER(-2, "-2");

	TEST_NUMBER(3, "3");
	TEST_NUMBER(4, "4");
	TEST_NUMBER(5, "5");
	TEST_NUMBER(6, "6");
	TEST_NUMBER(23, "23");

	TEST_NUMBER(4.9e-324, "4.9e-324"); // Min. subnormal positive double
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); // Max. subnormal double
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); // Min. normal positive double
	TEST_NUMBER(1.7976931348623157e308, "1.7976931348623157e308"); // Max. Double

}

static void test_parse_string()
{
	TEST_STRING("", "\"\"");
	TEST_STRING("A", "\"A\"");
	TEST_STRING("\"", "\"\\\"\"");
	TEST_STRING("\\", "\"\\\\\"");
	TEST_STRING("/", "\"\\/\"");
	TEST_STRING("\b", "\"\\b\"");
	TEST_STRING("\f", "\"\\f\"");
	TEST_STRING("\n", "\"\\n\"");
	TEST_STRING("\r", "\"\\r\"");
	TEST_STRING("\t", "\"\\t\"");

	TEST_STRING("hello world", "\"hello world\"");

	TEST_STRING("\xE4\xB8\xA5", "\"\\u4E25\"");

	TEST_STRING("\xF0\x90\x80\x80", "\"\\uD800\\uDC00\"");

	TEST_STRING("\xF4\x8F\xBF\xBF", "\"\\uDBFF\\uDFFF\"");

	TEST_STRING_WITH_NULL("hello\0world", "\"hello\\u0000world\"");
}

static void test_parse_array()
{
	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "[ ]"));
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_ARRAY, v.get_json_type());
	}

	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "[1, true, false, null, \"hello\"]"));

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, (v.get_json_array_elem(0)).get_json_type());
		EXPECT_EQ_DOUBLE(1, (v.get_json_array_elem(0)).get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_TRUE, (v.get_json_array_elem(1)).get_json_type());
		
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_FALSE, (v.get_json_array_elem(2)).get_json_type());
		
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NULL, (v.get_json_array_elem(3)).get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, (v.get_json_array_elem(4)).get_json_type());
		EXPECT_EQ_STRING("hello", (v.get_json_array_elem(4)).get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_ARRAY, v.get_json_type());
	}

	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "[1.23, [ true, false ], null]"));

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, (v.get_json_array_elem(0)).get_json_type());
		EXPECT_EQ_DOUBLE(1.23, (v.get_json_array_elem(0)).get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_ARRAY, (v.get_json_array_elem(1)).get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_TRUE, (v.get_json_array_elem(1)).get_json_array_elem(0).get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_FALSE, (v.get_json_array_elem(1)).get_json_array_elem(1).get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NULL, (v.get_json_array_elem(2)).get_json_type());
		
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_ARRAY, v.get_json_type());
	}
}

static void test_parse_object()
{
	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "{ }"));
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_OBJECT, v.get_json_type());
	}

	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "{ \"precision\": \"zip\", \"Latitude\": 37.7668, \"Longitude\": -122.3959, \"Address\": \"\", \"City\": \"SAN FRANCISCO\", \"State\": \"CA\", \"Zip\": 94107, \"Country\": \"US\" }"));

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(0).get_value().get_json_type());
		EXPECT_EQ_STRING("zip", v.get_json_object_arr_elem(0).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(1).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(37.7668, v.get_json_object_arr_elem(1).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(2).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(-122.3959, v.get_json_object_arr_elem(2).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(3).get_value().get_json_type());
		EXPECT_EQ_STRING("", v.get_json_object_arr_elem(3).get_value().get_json_string());
		
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(4).get_value().get_json_type());
		EXPECT_EQ_STRING("SAN FRANCISCO", v.get_json_object_arr_elem(4).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(5).get_value().get_json_type());
		EXPECT_EQ_STRING("CA", v.get_json_object_arr_elem(5).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(6).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(94107, v.get_json_object_arr_elem(6).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(7).get_value().get_json_type());
		EXPECT_EQ_STRING("US", v.get_json_object_arr_elem(7).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_OBJECT, v.get_json_type());
	}

	//{        
	//	"Image": {            
	//		"Width":  800, 
	//		"Height" : 600, 
	//		"Title" : "View from 15th Floor", 
	//		"Thumbnail" : {                
	//			"Url":    "http://www.example.com/image/481989943", 
	//			"Height" : 125, 
	//			"Width" : 100            
	//		}, 
	//		"Animated" : false, 
	//		"IDs" : [116, 943, 234, 38793]          
	//	}      
	//}

	{
		json_value v;
		v.set_json_type(json_type::JSON_NULL);
		EXPECT_EQ_RET_TYPE(parse_ret_type::JSON_PARSE_OK, json_parse(&v, "{ \"Image\": { \"Width\": 800, \"Height\": 600, \"Title\": \"View from 15th Floor\", \"Thumbnail\": { \"Url\": \"http:\\/\\/www.example.com\\/image\\/481989943\", \"Height\": 125, \"Width\": 100 }, \"Animated\" : false, \"IDs\": [116, 943, 234, 38793] } }"));

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_OBJECT, v.get_json_object_arr_elem(0).get_value().get_json_type());
		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(0).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(800, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(0).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(1).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(600, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(1).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(2).get_value().get_json_type());
		EXPECT_EQ_STRING("View from 15th Floor", v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(2).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_OBJECT, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_STRING, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(0).get_value().get_json_type());
		EXPECT_EQ_STRING("http://www.example.com/image/481989943", v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(0).get_value().get_json_string());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(1).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(125, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(1).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(2).get_value().get_json_type());
		EXPECT_EQ_DOUBLE(100, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(3).get_value().get_json_object_arr_elem(2).get_value().get_json_number());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_FALSE, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(4).get_value().get_json_type());

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_ARRAY, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(5).get_value().get_json_type());
		int arr[4] = { 116, 943, 234, 38793 };
		for (size_t i = 0; i < 4; i++)
		{
			EXPECT_EQ_VALUE_TYPE(json_type::JSON_NUMBER, v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(5).get_value().get_json_array_elem(i).get_json_type());
			EXPECT_EQ_DOUBLE(arr[i], v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(5).get_value().get_json_array_elem(i).get_json_number());
		}

		EXPECT_EQ_VALUE_TYPE(json_type::JSON_OBJECT, v.get_json_type());
	}
}

static void test_json_generator()
{
	json_value v;
	v.set_json_type(json_type::JSON_NULL);
	json_parse(&v, "{ \"Image\": { \"Width\": 800, \"Height\": 600, \"Title\": \"View from 15th Floor\", \"Thumbnail\": { \"Url\": \"http:\\/\\/www.example.com\\/image\\/481989943\", \"Height\": 125, \"Width\": 100 }, \"Animated\" : false, \"IDs\": [116, 943, 234, 38793] } }");
	simple_vector<char> buf;
	json_generator(&v, buf);
	EXPECT_EQ_STRING("{\"Image\":{\"Width\":800,\"Height\":600,\"Title\":\"View from 15th Floor\",\"Thumbnail\":{\"Url\":\"http:\\/\\/www.example.com\\/image\\/481989943\",\"Height\":125,\"Width\":100},\"Animated\":false,\"IDs\":[116,943,234,38793]}}", buf);
}

static void test_json_access_by_name()
{
	json_value v;
	v.set_json_type(json_type::JSON_NULL);
	json_parse(&v, "{ \"precision\": \"zip\", \"Latitude\": 37.7668, \"Longitude\": -122.3959, \"Address\": \"\", \"City\": \"SAN FRANCISCO\", \"State\": \"CA\", \"Zip\": 94107, \"Country\": \"US\" }");
	EXPECT_EQ_STRING("zip", v.get_json_value_by_name("precision")->get_json_string());
	EXPECT_EQ_DOUBLE(37.7668, v.get_json_value_by_name("Latitude")->get_json_number());
	EXPECT_EQ_DOUBLE(-122.3959, v.get_json_value_by_name("Longitude")->get_json_number());
	EXPECT_EQ_STRING("", v.get_json_value_by_name("Address")->get_json_string());
	EXPECT_EQ_STRING("SAN FRANCISCO", v.get_json_value_by_name("City")->get_json_string());
	EXPECT_EQ_STRING("CA", v.get_json_value_by_name("State")->get_json_string());
	EXPECT_EQ_DOUBLE(94107, v.get_json_value_by_name("Zip")->get_json_number());
	EXPECT_EQ_STRING("US", v.get_json_value_by_name("Country")->get_json_string());
}

static void test_parse_invalid_value()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID, "nul");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID, "?");

	// number
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID, "+0");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID, "12.a");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID, "1.23e`");
}

static void test_parse_not_root_singular()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_ROOT_NOT_SINGULAR, "null a");
}

static void test_parse_number_too_big()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_NUMBER_TOO_BIG, "1.7976931348623157e309");
	TEST_ERROR(parse_ret_type::JSON_PARSE_NUMBER_TOO_BIG, "1e309");
}

/*
JSON_PARSE_INVALID_STRING_ESCAPE
JSON_PARSE_INVALID_STRING_CHAR
JSON_PARSE_INVALID_UNICODE_HEX
JSON_PARSE_INVALID_UNICODE_SURROGATE
*/

static void test_parse_invalide_string_escape()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\a\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\1\"");
}

static void test_parse_invalide_string_char()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_STRING_CHAR, "\"\x19\"");
}

static void test_parse_invalide_unicode_hex()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u4Z25\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\uDG00\"");
}

static void test_parse_invalide_unicode_surrogate()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800uDC00\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\vDC00\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_invalide_array_char()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_ARRAY_CHAR, "[1 a ]");
}

static void test_parse_invalide_object_char()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_OBJECT_CHAR, "{\"a\" : 1 a }");
}

static void test_parse_invalide_separator()
{
	TEST_ERROR(parse_ret_type::JSON_PARSE_INVALID_SEPARATOR, "{\"a\" ; 1 a }");
}

static void test_parse()
{
	test_parse_null();
#if 1
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_array();
	test_parse_object();
	test_json_generator();
	test_json_access_by_name();

	test_parse_invalid_value();
	test_parse_not_root_singular();
	test_parse_invalide_string_escape();
	test_parse_invalide_string_char();
	test_parse_invalide_unicode_hex();
	test_parse_invalide_unicode_surrogate();
	test_parse_invalide_array_char();
	test_parse_invalide_object_char();
	test_parse_invalide_separator();
#endif // 0
}

int main()
{
	test_parse();

	std::cout << test_pass << "/" << test_count << " (" << std::setprecision(3) << test_pass * 100.0 / test_count << ") passed" << std::endl;
	system("PAUSE");
	return main_ret;
}