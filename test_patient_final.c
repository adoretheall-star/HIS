#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "data/"

// 患者节点结构
typedef struct PatientNode {
    char id[20];
    char name[50];
    char gender[10];
    int age;
    char id_card[20];
    struct PatientNode* next;
} PatientNode;

// 链表头结构
typedef struct PatientList {
    PatientNode* next;
} PatientList;

// 初始化患者链表
PatientList* init_patient_list() {
    PatientList* list = (PatientList*)malloc(sizeof(PatientList));
    if (list) {
        list->next = NULL;
    }
    return list;
}

// 修剪换行符
void trim_newline(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len = strlen(str);
    }
}

// 分割字符串
int split_line_by_delimiter(const char* line, char delimiter, char** fields, int max_fields) {
    if (line == NULL || fields == NULL || max_fields <= 0) return 0;
    
    static char line_copy[8192];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    int count = 0;
    char* start = line_copy;
    char* p = line_copy;
    
    while (*p && count < max_fields) {
        if (*p == delimiter) {
            *p = '\0';
            fields[count++] = start;
            start = p + 1;
        }
        p++;
    }
    fields[count++] = start;
    
    return count;
}

// 加载患者数据
int load_patient_list(PatientList** head) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%spatients.txt", DATA_DIR);
    printf("DEBUG: 正在读取患者文件: %s\n", file_path);
    
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("DEBUG: 无法打开患者文件: %s\n", file_path);
        return 0;
    }

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    char line[512];
    int loaded_count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char* fields[30];
        int field_count = split_line_by_delimiter(line, '|', fields, 30);
        
        if (field_count < 5) {
            printf("DEBUG: 字段数不足，跳过此行: %s (字段数: %d)\n", line, field_count);
            continue;
        }
        
        // 创建新节点
        PatientNode* new_node = (PatientNode*)malloc(sizeof(PatientNode));
        if (new_node == NULL) continue;

        // 只加载必要的字段
        strcpy(new_node->id, fields[0]);
        strcpy(new_node->name, fields[1]);
        strcpy(new_node->gender, fields[2]);
        new_node->age = atoi(fields[3]);
        strcpy(new_node->id_card, fields[4]);
        new_node->next = NULL;

        // 添加到链表
        if ((*head)->next == NULL) {
            (*head)->next = new_node;
        } else {
            PatientNode* curr = (*head)->next;
            while (curr->next != NULL) {
                curr = curr->next;
            }
            curr->next = new_node;
        }
        loaded_count++;
        printf("DEBUG: 加载患者: ID=%s, 姓名=%s, 身份证=%s\n", new_node->id, new_node->name, new_node->id_card);
    }
    fclose(fp);
    printf("DEBUG: 成功加载 %d 个患者\n", loaded_count);
    return 1;
}

// 根据身份证号查找患者
PatientNode* find_patient_by_id_card(PatientList* list, const char* id_card) {
    if (list == NULL || id_card == NULL) return NULL;
    
    PatientNode* curr = list->next;
    while (curr != NULL) {
        if (strcmp(curr->id_card, id_card) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

int main() {
    // 初始化患者链表
    PatientList* patient_list = init_patient_list();
    if (patient_list == NULL) {
        printf("初始化链表失败\n");
        return 1;
    }
    
    // 加载患者数据
    int load_result = load_patient_list(&patient_list);
    printf("加载结果: %d\n", load_result);
    
    // 测试查询
    const char* test_id_card = "110105199004041237";
    const char* test_patient_id = "P-1004";
    
    printf("\n测试查询: 患者编号=%s, 身份证号=%s\n", test_patient_id, test_id_card);
    
    PatientNode* patient = find_patient_by_id_card(patient_list, test_id_card);
    if (patient != NULL) {
        printf("找到患者: ID=%s, 姓名=%s, 身份证=%s\n", patient->id, patient->name, patient->id_card);
        
        if (strcmp(patient->id, test_patient_id) == 0) {
            printf("✅ 患者编号与身份证号匹配\n");
        } else {
            printf("❌ 患者编号与身份证号不匹配\n");
            printf("  输入的患者编号: %s\n", test_patient_id);
            printf("  找到的患者编号: %s\n", patient->id);
        }
    } else {
        printf("❌ 未找到身份证号为 %s 的患者\n", test_id_card);
    }
    
    return 0;
}
