// ==========================================
// 文件名: doctor_service.h
// 作用: 医生接诊相关业务层声明
// ==========================================

#ifndef DOCTOR_SERVICE_H
#define DOCTOR_SERVICE_H

#include "global.h"

// 1. 查看指定医生的待诊患者
void show_waiting_patients_by_doctor(const char* doctor_id);

// 获取指定医生的待诊患者列表（返回患者数量）
int get_waiting_patients_by_doctor(const char* doctor_id, PatientNode** patient_list);

// 获取指定医生的已处理患者列表（返回患者数量）
int get_processed_patients_by_doctor(const char* doctor_id, PatientNode** patient_list);

// 2. 医生查看患者接诊前概览
void doctor_view_patient_overview(const char* doctor_id, const char* patient_id);

// 3. 医生接诊并做最小诊疗决策
int doctor_consult_patient(
    const char* doctor_id,
    const char* patient_id,
    int decision,
    const char* diagnosis_text,
    const char* treatment_advice
);

// 4. 医生查看已处理患者
void doctor_view_processed_patients(const char* doctor_id);

// 5. 医生查看已处理患者详情
void doctor_view_processed_patient_detail(const char* doctor_id, const char* patient_id);

// 6. 查看历史接诊记录
void doctor_view_consult_history(const char* doctor_id, const char* patient_id);

// 7. 检查完成回诊登记
int complete_exam_and_return_to_doctor(const char* patient_id);

#endif