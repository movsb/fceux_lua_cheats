#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#define array_size(array) (sizeof(array)/sizeof(array[0]))

//�Լ���������ݽṹ������
//Ҫ����list_s�ṹ�����ͱ���
//list_s����λ������
typedef struct{
	int num;
	list_s list_entry;
}my_data;

int main(void)
{
	list_s list_head;
	//Ϊ�˼��,��ֱ��ʹ�þֲ���������
	my_data md[16];
	int it1,it2;
	//���ﱣ����4�ַ�ʽ����ʱ�ĺ���ָ�������Ϣ
	struct{
		void (*insert)(list_s*,list_s*);
		list_s* (*remove)(list_s*);
		char* msg;
	} func_ptr[] = {
		{list->insert_head,list->remove_head,"ͷͷ"},
		{list->insert_head,list->remove_tail,"ͷβ"},
		{list->insert_tail,list->remove_head,"βͷ"},
		{list->insert_tail,list->remove_tail,"ββ"},
	};

	for(it2=0; it2<array_size(func_ptr); it2++){
		printf("\n���Ե�%d�ַ�ʽ,��ʽ:%s:\n", it2+1, func_ptr[it2].msg);
		//������г�ʼ������
		list->init(&list_head);
		//�������ݲ���˫������,����2�ַ�ʽ֮һ
		for(it1=0; it1<array_size(md); it1++){
			md[it1].num = it1;
			func_ptr[it2].insert(&list_head, &md[it1].list_entry);
		}
		//������������˫������,����2�ַ�ʽ֮һ
		while(!list->is_empty(&list_head)){
			//�õ����Ƴ���������ָ��
			list_s* plist = func_ptr[it2].remove(&list_head);
			//ͨ��������ָ��õ�����������ṹ�Ľṹ���ָ��
			my_data* pmd = list_data(plist, my_data, list_entry);
			//����������,��֤��ȷ��
			printf("md[0x%08X].num = %d\n", pmd, pmd->num);
		}
	}
	return 0;
}
