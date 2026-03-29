// ==========================================
// 文件名: patient_service.c
// 作用: 患者建档相关业务层实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "patient_service.h"
// 生成下一个患者编号
static void generate_patient_id(char* new_id)
{
    int max_no = 0;
    PatientNode* curr = NULL;
    if (new_id == NULL) return;
    if (g_patient_list != NULL)
    {
        curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (strncmp(curr->id, "P-", 2) == 0)
            {
                int current_no = atoi(curr->id + 2);
                if (current_no > max_no)
                {
                    max_no = current_no;
                }
            }
            curr = curr->next;
        }
    }
    snprintf(new_id, MAX_ID_LEN, "P-%03d", max_no + 1);
}
// 按身份证号查找患者
PatientNode* find_patient_by_id_card(const char* id_card)
{
    PatientNode* curr = NULL;
    if (g_patient_list == NULL || id_card == NULL) return NULL;
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->id_card, id_card) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}
// 患者建档
PatientNode* register_patient(const char* name, int age, const char* id_card)
{
    char new_id[MAX_ID_LEN];
    char masked_id[19];
    PatientNode* new_patient = NULL;
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法建档！\n");
        return NULL;
    }
    if (name == NULL || strlen(name) == 0)
    {
        printf("⚠️ 患者姓名不能为空！\n");
        return NULL;
    }
    if (age <= 0)
    {
        printf("⚠️ 患者年龄必须大于 0！\n");
        return NULL;
    }
    if (!validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法，建档失败！\n");
        return NULL;
    }
    if (find_patient_by_id_card(id_card) != NULL)
    {
        printf("⚠️ 该身份证号已存在，不能重复建档！\n");
        return NULL;
    }
    generate_patient_id(new_id);
    new_patient = create_patient_node(new_id, name, age, id_card);
    if (new_patient == NULL)
    {
        printf("⚠️ 患者节点创建失败！\n");
        return NULL;
    }
    strncpy(new_patient->id_card, id_card, MAX_ID_LEN - 1);
    new_patient->id_card[MAX_ID_LEN - 1] = '\0';
    insert_patient_tail(g_patient_list, new_patient);
    mask_id_card(new_patient->id_card, masked_id);
    printf("✅ 患者建档成功！患者编号：%s，身份证号：%s\n", new_patient->id, masked_id);
    return new_patient;
}