#pragma once
//base64变种，调换标准base64字符表排序
//base64 variation (bs64v) library
//Version 1.0
//Date: 2020-11-17 17:03 Beijing
//Author: shinsya


#ifndef BS64_VA_H
#define BS64_VA_H
#define BS64_V "1.0"
#define MAX_SOURCE_LEN 4096
#define MAX_CODE_LEN (MAX_SOURCE_LEN / 3 * 4)

#include <stdio.h>
#include <string.h>
const char* _bs64v_table = "uvwxyzhijklmnOPQRSTabcdefgABCDEFGopqrstHIJKLMN+/UVWXYZ9876012345";
const int _bs64v_detable[] = {0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,46,0,0,0,47,
							58,59,60,61,62,63,57,56,55,54,0,0,
							0,0,0,0,0,26,27,28,29,30,31,32,
							39,40,41,42,43,44,45,13,14,15,16,17,
							18,48,49,50,51,52,53,0,0,0,0,0,
							0,19,20,21,22,23,24,25,6,7,8,9,
							10,11,12,33,34,35,36,37,38,0,1,2,
							3,4,5};

char* bs64v_encode(char* source) {
	int source_len = strnlen_s(source, MAX_SOURCE_LEN);
	int bs64code_len = source_len / 3 * 4;	//源数据3字节化4字节
	if (source_len % 3)bs64code_len+=4;		//若原数据长度不为3的倍数字节，则额外4字节
	if (!source_len)return 0;
	char* rt_code = new char[bs64code_len + 1];	//将返回的数据
	if (!rt_code)return 0;
	rt_code[bs64code_len] = '\0';			//字符串末尾结束符
	
	int i = 0, j = 0;
	for(;i < bs64code_len-4;i+=4,j+=3){				//3字节化4字节，3字节为一组，6位取一字节
		rt_code[i] = _bs64v_table[(source[j]>>2)];	//原数据第一字节前6位
		rt_code[i + 1] = _bs64v_table[((source[j] & 0x3) << 4) | (source[j + 1] >> 4)];	//原数据第一字节后2位拼接第二字节前4位
		rt_code[i + 2] = _bs64v_table[((source[j + 1] & 0xf) << 2) | (source[j + 2] >> 6)];//原数据第二字节后4字节拼接第三字节前2位
		rt_code[i + 3] = _bs64v_table[(source[j + 2] & 0x3f)];			//原数据第三字节后6位
		//6位最大取值64，即刚好为字符表内字符数，以值作为下标取字符
	}
	//处理原数据长度不为3倍数的情况
	memset(&rt_code[i], '=', 4);
	switch (source_len % 3)
	{
	case 1:
		rt_code[i] = _bs64v_table[(source[j] >> 2)];
		rt_code[i + 1] = _bs64v_table[(source[j] & 0x3) << 4];
		break;
	case 2:
		rt_code[i] = _bs64v_table[(source[j] >> 2)];
		rt_code[i + 1] = _bs64v_table[((source[j] & 0x3) << 4) | (source[j + 1] >> 4)];
		rt_code[i + 2] = _bs64v_table[(source[j + 1] & 0xf) << 2];
		break;
	default:
		rt_code[i] = _bs64v_table[(source[j] >> 2)];
		rt_code[i + 1] = _bs64v_table[((source[j] & 0x3) << 4) | (source[j + 1] >> 4)];	
		rt_code[i + 2] = _bs64v_table[((source[j + 1] & 0xf) << 2) | (source[j + 2] >> 6)];
		rt_code[i + 3] = _bs64v_table[(source[j + 2] & 0x3f)];
		break;
	}
	return rt_code;
}

char* bs64v_decode(char* bs64v_code) {
	int bscode_len = strnlen_s(bs64v_code, MAX_CODE_LEN);
	if (bscode_len % 4)return 0;	//base64v字符串一定是4的倍数
	int nullcount = 0;
	if (strstr(bs64v_code + bscode_len - 4, "==")) {
		nullcount += 2;
	}
	else if (strstr(bs64v_code + bscode_len - 4, "=")) {
		nullcount += 1;
	};//判断空字节符 '='的数量

	int decode_len = (bscode_len - 4) / 4 * 3 + 3 - nullcount;	//解码后字符串长度
	printf("debug: decode len:%d\n", decode_len);
	char* decode_tmp = new char[decode_len + 1];
	if (!decode_tmp)return 0;
	decode_tmp[decode_len] = '\0';	//环境初始化完成

	int i = 0, j = 0;
	for (; i < bscode_len - 4; i += 4, j += 3) {
		decode_tmp[j] = ((_bs64v_detable[bs64v_code[i]] << 2) | (_bs64v_detable[bs64v_code[i + 1]] >> 4));
		decode_tmp[j + 1] = (((_bs64v_detable[bs64v_code[i + 1]]) << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
		decode_tmp[j + 2] = ((_bs64v_detable[bs64v_code[i + 2]] << 6) | (_bs64v_detable[bs64v_code[i + 3]]));
	}
	//对末尾4个字符的三种情况处理
	decode_tmp[j] = ((_bs64v_detable[bs64v_code[i]] << 2) | (_bs64v_detable[bs64v_code[i + 1]] >> 4));	//无论有多少个 '=',其第一个字节总是存在
	switch (nullcount)
	{
	case 0:	//没有 '=' 字符
		decode_tmp[j + 1] = (((_bs64v_detable[bs64v_code[i + 1]]) << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
		decode_tmp[j + 2] = ((_bs64v_detable[bs64v_code[i + 2]] << 6) | (_bs64v_detable[bs64v_code[i + 3]]));
		break;
	case 1:	//有一个 '=' 字符
		decode_tmp[j + 1] = ((_bs64v_detable[bs64v_code[i + 1]] << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
	default:
		break;
	}
	return decode_tmp;
}

#endif