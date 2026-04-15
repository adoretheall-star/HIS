//==========================================
// 文件名: patient_service.h
// 作用: 患者建档、档案管理与基础病历查询业务接口
// 描述: 提供患者信息管理的核心业务逻辑，包括患者建档、档案查询、信息修改等功能
// ==========================================
#ifndef PATIENT_SERVICE_H
#define PATIENT_SERVICE_H
#include "global.h"
<<<<<<< HEAD

// ==========================================
// 1. 患者建档功能
// ==========================================
/**
 * @brief 患者建档
 * @param name 患者姓名
 * @param age 患者年龄
 * @param id_card 患者身份证号
 * @param symptom 症状描述
 * @param target_dept 目标科室
 * @return 成功返回患者节点指针，失败返回NULL
 */
PatientNode* register_patient(
    const char* name,
    int age,
    const char* id_card,
    const char* symptom,
    const char* target_dept
);

// ==========================================
// 2. 常用查询功能
// ==========================================
/**
 * @brief 根据身份证号查找患者
 * @param id_card 患者身份证号
 * @return 找到返回患者节点指针，未找到返回NULL
 */
=======
// 患者建档
PatientNode* register_patient(const char* name, int age, const char* id_card, const char* symptom);
// 2. 按身份证号查找患者
>>>>>>> medicine
PatientNode* find_patient_by_id_card(const char* id_card);

/**
 * @brief 获取患者状态文本描述
 * @param status 患者就医状态枚举值
 * @return 状态对应的中文描述字符串
 */
const char* get_patient_status_text(MedStatus status);

// ==========================================
// 3. 患者档案管理功能
// ==========================================
/**
 * @brief 显示患者档案详细信息
 * @param patient 患者节点指针
 */
void display_patient_archive(const PatientNode* patient);

/**
 * @brief 根据患者编号查询档案
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id(const char* patient_id);

/**
 * @brief 根据身份证号查询档案
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id_card(const char* id_card);

/**
 * @brief 根据姓名关键词查询档案（支持模糊查询）
 * @param name_keyword 姓名关键词
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_name(const char* name_keyword);

/**
 * @brief 显示患者就诊概览信息
 * @param patient 患者节点指针
 */
void display_patient_visit_overview(const PatientNode* patient);

/**
 * @brief 根据患者编号查询就诊概览
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id(const char* patient_id);

/**
 * @brief 根据身份证号查询就诊概览
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id_card(const char* id_card);

/**
 * @brief 更新患者档案信息
 * @param patient_id 患者编号（不可修改）
 * @param name 新姓名（留空表示不修改）
 * @param age 新年龄（0表示不修改）
 * @param symptom 新症状描述（留空表示不修改）
 * @param target_dept 新目标科室（留空表示不修改）
 * @param id_card 新身份证号（留空表示不修改）
 * @param balance 新账户余额
 * @return 成功返回1，失败返回0
 */
int update_patient_archive(const char* patient_id, const char* name, int age, const char* symptom, const char* target_dept, const char* id_card, double balance);

// ==========================================
// 4. 患者自助基础病历查询功能
// ==========================================
/**
 * @brief 患者自助查询基础病历信息（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_basic_patient_record(const char* patient_id, const char* id_card);
#endif
