// 文件名: utils.c
// 作用: 工具函数的具体实现逻辑
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "global.h" // 需要访问全局链表和结构体
#include "utils.h" // 必须把自己的说明书引进来
//1.整数拦截器
int get_safe_int(const char* prompt) 
{
    int num;
    printf("%s", prompt);
    // 拦截一切非数字的乱码输入，这个地方以后要加入我们的提示词来提醒用户正确输入
    while (scanf("%d", &num) != 1)
    //scanf的返回值是成功赋值的变量个数，这里只有num一个变量，所以若成功赋值，返回值应该是1
    // 当用户输入的不是纯数字时，进入循环
    {
        while(getchar() != '\n'); // 清空缓冲区的脏数据，否则可能会导致scanf反复读取同一个非法字符产生死循环或者影响后续输入
        printf("  ⚠️ [系统拦截] 检测到非法字符！请重新输入纯数字: ");//它不跟上一个while循环，它跟的是外层循环，并不会重复打印
    }
    while(getchar() != '\n'); // 正确输入也要吃掉最后的回车符，防止影响后续输入
    return num;
}
//2.浮点数拦截器（专治财务乱码）
double get_safe_double(const char* prompt)
{
    double num;
    printf("%s", prompt);
    while (scanf("%lf", &num) != 1)
    {
        while(getchar() != '\n'); // 清空缓冲区的脏数据，否则可能会导致scanf反复读取同一个非法字符产生死循环或者影响后续输入
        printf("  ⚠️ [系统拦截] 检测到非法字符！请输入有效金额: ");
    }
    while(getchar() != '\n'); // 正确输入也要吃掉最后的回车符，防止影响后续输入
    return num;
}
//3.字符串安全截断器（防止内存溢出导致程序崩溃）
void get_safe_string(const char* prompt, char* buffer, int max_len) 
{
    printf("%s", prompt);
    // 使用安全的 fgets 代替危险的 scanf
    if (fgets(buffer, max_len, stdin) != NULL)//当读取失败时会返回NULL 
   {
        size_t len = strlen(buffer);//strlen算的长度不包括\0
        // 如果用户正常输入，fgets 会把回车符 \n 也读进来，我们要把它去掉
        if (len > 0 && buffer[len - 1] == '\n') 
        {
            buffer[len - 1] = '\0';
        } 
        else 
        {
            // 如果走到这里，说明用户输入了极其长的内容，超过了 max_len
            // fgets 截取了安全长度，剩下的脏东西还在缓冲区，必须清空，否则会影响下一次输入！
            while(getchar() != '\n');
        }
    } 
    else 
    {
        buffer[0] = '\0'; // 异常情况，返回空字符串
    }
}
//4.身份证基础格式校验
int validate_id_card(const char* id_card) 
{
    int i;
    if (id_card == NULL) return 0;
    if (strlen(id_card) != 18) return 0;
    for (i = 0; i < 17; i++) 
    {
        if (!isdigit((unsigned char)id_card[i])) 
        {
            return 0;
        }
    }
    if (!isdigit((unsigned char)id_card[17]) && id_card[17] != 'X' && id_card[17] != 'x') 
    {
        return 0;
    }
    return 1;
}
//5.身份证号脱敏
void mask_id_card(const char* src, char* dest) 
{
    int i;
    if (dest == NULL) return;
    if (src == NULL || strlen(src) < 18) 
    {
        dest[0] = '\0';
        return;
    }
    for (i = 0; i < 6; i++) 
    {
        dest[i] = src[i];
    }
    for (i = 6; i < 14; i++) 
    {
        dest[i] = '*';
    }
    for (i = 14; i < 18; i++) 
    {
        dest[i] = src[i];
    }
    dest[18] = '\0';
}

// 获取预约状态文本
const char* get_appointment_status_text(int status)
{
    switch (status)
    {
        case RESERVED: return "已预约";
        case CHECKED_IN: return "已签到";
        case CANCELLED: return "已取消";
        case MISSED: return "已过号";
        default: return "未知状态";
    }
}

// 查找患者最新预约记录
struct AppointmentNode* find_latest_appointment_by_patient_id(const char* patient_id)
{
    AppointmentNode* curr = NULL;
    AppointmentNode* latest = NULL;

    // 参数校验
    if (g_appointment_list == NULL || patient_id == NULL || patient_id[0] == '\0')
    {
        return NULL;
    }

    curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            latest = curr;
        }
        curr = curr->next;
    }

    return latest;
}
