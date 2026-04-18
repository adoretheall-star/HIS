// 文件名: utils.c
// 作用: 工具函数的具体实现逻辑
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h> // 用于时间相关操作
#include "global.h" // 需要访问全局链表和结构体
#include "utils.h" // 必须把自己的说明书引进来
// 1. 工业级安全整数读取（彻底吃掉残留回车）
int get_safe_int(const char* prompt) 
{
    char buffer[128];
    int val = 0;
    while (1) 
    {
        if (prompt) printf("%s", prompt);
        
        // fgets 会把一整行连同最后的回车 \n 一起读走
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) 
        {
            // 如果读到的只有回车（比如前面残留的），直接丢弃，继续等输入
            if (buffer[0] == '\n') continue;
            
            // 尝试提取纯数字
            if (sscanf(buffer, "%d", &val) == 1) 
            {
                return val;
            }
        }
        printf("  ⚠️ [系统拦截] 检测到非法输入！请重新输入纯数字: \n");
    }
}

// 2. 工业级安全浮点数读取（专治财务乱码）
double get_safe_double(const char* prompt) 
{
    char buffer[128];
    double val = 0.0;
    while (1) 
    {
        if (prompt) printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) 
        {
            if (buffer[0] == '\n') continue;
            if (sscanf(buffer, "%lf", &val) == 1) 
            {
                return val;
            }
        }
        printf("  ⚠️ [系统拦截] 检测到非法输入！请重新输入正确的金额: \n");
    }
}

// 3. 工业级安全字符串读取（完美解决吃回车和输入超长）
void get_safe_string(const char* prompt, char* buffer, int max_len) 
{
    while (1) 
    {
        if (prompt) printf("%s", prompt);
        
        if (fgets(buffer, max_len, stdin) != NULL) 
        {
            // 剔除字符串末尾自带的换行符
            buffer[strcspn(buffer, "\n")] = '\0';
            
            // 防溢出保护：如果用户输入的长度超过了 max_len，吃掉缓冲区里多余的字符
            if (strlen(buffer) == max_len - 1) {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            }
            break;
        }
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

// 判断是否为夜间班次
int is_night_shift()
{
    time_t current_time;
    struct tm* local_time;
    int hour;
    
    // 获取当前时间
    current_time = time(NULL);
    // 转换为本地时间
    local_time = localtime(&current_time);
    // 获取当前小时
    hour = local_time->tm_hour;
    
    // 判断是否为夜间（17:00-8:00）
    if (hour >= 17 || hour < 8)
    {
        return 1; // 夜间
    }
    else
    {
        return 0; // 白天
    }
}

// 6. 检查字符串是否为空白 (只包含空格、制表符、换行符等)
int is_blank_string(const char* str)
{
    int i;

    if (str == NULL)
    {
        return 1;
    }

    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isspace((unsigned char)str[i]))
        {
            return 0;
        }
    }

    return 1;
}

// Helper functions for date validation, static to utils.c
static int is_leap_year(int year)
{
    return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

static int get_days_in_month(int year, int month)
{
    static const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month < 1 || month > 12)
    {
        return 0;
    }

    if (month == 2 && is_leap_year(year))
    {
        return 29;
    }

    return days_in_month[month - 1];
}

static int is_basic_date_format_valid(const char* date_str)
{
    int i;

    if (is_blank_string(date_str))
    {
        return 0;
    }

    if (strlen(date_str) != 10)
    {
        return 0;
    }

    for (i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
        {
            if (date_str[i] != '-')
            {
                return 0;
            }
        }
        else if (!isdigit((unsigned char)date_str[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int is_date_value_valid(const char* date_str)
{
    int year;
    int month;
    int day;
    int max_day;

    if (!is_basic_date_format_valid(date_str))
    {
        return 0;
    }

    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    if (year <= 0)
    {
        return 0;
    }

    if (month < 1 || month > 12)
    {
        return 0;
    }

    max_day = get_days_in_month(year, month);
    if (day < 1 || day > max_day)
    {
        return 0;
    }

    return 1;
}

// 7. 检查日期字符串是否合法 (YYYY-MM-DD)
int is_valid_date_string(const char* date_str)
{
    return is_basic_date_format_valid(date_str) && is_date_value_valid(date_str);
}

// 8. 安全的字符串复制函数
void safe_copy_string(char* dest, int dest_size, const char* src)
{
    if (dest == NULL || src == NULL || dest_size <= 0)
    {
        return;
    }
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}
