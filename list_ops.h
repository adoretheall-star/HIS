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
PatientNode* create_patient_node(const char* id, const char* name, int age, const char* gender, const char* id_card);
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
DoctorNode* create_doctor_node(const char* id, const char* name, const char* gender, const char* dept);
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
WardNode* create_ward_node(const char* room_id, const char* bed_id, WardType ward_type);
void insert_ward_tail(WardNode* head, WardNode* new_node);
WardNode* find_ward_by_id(WardNode* head, const char* target_bed_id);
// ==========================================
// ==========================================
//五、系统账号链表操作
AccountNode* init_account_list();
AccountNode* create_account_node(const char* username, const char* pwd, const char* real_name, const char* gender, RoleType role);
void insert_account_tail(AccountNode* head, AccountNode* new_node);
AccountNode* find_account_by_username(AccountNode* head, const char* target_username);
// ==========================================
// ==========================================
//六、接诊记录链表操作
ConsultRecordNode* init_consult_record_list();
ConsultRecordNode* create_consult_record_node(
    const char* record_id,
    const char* patient_id,
    const char* doctor_id,
    const char* appointment_id,
    const char* consult_time,
    const char* diagnosis_text,
    const char* treatment_advice,
    int decision,
    MedStatus pre_status,
    MedStatus post_status
);
void insert_consult_record_tail(ConsultRecordNode* head, ConsultRecordNode* new_node);
ConsultRecordNode* find_consult_record_by_id(ConsultRecordNode* head, const char* target_record_id);

// ==========================================
// 七、检查项目字典链表操作
// ==========================================
CheckItemNode* init_check_item_list();
CheckItemNode* create_check_item_node(const char* item_id, const char* item_name, const char* dept, double price, MedicareType m_type);
void insert_check_item_tail(CheckItemNode* head, CheckItemNode* new_node);
CheckItemNode* find_check_item_by_id(CheckItemNode* head, const char* target_item_id);

// ==========================================
// 八、检查记录链表操作
// ==========================================
CheckRecordNode* init_check_record_list();
CheckRecordNode* create_check_record_node(
    const char* record_id,
    const char* patient_id,
    const char* item_id,
    const char* item_name,
    const char* dept,
    const char* check_time,
    const char* result,
    int is_completed,
    int is_paid
);
void insert_check_record_tail(CheckRecordNode* head, CheckRecordNode* new_node);
CheckRecordPtrNode* get_check_records_by_patient(CheckRecordNode* head, const char* patient_id);
void free_check_record_ptr_list(CheckRecordPtrNode* head);
CheckRecordNode* find_check_record_by_id(CheckRecordNode* head, const char* target_record_id);
CheckItemPtrNode* find_check_items_by_dept(CheckItemNode* head, const char* dept);
void free_check_item_ptr_list(CheckItemPtrNode* head);
int delete_check_item_by_id(CheckItemNode* head, const char* target_id);

// ==========================================
// 九、安全预警队列操作
// ==========================================
AlertNode* init_alert_list();
void push_system_alert(const char* msg);

// ==========================================
// 十、投诉工单链表操作
// ==========================================
ComplaintNode* init_complaint_list();
ComplaintNode* create_complaint_node(
    const char* complaint_id,
    const char* patient_id,
    int target_type,
    const char* target_id,
    const char* target_name,
    const char* content,
    int status,
    const char* response,
    const char* submit_time
);
void insert_complaint_tail(ComplaintNode* head, ComplaintNode* new_node);


// ==========================================
//六、住院记录链表操作
// ==========================================
InpatientRecord* create_inpatient_record_head();
InpatientRecord* create_inpatient_record_node(
    const char* inpatient_id,
    const char* patient_id,
    const char* bed_id,
    WardType ward_type,
    WardType recommended_ward_type,
    int estimated_days,
    int days_stayed,
    double deposit_balance,
    int is_active
);
void insert_inpatient_record_tail(InpatientRecord* head, InpatientRecord* new_node);
// ==========================================
// 链表销毁函数
// ==========================================
void destroy_all_lists();

#endif // LIST_OPS_H