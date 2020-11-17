#pragma once
//base64���֣�������׼base64�ַ�������
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
	int bs64code_len = source_len / 3 * 4;	//Դ����3�ֽڻ�4�ֽ�
	if (source_len % 3)bs64code_len+=4;		//��ԭ���ݳ��Ȳ�Ϊ3�ı����ֽڣ������4�ֽ�
	if (!source_len)return 0;
	char* rt_code = new char[bs64code_len + 1];	//�����ص�����
	if (!rt_code)return 0;
	rt_code[bs64code_len] = '\0';			//�ַ���ĩβ������
	
	int i = 0, j = 0;
	for(;i < bs64code_len-4;i+=4,j+=3){				//3�ֽڻ�4�ֽڣ�3�ֽ�Ϊһ�飬6λȡһ�ֽ�
		rt_code[i] = _bs64v_table[(source[j]>>2)];	//ԭ���ݵ�һ�ֽ�ǰ6λ
		rt_code[i + 1] = _bs64v_table[((source[j] & 0x3) << 4) | (source[j + 1] >> 4)];	//ԭ���ݵ�һ�ֽں�2λƴ�ӵڶ��ֽ�ǰ4λ
		rt_code[i + 2] = _bs64v_table[((source[j + 1] & 0xf) << 2) | (source[j + 2] >> 6)];//ԭ���ݵڶ��ֽں�4�ֽ�ƴ�ӵ����ֽ�ǰ2λ
		rt_code[i + 3] = _bs64v_table[(source[j + 2] & 0x3f)];			//ԭ���ݵ����ֽں�6λ
		//6λ���ȡֵ64�����պ�Ϊ�ַ������ַ�������ֵ��Ϊ�±�ȡ�ַ�
	}
	//����ԭ���ݳ��Ȳ�Ϊ3���������
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
	if (bscode_len % 4)return 0;	//base64v�ַ���һ����4�ı���
	int nullcount = 0;
	if (strstr(bs64v_code + bscode_len - 4, "==")) {
		nullcount += 2;
	}
	else if (strstr(bs64v_code + bscode_len - 4, "=")) {
		nullcount += 1;
	};//�жϿ��ֽڷ� '='������

	int decode_len = (bscode_len - 4) / 4 * 3 + 3 - nullcount;	//������ַ�������
	printf("debug: decode len:%d\n", decode_len);
	char* decode_tmp = new char[decode_len + 1];
	if (!decode_tmp)return 0;
	decode_tmp[decode_len] = '\0';	//������ʼ�����

	int i = 0, j = 0;
	for (; i < bscode_len - 4; i += 4, j += 3) {
		decode_tmp[j] = ((_bs64v_detable[bs64v_code[i]] << 2) | (_bs64v_detable[bs64v_code[i + 1]] >> 4));
		decode_tmp[j + 1] = (((_bs64v_detable[bs64v_code[i + 1]]) << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
		decode_tmp[j + 2] = ((_bs64v_detable[bs64v_code[i + 2]] << 6) | (_bs64v_detable[bs64v_code[i + 3]]));
	}
	//��ĩβ4���ַ��������������
	decode_tmp[j] = ((_bs64v_detable[bs64v_code[i]] << 2) | (_bs64v_detable[bs64v_code[i + 1]] >> 4));	//�����ж��ٸ� '=',���һ���ֽ����Ǵ���
	switch (nullcount)
	{
	case 0:	//û�� '=' �ַ�
		decode_tmp[j + 1] = (((_bs64v_detable[bs64v_code[i + 1]]) << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
		decode_tmp[j + 2] = ((_bs64v_detable[bs64v_code[i + 2]] << 6) | (_bs64v_detable[bs64v_code[i + 3]]));
		break;
	case 1:	//��һ�� '=' �ַ�
		decode_tmp[j + 1] = ((_bs64v_detable[bs64v_code[i + 1]] << 4) | (_bs64v_detable[bs64v_code[i + 2]] >> 2));
	default:
		break;
	}
	return decode_tmp;
}

#endif