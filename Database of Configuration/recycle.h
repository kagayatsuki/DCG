#pragma once
//==========================================
//Database rubbish recycle module
//Author: shinsya
//==========================================

#ifndef RECYCLE_H
#define RECYCLE_H
#include <stdio.h>
#include <new>
#include <string>

#define RC_MAX_FILENAME_LEN 2048
#define RC_FRC_STRUCT_SIZE (sizeof(unsigned int)+sizeof(long))

namespace FRC_STRUCT {	//File space recycle struct
	//structs
	struct recycle_block
	{
		long local_ptr;
		unsigned int length;
		long ptr;

		recycle_block* last_block;
		recycle_block* next_block;
	};

	struct recycle_file
	{
		int obj_id;
		char* filename;
		FILE* local;
		int block_count;
		long arc_length;	//already be recycle(length)
		recycle_block* block_list;

		recycle_file* last_obj;
		recycle_file* next_obj;
	};

	//basic args config
	const int obj_start = 1;	//where start of object id
	int obj_opened_count = 0;
	int obj_count = 0;

	//environment variable
	recycle_file* recycle_obj_list = 0;

	void free_block_list(recycle_block* firstPtr) {
		if (!firstPtr)return;	//nullptr
		if (firstPtr->last_block && (firstPtr->last_block != firstPtr))firstPtr->last_block->next_block = 0;
		recycle_block* tmp = firstPtr, * next_tmp = 0;
		while (tmp) {	//free memory
			if (tmp->next_block == tmp)firstPtr->next_block = 0;
			next_tmp = firstPtr->next_block;
			delete tmp;
			tmp = next_tmp;
		}
	}

	int load_block_list(recycle_file* obj) {
		if (!obj)return -1;	//nullptr
		fseek(obj->local, 0, SEEK_END);
		int flen = ftell(obj->local);
		int block_t = flen / RC_FRC_STRUCT_SIZE;	//block count
		fseek(obj->local, sizeof(int) * 2, SEEK_SET);

		//Initialize linked list
		recycle_block* new_tmp = 0;
		for (int i = 0; i < block_t; i++) {
			try {	//new member
				new_tmp = new recycle_block();
				//set linked list
				if (obj->block_list) {
					new_tmp->last_block = obj->block_list;
					new_tmp->next_block = obj->block_list->next_block;
					obj->block_list->next_block = new_tmp;
					obj->block_list->next_block->last_block = new_tmp;
				}
				else {
					new_tmp->next_block = new_tmp;
					new_tmp->last_block = new_tmp;
				}
				obj->block_list = new_tmp;
			}
			catch (std::bad_alloc) {	//exception catch
				free_block_list(obj->block_list);
				return -2;
			}

			//read local data
			new_tmp->local_ptr = ftell(obj->local);
			fread(&new_tmp->ptr, sizeof(long), 1, obj->local);
			fread(&new_tmp->length, sizeof(unsigned int), 1, obj->local);
		}
		return 0;	//Succuss
	}

	recycle_file* get_obj(int id) {
		if (id > obj_start + obj_count - 1)return 0;	//Object was not existed
		for (int i = 0; i < obj_count; i++) {
			if (recycle_obj_list->obj_id == id)return recycle_obj_list;
			recycle_obj_list = recycle_obj_list->next_obj;
		}
		return 0;
	}

	int new_recycle_obj(char* filename) {
		if (!filename)return -1;	//nullptr
		recycle_file* this_tmp;
		try { this_tmp = new recycle_file(); }
		catch (std::bad_alloc) { return -2; }
		
		//Initialize target file
		errno_t fop_tmp = fopen_s(&this_tmp->local, filename, "r+");
		if (fop_tmp == 2) {		//Target file was not existed.Try to create it.
			fop_tmp = fopen_s(&this_tmp->local, filename, "w+");
		}
		switch (fop_tmp)		//fopen_s status
		{
		case 0:
			break;				//Succussed
		default:
			delete this_tmp;
			return -3;			//Target file was existed.But no permission.
		}
		
		//Initialize object information
		int fnlen = strnlen_s(filename, RC_MAX_FILENAME_LEN);
		try { this_tmp->filename = new char[fnlen + 1](); }	//copy filename
		catch (std::bad_alloc) {
			if (this_tmp)fclose(this_tmp->local);
			delete this_tmp;
			return -4;
		}
		memcpy_s(this_tmp->filename, fnlen + 1, filename, fnlen);
		
		int filelen = fseek(this_tmp->local, 0, SEEK_END);
		fseek(this_tmp->local, 0, SEEK_SET);
		if (filelen < sizeof(int)*2) {		//check new file or blank file
			fwrite(&this_tmp->block_count, sizeof(int), 1, this_tmp->local);	//Write 0 to set block count and already block length
			fwrite(&this_tmp->arc_length, sizeof(long), 1, this_tmp->local);
		}
		else {
			fread(&this_tmp->block_count, sizeof(int), 1, this_tmp->local);		//Read
			fread(&this_tmp->arc_length, sizeof(long), 1, this_tmp->local);
		}
		if (recycle_obj_list) {			//Set linked list
			this_tmp->last_obj = recycle_obj_list;
			this_tmp->next_obj = recycle_obj_list->next_obj;
			recycle_obj_list->next_obj->last_obj = this_tmp;
			recycle_obj_list->next_obj = this_tmp;
		}
		else {
			this_tmp->next_obj = this_tmp;
			this_tmp->last_obj = this_tmp;
		}
		recycle_obj_list = this_tmp;

		//Initialize object block list
		int ist_rt = 0;
		ist_rt = load_block_list(this_tmp);
		if (ist_rt) {	//Initializing block list was wrong
			if (this_tmp->local)fclose(this_tmp->local);
			delete[] this_tmp->filename;
			delete this_tmp;
			return -5;
		}

		//Apply to linked list
		this_tmp->last_obj->next_obj = this_tmp;
		this_tmp->next_obj->last_obj = this_tmp;
		recycle_obj_list = this_tmp;

		this_tmp->obj_id = obj_start + obj_opened_count;	//Set id
		obj_opened_count++;
		obj_count++;
		return this_tmp->obj_id;
	}

	int close_recycle_obj(int id) {
		recycle_file* tmp = get_obj(id);
		if (!tmp)return -1;
		free_block_list(tmp->block_list);
		if (tmp->local)fclose(tmp->local);
		if (tmp->filename)delete[] tmp->filename;
		if (tmp->next_obj == tmp) {
			recycle_obj_list = 0;
		}
		else {
			recycle_obj_list = tmp->next_obj;
		}
		delete tmp;
		return 0;
	}

	recycle_block* safeguard_get_blankblock(recycle_file* obj) {
		if (!obj)return 0;
		recycle_block* list_t = obj->block_list;
		for (int i = 0; i < obj->block_count; i++) {
			if (!list_t->length)return list_t;
			list_t = list_t->next_block;
		}
		return 0;
	}

	recycle_block* safeguard_get_block(recycle_file* obj, long localptr) {
		if (!obj)return 0;
		recycle_block* list_t = obj->block_list;
		for (int i = 0; i < obj->block_count; i++) {
			if (list_t->local_ptr==localptr)return list_t;
			list_t = list_t->next_block;
		}
		return 0;
	}

	recycle_block* get_block(int id, int minimize) {
		recycle_file* obj_t = get_obj(id);
		if (!obj_t)return 0;
		recycle_block* rt = 0, * loop_t = obj_t->block_list;
		for (int i = 0; i < obj_t->block_count; i++) {
			if (loop_t->length >= minimize) {
				if (rt) {
					if (rt->length > loop_t->length)rt = loop_t;
				}
				else {
					rt = loop_t;
				}
			}
		}
		return rt;
	}

	void safeguard_write(recycle_file* obj,recycle_block* block_) {
		fseek(obj->local, 0, SEEK_SET);
		fwrite(&obj->block_count, sizeof(int), 1, obj->local);
		fwrite(&obj->arc_length, sizeof(long), 1, obj->local);
		fseek(obj->local, block_->local_ptr, SEEK_SET);
		fwrite(&block_->ptr, sizeof(long), 1, obj->local);
		fwrite(&block_->length, sizeof(int), 1, obj->local);
	}

	void report_block(int id, long local_ptr,long new_ptr,int length) {
		recycle_file* obj_tmp = get_obj(id);
		if (!obj_tmp)return;
		recycle_block* blank_t = 0, * block_t = 0, * a_w = 0;
		
		if (block_t = safeguard_get_block(obj_tmp, local_ptr)) {
			block_t->ptr = new_ptr;
			block_t->length = length;
			a_w = block_t;
		}else
		if ((blank_t = safeguard_get_blankblock(obj_tmp))) {
			blank_t->ptr = new_ptr;
			blank_t->length = length;
			a_w = blank_t;
		}
		else {
			blank_t = new recycle_block();
			blank_t->ptr = new_ptr;
			blank_t->length = length;
			blank_t->local_ptr = obj_tmp->block_count * RC_FRC_STRUCT_SIZE + sizeof(int) + sizeof(long);
			if (obj_tmp->block_list) {
				blank_t->last_block = obj_tmp->block_list;
				blank_t->next_block = obj_tmp->block_list->next_block;
				obj_tmp->block_list->next_block->last_block = blank_t;
				obj_tmp->block_list->next_block = blank_t;
			}
			else {
				blank_t->last_block = blank_t;
				blank_t->next_block = blank_t;
			}
			obj_tmp->block_list = blank_t;
			a_w = blank_t;
		}
		block_t = safeguard_get_block(obj_tmp, a_w->ptr + a_w->length);
		if (block_t) {
			a_w->length += block_t->length;
			block_t->ptr = 0;
			block_t->length = 0;
			safeguard_write(obj_tmp, a_w);
			safeguard_write(obj_tmp, block_t);
		}
		else {
			safeguard_write(obj_tmp, a_w);
		}
	}
}

#endif // !RECYCLE_H
