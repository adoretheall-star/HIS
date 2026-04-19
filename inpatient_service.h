// ==========================================
// 文件名: inpatient_service.h
// 作用: 病房管理与住院业务流程接口
// 描述: 提供住院登记、床位分配、押金管理、日结计费、转床、出院等功能
// ==========================================

#ifndef INPATIENT_SERVICE_H
#define INPATIENT_SERVICE_H

#include "global.h"

// ==========================================
// 1. 病房/床位查询函数
// ==========================================

/**
 * @brief 显示所有床位信息
 */
void show_all_beds();

/**
 * @brief 显示空闲床位信息
 */
void show_free_beds();

/**
 * @brief 显示住院患者信息
 */
void show_hospitalized_patients();

/**
 * @brief 根据患者编号查询住院记录
 * @param patient_id 患者编号
 */
void show_inpatient_record_by_patient_id(const char* patient_id);

// ==========================================
// 2. 住院流程函数
// ==========================================

/**
 * @brief 住院登记
 * @param patient_id 患者编号
 * @param estimated_days 预计住院天数
 * @param deposit 初始押金
 * @param condition_level 病情级别 (1=普通/2=重症)
 * @return 成功返回1，失败返回0
 */
int register_inpatient(const char* patient_id, int estimated_days, double deposit, int condition_level);

/**
 * @brief 为患者分配床位
 * @param patient_id 患者编号
 * @param bed_id 床位编号
 * @return 成功返回1，失败返回0
 */
int assign_bed_to_patient(const char* patient_id, const char* bed_id);

/**
 * @brief 转床
 * @param patient_id 患者编号
 * @param old_bed_id 原床位编号
 * @param new_bed_id 新床位编号
 * @return 成功返回1，失败返回0
 */
int transfer_bed(const char* patient_id, const char* old_bed_id, const char* new_bed_id);

/**
 * @brief 办理出院
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int discharge_patient(const char* patient_id);

// ==========================================
// 3. 押金与计费函数
// ==========================================

/**
 * @brief 为住院患者充值押金
 * @param patient_id 患者编号
 * @param amount 充值金额
 * @return 成功返回1，失败返回0
 */
int recharge_inpatient_deposit(const char* patient_id, double amount);

/**
 * @brief 日结计费
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int daily_settlement(const char* patient_id);

/**
 * @brief 显示押金预警信息
 */
void show_deposit_warnings();

// ==========================================
// 4. 辅助校验函数
// ==========================================

/**
 * @brief 根据床位编号查找床位
 * @param bed_id 床位编号
 * @return 找到返回床位节点，否则返回NULL
 */
WardNode* find_bed_by_id(const char* bed_id);

/**
 * @brief 根据病房类型查找空闲床位
 * @param ward_type 病房类型
 * @return 找到返回床位节点，否则返回NULL
 */
WardNode* find_free_bed_by_type(WardType ward_type);

/**
 * @brief 根据患者编号查找活跃的住院记录
 * @param patient_id 患者编号
 * @return 找到返回住院记录节点，否则返回NULL
 */
InpatientRecord* find_active_inpatient_by_patient_id(const char* patient_id);

/**
 * @brief 检查患者是否已占用床位
 * @param patient_id 患者编号
 * @return 已占用返回1，未占用返回0
 */
int patient_has_bed(const char* patient_id);

/**
 * @brief 检查床位是否可用
 * @param bed_id 床位编号
 * @return 可用返回1，不可用返回0
 */
int bed_is_available(const char* bed_id);

#endif // INPATIENT_SERVICE_H
