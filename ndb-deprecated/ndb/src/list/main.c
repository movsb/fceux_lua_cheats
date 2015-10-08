#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#define array_size(array) (sizeof(array)/sizeof(array[0]))

//自己定义的数据结构体类型
//要包含list_s结构体类型变量
//list_s变量位置任意
typedef struct{
	int num;
	list_s list_entry;
}my_data;

int main(void)
{
	list_s list_head;
	//为了简便,我直接使用局部数组数据
	my_data md[16];
	int it1,it2;
	//这里保存了4种方式遍历时的函数指针相关信息
	struct{
		void (*insert)(list_s*,list_s*);
		list_s* (*remove)(list_s*);
		char* msg;
	} func_ptr[] = {
		{list->insert_head,list->remove_head,"头头"},
		{list->insert_head,list->remove_tail,"头尾"},
		{list->insert_tail,list->remove_head,"尾头"},
		{list->insert_tail,list->remove_tail,"尾尾"},
	};

	for(it2=0; it2<array_size(func_ptr); it2++){
		printf("\n测试第%d种方式,方式:%s:\n", it2+1, func_ptr[it2].msg);
		//必须进行初始化操作
		list->init(&list_head);
		//测试数据插入双向链表,采用2种方式之一
		for(it1=0; it1<array_size(md); it1++){
			md[it1].num = it1;
			func_ptr[it2].insert(&list_head, &md[it1].list_entry);
		}
		//测试数据脱离双向链表,采用2种方式之一
		while(!list->is_empty(&list_head)){
			//得到被移除的链表结点指针
			list_s* plist = func_ptr[it2].remove(&list_head);
			//通过链表结点指针得到包含该链表结构的结构体的指针
			my_data* pmd = list_data(plist, my_data, list_entry);
			//输出相关数据,验证正确性
			printf("md[0x%08X].num = %d\n", pmd, pmd->num);
		}
	}
	return 0;
}
