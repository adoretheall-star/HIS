// ==========================================
// 文件名: list_ops.h
// 作用: 封装所有底层双向链表的增删改查操作
// ==========================================

#ifndef LIST_OPS_H
#define LIST_OPS_H

#include "global.h" // 必须引入全局图纸，才能认识 PatientNode 等结构体
//一、患者链表操作
// 1. 初始化带头结点的双向链表
PatientNode* init_patient_list();
// 2. 创建一个干净的患者节点
PatientNode* create_patient_node(const char* id, const char* name, int age, const char* id_card);
// 3. 尾部安全插入法
void insert_patient_tail(PatientNode* head, PatientNode* new_node);
// 4. 按 ID 查找患者 (防瞎找)
PatientNode* find_patient_by_id(PatientNode* head, const char* target_id);
// ==========================================
// ==========================================
//二、预约记录链表操作
AppointmentNode* init_appointment_list();
AppointmentNode* create_appointment_node(const char* appointment_id, const char* patient_id, const char* appointment_date, const char* appointment_slot, const char* appoint_doctor, const char* appoint_dept, AppointmentStatus appointment_status);
void insert_appointment_tail(AppointmentNode* head, AppointmentNode* new_node);
AppointmentNode* find_appointment_by_id(AppointmentNode* head, const char* target_appointment_id);
// ==========================================
// ==========================================
//三、医生链表操作
DoctorNode* init_doctor_list();
DoctorNode* create_doctor_node(const char* id, const char* name, const char* dept);
void insert_doctor_tail(DoctorNode* head, DoctorNode* new_node);
DoctorNode* find_doctor_by_id(DoctorNode* head, const char* target_id);
// ==========================================
// ==========================================
//三、药品链表操作
MedicineNode* init_medicine_list();
MedicineNode* create_medicine_node(
    const char* id,
    const char* name,
    const char* alias,
    const char* generic_name,
    double price,
    int stock,
    MedicareType m_type,
    const char* expiry_date
);
void insert_medicine_tail(MedicineNode* head, MedicineNode* new_node);
MedicineNode* find_medicine_by_id(MedicineNode* head, const char* target_id);
// ==========================================
// ==========================================
//四、病房链表操作
WardNode* init_ward_list();
WardNode* create_ward_node(const char* bed_id);
void insert_ward_tail(WardNode* head, WardNode* new_node);
WardNode* find_ward_by_id(WardNode* head, const char* target_bed_id);
// ==========================================
// ==========================================
//五、系统账号链表操作
AccountNode* init_account_list();
AccountNode* create_account_node(const char* username, const char* pwd, const char* real_name, RoleType role);
void insert_account_tail(AccountNode* head, AccountNode* new_node);
AccountNode* find_account_by_username(AccountNode* head, const char* target_username);
#endif // LIST_OPS_H