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

// 以后如果有通用功能，比如隐藏密码输入、暂停程序，也全声明在这里
// void clear_screen(); 

#endif