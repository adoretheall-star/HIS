#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"
#include "appointment.h"

int main() {
    printf("=== 测试医生列表显示 ===\n\n");
    
    // 初始化链表
    g_doctor_list = init_doctor_list();
    
    // 加载医生数据
    load_doctor_list(&g_doctor_list);
    
    // 测试显示内科医生
    printf("--- 测试显示内科医生 ---\n");
    display_doctors_by_dept("内科");
    
    // 测试显示妇产科医生
    printf("\n--- 测试显示妇产科医生 ---\n");
    display_doctors_by_dept("妇产科");
    
    // 测试显示不存在的科室
    printf("\n--- 测试显示不存在的科室 ---\n");
    display_doctors_by_dept("皮肤科");
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}