//==========================================
// 文件名: patient_service.h
// 作用: 患者建档相关业务层声明
// ==========================================
#ifndef PATIENT_SERVICE_H
#define PATIENT_SERVICE_H
#include "global.h"
// 患者建档
PatientNode* register_patient(const char* name, int age, const char* id_card, const char* symptom);
// 2. 按身份证号查找患者
PatientNode* find_patient_by_id_card(const char* id_card);
// 3. 查询基础病历信息
int query_basic_patient_record(const char* patient_id, const char* id_card);
#endif
