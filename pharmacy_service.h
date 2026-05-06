/*
 * 【代码分工说明】
 * 模块名称：药师发药模块 pharmacy_service.c / pharmacy_service.h
 * 主要负责人：陈苗苗 55251313
 * 主要内容：
 * 1. 实现药师端待发药患者查询、处方核对和发药确认；
 * 2. 实现库存扣减、患者状态更新和发药记录；
 * 3. 实现患者处方查询和用药指导提示；
 * 4. 参与药房业务流程联调和处方数据管理。
 * 参与说明：
 * 周宇轩 55251328 参与发药流程与患者状态联动逻辑。
 */
#ifndef PHARMACY_SERVICE_H
#define PHARMACY_SERVICE_H

#include "medicine_service.h"

void show_paid_patients_waiting_for_dispense(void);
int dispense_medicine_for_patient(const char* patient_id);

#endif // PHARMACY_SERVICE_H