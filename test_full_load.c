#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"

int main() {
    printf("=== 完整数据加载测试 ===\n\n");
    
    // 初始化所有链表
    printf("1. 初始化所有链表...\n");
    g_patient_list = init_patient_list();
    g_appointment_list = init_appointment_list();
    g_doctor_list = init_doctor_list();
    g_medicine_list = init_medicine_list();
    g_ward_list = init_ward_list();
    g_account_list = init_account_list();
    g_consult_record_list = init_consult_record_list();
    g_check_item_list = init_check_item_list();
    g_check_record_list = init_check_record_list();
    g_alert_list = init_alert_list();
    g_complaint_list = init_complaint_list();
    g_inpatient_list = create_inpatient_record_head();
    printf("   ✅ 所有链表初始化成功\n");
    
    // 加载所有数据
    printf("\n2. 加载所有数据...\n");
    int load_result = load_all_data();
    printf("   加载结果: %d (%s)\n", load_result, load_result ? "成功" : "失败");
    
    // 检查账号链表
    printf("\n3. 检查账号数据...\n");
    if (g_account_list->next == NULL) {
        printf("   ❌ 账号链表为空\n");
    } else {
        int count = 0;
        AccountNode* curr = g_account_list->next;
        while (curr != NULL) { count++; curr = curr->next; }
        printf("   ✅ 共加载 %d 个账号\n", count);
        
        // 测试查找账号
        AccountNode* account = find_account_by_username(g_account_list, "D-2001");
        if (account != NULL) {
            printf("   ✅ 成功找到账号 D-2001\n");
        } else {
            printf("   ❌ 未找到账号 D-2001\n");
        }
    }
    
    // 检查患者链表
    printf("\n4. 检查患者数据...\n");
    if (g_patient_list->next == NULL) {
        printf("   ❌ 患者链表为空\n");
    } else {
        int count = 0;
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL) { count++; curr = curr->next; }
        printf("   ✅ 共加载 %d 个患者\n", count);
    }
    
    // 检查医生链表
    printf("\n5. 检查医生数据...\n");
    if (g_doctor_list->next == NULL) {
        printf("   ❌ 医生链表为空\n");
    } else {
        int count = 0;
        DoctorNode* curr = g_doctor_list->next;
        while (curr != NULL) { count++; curr = curr->next; }
        printf("   ✅ 共加载 %d 个医生\n", count);
    }
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}