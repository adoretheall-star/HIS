// 文件名: utils.h
// 作用: 存放所有系统通用的工具函数的声明

// 这两行叫“头文件卫士”，防止这个头文件被队友重复引入导致编译报错
#ifndef UTILS_H
#define UTILS_H

// 1. 获取安全的整数 (菜单、年龄、数量)
int get_safe_int(const char* prompt);

// 2. 获取安全的浮点数 (金额、单价)
double get_safe_double(const char* prompt);

// 3. 获取安全的字符串 (防止超长崩溃，支持输入空格)
// 参数说明：prompt是提示语，buffer是存放文本的数组，max_len是数组最大长度
void get_safe_string(const char* prompt, char* buffer, int max_len);

// 4. 身份证基础格式校验
int validate_id_card(const char* id_card);

// 5. 患者编号格式校验
int validate_patient_id(const char* patient_id);

// 6. 身份证号脱敏
void mask_id_card(const char* src, char* dest);

// 6. 获取预约状态文本
const char* get_appointment_status_text(int status);

// 7. 查找患者最新预约记录
struct AppointmentNode* find_latest_appointment_by_patient_id(const char* patient_id);

// 8. 判断是否为夜间班次
int is_night_shift();

// 以后如果有通用功能，比如隐藏密码输入、暂停程序，也全声明在这里
// void clear_screen(); 

// 6. 检查字符串是否为空白 (只包含空格、制表符、换行符等)
int is_blank_string(const char* str);

// 7. 检查日期字符串是否合法 (YYYY-MM-DD)
int is_valid_date_string(const char* date_str);

// 8. 安全的字符串复制函数
void safe_copy_string(char* dest, int dest_size, const char* src);

// 9. 获取单个字符（用于 Y/N 确认等场景）
char get_single_char(const char* prompt);

// 10. 表单字符串输入（支持返回上一级，输入B/b）
int get_form_string(const char* prompt, char* buffer, int max_len);

// 11. 表单整数输入（支持返回上一级，输入B/b）
int get_form_int(const char* prompt, int* value, int min, int max, const char* error_msg);

// 12. 表单浮点数输入（支持返回上一级，输入B/b）
int get_form_double(const char* prompt, double* value, double min, const char* error_msg);

// 13. 表单日期输入（支持返回上一级，输入B/b）
int get_form_date(const char* prompt, char* buffer, int max_len);

// 14. 检查日期是否为未来日期（晚于今天）
int is_future_date(const char* date_str);

#endif