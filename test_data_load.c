#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"

int main() {
    printf("=== 测试数据加载功能 ===\n\n");
    
    // 初始化账号链表
    printf("1. 初始化账号链表...\n");
    g_account_list = init_account_list();
    if (g_account_list == NULL) {
        printf("   ❌ 初始化失败\n");
        return 1;
    }
    printf("   ✅ 初始化成功\n");
    
    // 加载账号数据
    printf("\n2. 加载账号数据...\n");
    int load_result = load_account_list(&g_account_list);
    printf("   加载结果: %d\n", load_result);
    
    // 检查账号链表状态
    printf("\n3. 检查账号链表状态...\n");
    if (g_account_list->next == NULL) {
        printf("   ❌ 账号链表为空\n");
        return 1;
    }
    
    // 打印所有账号
    printf("   ✅ 账号链表不为空\n");
    AccountNode* curr = g_account_list->next;
    int count = 0;
    while (curr != NULL) {
        printf("   账号[%d]: %s, 姓名: %s, 角色: %d\n", 
               count++, curr->username, curr->real_name, curr->role);
        curr = curr->next;
    }
    printf("   共加载 %d 个账号\n", count);
    
    // 测试查找账号
    printf("\n4. 测试查找账号 D-2001...\n");
    AccountNode* account = find_account_by_username(g_account_list, "D-2001");
    if (account != NULL) {
        printf("   ✅ 找到账号: %s, 姓名: %s\n", account->username, account->real_name);
    } else {
        printf("   ❌ 未找到账号 D-2001\n");
    }
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}