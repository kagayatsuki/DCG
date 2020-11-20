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
	char in_tmp[128];	//获取输入缓冲区
	char black_buffer[32];
	memset(in_tmp, 0, 128);
	memset(black_buffer, 0, 32);
	char ques_c = 'n';	//单键值接收
	errno_t erno;
	int startid = 0;	//启动配置文件id
	int id_tmp = 0;			//用户指定配置文件id

	Debug_mode(true);

	bool startinit = false;;
	//尝试读取启动配置文件
	startid = OpenConfigFile((char*)"./startup");
	if (startid) {		//加载配置项
		printf("存在启动配置文件，正尝试加载.\n");
		id_tmp = OpenConfigFile(GetItemValue(startid, (char*)"openconf"));
		startinit = true;
		setStartup(startid);	//从启动配置中读取其它额外设置
		CloseConfigFile(startid);
	}

	if (!startinit) {
		printf("创建配置文件？(y=yes/any key=no): ");
		ques_c = _getch();

		if (ques_c == 'y') {
			printf("\n给定文件路径: ");
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
		printf("无法打开文件 \"%s\" 错误代码: %d 请检查文件\n", in_tmp, id_tmp);
		printf("Open file: ");
		memset(in_tmp, 0, 128);
		getline(in_tmp);
		id_tmp = OpenConfigFile(in_tmp);
	}
	printf("已成功加载配置文件 id: %d\n", id_tmp);
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
		printf("\n操作 增/删/查/改/退出/Debug on/off (a/d/s/r/e/i): ");
		order = _getch();
		printf("\n");
		switch (order)
		{
		case 'e':
			CloseConfigFile(id_tmp);
			goto switch_exit;
		case 'r':
			printf("要改值的项: ");
			rt1 = getline(in_buffer);
			if (!rt1) {
				printf("输入了空项名\n");
				break;
			}
			if (!GetItemValue(id_tmp, in_buffer)) {
				printf("不存在项 \"%s\"\n",in_buffer);
				break;
			}
			printf("新的值: ");
			rt1 = getline(in_buffer2);
			if (!rt1) { printf("值未变动\n"); break; }
			rt1 = RewriteItem(id_tmp, in_buffer, in_buffer2);
			if (rt1) {
				printf("修改值时发生错误 错误代码 %d\n", rt1);
				break;
			}
			printf("新的值已写入\n");
			break;
		case 'd':
			printf("要删除的项名:");
			if (getline(in_buffer) == 0) {
				printf("输入了空项名\n");
				break;
			}
			rt1 = RemoveItem(id_tmp, in_buffer);
			if (rt1) {
				printf("移除项 \"%s\" 失败 错误代码:%d\n", in_buffer, rt1);
				break;
			}
			printf("移除成功\n");
			break;
		case 'a':
			printf("新增项名: ");
			rt2 = getline(in_buffer2);
			if ((!rt2) || (strncmp(in_buffer2, " ", strnlen_s(in_buffer2, 2)) == 0)) {
				printf("请不要使用空项名\n");
				break;
			}
			printf("值: ");
			rt2 = getline(in_buffer);
			rt1 = InsertItem(id_tmp, in_buffer2, in_buffer);
			if (rt1 != 0) {
				printf("插入未成功 %d\n", rt1);
				break;
			}
			printf("插入成功\n");
			break;
		case 's':
			printf("查询项目名: ");
			getline(in_buffer);
			rt4 = GetItemValue(id_tmp, in_buffer);
			if (rt4) {
				printf("值为: %s\n", rt4);
				break;
			}
			printf("查无此项\n");
			break;
		case 'i':
			Debug_mode(!debug_mode);
			if (debug_mode) {
				printf("Debug 信息显示已开启\n");
				break;
			}
			printf("Debug 信息显示已关闭\n");
			break;
		default:
			break;
		}
		memset(in_buffer2, 0, 512);
		memset(in_buffer, 0, 512);
	}

	return 0;
}