#pragma once
//================================
//�������ļ���¼Ϊ�����ķ���
//�����ļ��������㷨���ڴ�Ϊ����
//��¼δ��ʹ�õ���������
//Author: shinsya
//================================
#ifndef RECYCLE_MEMV_H
#define RECYCLE_MEMV_H
#define OBJ_ID_START 1

#define DebugInvoke if(debug_s)

#define table_getByOffset(object_id,offset) table_get(object_id,offset,0,true)
#define table_getByLength(object_id,length) table_get(object_id,0,length,false)
#define setTableObject_offset(object_id,table,offset) setTableObject(object_id,table,offset,table->length)
#define setTableObject_length(object_id,table,length) setTableObject(object_id,table,table->local_ptr,length)

#include <stdio.h>
#include <new>

struct mem_table
{
	long local_ptr;
	int length;

	mem_table* last;
	mem_table* next;
};

struct mem_object
{
	mem_table* list;
	int id;
	int table_count;
	int length_count;

	mem_object* last;
	mem_object* next;
};

bool debug_s = true;
int object_count = 0;
int created_count = 0;
mem_object* obj_table = 0;

mem_object* obj_getByID(int id) {	//�ڲ�������ͨ��id��ö����ڴ��ַ
	if (id > OBJ_ID_START + object_count - 1)return 0;
	mem_object* loop_t = obj_table;
	for (int i = 0; i < object_count; i++) {	//��ͳ�����Ե㵽Ϊֹ
		if (loop_t->id == id)return loop_t;		//��ͳ�����ǽ��жϵ�
		loop_t = loop_t->next;
	}
	return 0;
}


//��ȡtable(�ӱ�)�������������ɺ궨������
mem_table* table_get(int object_id, long offset, int length, bool offset_) {
	mem_object* tmp = obj_getByID(object_id);
	if (!tmp)return 0;
	mem_table* tmp_n = tmp->list;
	for (int i = 0; i < tmp->table_count; i++) {
		switch (offset_)
		{
		case true:
			if (tmp_n->local_ptr == offset)return tmp_n;
			break;
		default:
			if (tmp_n->length == length)return tmp_n;
			break;
		}
		tmp_n = tmp_n->next;
	}
	return 0;
}

mem_table* table_acquire(int object_id, int length) {

}

//�ͷŶ����е�ĳ���ӱ�
void freeRecycleTable(mem_object* object, mem_table* table) {	//��Ҫ�ṩ�����ĵ�ַ��Ϊ�˰�ȫ�����⵱ǰ��¼���ӱ��׵�ַ�ǽ��ͷŵ��ڴ�
	if (!object || !table)return;
	//��ȫ���1
	if (table_getByOffset(object->id, table->local_ptr) != table)return;
	
	//��ȫ���2 (������������)
	if (object->list == table) {
		if (table->next = table) {
			object->list = 0;
			object->length_count = table->length;
			object->table_count = 1;
		}
		else {
			object->list = table->next;
		}
	}
	//�Դ�������Ϣ��Ӧ�޸�
	object->length_count -= table->length;
	object->table_count--;

	delete table;
}

//�趨������ֵ(����ƫ�ƣ����ݳ���)
void setTableObject(int object_id, mem_table* table,long offset, int length) {
	if (!table)return;
	if (!length) {	//length == 0
		freeRecycleTable(obj_getByID(object_id), table);
		return;
	}
	mem_object* tmp = obj_getByID(object_id);
	if (!tmp)return;
	tmp->length_count -= (table->length - length);
	table->length = length;
	table->local_ptr = offset;
	mem_table* tmp2 = table_getByOffset(object_id, table->local_ptr + table->length);	//�ж����޽���������,�Խ��кϲ�
	if ((tmp2 == 0) || (tmp2 == table))return;
	table->length += tmp2->length;	//�����ټ�������Ϊ�ͷű��ϲ��ĺ���ʱ���һ�γ����ܺͣ�������ﲹ��
	tmp->length_count += tmp2->length;
	freeRecycleTable(tmp, tmp2);
}

//�����������ӱ�
void table_new(int object_id, long offset, int length) {
	mem_object* obj_tmp = obj_getByID(object_id);
	if (!obj_tmp)return;
	mem_table* table_tmp = 0;
	try { table_tmp = new mem_table(); }
	catch (std::bad_alloc) {
		DebugInvoke printf("Debug: recycle model exception (memory alloc exception)\n");
		return;
	}
	obj_tmp->table_count++;	//�����¼�ӱ��� +1

	if (obj_tmp->list) {
		table_tmp->next = obj_tmp->list->next;
		table_tmp->last = obj_tmp->list;
		obj_tmp->list->next->last = table_tmp;
		obj_tmp->list->next = table_tmp;
	}
	else
	{
		table_tmp->next = table_tmp;
		table_tmp->last = table_tmp;
	}
	obj_tmp->list = table_tmp;

	setTableObject(object_id, table_tmp, offset, length);
}


//�����ռ���ն���
int createRecycleTableObject() {
	mem_object* create_t;

	try{ create_t = new mem_object(); }	//��ʼ�������ڴ�
	catch (std::bad_alloc) { return -1; }

	create_t->id = created_count + OBJ_ID_START;	//Ϊ�������Ψһid ��id��ͬһʵ��������Ψһ����ʹ���ͷ�Ҳ�����ٱ�����
	created_count++;
	object_count++;	//���еĶ�����+1

	if (obj_table) {	//��������
		create_t->next = obj_table->next;
		create_t->last = obj_table;
		obj_table->next->last = create_t;
		obj_table->next = create_t;
	}
	else
	{
		create_t->next = create_t;
		create_t->last = create_t;
	}
	obj_table = create_t;
	return obj_table->id;	//���ض���id
}

//�ͷſռ���ն���
void freeRecycleTableObject(int id) {
	mem_object* obj = obj_getByID(id);
	if (!obj)return;
	mem_table* loop_t = obj->list, * loop_t2 = obj->list;
	for (int i = 0; i < obj->table_count; i++) {	//�ͷſռ��
		loop_t2 = loop_t->next;
		delete loop_t;
		loop_t = loop_t2;
	}
	//�������ָ����¸��ڵ��������ʾĿǰ����һ������
	if (obj->next == obj) {
		obj_table = 0;
	}
	else
	{
		obj->last->next = obj->next;
		obj->next->last = obj->last;
		obj_table = obj->next;
	}
	delete obj;	//�ͷŶ���
	object_count--;
}

void printRecycleObjectInfo(int object_id) {
	mem_object* tmp = obj_getByID(object_id);
	if (!tmp) {
		printf("Recycle: unkown object (id:%d)\n",object_id);
		return;
	}
	printf("*******Recycle Info*******\nID:%d\tTable:%d\nLength:%d\n**************************\n", tmp->id, tmp->table_count, tmp->length_count);
}

#endif // !RECYCLE_MEMV_H
