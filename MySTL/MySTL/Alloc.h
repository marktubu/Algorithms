#pragma once

#include <cstdlib>

namespace MySTL {
	//�ռ�������

	class alloc{
	private:
		//8�ֽڶ���
		enum { ALIGN = 8 };
		//�����������nodeΪ128�ֽ�
		enum { MAXBYTES = 128 };
		//����������Ŀ
		enum { NFREELISTS = 16 }; //(MAXBYTES / ALIGN)
		enum { NOBJS = 20 };

	private:
		union obj {
			union obj* next;
			char client[1];
		};

		//16�����������head�ڵ�
		static obj* free_list[NFREELISTS];

	private:
		//�ڴ�ؿ��в��ֵĿ�ʼ
		static char* start_free;
		//�ڴ�ؿ��в��ֵĽ���
		static char* end_free;
		static size_t heap_size;

	private:
		//8�ֽڶ���
		static size_t ROUND_UP(size_t bytes) {
			return ((bytes+ (size_t)ALIGN - 1) & ~((size_t)ALIGN - 1));
		}

		//���bytes��Ӧ��free_list
		static size_t FREELIST_INDEX(size_t bytes) {
			return (((bytes)+(size_t)ALIGN - 1) / (size_t)ALIGN - 1);
		}

		//ĳһ��������ռ䲻�㣬���ڴ�������·���
		static void* refill(size_t n);

		//��չ�ڴ��
		static char* chunk_alloc(size_t size, size_t& nobjs);

	public:
		//����
		static void* allocate(size_t bytes);
		//�ͷ�
		static void deallocate(void* ptr, size_t bytes);
		//���·���
		static void* reallocate(void* ptr, size_t old_sz, size_t new_sz);
	};
}