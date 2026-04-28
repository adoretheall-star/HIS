#include <stdio.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"
#include "patient_service.h"

// 全局变量
PatientNode* g_patient_list = NULL;

int main() {
    // 初始化患者链表
    g_patient_list = init_patient_list();
    
    // 加载患者数据
    printf("加载患者数据...\n");
    int load_result = load_patient_list(&g_patient_list);
    printf("加载结果: %d\n", load_result);
    
    // 检查患者链表
    if (g_patient_list->next == NULL) {
        printf("患者链表为空\n");
    } else {
        printf("患者链表不为空\n");
        // 遍历所有患者
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL) {
            printf("患者: ID=%s, 姓名=%s, 身份证=%s\n", 
                   curr->id, curr->name, curr->id_card);
            curr = curr->next;
        }
    }
    
    // 测试查询
    const char* id_card = "110105199004041237";
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient != NULL) {
        printf("\n通过身份证号 %s 找到患者:\n", id_card);
        printf("ID: %s\n", patient->id);
        printf("姓名: %s\n", patient->name);
        printf("身份证: %s\n", patient->id_card);
    } else {
        printf("\n未找到身份证号为 %s 的患者\n", id_card);
    }
    
    return 0;
}
