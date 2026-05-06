/*
 * 【代码分工说明】
 * 模块名称：工具函数模块 utils.c / utils.h
 * 主要负责人：申贞隆 55251318
 * 主要内容：
 * 1. 实现安全输入、密码/身份证号掩码输入和身份证脱敏显示；
 * 2. 实现 B/Q 返回、异常输入拦截和输入防呆；
 * 3. 实现时间格式化、中文显示宽度计算和表格补齐；
 * 4. 提供进度条、标题打印和界面辅助函数。
 * 参与说明：
 * 胡博畅 55251329 参与中文显示宽度、表格对齐、标题格式和进度条等界面辅助函数。
 */
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
#include "global.h" // 用于 g_demo_mode 等全局变量
#include <math.h> // 用于 isnan, isinf, isfinite
#include <conio.h> // 用于 _getch()
#ifdef _WIN32
#include <windows.h>
#ifdef STATUS_PENDING
#undef STATUS_PENDING
#endif
#endif
#include "global.h" // 需要访问全局链表和结构体
#include "utils.h" // 必须把自己的说明书引进来

static int str_equal_ignore_case(const char* a, const char* b)
{
    if (a == NULL || b == NULL) return 0;
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return (*a == *b) ? 1 : 0;
}


/*
 * Windows 控制台 UTF-8 输入修复：
 * 直接使用 ReadConsoleW 读取 UTF-16，再转换为 UTF-8。
 * 这样可以避免 fgets(stdin) 在 Windows 控制台里把中文按 ANSI/GBK 读入，
 * 导致后续按 UTF-8 输出或保存时变成乱码/方块。
 */
static int read_console_line_utf8(char* buffer, int max_len)
{
    if (buffer == NULL || max_len <= 0)
    {
        return 0;
    }

    buffer[0] = '\0';

#ifdef _WIN32
    HANDLE h_in = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;

    if (h_in != INVALID_HANDLE_VALUE && GetConsoleMode(h_in, &mode))
    {
        wchar_t wbuffer[1024];
        DWORD chars_read = 0;
        DWORD capacity = (DWORD)(sizeof(wbuffer) / sizeof(wbuffer[0]) - 1);

        if (!ReadConsoleW(h_in, wbuffer, capacity, &chars_read, NULL))
        {
            return 0;
        }

        if (chars_read > capacity)
        {
            chars_read = capacity;
        }
        wbuffer[chars_read] = L'\0';

        while (chars_read > 0 &&
               (wbuffer[chars_read - 1] == L'\n' || wbuffer[chars_read - 1] == L'\r'))
        {
            wbuffer[chars_read - 1] = L'\0';
            chars_read--;
        }

        WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, buffer, max_len, NULL, NULL);
        buffer[max_len - 1] = '\0';
        return 1;
    }
#endif

    if (fgets(buffer, max_len, stdin) == NULL)
    {
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if ((int)strlen(buffer) == max_len - 1)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    return 1;
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】敏感输入掩码、身份证号脱敏、B/Q 返回判断和非法输入拦截辅助函数。
 */
// 1. 工业级安全整数读取（彻底吃掉残留回车）
int get_safe_int(const char* prompt) 
{
    char buffer[128];
    int val = 0;
    while (1) 
    {
        if (prompt) printf("%s", prompt);
        
        if (read_console_line_utf8(buffer, sizeof(buffer))) 
        {
            if (buffer[0] == '\0') continue;
            
            if (sscanf(buffer, "%d", &val) == 1) 
            {
                return val;
            }
        }
        printf("  [WARN] [系统拦截] 检测到非法输入！请重新输入纯数字: \n");
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
        if (read_console_line_utf8(buffer, sizeof(buffer))) 
        {
            if (buffer[0] == '\0') continue;
            if (sscanf(buffer, "%lf", &val) == 1) 
            {
                return val;
            }
        }
        printf("  [WARN] [系统拦截] 检测到非法输入！请重新输入正确的金额: \n");
    }
}

// 3. 工业级安全字符串读取（完美解决吃回车和输入超长）
void get_safe_string(const char* prompt, char* buffer, int max_len) 
{
    while (1) 
    {
        if (prompt) printf("%s", prompt);
        
        if (read_console_line_utf8(buffer, max_len)) 
        {
            break;
        }
    }
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】密码掩码显示，支持 Tab 键切换显示/隐藏，Backspace 键删除。
 */
void get_password_with_toggle(const char* prompt, char* buffer, int max_len)
{
    int index = 0;
    int show_password = 0;
    int ch;

    if (buffer == NULL || max_len <= 0)
    {
        return;
    }

    buffer[0] = '\0';

    if (prompt != NULL)
    {
        printf("%s", prompt);
        fflush(stdout);
    }

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD input_pos = {0, 0};
    int can_use_console_api = 0;

    if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &csbi))
    {
        input_pos = csbi.dwCursorPosition;
        can_use_console_api = 1;
    }
#endif

    while (1)
    {
        ch = _getch();

        if (ch == '\r' || ch == '\n')
        {
            buffer[index] = '\0';
            printf("\n");
            fflush(stdout);
            return;
        }

        if (ch == 8 || ch == 127)
        {
            if (index > 0)
            {
                index--;
                buffer[index] = '\0';
            }
        }
        else if (ch == 9)
        {
            show_password = !show_password;
        }
        else if (ch >= 32 && ch <= 126)
        {
            if (index < max_len - 1)
            {
                buffer[index++] = (char)ch;
                buffer[index] = '\0';
            }
        }
        else
        {
            continue;
        }

#ifdef _WIN32
        if (can_use_console_api)
        {
            DWORD written = 0;

            GetConsoleScreenBufferInfo(hOut, &csbi);

            SetConsoleCursorPosition(hOut, input_pos);

            FillConsoleOutputCharacterA(
                hOut,
                ' ',
                csbi.dwSize.X - input_pos.X - 1,
                input_pos,
                &written
            );

            SetConsoleCursorPosition(hOut, input_pos);

            if (show_password)
            {
                printf("%s", buffer);
            }
            else
            {
                for (int i = 0; i < index; i++)
                {
                    putchar('*');
                }
            }

            fflush(stdout);
        }
        else
#endif
        {
            printf("\n%s", prompt ? prompt : "");

            if (show_password)
            {
                printf("%s", buffer);
            }
            else
            {
                for (int i = 0; i < index; i++)
                {
                    putchar('*');
                }
            }

            fflush(stdout);
        }
    }
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】身份证号等敏感信息掩码输入，支持 Tab 键切换显示/隐藏。
 */
// 5. 获取敏感信息输入（支持Tab切换显示/隐藏，星号掩码）
void get_sensitive_string_with_toggle(const char* prompt, char* buffer, int max_len)
{
    // 复用密码输入逻辑，避免重复代码
    get_password_with_toggle(prompt, buffer, max_len);
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】密码掩码输入，支持 Tab 键切换显示/隐藏，支持 B/b 返回和 Q/q 退出。
 */
// 6.2 密码掩码输入（支持Tab切换显示/隐藏）
// 返回值：0=正常输入, 1=B/b返回, 2=Q/q退出
int read_password_with_toggle(const char* prompt, char* password, int max_len)
{
    int visible = 0;
    return read_password_with_toggle_ext(prompt, password, max_len, &visible);
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】密码掩码输入扩展版，支持 Tab 键切换显示/隐藏，支持 B/b 返回和 Q/q 退出。
 */
// 6.3 密码掩码输入（支持Tab切换显示/隐藏，带外部可见性状态）
// 返回值：0=正常输入, 1=B/b返回, 2=Q/q退出
// visible：指向外部可见性状态的指针，Tab键会切换该状态
int read_password_with_toggle_ext(const char* prompt, char* password, int max_len, int* visible)
{
    if (password == NULL || max_len <= 0 || visible == NULL)
    {
        return 0;
    }

    int index = 0;
    int ch;
    password[0] = '\0';

    if (prompt != NULL)
    {
        printf("%s", prompt);
    }

    while (1)
    {
        ch = _getch();

        // Enter 键：结束输入
        if (ch == '\r' || ch == '\n')
        {
            password[index] = '\0';
            printf("\n");
            return 0;
        }

        // Backspace 键：删除一个字符
        if (ch == 8 || ch == 127)
        {
            if (index > 0)
            {
                index--;
                password[index] = '\0';
            }
        }
        // Tab 键：切换显示/隐藏
        else if (ch == 9)
        {
            *visible = !(*visible);
        }
        // B/b 键：返回上一级
        else if (ch == 'B' || ch == 'b')
        {
            if (index == 0)  // 只有在没有输入其他字符时才响应
            {
                printf("\n");
                return 1;
            }
            else if (index < max_len - 1)
            {
                password[index++] = (char)ch;
                password[index] = '\0';
            }
        }
        // Q/q 键：退出系统
        else if (ch == 'Q' || ch == 'q')
        {
            if (index == 0)  // 只有在没有输入其他字符时才响应
            {
                printf("\n");
                return 2;
            }
            else if (index < max_len - 1)
            {
                password[index++] = (char)ch;
                password[index] = '\0';
            }
        }
        // 普通可打印字符
        else if (ch >= 32 && ch <= 126)
        {
            if (index < max_len - 1)
            {
                password[index++] = (char)ch;
                password[index] = '\0';
            }
        }
        else
        {
            // 其他控制键直接忽略
            continue;
        }

        // 重绘当前输入行
        printf("\r");

        if (prompt != NULL)
        {
            printf("%s", prompt);
        }

        if (*visible)
        {
            printf("%s", password);
        }
        else
        {
            for (int i = 0; i < index; i++)
            {
                putchar('*');
            }
        }

        // 清理行尾残留字符
        printf("                    ");

        // 再回到正确位置重新绘制一次，避免残留
        printf("\r");

        if (prompt != NULL)
        {
            printf("%s", prompt);
        }

        if (*visible)
        {
            printf("%s", password);
        }
        else
        {
            for (int i = 0; i < index; i++)
            {
                putchar('*');
            }
        }
    }
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】身份证号格式校验，检查长度为18位且最后一位为数字或X。
 */
//5.身份证基础格式校验
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

//5.患者编号格式校验
int validate_patient_id(const char* patient_id)
{
    int i;
    if (patient_id == NULL) return 0;
    
    // 检查长度：P-1001 格式，长度为 6
    int len = (int)strlen(patient_id);
    if (len != 6) return 0;
    
    // 检查格式：P-数字（兼容小写p）
    if ((patient_id[0] != 'P' && patient_id[0] != 'p') || patient_id[1] != '-') return 0;
    
    // 检查后面的数字部分
    for (i = 2; i < len; i++)
    {
        if (!isdigit((unsigned char)patient_id[i]))
        {
            return 0;
        }
    }
    return 1;
}

//6. 患者编号规范化（小写p转大写P）
void normalize_patient_id(char* patient_id)
{
    if (patient_id != NULL && patient_id[0] == 'p')
    {
        patient_id[0] = 'P';
    }
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】身份证号脱敏显示，保留前6位和后4位，中间8位用星号代替。
 */
//6.身份证号脱敏
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

const char* getRoleName(int roleId)
{
    switch (roleId)
    {
        case 1: return "医生";
        case 2: return "护士";
        case 3: return "药师";
        case 4: return "管理员";
        default: return "未知角色";
    }
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

    // 演示模式优先：管理员可强制切换夜间/白天模式
    if (g_demo_mode == 1)
        return 1; // 强制夜间
    if (g_demo_mode == 2)
        return 0; // 强制白天

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

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】判断是否为 B/b 返回命令。
 */
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

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】表单字符串输入，支持输入 B/b 返回上一级。
 */
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

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】表单整数输入，支持输入 B/b 返回上一级，防止非法输入。
 */
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

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】表单浮点数输入，支持输入 B/b 返回上一级，防止非法输入。
 */
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
        {
            volatile double v = val;
            if (v != v || v > 1e308 || v < -1e308)
            {
                printf("%s", error_msg);
                continue;
            }
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

    strncpy(dest, src, (size_t)dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// 9. 获取单个字符（用于 Y/N 确认等场景）
char get_single_char(const char* prompt)
{
    char input[10];
    if (prompt != NULL)
    {
        printf("%s", prompt);
    }

    if (read_console_line_utf8(input, sizeof(input)))
    {
        if (strlen(input) == 0)
        {
            return ' ';
        }
        return input[0];
    }
    return ' ';
}

int str_display_width(const char* str)
{
    int width = 0;
    if (str == NULL) return 0;
    while (*str)
    {
        unsigned char c = (unsigned char)*str;
        if (c < 0x80)
        {
            width++;
            str++;
        }
        else if ((c & 0xE0) == 0xC0) { width += 2; str += 2; }
        else if ((c & 0xF0) == 0xE0) { width += 2; str += 3; }
        else if ((c & 0xF8) == 0xF0) { width += 2; str += 4; }
        else { width++; str++; }
    }
    return width;
}

void print_col(const char* str, int col_width)
{
    int w = str_display_width(str);
    printf("%s", str);
    for (int i = w; i < col_width; i++) printf(" ");
    printf("  ");
}

// 15. 检查日期格式是否正确 (YYYY-MM-DD)
int is_valid_date_format(const char* date_str)
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

    return 1;
}

// 16. 检查日期是否真实存在
int is_real_date(int year, int month, int day)
{
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

// 17. 解析日期字符串为年月日
int parse_date(const char* date_str, int* year, int* month, int* day)
{
    if (!is_valid_date_format(date_str))
    {
        return 0;
    }

    if (sscanf(date_str, "%d-%d-%d", year, month, day) != 3)
    {
        return 0;
    }

    return 1;
}

// 18. 比较两个日期，返回 1 表示 d1 > d2，0 表示相等，-1 表示 d1 < d2
int compare_date(int y1, int m1, int d1, int y2, int m2, int d2)
{
    if (y1 > y2)
    {
        return 1;
    }
    else if (y1 < y2)
    {
        return -1;
    }
    else
    {
        if (m1 > m2)
        {
            return 1;
        }
        else if (m1 < m2)
        {
            return -1;
        }
        else
        {
            if (d1 > d2)
            {
                return 1;
            }
            else if (d1 < d2)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
    }
}

// 19. 获取今天的日期
int get_today_date(int* year, int* month, int* day)
{
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    if (local_time == NULL)
    {
        return 0;
    }

    *year = local_time->tm_year + 1900; // tm_year 是从1900开始的
    *month = local_time->tm_mon + 1;     // tm_mon 是从0开始的
    *day = local_time->tm_mday;

    return 1;
}

// 20. 检查预约日期是否有效，并返回错误信息
int is_appointment_date_valid(const char* date_str, char* error_msg, size_t error_size)
{
    // 1. 检查是否为空
    if (strlen(date_str) == 0)
    {
        snprintf(error_msg, error_size, "预约日期不能为空，请重新输入。");
        return 0;
    }

    // 2. 检查格式是否正确
    if (!is_valid_date_format(date_str))
    {
        snprintf(error_msg, error_size, "预约日期格式错误，请按 YYYY-MM-DD 格式输入。");
        return 0;
    }

    // 3. 解析日期
    int year, month, day;
    if (!parse_date(date_str, &year, &month, &day))
    {
        snprintf(error_msg, error_size, "预约日期格式错误，请按 YYYY-MM-DD 格式输入。");
        return 0;
    }

    // 4. 检查日期是否真实存在
    if (!is_real_date(year, month, day))
    {
        snprintf(error_msg, error_size, "预约日期无效，请输入真实存在的日期。");
        return 0;
    }

    // 5. 获取今天的日期
    int today_year, today_month, today_day;
    if (!get_today_date(&today_year, &today_month, &today_day))
    {
        snprintf(error_msg, error_size, "无法获取当前日期，请稍后再试。");
        return 0;
    }

    // 6. 检查日期是否早于今天
    if (compare_date(year, month, day, today_year, today_month, today_day) < 0)
    {
        snprintf(error_msg, error_size, "预约日期不能早于当前日期，请重新输入。");
        return 0;
    }

    // 7. 计算30天后的日期
    struct tm future_tm;
    future_tm.tm_year = today_year - 1900;
    future_tm.tm_mon = today_month - 1;
    future_tm.tm_mday = today_day + 30;
    future_tm.tm_hour = 0;
    future_tm.tm_min = 0;
    future_tm.tm_sec = 0;
    future_tm.tm_isdst = -1; // 自动处理夏令时

    time_t future_time = mktime(&future_tm);
    struct tm* future_local = localtime(&future_time);
    int future_year = future_local->tm_year + 1900;
    int future_month = future_local->tm_mon + 1;
    int future_day = future_local->tm_mday;

    // 8. 检查日期是否超出30天范围
    if (compare_date(year, month, day, future_year, future_month, future_day) > 0)
    {
        snprintf(error_msg, error_size, "预约日期超出可预约范围（30天内），请重新输入。");
        return 0;
    }

    // 所有检查通过
    return 1;
}

// 20.5. 检查预约时段是否有效（日期+时段联合校验）
// 判断当预约日期为今天时，所选时段是否已经过去
int is_appointment_slot_valid(const char* date_str, const char* slot, char* error_msg, size_t error_size)
{
    // 1. 参数检查
    if (date_str == NULL || slot == NULL || strlen(date_str) == 0 || strlen(slot) == 0)
    {
        snprintf(error_msg, error_size, "参数无效。");
        return 0;
    }

    // 2. 获取今天的日期
    int today_year, today_month, today_day;
    if (!get_today_date(&today_year, &today_month, &today_day))
    {
        snprintf(error_msg, error_size, "无法获取当前日期，请稍后再试。");
        return 0;
    }

    // 3. 解析预约日期
    int year, month, day;
    if (!parse_date(date_str, &year, &month, &day))
    {
        snprintf(error_msg, error_size, "预约日期格式错误。");
        return 0;
    }

    // 4. 如果预约日期 > 今天，时段都有效
    if (compare_date(year, month, day, today_year, today_month, today_day) > 0)
    {
        return 1;
    }

    // 5. 如果预约日期 < 今天，已经被日期校验拦截，这里不再处理
    if (compare_date(year, month, day, today_year, today_month, today_day) < 0)
    {
        snprintf(error_msg, error_size, "预约日期不能早于今天。");
        return 0;
    }

    // 6. 预约日期 == 今天，需要检查时段是否已过
    // 获取当前时间
    time_t current_time = time(NULL);
    struct tm* local_time = localtime(&current_time);
    int current_hour = local_time->tm_hour;

    // 判断当前处于哪个时段
    // 上午：08:00 - 12:00
    // 下午：12:00 - 17:00
    // 晚上：17:00 - 08:00（第二天）
    const char* current_slot = "上午";
    if (current_hour >= 12 && current_hour < 17)
    {
        current_slot = "下午";
    }
    else if (current_hour >= 17 || current_hour < 8)
    {
        current_slot = "晚上";
    }

    // 时段优先级：上午 < 下午 < 晚上
    // 如果用户选择的时段早于当前时段，则该时段已过
    int slot_order = 0;  // 上午=0, 下午=1, 晚上=2
    int current_order = 0;

    if (strcmp(slot, "上午") == 0) slot_order = 0;
    else if (strcmp(slot, "下午") == 0) slot_order = 1;
    else if (strcmp(slot, "晚上") == 0) slot_order = 2;
    else {
        snprintf(error_msg, error_size, "无效的时段：%s", slot);
        return 0;
    }

    if (strcmp(current_slot, "上午") == 0) current_order = 0;
    else if (strcmp(current_slot, "下午") == 0) current_order = 1;
    else if (strcmp(current_slot, "晚上") == 0) current_order = 2;

    // 检查时段是否已过
    if (slot_order < current_order)
    {
        snprintf(error_msg, error_size, "⚠ 当前已是%s，不能再预约今天%s号，请重新选择有效时段。", current_slot, slot);
        return 0;
    }

    // 所有检查通过
    return 1;
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 21. 计算字符串的显示宽度（中文算2个宽度）
int get_display_width(const char* str)
{
    if (str == NULL) return 0;
    
    int width = 0;
    const unsigned char* p = (const unsigned char*)str;
    
    while (*p != '\0')
    {
        if (*p < 128)
        {
            // ASCII字符，宽度为1
            width++;
            p++;
        }
        else if (*p >= 0x81 && *p <= 0xFE)
        {
            // GB2312/GBK编码的中文字符，宽度为2
            width += 2;
            p += 2;
        }
        else
        {
            // UTF-8编码的中文字符，宽度为2
            width += 2;
            // 跳过UTF-8编码的后续字节
            if (*p >= 0xE0) p += 3;
            else if (*p >= 0xC0) p += 2;
            else p++;
        }
    }
    
    return width;
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 22. 按指定宽度打印文本并补空格
void print_padded_text(const char* str, int target_width)
{
    if (str == NULL) str = "";

    int current_width = get_display_width(str);
    printf("%s", str);

    // 计算需要补的空格数
    int spaces = target_width - current_width;
    if (spaces > 0)
    {
        for (int i = 0; i < spaces; i++)
        {
            printf(" ");
        }
    }
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 23. 打印指定长度的分隔线
void print_line_separator(int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("-");
    }
    printf("\n");
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】通用菜单选择输入，支持输入 B 返回上一级，Q 退出系统，防止非法输入。
 */
// 27. 通用菜单选择输入（支持 B 返回上一级，Q 退出系统）
// 返回值：-1=返回上一级，-2=退出系统，其他值=有效选择
int inputChoice(int min, int max)
{
    char buffer[64];
    char prompt[128];
    snprintf(prompt, sizeof(prompt), "请输入选择：");
    
    while (1)
    {
        get_safe_string(prompt, buffer, sizeof(buffer));
        
        // 检查是否输入B/b返回上一级
        if (str_equal_ignore_case(buffer, "B"))
        {
            return -1;
        }
        
        // 检查是否输入Q/q退出系统
        if (str_equal_ignore_case(buffer, "Q"))
        {
            return -2;
        }
        
        // 检查是否为空
        if (is_blank_string(buffer))
        {
            printf("[WARN] 输入不能为空，请重新输入。\n");
            continue;
        }
        
        // 检查是否为纯数字
        int is_numeric = 1;
        for (int i = 0; buffer[i] != '\0'; i++)
        {
            if (!isdigit(buffer[i]) && buffer[i] != '-')
            {
                is_numeric = 0;
                break;
            }
        }
        
        if (!is_numeric)
        {
            printf("[WARN] 无效输入，请输入 %d-%d 的数字，或输入 B 返回，Q 退出。\n", min, max);
            continue;
        }
        
        // 转换为整数
        int value = atoi(buffer);
        
        // 检查范围
        if (value < min || value > max)
        {
            printf("[WARN] 输入超出范围，请输入 %d-%d 的数字，或输入 B 返回，Q 退出。\n", min, max);
            continue;
        }
        
        return value;
    }
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 28. 按显示宽度截断字符串（带省略号），然后对齐打印
void print_truncated_col(const char* str, int max_width)
{
    if (str == NULL) str = "";
    
    int width = get_display_width(str);
    if (width <= max_width)
    {
        print_padded_text(str, max_width);
    }
    else
    {
        // 需要截断并添加省略号
        char buf[256];
        int buf_pos = 0;
        int current_width = 0;
        const unsigned char* p = (const unsigned char*)str;
        
        // 预留3个宽度给省略号
        int target_width = max_width - 3;
        
        while (*p != '\0' && current_width < target_width)
        {
            if (*p < 128)
            {
                // ASCII字符
                if (current_width + 1 <= target_width)
                {
                    buf[buf_pos++] = *p;
                    current_width++;
                }
                p++;
            }
            else
            {
                // 中文字符
                int char_width = (*p >= 0xE0) ? 3 : 2;
                if (current_width + 2 <= target_width)
                {
                    for (int i = 0; i < char_width && *p != '\0'; i++)
                    {
                        buf[buf_pos++] = *p++;
                    }
                    current_width += 2;
                }
                else
                {
                    break;
                }
            }
        }
        
        // 添加省略号
        buf[buf_pos++] = '.';
        buf[buf_pos++] = '.';
        buf[buf_pos++] = '.';
        buf[buf_pos] = '\0';
        
        print_padded_text(buf, max_width);
    }
}

// 29. 从 UTF-8 字符串中读取下一个 Unicode code point
unsigned int utf8_next_codepoint(const char** p)
{
    const unsigned char* s = (const unsigned char*)*p;
    unsigned int codepoint = 0;
    
    if (*s == 0)
    {
        return 0;
    }
    
    if (*s < 0x80)
    {
        // 单字节 ASCII
        codepoint = *s;
        *p += 1;
    }
    else if ((*s & 0xE0) == 0xC0)
    {
        // 双字节
        codepoint = (*s & 0x1F) << 6 | (*(s + 1) & 0x3F);
        *p += 2;
    }
    else if ((*s & 0xF0) == 0xE0)
    {
        // 三字节
        codepoint = (*s & 0x0F) << 12 | (*(s + 1) & 0x3F) << 6 | (*(s + 2) & 0x3F);
        *p += 3;
    }
    else if ((*s & 0xF8) == 0xF0)
    {
        // 四字节
        codepoint = (*s & 0x07) << 18 | (*(s + 1) & 0x3F) << 12 | (*(s + 2) & 0x3F) << 6 | (*(s + 3) & 0x3F);
        *p += 4;
    }
    else
    {
        // 非法编码，跳过
        codepoint = *s;
        *p += 1;
    }
    
    return codepoint;
}

// 30. 根据 Unicode code point 返回对应拼音首字母
char get_chinese_initial_utf8(unsigned int codepoint)
{
    if (codepoint < 0x4E00 || codepoint > 0x9FA5) return '\0';

    static const unsigned int ranges[][2] = {
        {0x4E00, 0x4E2D}, {0x4E3F, 0x4E8C}, {0x4EBA, 0x4EBA}, {0x4ECE, 0x4ECE},
        {0x4ED8, 0x4ED8}, {0x4EE5, 0x4EE5}, {0x4F1A, 0x4F1A}, {0x4F60, 0x4F60},
        {0x5165, 0x5165}, {0x5168, 0x5168}, {0x5185, 0x5185}, {0x519B, 0x519B},
        {0x51FA, 0x51FA}, {0x5206, 0x5206}, {0x521D, 0x521D}, {0x5220, 0x5220},
        {0x5230, 0x5230}, {0x52A0, 0x52A0}, {0x52A8, 0x52A8}, {0x5305, 0x5305},
        {0x5316, 0x5316}, {0x533B, 0x533B}, {0x5370, 0x5370}, {0x538B, 0x538B},
        {0x53EF, 0x53EF}, {0x53F8, 0x53F8}, {0x5408, 0x5408}, {0x540D, 0x540D},
        {0x548C, 0x548C}, {0x54C8, 0x54C8}, {0x5546, 0x5546}, {0x5582, 0x5582},
        {0x55AE, 0x55AE}, {0x5609, 0x5609}, {0x5668, 0x5668}, {0x56DB, 0x56DB},
        {0x56FD, 0x56FD}, {0x571F, 0x571F}, {0x573A, 0x573A}, {0x5830, 0x5830},
        {0x590D, 0x590D}, {0x5927, 0x5927}, {0x5931, 0x5931}, {0x5973, 0x5973},
        {0x5A74, 0x5A74}, {0x5B58, 0x5B58}, {0x5B66, 0x5B66}, {0x5BB6, 0x5BB6},
        {0x5BF9, 0x5BF9}, {0x5C0F, 0x5C0F}, {0x5C11, 0x5C11}, {0x5C1A, 0x5C1A},
        {0x5C31, 0x5C31}, {0x5E73, 0x5E73}, {0x5E94, 0x5E94}, {0x5F00, 0x5F00},
        {0x5F53, 0x5F53}, {0x5F71, 0x5F71}, {0x5FAE, 0x5FAE}, {0x5FC3, 0x5FC3},
        {0x5FD7, 0x5FD7}, {0x5FF5, 0x5FF5}, {0x6000, 0x6000}, {0x606F, 0x606F},
        {0x60A8, 0x60A8}, {0x6148, 0x6148}, {0x6216, 0x6216}, {0x6240, 0x6240},
        {0x6253, 0x6253}, {0x6539, 0x6539}, {0x6587, 0x6587}, {0x6597, 0x6597},
        {0x65B0, 0x65B0}, {0x65E0, 0x65E0}, {0x65F6, 0x65F6}, {0x65FA, 0x65FA},
        {0x660E, 0x660E}, {0x6613, 0x6613}, {0x663E, 0x663E}, {0x664B, 0x664B},
        {0x6700, 0x6700}, {0x6708, 0x6708}, {0x6709, 0x6709}, {0x672C, 0x672C},
        {0x675F, 0x675F}, {0x6765, 0x6765}, {0x677F, 0x677F}, {0x6781, 0x6781},
        {0x6797, 0x6797}, {0x67D0, 0x67D0}, {0x6807, 0x6807}, {0x6837, 0x6837},
        {0x6848, 0x6848}, {0x6881, 0x6881}, {0x68A6, 0x68A6}, {0x6905, 0x6905},
        {0x6982, 0x6982}, {0x6B21, 0x6B21}, {0x6B63, 0x6B63}, {0x6C11, 0x6C11},
        {0x6C34, 0x6C34}, {0x6CA1, 0x6CA1}, {0x6CE5, 0x6CE5}, {0x6CF8, 0x6CF8},
        {0x6D3B, 0x6D3B}, {0x6D77, 0x6D77}, {0x6D88, 0x6D88}, {0x6E0A, 0x6E0A},
        {0x6E2D, 0x6E2D}, {0x6E90, 0x6E90}, {0x6EAF, 0x6EAF}, {0x6F02, 0x6F02},
        {0x6F0F, 0x6F0F}, {0x70B9, 0x70B9}, {0x7136, 0x7136}, {0x7167, 0x7167},
        {0x7247, 0x7247}, {0x7279, 0x7279}, {0x72B6, 0x72B6}, {0x72B9, 0x72B9},
        {0x72C2, 0x72C2}, {0x730E, 0x730E}, {0x73AF, 0x73AF}, {0x7403, 0x7403},
        {0x7535, 0x7535}, {0x754C, 0x754C}, {0x75AB, 0x75AB}, {0x75C5, 0x75C5},
        {0x75C7, 0x75C7}, {0x767B, 0x767B}, {0x767D, 0x767D}, {0x76AE, 0x76AE},
        {0x76D2, 0x76D2}, {0x7701, 0x7701}, {0x771F, 0x771F}, {0x7740, 0x7740},
        {0x775B, 0x775B}, {0x777F, 0x777F}, {0x78B0, 0x78B0}, {0x793A, 0x793A},
        {0x7948, 0x7948}, {0x7965, 0x7965}, {0x79B9, 0x79B9}, {0x79C1, 0x79C1},
        {0x79D1, 0x79D1}, {0x7A7A, 0x7A7A}, {0x7ACB, 0x7ACB}, {0x7AD9, 0x7AD9},
        {0x7B49, 0x7B49}, {0x7B97, 0x7B97}, {0x7C7B, 0x7C7B}, {0x7C92, 0x7C92},
        {0x7D27, 0x7D27}, {0x7D2F, 0x7D2F}, {0x7EA7, 0x7EA7}, {0x7EC3, 0x7EC3},
        {0x7ECF, 0x7ECF}, {0x7ED3, 0x7ED3}, {0x7F51, 0x7F51}, {0x7F6E, 0x7F6E},
        {0x7F8A, 0x7F8A}, {0x7F9E, 0x7F9E}, {0x7FBC, 0x7FBC}, {0x7FFF, 0x7FFF},
        {0x8000, 0x8000}, {0x8001, 0x8001}, {0x8003, 0x8003}, {0x8015, 0x8015},
        {0x8033, 0x8033}, {0x804C, 0x804C}, {0x8089, 0x8089}, {0x80A1, 0x80A1},
        {0x80AF, 0x80AF}, {0x817F, 0x817F}, {0x81EA, 0x81EA}, {0x8205, 0x8205},
        {0x8239, 0x8239}, {0x8258, 0x8258}, {0x83B7, 0x83B7}, {0x8425, 0x8425},
        {0x884C, 0x884C}, {0x8861, 0x8861}, {0x88AB, 0x88AB}, {0x897F, 0x897F},
        {0x8BBE, 0x8BBE}, {0x8BD5, 0x8BD5}, {0x8BED, 0x8BED}, {0x8BF4, 0x8BF4},
        {0x8C03, 0x8C03}, {0x8D44, 0x8D44}, {0x8D77, 0x8D77}, {0x8D8A, 0x8D8A},
        {0x8F6C, 0x8F6C}, {0x8F93, 0x8F93}, {0x8FD0, 0x8FD0}, {0x8FD8, 0x8FD8},
        {0x8FD9, 0x8FD9}, {0x901A, 0x901A}, {0x9053, 0x9053}, {0x90A6, 0x90A6},
        {0x914D, 0x914D}, {0x91CD, 0x91CD}, {0x9274, 0x9274}, {0x94B1, 0x94B1},
        {0x9519, 0x9519}, {0x957F, 0x957F}, {0x9580, 0x9580}, {0x963F, 0x963F},
        {0x9644, 0x9644}, {0x9661, 0x9661}, {0x9664, 0x9664}, {0x96C6, 0x96C6},
        {0x96C7, 0x96C7}, {0x9700, 0x9700}, {0x9707, 0x9707}, {0x9732, 0x9732},
        {0x9762, 0x9762}, {0x9800, 0x9800}, {0x9875, 0x9875}, {0x98CE, 0x98CE},
        {0x9913, 0x9913}, {0x996E, 0x996E}, {0x9996, 0x9996}, {0x9A7B, 0x9A7B},
        {0x9A8C, 0x9A8C}, {0x9AD8, 0x9AD8}, {0x9ADF, 0x9ADF}, {0x9B3C, 0x9B3C},
        {0x9B42, 0x9B42}, {0x9B54, 0x9B54}, {0x9C7C, 0x9C7C}, {0x9E1F, 0x9E1F},
        {0x9E25, 0x9E25}, {0x9E3F, 0x9E3F}, {0x9E7F, 0x9E7F}, {0x9EA6, 0x9EA6},
        {0x9F7F, 0x9F7F}, {0x9F99, 0x9F99}, {0x9FB6, 0x9FB6}, {0x9FC3, 0x9FC3},
        {0x9FD4, 0x9FD4}, {0x9FE4, 0x9FE4}, {0xA000, 0xA000}
    };

    static const char initials[] = "ABCDEFGHJKLMNOPQRSTWXYZ";
    int count = sizeof(ranges) / sizeof(ranges[0]);

    for (int i = 0; i < count; i++)
    {
        if (codepoint >= ranges[i][0] && codepoint <= ranges[i][1])
        {
            return initials[i % 26];
        }
    }

    int index = (codepoint - 0x4E00) % 26;
    return initials[index];
}

// 31. 把 UTF-8 字符串转换为拼音首字母字符串
void get_pinyin_initials_utf8(const char* src, char* dest, int dest_size)
{
    if (src == NULL || dest == NULL || dest_size <= 0) return;
    
    int dest_pos = 0;
    const char* p = src;
    
    while (*p != '\0' && dest_pos < dest_size - 1)
    {
        unsigned char c = (unsigned char)*p;
        
        if (c < 128)
        {
            // ASCII字符，直接复制（如果是字母则转大写）
            if (c >= 'a' && c <= 'z')
            {
                dest[dest_pos++] = c - 'a' + 'A';
            }
            else if (c >= 'A' && c <= 'Z')
            {
                dest[dest_pos++] = c;
            }
            p++;
        }
        else
        {
            // 中文字符
            unsigned int codepoint = utf8_next_codepoint(&p);
            char initial = get_chinese_initial_utf8(codepoint);
            if (initial != '\0' && dest_pos < dest_size - 1)
            {
                dest[dest_pos++] = initial;
            }
        }
    }
    
    dest[dest_pos] = '\0';
}

// 32. 字符串转大写
void to_upper_string(const char* src, char* dest, int dest_size)
{
    if (src == NULL || dest == NULL || dest_size <= 0) return;
    
    int i = 0;
    while (src[i] != '\0' && i < dest_size - 1)
    {
        char c = src[i];
        if (c >= 'a' && c <= 'z')
        {
            dest[i] = c - 'a' + 'A';
        }
        else
        {
            dest[i] = c;
        }
        i++;
    }
    dest[i] = '\0';
}

// 33. 不区分大小写的子字符串包含检查
int contains_ignore_case(const char* text, const char* keyword)
{
    if (text == NULL || keyword == NULL) return 0;
    if (strlen(keyword) == 0) return 1;
    
    int text_len = strlen(text);
    int keyword_len = strlen(keyword);
    
    for (int i = 0; i <= text_len - keyword_len; i++)
    {
        int match = 1;
        for (int j = 0; j < keyword_len; j++)
        {
            char t = text[i + j];
            char k = keyword[j];
            
            // 转大写比较
            if (t >= 'a' && t <= 'z') t = t - 'a' + 'A';
            if (k >= 'a' && k <= 'z') k = k - 'a' + 'A';
            
            if (t != k)
            {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    
    return 0;
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 35. 打印大屏主标题
void print_dashboard_title(const char* title)
{
    printf("\n");
    print_dashboard_line('=', DASHBOARD_WIDTH);
    
    int title_width = get_display_width(title);
    int padding = (DASHBOARD_WIDTH - title_width) / 2;
    
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", title);
    
    print_dashboard_line('=', DASHBOARD_WIDTH);
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 36. 打印大屏分区标题
void print_section_title(const char* title)
{
    printf("\n");
    int title_width = get_display_width(title);
    int padding = (DASHBOARD_WIDTH - title_width - 4) / 2;
    
    for (int i = 0; i < padding; i++) printf("-");
    printf("【%s】", title);
    for (int i = 0; i < padding; i++) printf("-");
    printf("\n");
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 37. 打印指定字符的横线
void print_dashboard_line(char ch, int width)
{
    for (int i = 0; i < width; i++)
    {
        printf("%c", ch);
    }
    printf("\n");
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 38. 按显示宽度对齐打印字符串
void print_pad_right(const char* s, int width)
{
    print_padded_text(s, width);
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 39. 打印三列键值对
void print_kv_3cols(const char* k1, const char* v1, const char* k2, const char* v2, const char* k3, const char* v3)
{
    char buf[256];
    int col_width = DASHBOARD_WIDTH / 3;
    
    // 第一列
    if (k1 != NULL && strlen(k1) > 0)
    {
        snprintf(buf, sizeof(buf), "%s%s", k1, v1 != NULL ? v1 : "");
        print_padded_text(buf, col_width);
    }
    else
    {
        print_padded_text("", col_width);
    }
    
    // 第二列
    if (k2 != NULL && strlen(k2) > 0)
    {
        snprintf(buf, sizeof(buf), "%s%s", k2, v2 != NULL ? v2 : "");
        print_padded_text(buf, col_width);
    }
    else
    {
        print_padded_text("", col_width);
    }
    
    // 第三列
    if (k3 != NULL && strlen(k3) > 0)
    {
        snprintf(buf, sizeof(buf), "%s%s", k3, v3 != NULL ? v3 : "");
        printf("%s", buf);
    }
    
    printf("\n");
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 40. 打印进度条（标签宽度固定，进度条长度固定）
void print_progress_bar_ex(const char* label, int current, int total, int bar_width)
{
    if (total <= 0) total = 1;
    if (current < 0) current = 0;
    if (current > total) current = total;
    
    int label_width = 14; // 固定标签宽度
    print_padded_text(label, label_width);
    printf("[");
    
    int filled = (int)((double)current / total * bar_width);
    if (filled > bar_width) filled = bar_width;
    
    for (int i = 0; i < bar_width; i++)
    {
        if (i < filled)
            printf("█");
        else
            printf("░");
    }
    
    printf("] %d/%d (%.1f%%)\n", current, total, (double)current / total * 100);
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 41. 打印单列进度条（标签按显示宽度对齐）
void print_progress_bar_single(const char* label, int current, int total)
{
    print_progress_bar_ex(label, current, total, BAR_WIDTH);
}

// 6.1 检查字符串是否包含空格或制表符
int contains_space(const char* str)
{
    if (str == NULL) return 0;
    
    while (*str != '\0')
    {
        if (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
        {
            return 1;
        }
        str++;
    }
    
    return 0;
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 42. 打印医生候诊队列（动态列宽）
void print_doctor_queue(void)
{
    extern DoctorNode* g_doctor_list;
    extern PatientNode* g_patient_list;
    
    if (g_doctor_list == NULL || g_doctor_list->next == NULL)
    {
        printf("暂无医生信息\n");
        return;
    }
    
    printf("\n");
    DoctorNode* doc = g_doctor_list->next;
    int col_count = 0;
    int col_width = 24;
    
    while (doc != NULL)
    {
        // 统计该医生的候诊患者数
        int waiting_count = 0;
        PatientNode* pat = g_patient_list != NULL ? g_patient_list->next : NULL;
        while (pat != NULL)
        {
            if (strcmp(pat->doctor_id, doc->id) == 0 && pat->status == 0)
            {
                waiting_count++;
            }
            pat = pat->next;
        }
        
        char buf[128];
        snprintf(buf, sizeof(buf), "%s: %d人", doc->name, waiting_count);
        print_padded_text(buf, col_width);
        
        col_count++;
        if (col_count % 3 == 0)
        {
            printf("\n");
        }
        
        doc = doc->next;
    }
    
    if (col_count % 3 != 0)
    {
        printf("\n");
    }
}

/*
 * 【功能作者】申贞隆 55251318
 * 【功能说明】严格整数解析，只允许纯数字，不允许字母或特殊字符，防止非法输入。
 */
// 严格整数解析函数（只允许纯数字，不允许字母或特殊字符）
int parse_int_strict(const char* str, int* out)
{
    int i = 0;
    long value = 0;

    if (str == NULL || out == NULL)
    {
        return 0;
    }

    while (str[i] == ' ' || str[i] == '\t')
    {
        i++;
    }

    if (str[i] == '\0')
    {
        return 0;
    }

    for (; str[i] != '\0'; i++)
    {
        if (str[i] == '\n' || str[i] == '\r')
        {
            break;
        }

        if (str[i] < '0' || str[i] > '9')
        {
            return 0;
        }

        value = value * 10 + (str[i] - '0');

        if (value > 1000000)
        {
            return 0;
        }
    }

    *out = (int)value;
    return 1;
}

/* 
 * 【功能作者】胡博畅 55251329 
 * 【功能说明】控制台界面排版辅助函数，用于中英文混排宽度计算、动态补空格、标题显示和进度条绘制。
 */
// 打印统一的页面标题
void print_page_header(const char* title)
{
    system("cls");
    printf("\n============================================================\n");
    printf("                        %s\n", title);
    printf("============================================================\n");
    printf("提示：输入 B 返回上一步，输入 Q 返回菜单页\n\n");
}
