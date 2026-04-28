#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"

int main() {
    printf("=== 测试角色菜单分配 ===\n\n");
    
    // 初始化账号链表
    g_account_list = init_account_list();
    
    // 加载账号数据
    load_account_list(&g_account_list);
    
    // 查找admin账号
    AccountNode* admin = find_account_by_username(g_account_list, "admin");
    if (admin != NULL) {
        printf("1. admin 账号信息:\n");
        printf("   用户名: %s\n", admin->username);
        printf("   姓名: %s\n", admin->real_name);
        printf("   角色值: %d\n", admin->role);
        printf("   角色枚举值 ROLE_ADMIN = %d\n", ROLE_ADMIN);
        if (admin->role == ROLE_ADMIN) {
            printf("   ✅ 角色匹配正确，应该显示管理员菜单\n");
        } else {
            printf("   ❌ 角色不匹配\n");
        }
    }
    
    printf("\n");
    
    // 查找医生账号
    AccountNode* doctor = find_account_by_username(g_account_list, "D-2001");
    if (doctor != NULL) {
        printf("2. D-2001 账号信息:\n");
        printf("   用户名: %s\n", doctor->username);
        printf("   姓名: %s\n", doctor->real_name);
        printf("   角色值: %d\n", doctor->role);
        printf("   角色枚举值 ROLE_DOCTOR = %d\n", ROLE_DOCTOR);
        if (doctor->role == ROLE_DOCTOR) {
            printf("   ✅ 角色匹配正确，应该显示医生菜单\n");
        } else {
            printf("   ❌ 角色不匹配\n");
        }
    }
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}