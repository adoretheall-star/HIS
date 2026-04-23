// 文件名: utils.c
// 作用: 工具函数的具体实现逻辑
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h> // 用于时间相关操作
#include <stdlib.h> // 用于 atoi 函数
#include <limits.h> // 用于 INT_MIN, INT_MAX
#include <errno.h> // 用于 errno
#include <math.h> // 用于 isnan, isinf, isfinite
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
            if ((int)strlen(buffer) == max_len - 1) {
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

// 辅助函数：判断是否为返回命令 (B/b)
static int is_back_command(const char* input)
{
    return (strcmp(input, "B") == 0 || strcmp(input, "b") == 0);
}

// 7. 检查日期字符串是否合法 (YYYY-MM-DD)
int is_valid_date_string(const char* date_str)
{
    // 检查空指针
    if (date_str == NULL)
    {
        return 0;
    }

    // 检查长度
    if (strlen(date_str) != 10)
    {
        return 0;
    }

    // 检查格式：YYYY-MM-DD
    if (date_str[4] != '-' || date_str[7] != '-')
    {
        return 0;
    }

    // 检查所有字符是否为数字（除了第5位和第8位）
    int i;
    for (i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
        {
            continue; // 跳过横杠位置
        }
        if (!isdigit((unsigned char)date_str[i]))
        {
            return 0;
        }
    }

    // 解析年月日
    int year = 0, month = 0, day = 0;
    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    // 检查年份
    if (year <= 0)
    {
        return 0;
    }

    // 检查月份
    if (month < 1 || month > 12)
    {
        return 0;
    }

    // 检查日期
    if (day < 1 || day > 31)
    {
        return 0;
    }

    // 检查月份天数
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 处理闰年
    if (month == 2)
    {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            days_in_month[2] = 29;
        }
    }

    // 检查日期是否在合法范围内
    if (day > days_in_month[month])
    {
        return 0;
    }

    return 1;
}

// 8. 表单字符串输入（支持返回上一级，输入B/b）
int get_form_string(const char* prompt, char* buffer, int max_len)
{
    while (1)
    {
        get_safe_string(prompt, buffer, max_len);
        
        // 检查是否输入B/b返回上一级
        if (is_back_command(buffer))
        {
            return 0; // 返回0表示返回上一级
        }
        
        break;
    }
    return 1; // 返回1表示输入有效
}

// 9. 表单整数输入（支持返回上一级，输入B/b）
int get_form_int(const char* prompt, int* value, int min, int max, const char* error_msg)
{
    char buffer[128];
    char* endptr;
    long val;
    
    while (1)
    {
        get_safe_string(prompt, buffer, sizeof(buffer));
        
        // 检查是否输入B/b返回上一级
        if (is_back_command(buffer))
        {
            return 0; // 返回0表示返回上一级
        }
        
        // 检查是否为空或纯空格
        if (is_blank_string(buffer))
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 尝试转换为长整数
        errno = 0;
        val = strtol(buffer, &endptr, 10);
        
        // 检查是否整串都被消费完
        if (*endptr != '\0')
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 检查是否溢出
        if (errno != 0 || val < INT_MIN || val > INT_MAX)
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 转换为int
        *value = (int)val;
        
        // 检查范围
        if ((*value < min || *value > max) && !(min == 0 && max == 0))
        {
            printf("%s", error_msg);
            continue;
        }
        
        break;
    }
    return 1; // 返回1表示输入有效
}

// 10. 表单浮点数输入（支持返回上一级，输入B/b）
int get_form_double(const char* prompt, double* value, double min, const char* error_msg)
{
    char buffer[128];
    char* endptr;
    double val;
    
    while (1)
    {
        get_safe_string(prompt, buffer, sizeof(buffer));
        
        // 检查是否输入B/b返回上一级
        if (is_back_command(buffer))
        {
            return 0; // 返回0表示返回上一级
        }
        
        // 检查是否为空或纯空格
        if (is_blank_string(buffer))
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 尝试转换为浮点数
        errno = 0;
        val = strtod(buffer, &endptr);
        
        // 检查是否整串都被消费完
        if (*endptr != '\0')
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 检查是否为nan或inf
        if (isnan(val) || isinf(val))
        {
            printf("%s", error_msg);
            continue;
        }
        
        // 检查范围
        if (val <= min)
        {
            printf("%s", error_msg);
            continue;
        }
        
        *value = val;
        break;
    }
    return 1; // 返回1表示输入有效
}

// 11. 表单日期输入（支持返回上一级，输入B/b）
int get_form_date(const char* prompt, char* buffer, int max_len)
{
    while (1)
    {
        get_safe_string(prompt, buffer, max_len);
        
        // 检查是否输入B/b返回上一级
        if (is_back_command(buffer))
        {
            return 0; // 返回0表示返回上一级
        }
        
        // 检查日期格式
        if (is_valid_date_string(buffer))
        {
            // 检查是否为未来日期
            if (is_future_date(buffer))
            {
                break;
            }
            else
            {
                printf("药品效期不能早于或等于当前日期，请重新输入\n");
            }
        }
        else
        {
            printf("效期格式非法，请输入合法日期 YYYY-MM-DD\n");
        }
    }
    return 1; // 返回1表示输入有效
}

// 12. 检查日期是否为未来日期（晚于今天）
int is_future_date(const char* date_str)
{
    if (!is_valid_date_string(date_str))
    {
        return 0;
    }

    // 解析输入日期
    int input_year, input_month, input_day;
    if (sscanf(date_str, "%d-%d-%d", &input_year, &input_month, &input_day) != 3)
    {
        return 0;
    }

    // 获取当前日期
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    int current_year = local_time->tm_year + 1900; // tm_year 是从1900开始的
    int current_month = local_time->tm_mon + 1;     // tm_mon 是从0开始的
    int current_day = local_time->tm_mday;

    // 比较日期
    if (input_year > current_year)
    {
        return 1;
    }
    else if (input_year == current_year)
    {
        if (input_month > current_month)
        {
            return 1;
        }
        else if (input_month == current_month)
        {
            if (input_day > current_day)
            {
                return 1;
            }
        }
    }

    return 0; // 不晚于今天
}
void safe_copy_string(char* dest, int dest_size, const char* src)
{
    if (dest == NULL || src == NULL || dest_size <= 0)
    {
        return;
    }

    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// 9. 获取单个字符（用于 Y/N 确认等场景）
char get_single_char(const char* prompt)
{
    char input[10];
    printf("%s", prompt);
    if (fgets(input, sizeof(input), stdin) != NULL)
    {
        // 去掉换行符
        input[strcspn(input, "\n")] = '\0';
        // 如果输入为空，返回空格
        if (strlen(input) == 0)
        {
            return ' ';
        }
        // 返回第一个字符
        return input[0];
    }
    return ' ';
}
