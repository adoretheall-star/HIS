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
// 文件名: utils.h
// 作用: 存放所有系统通用的工具函数的声明

// 这两行叫"头文件卫士"，防止这个头文件被队友重复引入导致编译报错
#ifndef UTILS_H
#define UTILS_H

// 严格整数解析函数（只允许纯数字，不允许字母或特殊字符）
int parse_int_strict(const char* str, int* out);

// 打印统一的页面标题
void print_page_header(const char* title);

// 1. 获取安全的整数 (菜单、年龄、数量)
int get_safe_int(const char* prompt);

// 2. 获取安全的浮点数 (金额、单价)
double get_safe_double(const char* prompt);

// 3. 获取安全的字符串 (防止超长崩溃，支持输入空格)
// 参数说明：prompt是提示语，buffer是存放文本的数组，max_len是数组最大长度
void get_safe_string(const char* prompt, char* buffer, int max_len);

// 4. 获取密码输入（支持Tab切换显示/隐藏，星号掩码）
void get_password_with_toggle(const char* prompt, char* buffer, int max_len);

// 5. 获取敏感信息输入（支持Tab切换显示/隐藏，星号掩码）
void get_sensitive_string_with_toggle(const char* prompt, char* buffer, int max_len);

// 6. 身份证基础格式校验
int validate_id_card(const char* id_card);

// 5. 患者编号格式校验（兼容小写p）
int validate_patient_id(const char* patient_id);

// 6. 患者编号规范化（小写p转大写P）
void normalize_patient_id(char* patient_id);

// 7. 身份证号脱敏
void mask_id_card(const char* src, char* dest);

// 6. 获取预约状态文本
const char* get_appointment_status_text(int status);

// 7. 查找患者最新预约记录
struct AppointmentNode* find_latest_appointment_by_patient_id(const char* patient_id);

// 8. 判断是否为夜间班次
int is_night_shift();

// 角色名称获取函数
const char* getRoleName(int roleId);

// 以后如果有通用功能，比如隐藏密码输入、暂停程序，也全声明在这里
// void clear_screen(); 

// 6. 检查字符串是否为空白 (只包含空格、制表符、换行符等)
int is_blank_string(const char* str);

// 6.1 检查字符串是否包含空格或制表符
int contains_space(const char* str);

// 6.2 密码掩码输入（支持Tab切换显示/隐藏）
// 返回值: 0=正常输入, 1=B/b返回, 2=Q/q退出
int read_password_with_toggle(const char* prompt, char* password, int max_len);

// 6.3 密码掩码输入（支持Tab切换显示/隐藏，带外部可见性状态）
// 返回值: 0=正常输入, 1=B/b返回, 2=Q/q退出
// visible: 指向外部可见性状态的指针，Tab键会切换该状态
int read_password_with_toggle_ext(const char* prompt, char* password, int max_len, int* visible);

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

// 15. 计算字符串在控制台中的显示宽度（中文=2，ASCII=1）
int str_display_width(const char* str);

// 16. 按显示宽度对齐打印字符串
void print_col(const char* str, int col_width);

// 17. 检查日期格式是否正确 (YYYY-MM-DD)
int is_valid_date_format(const char* date_str);

// 18. 检查日期是否真实存在
int is_real_date(int year, int month, int day);

// 19. 解析日期字符串为年月日
int parse_date(const char* date_str, int* year, int* month, int* day);

// 20. 比较两个日期，返回 1 表示 d1 > d2，0 表示相等，-1 表示 d1 < d2
int compare_date(int y1, int m1, int d1, int y2, int m2, int d2);

// 21. 获取今天的日期
int get_today_date(int* year, int* month, int* day);

// 22. 检查预约日期是否有效，并返回错误信息
int is_appointment_date_valid(const char* date_str, char* error_msg, size_t error_size);

// 23. 检查预约时段是否有效（日期+时段联合校验）
int is_appointment_slot_valid(const char* date_str, const char* slot, char* error_msg, size_t error_size);

// 24. 计算字符串的显示宽度（中文算2个宽度）
int get_display_width(const char* str);

// 25. 按指定宽度打印文本并补空格
void print_padded_text(const char* str, int target_width);

// 26. 打印指定长度的分隔线
void print_line_separator(int length);

// 27. 通用菜单选择输入（支持 B 返回上一级，Q 退出系统）
// 返回值：-1=返回上一级，-2=退出系统，其他值=有效选择
int inputChoice(int min, int max);

// 28. 按显示宽度截断字符串（带省略号），然后对齐打印
void print_truncated_col(const char* str, int max_width);

// 29. 从 UTF-8 字符串中读取下一个 Unicode code point
unsigned int utf8_next_codepoint(const char** p);

// 30. 根据 Unicode code point 返回对应拼音首字母
char get_chinese_initial_utf8(unsigned int codepoint);

// 31. 把 UTF-8 字符串转换为拼音首字母字符串
void get_pinyin_initials_utf8(const char* src, char* dest, int dest_size);

// 32. 字符串转大写
void to_upper_string(const char* src, char* dest, int dest_size);

// 33. 不区分大小写的子字符串包含检查
int contains_ignore_case(const char* text, const char* keyword);

// 34. 控制台颜色宏定义（ANSI 转义序列，全系统通用）
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

// 35. 大屏统一宽度定义
#define DASHBOARD_WIDTH 72
#define BAR_WIDTH 24

// 35. 打印大屏主标题
void print_dashboard_title(const char* title);

// 36. 打印大屏分区标题
void print_section_title(const char* title);

// 37. 打印指定字符的横线
void print_dashboard_line(char ch, int width);

// 38. 按显示宽度对齐打印字符串
void print_pad_right(const char* s, int width);

// 39. 打印三列键值对
void print_kv_3cols(const char* k1, const char* v1, const char* k2, const char* v2, const char* k3, const char* v3);

// 40. 打印进度条（标签宽度固定，进度条长度固定）
void print_progress_bar_ex(const char* label, int current, int total, int bar_width);

// 41. 打印单列进度条（标签按显示宽度对齐）
void print_progress_bar_single(const char* label, int current, int total);

// 42. 打印医生候诊队列（动态列宽）
void print_doctor_queue(void);

#endif