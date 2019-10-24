#pragma once

#include <utility>

#include "Ref.h"

namespace MySTL
{
	template<class _T>
	class cow_ptr;

	template<class T>
	struct default_delete
	{
		void operator()(T* ptr) { if (ptr) delete ptr; }
	};

	template<class T>
	struct default_delete<T[]>
	{
		void operator()(T* ptr) { if (ptr) { delete[] ptr; } }
	};

	template<class T,class D = default_delete<T>>
	class unique_ptr
	{
	public:
		typedef T element_type;
		typedef D deleter_type;
		typedef element_type* pointer;
	public:
		explicit unique_ptr(T* data = nullptr) :_data(data) {}
		unique_ptr(T* data, deleter_type del) :_data(data), deleter(del) {}
		
		//ͨ����ֵ������Ϊ���캯���Ĳ���
		unique_ptr(unique_ptr&& up) :_data(nullptr)
		{
			MySTL::swap(_data, up.data);
		}

		//ͨ����ֵ���ø�ֵ
		unique_ptr& operator=(unique_ptr&& up)
		{
			if (&up != this)
			{
				clean();
				MySTL::swap(*this, up);
			}
			return *this;
		}

		//���ÿ�������
		unique_ptr(const unique_ptr&) = delete;
		//���ø�ֵ
		unique_ptr& operator = (const unique_ptr&) = delete;

		~unique_ptr() { clean(); }

		const pointer get()const { return _data; }
		pointer get() { return _data; }
		deleter_type& get_deleter() { return deleter; }
		const deleter_type& get_deleter()const { return deleter; }

		//��ʵ�ָ����Ͷ���bool���͵���ʽת��
		operator bool()const { return get() != nullptr; }

		//�ͷ�unique_ptrָ��Ķ���
		pointer release()
		{
			T* p = nullptr;
			MySTL::swap(p, _data);
			return p;
		}

		//����unique_ptrָ��Ķ���
		void reset(pointer p = pointer())
		{
			clean();
			_data = p;
		}

		void swap(unique_ptr& up) { MySTL::swap(_data, up._data); }

		//�����û�ȡָ��Ķ���
		const element_type& operator*()const { return *_data; }
		//��ȡ�����ԭʼָ��
		const pointer operator->()const { return _data; }

		//�����û�ȡָ��Ķ���
		element_type& operator*(){ return *_data; }
		//��ȡ�����ԭʼָ��
		pointer operator->(){ return _data; }
	private:
		inline void clean()
		{
			deleter(_data);
			_data = nullptr;
		}

	private:
		pointer _data;
		deleter_type deleter;
	};

	template <class T, class D>
	void swap(unique_ptr<T, D>& x, unique_ptr<T, D>& y) {
		x.swap(y);
	}

	//����unique_ptr�Ƚ�
	template <class T1, class D1, class T2, class D2>
	bool operator == (const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) {
		return lhs.get() == rhs.get();
	}

	//��ָ����unique_ptr�Ƚ�
	template <class T, class D>
	bool operator == (const unique_ptr<T, D>& up, nullptr_t p) {
		return up.get() == p;
	}

	//��ָ����unique_ptr�Ƚ�
	template <class T, class D>
	bool operator == (nullptr_t p, const unique_ptr<T, D>& up) {
		return up.get() == p;
	}

	//����unique_ptr�Ƚ�
	template <class T1, class D1, class T2, class D2>
	bool operator != (const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) {
		return !(lhs == rhs);
	}

	//��ָ����unique_ptr�Ƚ�
	template <class T, class D>
	bool operator != (const unique_ptr<T, D>& up, nullptr_t p) {
		return up.get() != p;
	}

	//��ָ����unique_ptr�Ƚ�
	template <class T, class D>
	bool operator != (nullptr_t p, const unique_ptr<T, D>& up) {
		return up.get() != p;
	}

	template <class T, class... Args>
	unique_ptr<T> make_unique(Args&& ... args) {
		return unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}