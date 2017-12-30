# simple_json
一个简单的JSON库。

## JSON语法

关于JSON的语法和自动机可以在这里查阅到（[json.org](http://json.org/json-zh.html)和[rfc7159](https://www.rfc-editor.org/info/rfc7159)）。

## 主要文件

`simple_vector.h`
`name_value.h`
`simplejson.h`
`simplejson.cpp`
`test.cpp`

## 数据结构

`json_value`，主要数据结构，用于存储处理过JSON字符串得到的数据。

`json_context`，保存未经处理的JSON字符串的指针。

`json_type`，枚举类型，用于标识`json_value`的类型。

`parse_ret_type`，枚举类型，用于标识`parse`函数的返回值。

`simple_vector`，模板类，自制的简单`std::vector`。

`name_value`，模板类，`JSON Object`。

## 使用方法

将文件放在同一文件夹中，然后`#include "simplejson.h"`。便可使用内部函数。

如：
```
char *json = "{        
	"Image": {            
		"Width":  800, 
		"Height" : 600, 
		"Title" : "View from 15th Floor", 
		"Thumbnail" : {                
			"Url":    "http://www.example.com/image/481989943", 
			"Height" : 125, 
			"Width" : 100            
		}, 
		"Animated" : false, 
		"IDs" : [116, 943, 234, 38793]          
	}      
}"

json_value v;
json_parse(&v, json);
int num = v.get_json_object_arr_elem(0).get_value().get_json_object_arr_elem(0).get_value().get_json_number(); // 800
```



