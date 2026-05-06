/*
 * 【代码分工说明】
 * 模块名称：药品管理模块 medicine_service.c / medicine_service.h
 * 主要负责人：陈苗苗 55251313
 * 主要内容：
 * 1. 实现药品信息新增、修改、查询和删除；
 * 2. 实现药品库存维护、低库存预警和近效期巡检；
 * 3. 实现药品分类管理、药品入库和库存盘点；
 * 4. 为药师端提供药品基础数据支持。
 * 参与说明：
 * 申贞隆 55251318 参与药品输入校验和异常拦截；
 * 胡博畅 55251329 参与药品界面排版和库存预警显示优化。
 */
#ifndef MEDICINE_SERVICE_H
#define MEDICINE_SERVICE_H

#include "global.h"

// 显示所有药品信息（这是药房管理界面的基础看板）
void show_all_medicines();

// 根据关键词搜索药品（返回是否找到结果）
int search_medicine_by_keyword(const char* keyword);

// 检查是否有药品匹配关键词（返回是否找到结果，不打印任何内容）
int has_medicine_match(const char* keyword);

// 显示库存不足的药品（库存不足预警引擎）
void show_low_stock_medicines(int threshold);
void show_low_stock_medicines_with_title(int threshold, int show_title);

// 显示即将过期的药品（近效期智能巡检）
void show_expiring_medicines(const char* today, int days_threshold);
void show_expiring_medicines_with_title(const char* today, int days_threshold, int show_title);

// 显示综合库存预警（近效期+低库存，分区显示）
void show_comprehensive_stock_alert(int low_stock_threshold, const char* today, int expiring_days_threshold);

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

// 软删除药品到回收站
int soft_remove_medicine_to_recycle(const char* med_id, const char* deleted_by, const char* reason);

// 检查药品名称是否已存在（用于提前检测重复）
int is_medicine_name_exists(const char* name);

#endif // MEDICINE_SERVICE_H
