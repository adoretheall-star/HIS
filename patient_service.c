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
// 就诊状态转文字
static const char* get_med_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING: return "待诊";
        case STATUS_EXAMINING: return "检查中";
        case STATUS_UNPAID: return "已看诊待缴费";
        case STATUS_WAIT_MED: return "已缴费待取药";
        case STATUS_HOSPITALIZED: return "住院中";
        case STATUS_COMPLETED: return "就诊结束";
        default: return "未知状态";
    }
}

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
// 患者建档（扩展版）
PatientNode* register_patient(
    const char* name,
    int age,
    const char* id_card,
    const char* symptom,
    const char* target_dept
)
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

    if (symptom != NULL)
    {
        strncpy(new_patient->symptom, symptom, MAX_SYMPTOM_LEN - 1);
        new_patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
    }

    if (target_dept != NULL)
    {
        strncpy(new_patient->target_dept, target_dept, MAX_NAME_LEN - 1);
        new_patient->target_dept[MAX_NAME_LEN - 1] = '\0';
    }

    insert_patient_tail(g_patient_list, new_patient);
    mask_id_card(new_patient->id_card, masked_id);
    printf("\n================ 患者建档成功 ================\n");
    printf("患者编号: %s\n", new_patient->id);
    printf("姓名: %s\n", new_patient->name);
    printf("年龄: %d\n", new_patient->age);
    printf("症状描述: %s\n", strlen(new_patient->symptom) > 0 ? new_patient->symptom : "暂无");
    printf("目标科室: %s\n", strlen(new_patient->target_dept) > 0 ? new_patient->target_dept : "暂无");
    printf("当前就诊状态: %s\n", get_med_status_text(new_patient->status));
    printf("身份证号: %s\n", masked_id);
    printf("==============================================\n");

    return new_patient;
}

// 身份核验后查询基础病历信息
int query_basic_patient_record(const char* patient_id, const char* id_card)
{
    char masked_id[19];
    PatientNode* patient = NULL;

    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询病历！\n");
        return 0;
    }

    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (!validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法，无法进行身份核验！\n");
        return 0;
    }

    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，病历查询失败！\n");
        return 0;
    }

    if (strcmp(patient->id_card, id_card) != 0)
    {
        printf("⚠️ 身份核验失败，禁止访问基础病历信息！\n");
        return 0;
    }

    mask_id_card(patient->id_card, masked_id);
    printf("\n================ 基础病历信息 ================\n");
    printf("患者编号: %s\n", patient->id);
    printf("姓名: %s\n", patient->name);
    printf("年龄: %d\n", patient->age);
    printf("症状描述: %s\n", strlen(patient->symptom) > 0 ? patient->symptom : "暂无");
    printf("目标科室: %s\n", strlen(patient->target_dept) > 0 ? patient->target_dept : "暂无");
    printf("当前就诊状态: %s\n", get_med_status_text(patient->status));
    printf("最近一次诊断结论: %s\n",
        strlen(patient->diagnosis_text) > 0 ? patient->diagnosis_text : "暂无诊断记录");
    printf("最近一次处理意见: %s\n",
        strlen(patient->treatment_advice) > 0 ? patient->treatment_advice : "暂无处理意见");
    printf("身份证号: %s\n", masked_id);

    return 1;
}