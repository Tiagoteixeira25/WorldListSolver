#pragma once
#define STRCAT_SUB(A,B) A ## B
#define STRCAT(A,B) STRCAT_SUB(A,B)
#define T_LL(T) STRCAT(T,_ll)
#define T_LL_STRUCT(T) STRCAT(struc,t T_LL(T))
#define T_LL_STRUCT_PTR(T) STRCAT(T_LL_STRUCT(T),*)
#define T_LL_STRUCT_DPTR(T) STRCAT(T_LL_STRUCT(T),**)
#define T_LL_SIZE(T) sizeof(T_LL_STRUCT(T))
#ifndef ULL_ARR_SIZE
#define ULL_ARR_SIZE 1024
#endif
#define DJ_BUILD_LL(T)    T_LL_STRUCT(T)\
			{T arr[ULL_ARR_SIZE];size_t size;T_LL_STRUCT_PTR(T) next;};\
			void STRCAT(T_LL(T),_add) (T_LL_STRUCT_DPTR(T) t_ll,T e){\
				T_LL_STRUCT(T)* ll = *t_ll,*new_cell;\
				if(ll==NULL||ll->size==ULL_ARR_SIZE){\
						new_cell=(T_LL_STRUCT_PTR(T))malloc(T_LL_SIZE(T));\
						if(new_cell==NULL){exit(-1);}\
						new_cell->next = ll; \
						new_cell->size = 1; \
						* (new_cell->arr) = e; \
						* t_ll = new_cell;	\
						return ;\
				}\
				ll->arr[ll->size++]=e;\
			}\
			void STRCAT(T_LL(T),_pop) (T_LL_STRUCT_DPTR(T) t_ll,T* e){\
				T_LL_STRUCT(T)* ll = *t_ll;\
				*e=ll->arr[--ll->size];\
				if (ll->size == 0) {*t_ll = ll->next;free(ll);}}\




#define DJ_BUILD_LL_STRUCT(T)    T_LL_STRUCT(T)\
			{T_LL_STRUCT(T) arr[ULL_ARR_SIZE];size_t size;T_LL_STRUCT_PTR(T) next;};\
			void STRCAT(T_LL(T),_add) (T_LL_STRUCT_DPTR(T) t_ll,T_LL_STRUCT(T)* e){\
				T_LL_STRUCT(T)* ll = *t_ll,*new_cell;\
				if(ll==NULL||ll->size==ULL_ARR_SIZE){\
						new_cell=(T_LL_STRUCT_PTR(T))malloc(T_LL_SIZE(T));\
						if(new_cell==NULL){exit(-1);}\
						new_cell->next = ll; \
						new_cell->size = 1; \
						* (new_cell->arr) = *e; \
						* t_ll = new_cell;	\
						return ;\
				}\
				ll->arr[ll->size++]=*e;\
			}\
			void STRCAT(T_LL(T),_pop) (T_LL_STRUCT_DPTR(T) t_ll,T_LL_STRUCT(T)* e){\
				T_LL_STRUCT(T)* ll = *t_ll;\
				*e=ll->arr[--ll->size];\
				if (ll->size == 0) {*t_ll = ll->next;free(ll);}\
			}

