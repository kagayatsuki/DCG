#pragma once
//�������ݿ�
//Author: shinsya
#ifndef CONFIG_H
#define CONFIG_H
#include <Windows.h>
#include <stdio.h>
#include <new>
#include "base64_variation.h"
#include "recycle.h"

#define Type_String 1
#define Type_Short 2
#define Type_Int 4
#define Type_Long 8
#define Type_Double 9

#define DebugInvolke if(debug_mode)

bool debug_mode = false;
char f_start = 12, f_name = 16, f_end = 18;

typedef struct config_item {	//�ֽڱ�ʶ��: 12-item start; 16-name end; 18-item end; 
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
	int recycle_id;
	config_item* item_list;

	config_info* last;
	config_info* next;
};

config_info* opened_obj = nullptr;
int default_id = 10;			//�����һ�����򿪵��ļ���id,����Ϊ0
int obj_count = 0;
int opened_count = 0;

void itemlist_free(config_item* init) {
	config_item* tmp = init, *tmp2;
	if (!init)return;
	while (tmp)
	{
		tmp2 = tmp;
		if ((tmp->last != tmp) && (tmp->next != tmp)) {	//����Ƿ����һ����Ա
			tmp->last->next = tmp->next;
			tmp->next->last = tmp->last;
			tmp = tmp->next;
			if (tmp2->item_name)delete[] tmp2->item_name;
			if (tmp2->value)delete[] tmp2->value;
			delete tmp2;
		}
		else {											//������һ���Ա������������ǰ��ָ������
			tmp = 0;
			if (tmp2->item_name)delete[] tmp2->item_name;
			if (tmp2->value)delete[] tmp2->value;
			delete tmp2;
		}
	}
	return;
}

//�ͷ�һ��������ڴ�ռ�
void obj_free(config_info* obj) {
	if (!obj)return;

	//free itemlist,file,name mem always
	itemlist_free(obj->item_list);
	if (obj->local)fclose(obj->local);
	if (obj->filename)delete[] obj->filename;

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
	delete obj;
	return;
}

//�����µĿ�ֵ����
config_info* obj_new() {
	config_info* tmp;
	try{ tmp = new config_info(); }
	catch (std::bad_alloc) { return 0; }

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

//�����Ա������; ��Ա��&ֵ �ɵ������ṩ
int obj_insertitem(config_info* obj,char* itemname, char* itemvalue, char readmode, long local_ptr) {

	//���ڴ��ڳ�Ա�����ʼ��
	if (!obj)return -1;
	config_item* tmp;
	try {tmp = new config_item();}
	catch (std::bad_alloc) { 
		DebugInvolke printf("Debug: exception of item memory alloc\n"); 
		return -2;
	}
	
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

	//��ʼ����Ա����ֵ
	tmp->ptr_local = local_ptr;
	this_->item_name = new char[namelen+1]();
	this_->value = new char[valuelen + 1]();

	//���Ż�
	char* bs64v_encrypt_buffer_w = 0;		//base64v����
	char* bs64v_encrypt_buffer_v = 0;

	if (this_->value) {
		if (!readmode) {														//���ڶ�ȡ�ͷǶ�ȡģʽ��������Ҫ�ֱ���
			memcpy_s(this_->value, valuelen + 1, itemvalue, valuelen);
			bs64v_encrypt_buffer_v = bs64v_encode(this_->value);
			this_->datalen += strnlen_s(bs64v_encrypt_buffer_v, 512 / 3 * 4);
		}
		else {																	//����ģʽ��������Ҫ�Զ������ݽ���bs64v������
			if(this_->value)delete[] this_->value;
			this_->value = bs64v_decode(itemvalue);
			this_->datalen += valuelen;
		}
	}

	if (obj->item_list->item_name && (namelen > 0)) 
	{
		if (!readmode) {														//ͬ��
			memcpy_s(this_->item_name, namelen + 1, itemname, namelen);
			bs64v_encrypt_buffer_w = bs64v_encode(this_->item_name);
			this_->datalen += strnlen_s(bs64v_encrypt_buffer_w, 512 / 3 * 4);
		}
		else {
			if(this_->item_name)delete[] this_->item_name;
			this_->item_name = bs64v_decode(itemname);
			this_->datalen += namelen;
		}
	}

	this_->datalen += 3;	//���ݳ���Ϊ�����ȼ�ֵ���ȼ�3���ֽڱ�ʶ������
	obj->item_count++;

	char bu_blank[32];
	memset(bu_blank, 0, 32);
	if (!readmode) {											//�ڷǶ���ģʽ�����²ŶԱ������ݽ���д�룬��������д���������ݸ�д
		DebugInvolke printf("Debug: Writing to local.\n");
		FRC_STRUCT::recycle_block* tmp_b = FRC_STRUCT::get_block(obj->recycle_id, this_->datalen);
		if (tmp_b) {
			FRC_STRUCT::report_block(obj->recycle_id, tmp_b->ptr, tmp_b->ptr + this_->datalen, tmp_b->length - this_->datalen);
			DebugInvolke printf("Debug: using recycle in offset: %d\n",tmp_b->ptr);
			fseek(obj->local, tmp_b->ptr, SEEK_SET);
		}
		else
		{
			fseek(obj->local, -32, SEEK_END);
		}
		this_->ptr_local = ftell(obj->local);
		fwrite(&f_start, 1, 1, obj->local);
		fwrite(bs64v_encrypt_buffer_w, strnlen_s(bs64v_encrypt_buffer_w,MAX_CODE_LEN), 1, obj->local);
		fwrite(&f_name, 1, 1, obj->local);
		fwrite(bs64v_encrypt_buffer_v, strnlen_s(bs64v_encrypt_buffer_v, MAX_CODE_LEN), 1, obj->local);
		fwrite(&f_end, 1, 1, obj->local);
		fwrite(bu_blank, 32, 1, obj->local);

		if (bs64v_encrypt_buffer_w)delete[] bs64v_encrypt_buffer_w;		//�����ڴ�
		if (bs64v_encrypt_buffer_v)delete[] bs64v_encrypt_buffer_v;
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

	if (obj_tmp == 0) {												//���󲻴���
		DebugInvolke printf("Debug: obj was not exist\n"); 
		return 0; 
	}
	if (obj_tmp->item_list == 0) {									//���󲻴�����Ч�ĳ�Ա��
		DebugInvolke printf("Debug: nullptr of obj: 0x%p\n", obj_tmp);
		return 0;
	}
	for (int i = 0; i < obj_tmp->item_count; i++) {
		if (strncmp(obj_tmp->item_list->item_name, itemname, strnlen_s(obj_tmp->item_list->item_name, 512)) == 0) {	//�Գ�Ա����б�����ƥ���Ա��
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

	//׼������ԭ���ݵĿ��ֽ�	Ŀǰ���Ƴ���ʽ���ǽ����ݸ�дΪ0����ΪĿǰ���㷨�ڶ�ȡ�ļ�ʱ�����ֽ�0��***�ռ������δ���Ż�
	char* buffer;
	try { buffer = new char[this_item->datalen](); }
	catch (std::bad_alloc) { return -3; }
	FRC_STRUCT::report_block(obj_tmp->recycle_id, this_item->ptr_local, this_item->ptr_local, this_item->datalen);//���տռ�
	DebugInvolke printf("Debug: recycle in offset: %d\n", this_item->ptr_local);
	fseek(obj_tmp->local, this_item->ptr_local, SEEK_SET);
	fwrite(buffer, this_item->datalen, 1, obj_tmp->local);
	delete[] buffer;								//�����ڴ�

	//�����Ƴ������ݵ��ڴ�
	{
		if (this_item->item_name)delete[] (this_item->item_name);
		if (this_item->value)delete[] (this_item->value);
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
	delete (this_item);
	return 0;
}

int obj_rewriteitem(int id, char* itemname, char* value) {
	DebugInvolke printf("Debug: try rewrite item in file %d\n",id);
	config_info* obj_tmp = obj_get(id);
	config_item* this_item;
	if (!obj_tmp)return -1;
	this_item = item_get(id, itemname);
	if (!this_item)return -2;

	char fixbytes[32];		//β������32�ֽ�
	memset(fixbytes, 0, 32);

	char* bs64v_encrypt_buffer_w = 0, * bs64v_encrypt_buffer_v = 0;
	bs64v_encrypt_buffer_w = bs64v_encode(itemname);	//������д������ݱ���
	bs64v_encrypt_buffer_v = bs64v_encode(value);
	int newvaluelen = strnlen_s(bs64v_encrypt_buffer_v, 512 / 3 * 4 + 4), nowvaluelen = strnlen_s(this_item->value, 512);
	int newnamelen = strnlen_s(bs64v_encrypt_buffer_w, 512 / 3 * 4 + 4), nownamelen = strnlen_s(this_item->item_name, 512);
	
	DebugInvolke printf("Debug: new data name: \"%s\"\nvalue: \"%s\"\n", itemname, value);
	char* buffer, *buffer2;
	char* tmp_thisvalue, * tmp_thisname;

	try { tmp_thisvalue = new char[strnlen_s(value, 512) + 1](); }			//�ڴ��д����ֵ���ڴ棬����һ�����ָ�����ʱ����
	catch (std::bad_alloc) {									//�ж��Ƿ�����ɹ�
		if (bs64v_encrypt_buffer_w)delete[](bs64v_encrypt_buffer_w);//��ָ�벻Ϊ�����ͷ�
		if (bs64v_encrypt_buffer_v)delete[](bs64v_encrypt_buffer_v);
		return -3;
	}

	try { tmp_thisname = new char[strnlen_s(itemname, 512) + 1](); }
	catch (std::bad_alloc) {									//ͬ��
		if (bs64v_encrypt_buffer_w)free(bs64v_encrypt_buffer_w);
		if (bs64v_encrypt_buffer_v)free(bs64v_encrypt_buffer_v);
		delete[](tmp_thisvalue);
		return -3;
	}

	memcpy_s(tmp_thisname, strnlen_s(itemname, 512) + 1, itemname, strnlen_s(itemname, 512));//�����ṩ�����ݵ��������ݸ���
	memcpy_s(tmp_thisvalue, strnlen_s(value, 512) + 1, value, strnlen_s(value, 512));

	if (this_item->value)delete[](this_item->value);		//�ͷ�ԭ�����ڴ棬�滻���µ����ݵ�ַ
	if (this_item->item_name)delete[](this_item->item_name);
	this_item->value = tmp_thisvalue;
	this_item->item_name = tmp_thisname;

	buffer = new char[this_item->datalen]();	//�������㵱ǰ�ļ�����ֵ���ڴ�

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
		FRC_STRUCT::report_block(obj_tmp->recycle_id, ftell(obj_tmp->local), ftell(obj_tmp->local), nowvaluelen - newvaluelen);//������������ƻ㱨�˴��ճ��Ŀռ�
		DebugInvolke printf("Debug: recycle in offset: %d length(%d)\n", ftell(obj_tmp->local), nowvaluelen - newvaluelen);
		delete[] buffer;
	}
	else
	{
		//���´��벻Ӧ��ִ��ʱ�������Ǵ��̿ռ䲻�㣬�������ü�⣬��������������M�������ļ����Ų��°�
		fseek(obj_tmp->local, this_item->ptr_local, SEEK_SET);
		fwrite(buffer, this_item->datalen, 1, obj_tmp->local);
		FRC_STRUCT::recycle_block tmp_aim;
		FRC_STRUCT::recycle_block* tmp_b = FRC_STRUCT::safeguard_get_block(FRC_STRUCT::get_obj(obj_tmp->recycle_id), ftell(obj_tmp->local));//�Խӻ��ջ���
		if (tmp_b && (tmp_b->length >= newvaluelen - nowvaluelen)) {
			tmp_aim.ptr = ftell(obj_tmp->local)-this_item->datalen;
			tmp_aim.length = this_item->datalen;
			FRC_STRUCT::report_block(obj_tmp->recycle_id, ftell(obj_tmp->local), ftell(obj_tmp->local) + (newvaluelen - nowvaluelen), tmp_b->length - (newvaluelen - nowvaluelen));//�㱨
			DebugInvolke printf("Debug: using recycle in offset: %d\n", tmp_b->ptr);
		}
		else if (tmp_b = FRC_STRUCT::get_block(obj_tmp->recycle_id, newvaluelen + newnamelen + 3)) {
			tmp_aim = *tmp_b;
			FRC_STRUCT::report_block(obj_tmp->recycle_id, tmp_b->ptr, tmp_b->ptr + this_item->datalen, tmp_b->length - this_item->datalen);
			DebugInvolke printf("Debug: using recycle in offset: %d\n", tmp_b->ptr);
		}
		else {
			tmp_aim.length = 0;
		}
		delete[](buffer);
		if (tmp_aim.length) {
			fseek(obj_tmp->local, tmp_b->ptr, SEEK_SET);	//���ջ����ҵ��Ŀռ�
		}
		else {
			fseek(obj_tmp->local, -32, SEEK_END);			//�µ�����д���ļ�ĩ��
		}
		
		this_item->ptr_local = ftell(obj_tmp->local);
		fwrite(&f_start, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_w, newnamelen, 1, obj_tmp->local);
		fwrite(&f_name, 1, 1, obj_tmp->local);
		fwrite(bs64v_encrypt_buffer_v, newvaluelen, 1, obj_tmp->local);
		fwrite(&f_end, 1, 1, obj_tmp->local);
		fwrite(fixbytes, 32, 1, obj_tmp->local);
	}
	if (bs64v_encrypt_buffer_w)delete[](bs64v_encrypt_buffer_w);
	if (bs64v_encrypt_buffer_v)delete[](bs64v_encrypt_buffer_v);
	DebugInvolke printf("Debug: new data was writed\n");
	this_item->datalen = newvaluelen + newnamelen + 3;	//�������ݳ���
	return 0;
}

int obj_loaditem(config_info* obj) {
	if (!obj || !(obj->local))return -1;	//��ָ���δ���ļ�
	char char_buff = 0, tmp_type = 0;
	char* buffer;		//��ʱ��¼���ݶε��ֽ�����
	try { buffer = new char[MAX_CODE_LEN](); }
	catch (std::bad_alloc) { return -2; }


	long filelen = 0;
	long datalen = 0;		//��ȥռλ����Ч�����ݳ���

	char* tmp_itemname = new char[512 / 3 * 4 + 4]();	//������ʱ������������ʱ�ڴ棬��Ϊֵ���ݣ�����������ʱ�����
	char* tmp_itemvalue = new char[512 / 3 * 4 + 4]();

	if (!tmp_itemname) { delete[](buffer); return -2; }
	if (!tmp_itemvalue) { delete[](buffer); delete[](tmp_itemname); return -2; }	//�����м���Ƿ�ɹ����뵽��ʱ�ڴ棬ʧ�ܷ���-2

	fseek(obj->local, 0, SEEK_END);
	filelen = ftell(obj->local);
	if (filelen < 32)return -1;				//�ļ�β��ʼ����32�������ֽ�ռλ��δ����ȱʡ����

	fseek(obj->local, 0, SEEK_SET);
	datalen = filelen - 32;
	int offset = 0, ptr_local = 0;

	DebugInvolke printf("Debug: loading file...\n");
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
			}

			offset = 0;		//�����ݶν����������±����
			continue;
		}
		buffer[offset] = char_buff;	//���ոն�����ֽ�������������
		offset++;	//���Ǳ�ʶ�����ֽ�0������Ϊ���ݶ������飬�����±�ƫ������
	}

	//�����ڴ�
	{
		delete[](tmp_itemname);
		delete[](tmp_itemvalue);
	}
	char* rc_name = new char[strnlen_s(obj->filename, 1024) + 5]();
	memcpy_s(rc_name, strnlen_s(obj->filename, 1024) + 5, obj->filename, strnlen_s(obj->filename, 1024) + 1);
	memcpy_s(rc_name + strnlen_s(obj->filename, 1024), 4, ".rcd", 4);
	obj->recycle_id = FRC_STRUCT::new_recycle_obj(rc_name);
	DebugInvolke printf("Debug: recycle file -> %s\nObject id: %d\n",rc_name,obj->recycle_id);
	delete[] rc_name;
	return 0;
}


//�������Ϊ�û�����API,���ϵĺ����û���Ӧ�õ���

//API: ����һ�������ļ�
int OpenConfigFile(char* filename) {
	config_info* tmp = obj_new();
	if (!tmp)return -3;												//���ն����Ƿ񴴽��ɹ�
	errno_t open_rt = fopen_s(&tmp->local, filename, "r+");
	if (open_rt) { obj_free(tmp); return 0; }						//����ļ��Ƿ�򿪳ɹ�
	int tmp_strn = strnlen_s(filename, 1024);
	try { tmp->filename = new char[tmp_strn + 1](); }
	catch(std::bad_alloc) { obj_free(tmp); return -1; }				//���ܴ��ļ�������-1
	//memset(tmp->filename, 0, tmp_strn + 1);
	memcpy_s(tmp->filename, tmp_strn + 1, filename, tmp_strn);		//����ļ������ڴ���
	int obj_rt = obj_loaditem(tmp);
	if (obj_rt) { obj_free(tmp); return -2; }			//���ּ��ش���ж�ض��󲢷���-2
	return tmp->struct_id;
}

//API: �ر�һ���Ѵ򿪵������ļ�
void CloseConfigFile(int id) {
	config_info* tmp = obj_get(id);
	if (tmp) {
		DebugInvolke printf("Debug: close file id %d\n",id);
		FRC_STRUCT::close_recycle_obj(tmp->recycle_id);
		obj_free(tmp);
	}
	return;
}

//API: ��ȡָ�������ļ���ָ����Ա��ֵ
char* GetItemValue(int id, char* itemname) {
	config_item* item_tmp = item_get(id, itemname);
	if (item_tmp) {
		return item_tmp->value;
	}
	return 0;
}

//API: ʵ���ã���ȡ��ǰ�汾�Ŀ��ж���ĵ�һ�������ļ�id
int GetObjectFirstId(int id) {
	return default_id;
}

//API: ��ȡָ�������ļ��е���������
int GetItemCount(int id) {
	config_info* tmp = obj_get(id);
	if (tmp) { return tmp->item_count; }
	return 0;
}

//API: ��ָ�������ļ��в���һ���µĳ�Ա����Ա����ֵ�����ɿ�
int InsertItem(int id, char* itemname, char* value) {
	config_info* tmp = obj_get(id);
	if (tmp) {
		if (item_get(id, itemname))return -2;
		return obj_insertitem(tmp, itemname, value, 0, -1);
	}
	return -1;
}

//API: ɾȥָ�������ļ���ָ����Ա
int RemoveItem(int id, char* itemname) {
	return obj_removeitem(id, itemname);
}

//API: ��дָ�������ļ���ָ����Ա��ֵ
int RewriteItem(int id, char* itemname, char* value) {
	return obj_rewriteitem(id, itemname, value);
}

//�򿪻�رտ���̨�е�debug��Ϣ���
void Debug_mode(bool de) { debug_mode = de; return; }

#endif