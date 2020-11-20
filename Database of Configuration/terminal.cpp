#include "config.h"
#include <conio.h>
#include "base64_variation.h"

int getline(char* param) {
	char i = 0;
	int len = 0;
	i = getchar();
	while (i != '\n') {
		*(param + len) = i;
		len++;
		i = getchar();
	}
	return len;
}

void testbs64v() {
	char teststring[] = "Test String: shinsya | hentai = 1";
	char* rt_tmp = bs64v_encode(teststring);
	char* try_decode = bs64v_decode(rt_tmp);
	printf("base64 variation test\nsource:%s\nencode:%s\ndecode:%s\n\n", teststring, rt_tmp,try_decode);
	getchar();
}

void setStartup(int id) {
	char* text_debug = GetItemValue(id,(char*)"debug");
	if (strncmp(text_debug, "true", strnlen_s(text_debug, 32))) { Debug_mode(false); printf("startup config: debug info output was off\n");}
	else { Debug_mode(true); printf("startup config: debug info output was on\n"); }
}

int main() {
	//testbs64v();
	printf("DFM service alpha 0.2\nbs64v lib ver_%s\n", BS64_V);
	FILE* tmp = 0;
	char in_tmp[128];	//��ȡ���뻺����
	char black_buffer[32];
	memset(in_tmp, 0, 128);
	memset(black_buffer, 0, 32);
	char ques_c = 'n';	//����ֵ����
	errno_t erno;
	int startid = 0;	//���������ļ�id
	int id_tmp = 0;			//�û�ָ�������ļ�id

	Debug_mode(true);

	bool startinit = false;;
	//���Զ�ȡ���������ļ�
	startid = OpenConfigFile((char*)"./startup");
	if (startid) {		//����������
		printf("�������������ļ��������Լ���.\n");
		id_tmp = OpenConfigFile(GetItemValue(startid, (char*)"openconf"));
		startinit = true;
		setStartup(startid);	//�����������ж�ȡ������������
		CloseConfigFile(startid);
	}

	if (!startinit) {
		printf("���������ļ���(y=yes/any key=no): ");
		ques_c = _getch();

		if (ques_c == 'y') {
			printf("\n�����ļ�·��: ");
			getline(in_tmp);
			erno = fopen_s(&tmp, in_tmp, "w+");
			if (erno) { printf("Error: %d\n", erno); getchar(); return 0; }
			fwrite(black_buffer, 32, 1, tmp);
			fclose(tmp);
			printf("Success!\n");
		}
		switch_exit:
		printf("\nOpen file: ");
		getline(in_tmp);
		if (!strncmp(in_tmp, "exit", strnlen_s(in_tmp, 128)))return 0;
		id_tmp = OpenConfigFile(in_tmp);
		memset(in_tmp, 0, 128);
	}
	while (!id_tmp) {
		printf("�޷����ļ� \"%s\" �������: %d �����ļ�\n", in_tmp, id_tmp);
		printf("Open file: ");
		memset(in_tmp, 0, 128);
		getline(in_tmp);
		id_tmp = OpenConfigFile(in_tmp);
	}
	printf("�ѳɹ����������ļ� id: %d\n", id_tmp);
	int order = 0;
	char in_buffer[512];
	char in_buffer2[512];
	memset(in_buffer2, 0, 512);
	memset(in_buffer, 0, 512);
	int rt1 = 0, rt2 = 0, rt3 = 0;
	char* rt4 = 0;

	//int now_file = 0;

	while (true)
	{
		printf("\n���� ��/ɾ/��/��/�˳�/Debug on/off (a/d/s/r/e/i): ");
		order = _getch();
		printf("\n");
		switch (order)
		{
		case 'e':
			CloseConfigFile(id_tmp);
			goto switch_exit;
		case 'r':
			printf("Ҫ��ֵ����: ");
			rt1 = getline(in_buffer);
			if (!rt1) {
				printf("�����˿�����\n");
				break;
			}
			if (!GetItemValue(id_tmp, in_buffer)) {
				printf("�������� \"%s\"\n",in_buffer);
				break;
			}
			printf("�µ�ֵ: ");
			rt1 = getline(in_buffer2);
			if (!rt1) { printf("ֵδ�䶯\n"); break; }
			rt1 = RewriteItem(id_tmp, in_buffer, in_buffer2);
			if (rt1) {
				printf("�޸�ֵʱ�������� ������� %d\n", rt1);
				break;
			}
			printf("�µ�ֵ��д��\n");
			break;
		case 'd':
			printf("Ҫɾ��������:");
			if (getline(in_buffer) == 0) {
				printf("�����˿�����\n");
				break;
			}
			rt1 = RemoveItem(id_tmp, in_buffer);
			if (rt1) {
				printf("�Ƴ��� \"%s\" ʧ�� �������:%d\n", in_buffer, rt1);
				break;
			}
			printf("�Ƴ��ɹ�\n");
			break;
		case 'a':
			printf("��������: ");
			rt2 = getline(in_buffer2);
			if ((!rt2) || (strncmp(in_buffer2, " ", strnlen_s(in_buffer2, 2)) == 0)) {
				printf("�벻Ҫʹ�ÿ�����\n");
				break;
			}
			printf("ֵ: ");
			rt2 = getline(in_buffer);
			rt1 = InsertItem(id_tmp, in_buffer2, in_buffer);
			if (rt1 != 0) {
				printf("����δ�ɹ� %d\n", rt1);
				break;
			}
			printf("����ɹ�\n");
			break;
		case 's':
			printf("��ѯ��Ŀ��: ");
			getline(in_buffer);
			rt4 = GetItemValue(id_tmp, in_buffer);
			if (rt4) {
				printf("ֵΪ: %s\n", rt4);
				break;
			}
			printf("���޴���\n");
			break;
		case 'i':
			Debug_mode(!debug_mode);
			if (debug_mode) {
				printf("Debug ��Ϣ��ʾ�ѿ���\n");
				break;
			}
			printf("Debug ��Ϣ��ʾ�ѹر�\n");
			break;
		default:
			break;
		}
		memset(in_buffer2, 0, 512);
		memset(in_buffer, 0, 512);
	}

	return 0;
}