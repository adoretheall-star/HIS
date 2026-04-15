#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "utils.h"


static void print_waiting_patient_info(const PatientNode* patient)
{
    if (patient == NULL)
    {
        return;
    }

    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("当前状态：已缴费待取药\n");
    printf("待发药品种数：%d\n", patient->script_count);
    printf("------------------------------------------------------\n");
}

void show_paid_patients_waiting_for_dispense()
{
    int found = 0;
    PatientNode* curr = NULL;

    if (g_patient_list == NULL)
    {
        printf("提示：患者链表尚未初始化，无法查询待发药患者。\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                  待发药患者列表\n");
    printf("======================================================\n");

    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (curr->status == STATUS_PAID && curr->script_head != NULL && curr->script_count > 0)
        {
            print_waiting_patient_info(curr);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found)
    {
        printf("当前无待发药患者\n");
    }
}

int dispense_medicine_for_patient(const char* patient_id)
{
    PatientNode* patient = NULL;
    PrescriptionNode* curr_script = NULL;
    const int LOW_STOCK_THRESHOLD = 5;

    if (g_patient_list == NULL || g_medicine_list == NULL)
    {
        printf("提示：系统数据尚未初始化，无法执行发药。\n");
        return 0;
    }

    if (is_blank_string(patient_id))
    {
        printf("提示：患者编号不能为空。\n");
        return 0;
    }

    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("提示：未找到对应患者，发药失败。\n");
        return 0;
    }

    if (patient->status != STATUS_PAID)
    {
        printf("提示：当前患者不处于待发药状态，无法发药。\n");
        return 0;
    }

    if (patient->script_head == NULL || patient->script_count <= 0)
    {
        printf("提示：当前患者无待发药处方，无法发药。\n");
        return 0;
    }

    curr_script = patient->script_head;
    while (curr_script != NULL)
    {
        MedicineNode* med = find_medicine_by_id(g_medicine_list, curr_script->med_id);

        if (med == NULL)
        {
            printf("提示：处方中存在无效药品编号 %s，发药失败。\n", curr_script->med_id);
            return 0;
        }

        if (curr_script->quantity <= 0)
        {
            printf("提示：处方中药品 %s 的数量非法，发药失败。\n", curr_script->med_id);
            return 0;
        }

        if (med->stock < curr_script->quantity)
        {
            printf("提示：药品库存不足，无法完成发药。\n");
            printf("药品编号：%s\n", med->id);
            printf("商品名：%s\n", med->name);
            printf("当前库存：%d\n", med->stock);
            printf("处方数量：%d\n", curr_script->quantity);
            return 0;
        }

        curr_script = curr_script->next;
    }

    printf("\n======================================================\n");
    printf("                    发药明细\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("------------------------------------------------------\n");

    curr_script = patient->script_head;
    while (curr_script != NULL)
    {
        MedicineNode* med = find_medicine_by_id(g_medicine_list, curr_script->med_id);

        med->stock -= curr_script->quantity;
        printf("药品编号：%s\n", med->id);
        printf("商品名：%s\n", med->name);
        printf("发药数量：%d\n", curr_script->quantity);
        printf("发药后库存：%d\n", med->stock);
        
        // 低库存预警
        if (med->stock == 0)
        {
            printf("⚠️ 预警：药品 [%s] [%s] 当前库存为 0，已缺货，请及时补货。\n", med->id, med->name);
        }
        else if (med->stock < LOW_STOCK_THRESHOLD)
        {
            printf("⚠️ 预警：药品 [%s] [%s] 当前库存为 %d，已进入低库存状态，请及时补货。\n", med->id, med->name, med->stock);
        }
        
        printf("------------------------------------------------------\n");

        curr_script = curr_script->next;
    }

    patient->status = STATUS_COMPLETED;
    printf("提示：发药完成，患者就诊状态已更新为就诊结束。\n");
    return 1;
}