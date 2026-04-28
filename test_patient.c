#include <stdio.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "data_io.h"
#include "patient_service.h"

int main() {
    // 初始化患者链表（使用全局变量）
    g_patient_list = init_patient_list();
    
    // 加载患者数据
    int load_result = load_patient_list(&g_patient_list);
    printf("Load result: %d\n", load_result);
    
    // 打印链表状态
    if (g_patient_list->next == NULL) {
        printf("Patient list is empty\n");
    } else {
        printf("Patient list is not empty\n");
        // 打印第一个患者信息
        PatientNode* first = g_patient_list->next;
        printf("First patient: ID=%s, Name=%s, ID Card=%s\n", 
               first->id, first->name, first->id_card);
    }
    
    // 查找患者
    PatientNode* patient = find_patient_by_id_card("110105199004041237");
    if (patient != NULL) {
        printf("Found patient: ID=%s, Name=%s, ID Card=%s\n", 
               patient->id, patient->name, patient->id_card);
    } else {
        printf("Patient not found\n");
    }
    
    // 测试患者编号和身份证号的匹配
    const char* patient_id = "P-1004";
    const char* id_card = "110105199004041237";
    
    patient = find_patient_by_id_card(id_card);
    if (patient != NULL) {
        printf("\nInput: patient_id=%s, id_card=%s\n", patient_id, id_card);
        printf("Found: patient_id=%s, id_card=%s\n", patient->id, patient->id_card);
        printf("Match: %s\n", strcmp(patient->id, patient_id) == 0 ? "Yes" : "No");
    } else {
        printf("Patient not found by ID card\n");
    }
    
    return 0;
}
