#pragma once

#include "Alloc.h"
#include "Constructor.h"

#include <cassert>
#include <new>

namespace MySTL
{
	template<class T>
	class allocator
	{
	public:
		typedef T			value_type;
		typedef T*			pointer;
		typedef const T*	const_pointer;
		typedef T&			reference;
		typedef const T&	const_reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;
	public:
		static T* allocate();
		static T* allocate(size_t n);
		static void deallocate(T* ptr);
		static void deallocate(T* ptr, size_t n);

		static void construct(T* ptr);
		static void construct(T* ptr, const T& value);
		static void destroy(T* ptr);
		static void destroy(T* first, T* last);
	};

	template<class T>
	T* allocator<T>::allocate()
	{
		return static_cast<T*>(alloc::allocate(sizeof(T)));
	}

	template<class T>
	T* allocator<T>::allocate(size_t n)
	{
		if (n == 0)return 0;
		return static_cast<T*>(alloc::allocate(sizeof(T) * n));
	}

	template<class T>
	inline void allocator<T>::deallocate(T* ptr)
	{
		alloc::deallocate(ptr, sizeof(T));
	}

	template<class T>
	inline void allocator<T>::deallocate(T* ptr, size_t n)
	{
		alloc::deallocate(ptr, sizeof(T) * n);
	}

	template<class T>
	inline void allocator<T>::construct(T* ptr)
	{
		new(ptr) T();
	}

	template<class T>
	inline void allocator<T>::construct(T* ptr, const T& value)
	{
		new (ptr) T(value);
	}

	template<class T>
	inline void allocator<T>::destroy(T* ptr)
	{
		ptr->~T();
	}

	template<class T>
	inline void allocator<T>::destroy(T* first, T* last)
	{
		for (; first != last; ++first)
		{
			first->~T();
		}
	}
}
