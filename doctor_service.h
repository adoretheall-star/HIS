// ==========================================
// 文件名: doctor_service.h
// 作用: 医生接诊相关业务层声明
// ==========================================

#ifndef DOCTOR_SERVICE_H
#define DOCTOR_SERVICE_H

#include "global.h"

// 1. 查看指定医生的待诊患者
void show_waiting_patients_by_doctor(const char* doctor_id);

// 2. 医生接诊并做最小诊疗决策
int doctor_consult_patient(
    const char* doctor_id,
    const char* patient_id,
    int decision,
    const char* diagnosis_text,
    const char* treatment_advice
);

#endif