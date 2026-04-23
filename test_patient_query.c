#include <stdio.h>
#include "list_ops.h"
#include "data_io.h"

int main() {
    // 初始化患者链表
    PatientNode* patient_list = init_patient_list();
    printf("1. 初始化患者链表成功\n");
    
    // 加载患者数据
    int load_result = load_patient_list(&patient_list);
    printf("2. 加载患者数据结果: %d\n", load_result);
    
    // 测试查询患者
    PatientNode* patient = find_patient_by_id(patient_list, "P-1001");
    if (patient != NULL) {
        printf("3. 找到患者: %s, %s\n", patient->id, patient->name);
    } else {
        printf("3. 未找到患者 P-1001\n");
    }
    
    // 统计患者数量
    PatientNode* curr = patient_list->next;
    int count = 0;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    printf("4. 总共有 %d 个患者\n", count);
    
    return 0;
}