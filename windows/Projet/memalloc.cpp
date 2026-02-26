#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#ifdef NC_DEBUG
namespace nc_debug {
	
	struct ptr_arr {
		void** data = 0;
		size_t size = 0, capacity = 0;


		void add(void* e) {
			void** temp;
			printf("size %llu   capacity %llu\n", size, capacity);
			if (size == capacity) {
				capacity += 8;
				temp = (void**)malloc(capacity * sizeof(void*));
				if (temp==NULL) {
					printf("Memory Allocation Error : Tried to allocate %llu bytes", capacity * sizeof(void*));
					exit(-1);
				}
				memcpy(temp, data, size * sizeof(void*));
				free(data);
				data = temp;
			}
			data[size++] = e;
		}

		void del(void* e) {
			size_t i = 0;
			for(i;i!=size;i++){
				if (data[i] == e) {
					break;
				}
			}
			if (i == size) { return; }
			const size_t s = size -1 ;
			if (s == 0) {
				free(data);
				data = 0;
				size = 0;
				capacity = 0;
				return;
			}
			void** temp = (void**)malloc(sizeof(void*) * s);
			if(!temp){
				printf("Memory Allocation Error : Tried to allocate %llu bytes", sizeof(void*) * s);
				exit(-1);
			}
			memcpy(temp, data, sizeof(void*) * i);
			memcpy(temp+i, data+i+1, sizeof(void*) * (s-i));
			free(data);
			data = temp;
			size=s;
			capacity = s;
		}

		void print(){
			for (size_t i =0; i != size; i++) {
				printf("%p\n",data[i]);
			}
		}

		void destroy() {
			free(data); size = 0; capacity = 0;
		}
	};
	ptr_arr allocs;
};


void memfree(void* ptr) {
	printf("freeing %p\n", ptr);
	nc_debug::allocs.del(ptr);
	free(ptr);
}
#endif
void* memalloc(size_t s) {
	
	void* ret = malloc(s);
	if (!ret) {
		printf("Memory Allocation Error : Tried to allocate %llu bytes", s);
		exit(-1);
	}
#ifdef NC_DEBUG
	nc_debug::allocs.add(ret);
	printf("adding %p size %llu\n", ret,s);
#endif
	return ret;
}


#ifndef NC_DEBUG
#define memfree(PTR) free(PTR)
#endif 





#ifdef _MEMALLOC_WIN32 



#ifndef DEBUG

void* memalloc2(size_t s) {
	void* ret = malloc(s);
	if (!ret) {
		printf("Memory Allocation Error : Tried to allocate %llu bytes", s);
		exit(-1);
	}
	return ret;
}
#endif
#endif