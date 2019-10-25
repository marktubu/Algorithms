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


	template<class T>
	class shared_ptr
	{
	public:
		typedef T element_type;
		typedef T* raw_pointer;
		typedef size_t size_type;

	private:
		template<class Type>
		using ref_t = Impl::ref_t<Type>;
	public:
		shared_ptr(raw_pointer p) :_ref(new ref_t<T>(p)) {}

		template<class D>
		shared_ptr(raw_pointer p, D del) :_ref(new ref_t<T>(p, del)) {}

		shared_ptr(const shared_ptr& sp)
		{
			copy_ref(sp._ref);
		}

		~shared_ptr() { decrease_ref(); }

		shared_ptr& operator=(const shared_ptr& sp)
		{
			if (this != &sp)
			{
				decrease_ref();
				copy_ref(sp._ref);
			}
			
			return *this;
		}
	public:
		ref_t<T>* _ref;

	public:
		raw_pointer operator->() { return get(); }
		element_type& operator*() { return *get(); }
		const raw_pointer operator->()const { return get(); }
		const element_type& operator*()const { return *get(); }

		operator bool() { return get() != nullptr; }

		void reset(raw_pointer p)
		{
			if (p != get())
			{
				decrease_ref();
			}
		}

		raw_pointer get() { return _ref->get_data(); }
		const raw_pointer get()const { return _ref->get_data(); }
		size_type use_count() { return _ref->count(); }
	private:
		//��������
		void copy_ref(ref_t<T>* ref)
		{
			_ref = ref;
			++(*ref);
		}

		//������
		void decrease_ref()
		{
			if (_ref->get_data())
			{
				--(*_ref);
				if (use_count() == 0)
				{
					delete _ref;
				}
			}
		}
	};

	template <class T, class... Args>
	shared_ptr<T> make_shared(Args&& ... args)
	{
		return shared_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<class D, class T>
	D* get_deleter(const shared_ptr<T>& sp)
	{
		return sp._ref->_deleter;
	}

	template<class T1, class T2>
	bool operator == (const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) 
	{
		return lhs.get() == rhs.get();
	}

	template<class T>
	bool operator == (const shared_ptr<T>& sp, nullptr_t p) 
	{
		return sp.get() == p;
	}

	template<class T>
	bool operator == (nullptr_t p, const shared_ptr<T>& sp) 
	{
		return sp == p;
	}

	template<class T1, class T2>
	bool operator != (const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) 
	{
		return !(lhs == rhs);
	}

	template<class T>
	bool operator != (const shared_ptr<T>& sp, nullptr_t p) 
	{
		return !(sp == p);
	}

	template<class T>
	bool operator != (nullptr_t p, const shared_ptr<T>& sp) 
	{
		return !(sp == p);
	}
}