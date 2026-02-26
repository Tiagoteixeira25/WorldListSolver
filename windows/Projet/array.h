#pragma once
#include<stdlib.h>
#define STRCAT_SUB(A,B) A ## B
#define STRCAT(A,B) STRCAT_SUB(A,B)
#define T_ARR(T) STRCAT(T,_arr)
#define T_ARR_STRUCT(T) STRCAT(struc,t T_ARR(T))
#define T_ARR_STRUCT_PTR(T) STRCAT(T_ARR_STRUCT(T),*)


#define DJ_BUILD_ARR(T)    T_ARR_STRUCT(T)\
					{T* data;size_t size,capacity;};\
				void STRCAT(T_ARR(T),_add) (T_ARR_STRUCT_PTR(T) arr,T e){\
						if(arr->capacity==arr->size){\
							arr->capacity+=8;\
							arr->data=(T*)realloc(arr->data,arr->capacity);\
							if(!arr->data)	exit(-1);\
						}\
					arr->data[arr->size++]=e;\
				}
