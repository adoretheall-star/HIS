//==========================================
// 文件名: patient_service.h
// 作用: 患者相关业务服务层 / 患者数据服务层
// 描述: 提供患者信息管理的核心业务逻辑，被内部业务端和患者自助端共同复用
// 说明: 本模块是底层服务层，不包含菜单和界面逻辑，只提供业务数据处理能力
// 注意: 底层业务函数可被内部业务端（代患者办理）和患者自助端（本人自助服务）共同调用
// ==========================================
#ifndef PATIENT_SERVICE_H
#define PATIENT_SERVICE_H
#include "global.h"

// ==========================================
// 内部业务端会调用的患者数据管理能力（需要内部登录）
// ==========================================
/**
 * @brief 患者建档（内部业务功能：代患者办理建档）
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
    const char* gender,
    const char* id_card,
    const char* symptom,
    const char* target_dept
);

/**
 * @brief 根据身份证号查找患者（通用辅助函数：内部业务端和患者自助端均可使用）
 * @param id_card 患者身份证号
 * @return 找到返回患者节点指针，未找到返回NULL
 */
PatientNode* find_patient_by_id_card(const char* id_card);

/**
 * @brief 获取患者状态文本描述（通用辅助函数：内部业务端和患者自助端均可使用）
 * @param status 患者就医状态枚举值
 * @return 状态对应的中文描述字符串
 */
const char* get_patient_status_text(MedStatus status);

/**
 * @brief 根据症状智能推荐科室（智能导诊功能）
 * @param symptom 患者症状描述
 * @return 推荐的科室名称字符串
 */
const char* recommend_dept_by_symptom(const char* symptom);

/**
 * @brief 显示患者档案详细信息（内部业务功能：档案管理）
 * @param patient 患者节点指针
 */
void display_patient_archive(const PatientNode* patient);

/**
 * @brief 根据患者编号查询档案（内部业务功能：档案管理）
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id(const char* patient_id);

/**
 * @brief 根据身份证号查询档案（内部业务功能：档案管理）
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id_card(const char* id_card);

/**
 * @brief 根据姓名关键词查询档案（内部业务功能：档案管理，支持模糊查询）
 * @param name_keyword 姓名关键词
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_name(const char* name_keyword);

/**
 * @brief 提交患者满意度评价
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int submit_patient_evaluation(const char* patient_id);

/**
 * @brief 提交新的服务投诉
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int submit_new_complaint(const char* patient_id);

/**
 * @brief 查询患者的历史投诉记录
 * @param patient_id 患者编号
 */
void query_patient_complaints(const char* patient_id);

/**
 * @brief 更新患者档案信息（内部业务功能：档案维护）
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
// 业务辅助功能
// ==========================================
/**
 * @brief 检查并作废过期的待缴费订单
 * @param patient 患者节点指针
 */
void check_and_void_expired_orders(PatientNode* patient);

// ==========================================
// 患者自助端会调用的查询能力（外部直接访问）
// ==========================================
/**
 * @brief 患者自助查询基础病历信息（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_basic_patient_record(const char* patient_id, const char* id_card);

/**
 * @brief 根据患者编号查询就诊概览（患者自助功能：查询本人就诊概览）
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id(const char* patient_id);

/**
 * @brief 根据身份证号查询就诊概览（患者自助功能：查询本人就诊概览）
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id_card(const char* id_card);

/**
 * @brief 患者自助查询自己的历史就诊记录（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_patient_consult_history_verified(const char* patient_id, const char* id_card);

/**
 * @brief 处理急诊逃单
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int handle_emergency_escape(const char* patient_id);

/**
 * @brief 补缴欠费并核销黑名单
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int settle_blacklisted_debt(const char* patient_id);

/**
 * @brief 处理患者自助缴费（扣减余额并推进状态）
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int process_patient_payment(const char* patient_id);

#endif
