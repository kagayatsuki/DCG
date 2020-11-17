#pragma once
//�������ݿ�
//Author: shinsya
#ifndef CONFIG_H
#define CONFIG_H
#include <Windows.h>
#include <stdio.h>
#include "base64_variation.h"

#define Type_String 1
#define Type_Short 2
#define Type_Int 4
#define Type_Long 8
#define Type_Double 9

#define DebugInvolke if(debug_mode)

bool debug_mode = false;
char f_start = 12, f_name = 16, f_end = 18;

typedef struct config_item {	//flag byte: 12-item start; 16-name end; 18-item end; 
	char* item_name;
	char* value;
	//char value_type;
	
	int ptr_local;
	int datalen;

	config_item* last;
	config_item* next;
};

typedef struct config_info {
	int struct_id;
	char* filename;
	FILE* local;
	int item_count;
	config_item* item_list;

	config_info* last;
	config_info* next;
};

config_info* opened_obj = nullptr;
int default_id = 10;			//struct id start, not equal 0
int obj_count = 0;
int opened_count = 0;

void itemlist_free(config_item* init) {
	config_item* tmp = init, *tmp2;
	if (!init)return;
	while (tmp)
	{
		tmp2 = tmp;
		if ((tmp->last != tmp) && (tmp->next != tmp)) {	//Check only this one item
			tmp->last->next = tmp->next;
			tmp->next->last = tmp->last;
			tmp = tmp->next;
			if (tmp2->item_name)free(tmp2->item_name);
			if (tmp2->value)free(tmp2->value);
			free(tmp2);
		}
		else {											//Only one item then don't change 'last' and 'next'
			tmp = 0;
			if (tmp2->item_name)free(tmp2->item_name);
			if (tmp2->value)free(tmp2->value);
			free(tmp2);
		}
	}
	return;
}

//Free a file object
void obj_free(config_info* obj) {
	if (!obj)return;

	//free itemlist,file,name mem always
	itemlist_free(obj->item_list);
	if (obj->local)fclose(obj->local);
	if (obj->filename)free(obj->filename);

	if ((obj->next == obj) && (obj->last == obj)) {
		obj_count--;
		opened_obj = 0;
	}
	else {
		obj->last->next = obj->next;
		obj->next->last = obj->last;
		if (opened_obj == obj)opened_obj = obj->next;
		obj_count--;
	}
	free(obj);
	return;
}

//inside: create a object
config_info* obj_new() {
	config_info* tmp = (config_info*)malloc(sizeof(config_info));
	if (!tmp)return 0;
	memset(tmp, 0, sizeof(config_info));
	tmp->struct_id = default_id + opened_count;
	if (opened_obj) {
		tmp->last = opened_obj;
		tmp->next = opened_obj->next;
		opened_obj->next = tmp;
		tmp->next->last = tmp;
	}
	else {
		tmp->last = tmp;
		tmp->next = tmp;
	}
	opened_obj = tmp;
	opened_count++;
	obj_count++;
	return tmp;
}

//Inserting item to file; itemname&itemvalue provided by involker
int obj_insertitem(config_info* obj,char* itemname, char* itemvalue, char readmode, long local_ptr) {
	//��:��δ�ﵽ�������������Ӧ���base64�����Զ����ݽ��и��õĻ���
	//��:�Ա��������ѽ���bs64v����

	//���ڴ��ڳ�Ա�����ʼ��
	if (!obj)return -1;
	config_item* tmp = (config_item*)malloc(sizeof(config_item));
	if (!tmp)return -2;
	memset(tmp, 0, sizeof(config_item));
	if (obj->item_list == 0) {
		tmp->next = tmp;
		tmp->last = tmp;
	}
	else {
		tmp->next = obj->item_list->next;
		tmp->last = obj->item_list;
		tmp->next->last = tmp;
		obj->item_list->next = tmp;
	}
	obj->item_list = tmp;

	config_item* this_ = obj->item_list;		//��;�Ż���һ��������ŵ�ǰ�����ĳ�Ա�ڴ棬��ʡ����ռ�
	int namelen = strnlen_s(itemname, 512);
	int valuelen = strnlen_s((const char*)itemvalue, 512);

	//�Զ���ֵ
	tmp->ptr_local = local_ptr;
	this_->item_name = (char*)malloc(namelen + 1);
	this_->value = (char*)malloc(valuelen + 1);

	//���Ż�
	char* bs64v_encrypt_buffer_w = 0;		//base64v����
	char* bs64v_encrypt_buffer_v = 0;

	if (this_->value) {
		if (!readmode) {	
			memset(this_->value, 0, valuelen + 1);
			memcpy_s(this_->value, valuelen, itemvalue, valuelen);
			bs64v_encrypt_buffer_v = bs64v_encode(this_->value);
			this_->datalen += strnlen_s(bs64v_encrypt_buffer_v, 512 / 3 * 4);
		}
		else {										//����ģʽ��������Ҫ�Զ������ݽ���bs64v������
			if(this_->value)free(this_->value);
			this_->value = bs64v_decode(itemvalue);
			this_->datalen += valuelen;
		}
	}

	if (obj->item_list->item_name && (namelen > 0)) 
	{
		if (!readmode) {
			memset(this_->item_name, 0, namelen + 1);
			memcpy_s(this_->item_name, namelen + 1, itemname, namelen);
			bs64v_encrypt_buffer_w = bs64v_encode(this_->item_name);
			this_->datalen += strnlen_s(bs64v_encrypt_buffer_w, 512 / 3 * 4);
			//if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v); ����ɵ�ƣ�����Ҫ�������ͷ��ڴ�
		}
		else {
			if(this_->item_name)free(this_->item_name);
			this_->item_name = bs64v_decode(itemname);
			this_->datalen += namelen;
		}
	}
	
	DebugInvolke printf("Debug: value len %d (%s)\n",valuelen,itemvalue);
	

	this_->datalen += 3;	//���ݳ���Ϊ�����ȼ�ֵ���ȼ�3���ֽڱ�ʶ������
	obj->item_count++;

	char bu_blank[32];
	memset(bu_blank, 0, 32);
	if (!readmode) {	//�ڷǶ���ģʽ�����²ŶԱ������ݽ���д�룬��������д���������ݸ�д
		DebugInvolke printf("Debug: Writing to local.\n");

		fseek(obj->local, -32, SEEK_END);
		this_->ptr_local = ftell(obj->local);
		fwrite(&f_start, 1, 1, obj->local);
		fwrite(bs64v_encrypt_buffer_w, strnlen_s(bs64v_encrypt_buffer_w,MAX_CODE_LEN), 1, obj->local);
		fwrite(&f_name, 1, 1, obj->local);
		fwrite(bs64v_encrypt_buffer_v, strnlen_s(bs64v_encrypt_buffer_v, MAX_CODE_LEN), 1, obj->local);
		fwrite(&f_end, 1, 1, obj->local);
		fwrite(bu_blank, 32, 1, obj->local);

		if (bs64v_encrypt_buffer_w)free(bs64v_encrypt_buffer_w);//�����ڴ�
		if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v);
		DebugInvolke printf("Debug: new item (name: %s)\n", itemname);
	}
	return 0;
}

config_info* obj_get(int id) {
	if ((id < default_id) || (id > default_id + opened_count))return 0;
	for (int i = 0; i < obj_count; i++) {
		if (opened_obj->struct_id == id)return opened_obj;
		opened_obj = opened_obj->next;
	}
	return 0;
}

config_item* item_get(int id, char* itemname) {
	config_info* obj_tmp = obj_get(id);
	if (debug_mode && obj_tmp)printf("Debug: want id:%d  item:%s then get 0x%p\n", id, itemname, obj_tmp->item_list);
	if (obj_tmp == 0) { printf("obj_tmp == 0"); return 0; }
	if (obj_tmp->item_list == 0) { DebugInvolke printf("Debug: nullptr of obj: 0x%p\n", obj_tmp); return 0; }
	for (int i = 0; i < obj_tmp->item_count; i++) {
		//printf("Searching. Now:%s\n", obj_tmp->item_list->item_name);
		if (strncmp(obj_tmp->item_list->item_name, itemname, strnlen_s(obj_tmp->item_list->item_name, 512)) == 0) {
			return obj_tmp->item_list;
		}
		obj_tmp->item_list = obj_tmp->item_list->next;
	}
	return 0;
}

int obj_removeitem(int id, char* itemname) {
	config_info* obj_tmp = obj_get(id);
	config_item* this_item;
	if (!obj_tmp)return -1;
	this_item = item_get(id, itemname);
	if (!this_item)return -2;
	void* buffer = malloc(this_item->datalen);	//׼������ԭ���ݵĿ��ֽ�	Ŀǰ���Ƴ���ʽ���ǽ����ݸ�дΪ0����ΪĿǰ���㷨�ڶ�ȡ�ļ�ʱ�����ֽ�0���ռ������δ���Ż�
	if (!buffer)return -3;
	memset(buffer, 0, this_item->datalen);		//���ڴ�����
	fseek(obj_tmp->local, this_item->ptr_local, SEEK_SET);
	fwrite(buffer, this_item->datalen, 1, obj_tmp->local);
	free(buffer);								//�����ڴ�
	//�����Ƴ������ݵ��ڴ�
	{
		if (this_item->item_name)free(this_item->item_name);
		if (this_item->value)free(this_item->value);
	}
	if (this_item->next = this_item) {			//������ָ�����һ��Ϊ�������ʾ���ļ�����һ��
		obj_tmp->item_list = 0;
	}
	else {
		this_item->last->next = this_item->next;
		this_item->next->last = this_item->last;
		obj_tmp->item_list = this_item->next;
	}
	obj_tmp->item_count--;						//������1
	free(this_item);
	return 0;
}

int obj_rewriteitem(int id, char* itemname, char* value) {
	DebugInvolke printf("Debug: try rewrite item in file %d\n",id);
	config_info* obj_tmp = obj_get(id);
	config_item* this_item;
	if (!obj_tmp)return -1;
	this_item = item_get(id, itemname);
	if (!this_item)return -2;

	char fixbytes[32];	//β������32�ֽ�
	memset(fixbytes, 0, 32);

	char* bs64v_encrypt_buffer_w = 0, * bs64v_encrypt_buffer_v = 0;
	bs64v_encrypt_buffer_w = bs64v_encode(itemname);
	bs64v_encrypt_buffer_v = bs64v_encode(value);
	int newvaluelen = strnlen_s(bs64v_encrypt_buffer_v, 512 / 3 * 4 + 4), nowvaluelen = strnlen_s(this_item->value, 512);
	int newnamelen = strnlen_s(bs64v_encrypt_buffer_w, 512 / 3 * 4 + 4), nownamelen = strnlen_s(this_item->item_name, 512);
	DebugInvolke printf("Debug: new data name: \"%s\"\nvalue: \"%s\"\n", bs64v_encrypt_buffer_w, bs64v_encrypt_buffer_v);
	void* buffer, *buffer2;

	char* tmp_thisvalue, * tmp_thisname;
	tmp_thisvalue = (char*)malloc(strnlen_s(value, 512) + 1);
	if (!tmp_thisvalue) {
		if (bs64v_encrypt_buffer_w)free(bs64v_encrypt_buffer_w);
		if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v);
		return -3;
	}
	tmp_thisvalue[strnlen_s(value, 512)] = '\0';
	tmp_thisname = (char*)malloc(strnlen_s(itemname, 512) + 1);
	if (!tmp_thisname) {
		if (bs64v_encrypt_buffer_w)free(bs64v_encrypt_buffer_w);
		if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v);
		free(tmp_thisvalue);
		return -3;
	}
	tmp_thisname[strnlen_s(itemname, 512)] = '\0';
	//Ǩ�Ʋ�������
	memcpy_s(tmp_thisname, strnlen_s(itemname, 512) + 1, itemname, strnlen_s(itemname, 512));
	memcpy_s(tmp_thisvalue, strnlen_s(value, 512) + 1, value, strnlen_s(value, 512));

	if (this_item->value)free(this_item->value);
	if (this_item->item_name)free(this_item->item_name);

	this_item->value = tmp_thisvalue;
	this_item->item_name = tmp_thisname;

	buffer = malloc(this_item->datalen);	//�������㵱ǰ�ļ�����ֵ���ڴ�
	if (!buffer) { return -3; }

	memset(buffer, 0, this_item->datalen);
	if (newvaluelen <= nowvaluelen) {		
		fseek(obj_tmp->local, this_item->ptr_local, SEEK_SET);
		fwrite(buffer, this_item->datalen, 1, obj_tmp->local);	//����flag end���ڣ���дΪ��
		fseek(obj_tmp->local, -(this_item->datalen), SEEK_CUR);
		fwrite(&f_start, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_w, newnamelen, 1, obj_tmp->local);
		fwrite(&f_name, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_v, newvaluelen, 1, obj_tmp->local);		//д����ֵ����
		fwrite(&f_end, 1, 1, obj_tmp->local);				//д��flag end
		free(buffer);
	}
	else
	{
		//���´��벻Ӧ��ִ��ʱ�������Ǵ��̿ռ䲻�㣬�������ü�⣬��������������M�������ļ����Ų��°�
		fseek(obj_tmp->local, this_item->ptr_local, SEEK_SET);
		fwrite(buffer, this_item->datalen, 1, obj_tmp->local);
		free(buffer);
		fseek(obj_tmp->local, -32, SEEK_END);	//�µ�����д���ļ�ĩ��
		this_item->ptr_local = ftell(obj_tmp->local);
		fwrite(&f_start, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_w, newnamelen, 1, obj_tmp->local);
		fwrite(&f_name, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_v, newvaluelen, 1, obj_tmp->local);
		fwrite(&f_end, 1, 1, obj_tmp->local);
		fwrite(fixbytes, 32, 1, obj_tmp->local);
	}
	if (bs64v_encrypt_buffer_w)free(bs64v_encrypt_buffer_w);
	if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v);
	DebugInvolke printf("Debug: new data was writed\n");
	this_item->datalen = newvaluelen + newnamelen + 3;	//�������ݳ���
	return 0;
}

int obj_loaditem(config_info* obj) {
	if (!obj || !(obj->local))return -1;	//nullptr or unopened file
	char char_buff = 0, tmp_type = 0;
	char* buffer = (char*)malloc(MAX_CODE_LEN);		//��ʱ��¼���ݶε��ֽ�����
	if (!buffer)return -2;

	long filelen = 0;
	long datalen = 0;		//��ȥռλ����Ч�����ݳ���

	char* tmp_itemname = (char*)malloc(512 / 3 * 4 + 4);	//������ʱ������������ʱ�ڴ棬��Ϊֵ���ݣ�����������ʱ�����
	char* tmp_itemvalue = (char*)malloc(512 / 3 * 4 + 4);

	if (!tmp_itemname) { free(buffer); return -2; }
	if (!tmp_itemvalue) { free(buffer); free(tmp_itemname); return -2; }	//�����м���Ƿ�ɹ����뵽��ʱ�ڴ棬ʧ�ܷ���-2

	fseek(obj->local, 0, SEEK_END);
	filelen = ftell(obj->local);
	if (filelen < 32)return -1;				//�ļ�β��ʼ����32�������ֽ�ռλ��δ����ȱʡ����

	memset(buffer, 0, MAX_CODE_LEN);
	fseek(obj->local, 0, SEEK_SET);
	datalen = filelen - 32;
	int offset = 0, ptr_local = 0;

	DebugInvolke printf("Loading file.\n");
	for (int i = 0; i < datalen; i++) {
		fread(&char_buff, 1, 1, obj->local);
		if (char_buff == 0)continue;	//�ֽ�0 ����

		//�����ֽڱ�ʶ�� 18 - ��Աβ
		if (char_buff == 18) {
			memset(tmp_itemvalue, 0, 512 / 3 * 4 + 4);
			memcpy_s(tmp_itemvalue, 512 / 3 * 4 + 4, buffer, offset);

			obj_insertitem(obj, tmp_itemname, tmp_itemvalue, 1, ptr_local);
			memset(buffer, 0, 512 / 3 * 4 + 4);
			//obj->item_count++;
			offset = 0;		//ֵ���ݶν����������±����
			continue;
		}

		//�����ֽڱ�ʶ�� 12 - ��Աͷ
		if (char_buff == 12) 
		{ 
			offset = 0;		//�����ݶο�ʼ�������±���㣬ͬʱ��ʼ����ֵ���ݶ�
			ptr_local = ftell(obj->local) - 1;	//����ͷ���ļ��е�λ��(������ļ���0���ֽ�)
			continue; 
		}

		//�����ֽڱ�ʶ�� 16 - ��Ա�ж�
		if (char_buff == 16) {
			if (tmp_itemname) {
				memset(tmp_itemname, 0, 512 / 3 * 4 + 4);
				memcpy_s(tmp_itemname, 512 / 3 * 4 + 4, buffer, offset);
				DebugInvolke printf("Debug(old): Item: %s\n",tmp_itemname);
			}

			offset = 0;		//�����ݶν����������±����
			continue;
		}
		buffer[offset] = char_buff;		//��:��insert�����ѽ���base64v����  ��:�򵥻�����9��ȡֵԭ���ǿɴ�ӡ�ַ�-9������Ϊ0���߱�ʶ�ֽڣ�δ����base64����ȡ��
		offset++;	//���Ǳ�ʶ�����ֽ�0������Ϊ���ݶ������飬�����±�ƫ������
	}

	//�����ڴ�
	{
		free(tmp_itemname);
		free(tmp_itemvalue);
	}
	return 0;
}

//API: Load configure file
int OpenConfigFile(char* filename) {
	config_info* tmp = obj_new();
	if (!tmp)return -3;												//check object creating success
	errno_t open_rt = fopen_s(&tmp->local, filename, "r+");
	if (open_rt) { obj_free(tmp); return 0; }						//check file open success
	int tmp_strn = strnlen_s(filename, 1024);
	tmp->filename = (char*)malloc(tmp_strn + 1);
	if (!tmp->filename) { obj_free(tmp); return -1; }				//-1, can not open file
	memset(tmp->filename, 0, tmp_strn + 1);
	memcpy_s(tmp->filename, tmp_strn + 1, filename, tmp_strn);		//copy file local name
	int obj_rt = obj_loaditem(tmp);
	if (obj_rt) { obj_free(tmp); return -2; }			//loading error
	return tmp->struct_id;
}

void CloseConfigFile(int id) {
	config_info* tmp = obj_get(id);
	if (tmp) {
		printf("Close file id %d\n",id);
		obj_free(tmp);
	}
	return;
}

char* GetItemValue(int id, char* itemname) {
	config_item* item_tmp = item_get(id, itemname);
	if (item_tmp) {
		return item_tmp->value;
	}
	return 0;
}


int GetObjectFirstId(int id) {
	return default_id;
}

int GetItemCount(int id) {
	config_info* tmp = obj_get(id);
	if (tmp) { return tmp->item_count; }
	return 0;
}

int InsertItem(int id, char* itemname, char* value) {
	config_info* tmp = obj_get(id);
	if (tmp) {
		if (item_get(id, itemname))return -2;
		return obj_insertitem(tmp, itemname, value, 0, -1);
	}
	return -1;
}

int RemoveItem(int id, char* itemname) {
	return obj_removeitem(id, itemname);
}

int RewriteItem(int id, char* itemname, char* value) {
	return obj_rewriteitem(id, itemname, value);
}

void Debug_mode(bool de) { debug_mode = de; return; }

#endif