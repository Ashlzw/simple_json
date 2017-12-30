#pragma once
#ifndef SIMPLE_VECTOR_H
#define SIMPLE_VECTOR_H

#include <utility>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <iostream>

template <typename T>
class simple_vector
{
	friend bool operator==(const simple_vector<T> &lhs, const simple_vector<T> &rhs)
	{
		if (lhs.size() != rhs.size())
			return false;
		for (size_t i = 0; i < lhs.size(); ++i)
			if (lhs[i] != rhs[i])
				return false;

		return true;
	}
	friend std::ostream &operator<<(std::ostream &os, const simple_vector &vec)
	{
		for (size_t i = 0; i < vec.size(); i++)
			os << vec[i] << " ";
		return os;
	}
public:
	simple_vector() : _elem(0), _first_free(0), _cap(0) {}
	simple_vector(const simple_vector &);
	simple_vector(simple_vector &&);
	simple_vector(const T *, const T *);
	simple_vector(std::initializer_list<T>);
	simple_vector(size_t cnt);
	simple_vector &operator=(simple_vector);
	simple_vector &operator=(simple_vector &&) noexcept;
	simple_vector &operator=(std::initializer_list<T>);
	~simple_vector() { simple_free(); }

	const T &operator[](size_t n) const;
	const T &back() const;
	const T &front() const;
	T &operator[](size_t n);
	T &back();
	T &front();

	const T *begin() const { return _elem; }
	const T *end() const { return _first_free; }

	T *begin() { return _elem; }
	T *end() { return _first_free; }

	size_t size() const { return _first_free - _elem; }
	size_t capacity() const { return _cap - _elem; }

	void push_back(const T & value) { chk_n_alloc(); alloc.construct(_first_free++, value); }
	void push_back(T && value) { chk_n_alloc(); alloc.construct(_first_free++, value); }
	void pop_back() { alloc.destroy(--_first_free); }
	T *insert(T * pos, const T &value);
	T *erase(T * pos);


private:
	std::allocator<T> alloc;

	void simple_free();
	void chk_n_alloc() { if (size() == capacity()) reallocate(size() ? 2 * size() : 1, size()); }
	void reallocate(size_t, size_t);
	std::pair<T*, T*> copy_n_alloc(const T*, const T*);
	void swap(simple_vector &);

	T *_elem;
	T *_first_free;
	T *_cap;
};

#endif // !SIMPLE_VECTOR_H

template<typename T>
inline void simple_vector<T>::simple_free()
{
	if (_elem)
	{
		for (auto p = _first_free; p != _elem;)
			alloc.destroy(--p);
		alloc.deallocate(_elem, _cap - _elem);
	}
}

template<typename T>
inline void simple_vector<T>::reallocate(size_t capacity, size_t size)
{
	T *newdata = alloc.allocate(capacity);

	T *dest = newdata;
	T *elem = _elem;

	for (size_t i = 0; i < size; ++i)
		alloc.construct(dest++, *elem++);

	simple_free();

	_elem = newdata;
	_first_free = dest;
	_cap = newdata + capacity;
}

template<typename T>
inline std::pair<T*, T*> simple_vector<T>::copy_n_alloc(const T *b, const T *e)
{
	T *data = alloc.allocate(e - b);

	return std::pair<T*, T*>(data, std::uninitialized_copy(b, e, data));
}

template<typename T>
inline void simple_vector<T>::swap(simple_vector & temp)
{
	T *elem = _elem;
	T *first_free = _first_free;
	T *cap = _cap;

	_elem = temp._elem;
	_first_free = temp._first_free;
	_cap = temp._cap;

	temp._elem = elem;
	temp._first_free = first_free;
	temp._cap = cap;
}

template<typename T>
inline simple_vector<T>::simple_vector(const simple_vector &sv)
{
	auto data = copy_n_alloc(sv.begin(), sv.end());
	_elem = data.first;
	_first_free = _cap = data.second;
}

template<typename T>
inline simple_vector<T>::simple_vector(simple_vector &&sv) : 
	_elem(sv._elem), _first_free(sv._first_free), _cap(sv._cap)
{
	sv._elem = sv._first_free = sv._cap = nullptr;
}

template<typename T>
inline simple_vector<T>::simple_vector(const T *b, const T *e)
{
	auto data = copy_n_alloc(b, e);
	_elem = data.first;
	_first_free = _cap = data.second;
}

template<typename T>
inline simple_vector<T>::simple_vector(std::initializer_list<T> il)
{
	auto data = copy_n_alloc(il.begin(), il.end());
	_elem = data.first;
	_first_free = _cap = data.second;
}

template<typename T>
inline simple_vector<T>::simple_vector(size_t cnt)
{
	_elem = _first_free = alloc.allocate(cnt);
	_cap = _elem + cnt;
	for (size_t i = 0; i < cnt; ++i)
		push_back(T());
}

template<typename T>
inline simple_vector<T> & simple_vector<T>::operator=(simple_vector sv)
{
	swap(sv);
	return *this;
}

template<typename T>
inline simple_vector<T> & simple_vector<T>::operator=(simple_vector &&sv) noexcept
{
	if (this != &rhs)
	{
		simple_free();
		_elem = sv._elem;
		_first_free = sv._first_free;
		_cap = sv._cap;
		sv._elem = sv._first_free = sv._cap = nullptr;
	}

	return *this;
}

template<typename T>
inline simple_vector<T> & simple_vector<T>::operator=(std::initializer_list<T> il)
{
	simple_vector<T> temp(il.begin(), il.end());
	swap(temp);
	return *this;
}

template<typename T>
inline const T & simple_vector<T>::operator[](size_t n) const
{
	if (n < size())
		return _elem[n];
	else
		throw std::out_of_range("index out of range!");
}

template<typename T>
inline const T & simple_vector<T>::back() const
{
	return operator[](size() - 1);
}

template<typename T>
inline const T & simple_vector<T>::front() const
{
	return operator[](0);
}

template<typename T>
inline T & simple_vector<T>::operator[](size_t n)
{
	return const_cast<T&>(static_cast<const simple_vector&>(*this)[n]);
}

template<typename T>
inline T & simple_vector<T>::back()
{
	return const_cast<T&>(static_cast<const simple_vector&>(*this).back());
}

template<typename T>
inline T & simple_vector<T>::front()
{
	return const_cast<T&>(static_cast<const simple_vector&>(*this).front());
}

template<typename T>
inline T * simple_vector<T>::insert(T * pos, const T & value)
{
	size_t s = pos - _elem;
	if (s <= size())
	{
		chk_n_alloc();
		alloc.construct(_first_free++, T());
		for (size_t i = size() - 1; i > s; --i)
			_elem[i] = _elem[i - 1];
		_elem[s] = value;
		return _elem + s;
	}
	else
		throw std::out_of_range("iterator out of range!");
}

template<typename T>
inline T * simple_vector<T>::erase(T * pos)
{
	size_t s = pos - _elem;
	if (s < size())
	{
		for (T * it = pos + 1; it != _first_free; ++it)
			*(it - 1) = *it;
		alloc.destroy(--_first_free);
		return _elem + s;
	}
	else
		throw std::out_of_range("iterator out of range!");
}