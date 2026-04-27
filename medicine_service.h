#ifndef MEDICINE_SERVICE_H
#define MEDICINE_SERVICE_H

#include "global.h"

// 显示所有药品信息（这是药房管理界面的基础看板）
void show_all_medicines();

// 根据关键词搜索药品
void search_medicine_by_keyword(const char* keyword);

// 显示库存不足的药品（库存不足预警引擎）
void show_low_stock_medicines(int threshold);

// 显示即将过期的药品（近效期智能巡检）
void show_expiring_medicines(const char* today, int days_threshold);

// 注册新药品（自动生成规则 ID）
MedicineNode* register_medicine(
    const char* name,
    const char* alias,
    const char* generic_name,
    double price,
    int stock,
    MedicareType m_type,
    const char* expiry_date
);

// 更新药品库存
int update_medicine_stock(const char* med_id, int new_stock);

// 更新药品基本信息
int update_medicine_basic_info(
    const char* med_id,
    const char* new_name,
    const char* new_alias,
    const char* new_generic_name,
    double new_price,
    const char* new_expiry_date
);

// 下架药品
int remove_medicine(const char* med_id);

// 检查药品名称是否已存在（用于提前检测重复）
int is_medicine_name_exists(const char* name);

#endif // MEDICINE_SERVICE_H
