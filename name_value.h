#pragma once
#include "simple_vector.h"

class json_value;

template<typename Name = simple_vector<char>, typename Value = json_value>
class json_obj
{
public:
	json_obj() : _name(0), _value(0) {}
	json_obj(const json_obj &jo) { _name = new Name(*jo._name); _value = new Value(*jo._value); }
	~json_obj() { delete _value; delete _name; }

	const json_obj &operator=(json_obj jo) { swap(jo); return *this; }

	Name &get_name() const { return *_name; }
	Value &get_value() const { return *_value; }

	void set_name(const Name &name) { _name = new Name(name); }
	void set_value(const Value &value) { _value = new Value(value); }

private:
	void swap(json_obj jo);
	Name *_name;
	Value *_value;
};

template<typename Name, typename Value>
inline void json_obj<Name, Value>::swap(json_obj jo)
{
	_name = jo._name;
	_value = jo._value;
}
