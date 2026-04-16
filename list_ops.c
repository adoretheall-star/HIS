// ==========================================
// 文件名: list_ops.c
// 作用: 链表核心操作的具体实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h" 
#include "list_ops.h"

// 🚀 极其重要：在这里为 global.h 中声明的全局头指针真正分配内存空间！
// 如果不写这几行，整个系统就会报“未解析的外部符号”错误。
PatientNode* g_patient_list = NULL;
AppointmentNode* g_appointment_list = NULL;
DoctorNode* g_doctor_list  = NULL;
MedicineNode* g_medicine_list = NULL;
WardNode* g_ward_list    = NULL;
AccountNode* g_account_list = NULL;
ConsultRecordNode* g_consult_record_list = NULL;
CheckItemNode* g_check_item_list = NULL;     // 检查项目字典
CheckRecordNode* g_check_record_list = NULL; // 检查记录
AlertNode* g_alert_list = NULL;              // 安全预警队列
ComplaintNode* g_complaint_list = NULL;      // 投诉工单链表
//一、患者链表操作
// ---------------------------------------------------------
// 功能 1：初始化带头结点的双向链表
// ---------------------------------------------------------
PatientNode* init_patient_list() 
{
    PatientNode* head = (PatientNode*)malloc(sizeof(PatientNode));
    if (head == NULL) 
    {
        printf("🔥 致命错误：内存分配失败，系统无法启动！\n");
        exit(1); // 内存都没了，直接让程序自杀（1表示程序异常退出）
    }
    
    // 给头结点打上标记，防止和真实数据混淆
    // 安全的写法，确保不会超出字符数组的边界，防止缓冲区溢出
    strncpy(head->id, "HEAD", MAX_ID_LEN - 1);
    head->id[MAX_ID_LEN - 1] = '\0';

    strncpy(head->name, "SYSTEM_HEAD", MAX_NAME_LEN - 1);
    head->name[MAX_NAME_LEN - 1] = '\0';
    
    // 💡 防御阵地：头结点的指针必须干干净净
    head->script_head = NULL; 
    head->prev = NULL;
    head->next = NULL;
    
    return head;
}

// ---------------------------------------------------------
// 功能 2：在内存中捏造一个“干净”的患者节点
// ---------------------------------------------------------
PatientNode* create_patient_node(const char* id, const char* name, int age, const char* id_card) {
    PatientNode* new_node = (PatientNode*)malloc(sizeof(PatientNode));
    if (new_node == NULL) return NULL;

    // 录入基础信息
    strncpy(new_node->id, id, MAX_ID_LEN - 1);
    new_node->id[MAX_ID_LEN - 1] = '\0';

    strncpy(new_node->name, name, MAX_NAME_LEN - 1);
    new_node->name[MAX_NAME_LEN - 1] = '\0';
    new_node->age = age;
    
    strncpy(new_node->id_card, id_card, MAX_ID_LEN - 1);
    new_node->id_card[MAX_ID_LEN - 1] = '\0';
    
    // 初始化字符串字段
    new_node->symptom[0] = '\0';
    new_node->target_dept[0] = '\0';
    new_node->doctor_id[0] = '\0';
    new_node->diagnosis_text[0] = '\0';
    new_node->treatment_advice[0] = '\0';
    new_node->card_id[0] = '\0';
    
    // 初始化业务状态
    new_node->balance = 0.0;
    new_node->status = STATUS_PENDING; // 默认待诊状态
    
    // 🚀 终极防御：必须把该患者的处方小链表头指针置空！
    // 否则里面是乱码内存，药房一发药就会直接导致系统闪退！
    new_node->script_head = NULL;
    new_node->script_count = 0;
    
    // 初始化信用黑名单相关字段
    new_node->missed_time_1 = 0;
    new_node->missed_time_2 = 0;
    new_node->missed_time_3 = 0;
    new_node->missed_count = 0;
    new_node->blacklist_expire = 0;
    new_node->is_blacklisted = 0;
    new_node->is_emergency = 0;
    new_node->queue_time = 0; // 初始化排队时间戳为0
    new_node->call_count = 0; // 初始化叫号未到次数为0
    new_node->emergency_debt = 0.0; // 初始化急诊欠费金额为0
    new_node->unpaid_time = 0;

    // 断开一切外界联系，等待被插入大链表
    new_node->prev = NULL;
    new_node->next = NULL;

    return new_node;
}

// ---------------------------------------------------------
// 功能 3：尾插法 (将新患者排到链表最后)
// ---------------------------------------------------------
void insert_patient_tail(PatientNode* head, PatientNode* new_node)
 {
    // 💡 防御阵地：永远怀疑指针是空的
    if (head == NULL || new_node == NULL) return;

    PatientNode* curr = head;
    // 沿着 next 指针一直走到黑，直到遇到 NULL
    while (curr->next != NULL) {
        curr = curr->next;
    }

    // 此时 curr 绝对停在最后一个节点上，开始双向绑定！
    curr->next = new_node;       // 老尾巴的 next 指向新节点
    new_node->prev = curr;       // 新节点的 prev 指向老尾巴
    // new_node 的 next 已经在 create 阶段设为 NULL 了，完美收尾。
}
// 功能 4：查找
PatientNode* find_patient_by_id(PatientNode* head, const char* target_id) 
{
    if (head == NULL || target_id == NULL) return NULL;
    PatientNode* curr = head->next; 
// 🚀 细节：跳过无用的 HEAD 头结点，直接从第一个真实数据开始找
    while (curr != NULL)
    {
        if (strcmp(curr->id, target_id) == 0) return curr; // 找到了，直接把这个人拎出来！
        curr = curr->next;
    }
    return NULL; // 找遍了也没这个人，返回空指针
}
// 二、预约记录链表操作
AppointmentNode* init_appointment_list() 
{
    AppointmentNode* head = (AppointmentNode*)malloc(sizeof(AppointmentNode));
    if (head == NULL) exit(1);
    strncpy(head->appointment_id, "HEAD", MAX_ID_LEN - 1);
    head->appointment_id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

AppointmentNode* create_appointment_node(const char* appointment_id, const char* patient_id, const char* appointment_date, const char* appointment_slot, const char* appoint_doctor, const char* appoint_dept, AppointmentStatus appointment_status) 
{
    AppointmentNode* new_node = (AppointmentNode*)malloc(sizeof(AppointmentNode));
    if (new_node == NULL) return NULL;
    
    strncpy(new_node->appointment_id, appointment_id, MAX_ID_LEN - 1);
    new_node->appointment_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->patient_id, patient_id, MAX_ID_LEN - 1);
    new_node->patient_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->appointment_date, appointment_date, MAX_NAME_LEN - 1);
    new_node->appointment_date[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(new_node->appointment_slot, appointment_slot, MAX_NAME_LEN - 1);
    new_node->appointment_slot[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(new_node->appoint_doctor, appoint_doctor, MAX_NAME_LEN - 1);
    new_node->appoint_doctor[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(new_node->appoint_dept, appoint_dept, MAX_NAME_LEN - 1);
    new_node->appoint_dept[MAX_NAME_LEN - 1] = '\0';
    
    new_node->appointment_status = appointment_status;
    
    new_node->prev = NULL; new_node->next = NULL;
    return new_node;
}

void insert_appointment_tail(AppointmentNode* head, AppointmentNode* new_node) 
{
    if (head == NULL || new_node == NULL) return;
    AppointmentNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node; new_node->prev = curr;
}

AppointmentNode* find_appointment_by_id(AppointmentNode* head, const char* target_appointment_id) 
{
    if (head == NULL || target_appointment_id == NULL) return NULL;
    AppointmentNode* curr = head->next;
    while (curr != NULL) 
    {
        if (strcmp(curr->appointment_id, target_appointment_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}
// 三、医生链表操作
DoctorNode* init_doctor_list() 
{
    DoctorNode* head = (DoctorNode*)malloc(sizeof(DoctorNode));
    if (head == NULL) exit(1);
    strncpy(head->id, "HEAD", MAX_ID_LEN - 1);
    head->id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

DoctorNode* create_doctor_node(const char* id, const char* name, const char* dept) 
{
    DoctorNode* new_node = (DoctorNode*)malloc(sizeof(DoctorNode));
    if (new_node == NULL) return NULL;
    strncpy(new_node->id, id, MAX_ID_LEN - 1);
    new_node->id[MAX_ID_LEN - 1] = '\0';
    strncpy(new_node->name, name, MAX_NAME_LEN - 1);
    new_node->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(new_node->department, dept, MAX_NAME_LEN - 1);
    new_node->department[MAX_NAME_LEN - 1] = '\0';
    new_node->queue_length = 0; // 初始排队人数为0
    new_node->prev = NULL; 
    new_node->next = NULL;
    return new_node;
}

void insert_doctor_tail(DoctorNode* head, DoctorNode* new_node) 
{
    if (head == NULL || new_node == NULL) return;
    DoctorNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node; new_node->prev = curr;
}
DoctorNode* find_doctor_by_id(DoctorNode* head, const char* target_id) 
{
    if (head == NULL || target_id == NULL) return NULL;
    DoctorNode* curr = head->next;
    while (curr != NULL) 
    {
        if (strcmp(curr->id, target_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}
//四、药品链表操作
MedicineNode* init_medicine_list() 
{
    MedicineNode* head = (MedicineNode*)malloc(sizeof(MedicineNode));
    if (head == NULL) exit(1);
    strncpy(head->id, "HEAD", MAX_ID_LEN - 1);
    head->id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

MedicineNode* create_medicine_node(const char* id, const char* name, double price, int stock, MedicareType m_type) 
{
    MedicineNode* new_node = (MedicineNode*)malloc(sizeof(MedicineNode));
    if (new_node == NULL) return NULL;
    strncpy(new_node->id, id, MAX_ID_LEN - 1);
    new_node->id[MAX_ID_LEN - 1] = '\0';
    strncpy(new_node->name, name, MAX_NAME_LEN - 1);
    new_node->name[MAX_NAME_LEN - 1] = '\0';
    new_node->price = price; new_node->stock = stock; new_node->m_type = m_type;
    new_node->prev = NULL; new_node->next = NULL;
    return new_node;
}

void insert_medicine_tail(MedicineNode* head, MedicineNode* new_node) 
{
    if (head == NULL || new_node == NULL) return;
    MedicineNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node; new_node->prev = curr;
}
MedicineNode* find_medicine_by_id(MedicineNode* head, const char* target_id) 
{
    if (head == NULL || target_id == NULL) return NULL;
    MedicineNode* curr = head->next;
    while (curr != NULL) 
    {
        if (strcmp(curr->id, target_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}
//五、病房床位链表操作
WardNode* init_ward_list() 
{
    WardNode* head = (WardNode*)malloc(sizeof(WardNode));
    if (head == NULL) exit(1);
    strncpy(head->bed_id, "HEAD", MAX_ID_LEN - 1);
    head->bed_id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

WardNode* create_ward_node(const char* bed_id) 
{
    WardNode* new_node = (WardNode*)malloc(sizeof(WardNode));
    if (!new_node) return NULL;
    strncpy(new_node->bed_id, bed_id, MAX_ID_LEN - 1);
    new_node->bed_id[MAX_ID_LEN - 1] = '\0';
    new_node->is_occupied = 0; // 默认空闲
    new_node->patient_id[0] = '\0'; // 暂无病人
    new_node->prev = NULL; new_node->next = NULL;
    return new_node;
}

void insert_ward_tail(WardNode* head, WardNode* new_node) 
{
    if (head == NULL || new_node == NULL) return;
    WardNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node; new_node->prev = curr;
}
WardNode* find_ward_by_id(WardNode* head, const char* target_bed_id) 
{
    if  (head == NULL || target_bed_id == NULL) return NULL;
    WardNode* curr = head->next;
    while (curr != NULL)
     {
        if (strcmp(curr->bed_id, target_bed_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}
//六、账号权限链表操作
AccountNode* init_account_list() 
{
    AccountNode* head = (AccountNode*)malloc(sizeof(AccountNode));
    if (head == NULL) exit(1);
    strncpy(head->username, "HEAD", MAX_ID_LEN - 1);
    head->username[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

AccountNode* create_account_node(const char* username, const char* pwd, const char* real_name, RoleType role) 
{
    AccountNode* new_node = (AccountNode*)malloc(sizeof(AccountNode));
    if (new_node == NULL) return NULL;
    strncpy(new_node->username, username, MAX_ID_LEN - 1);
    new_node->username[MAX_ID_LEN - 1] = '\0';
    strncpy(new_node->password, pwd, MAX_ID_LEN - 1);
    new_node->password[MAX_ID_LEN - 1] = '\0';
    strncpy(new_node->real_name, real_name, MAX_NAME_LEN - 1);
    new_node->real_name[MAX_NAME_LEN - 1] = '\0';
    new_node->role = role;
    new_node->error_count = 0;
    new_node->lock_time = 0;
    new_node->prev = NULL; new_node->next = NULL;
    return new_node;
}

void insert_account_tail(AccountNode* head, AccountNode* new_node) 
{
    if (head == NULL || new_node == NULL ) return;
    AccountNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node; new_node->prev = curr;
}
AccountNode* find_account_by_username(AccountNode* head, const char* target_username) 
{
    if (head == NULL || target_username == NULL) return NULL;
    AccountNode* curr = head->next;
    while (curr != NULL) 
    {
        if (strcmp(curr->username, target_username) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}
// ==========================================
// 六、 功能：安全删除节点 (双向链表断链重建术)
// ==========================================

int delete_patient_by_id(PatientNode* head, const char* target_id) 
{
    PatientNode* target = find_patient_by_id(head, target_id);
    if (target == NULL) return 0; // 没找到这个人，删除失败

    // 1. 让前面的节点，指向后面的节点
    if (target->prev != NULL) { target->prev->next = target->next; }
    
    // 2. 让后面的节点，指向前面的节点 (如果 target 不是最后一个节点的话)
    if (target->next != NULL) { target->next->prev = target->prev; }

    // 🚀 终极内存防御：销毁患者前，必须先销毁他口袋里的处方本！
    PrescriptionNode* p_curr = target->script_head;
    while (p_curr != NULL)
     {
        PrescriptionNode* temp = p_curr;
        p_curr = p_curr->next;
        free(temp); // 把处方单一张张撕掉
    }

    // 3. 彻底释放该患者占用的内存
    free(target);
    return 1; // 删除成功
}

// 医生、药品、病房、账号的删除逻辑完全一致 (纯粹的指针手术)
int delete_doctor_by_id(DoctorNode* head, const char* target_id) 
{
    DoctorNode* target = find_doctor_by_id(head, target_id);
    if (!target) return 0;
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;
    free(target);
    return 1;
}

int delete_medicine_by_id(MedicineNode* head, const char* target_id) 
{
    MedicineNode* target = find_medicine_by_id(head, target_id);
    if (!target) return 0;
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;
    free(target);
    return 1;
}

int delete_ward_by_id(WardNode* head, const char* target_bed_id)
 {
    WardNode* target = find_ward_by_id(head, target_bed_id);
    if (!target) return 0;
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;
    free(target);
    return 1;
}

int delete_account_by_username(AccountNode* head, const char* target_username) 
{
    AccountNode* target = find_account_by_username(head, target_username);
    if (!target) return 0;
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;
    free(target);
    return 1;
}

// ==========================================
// 7. 嵌套链表挂载：给患者的“处方本”开药
// ==========================================
void add_prescription_to_patient(PatientNode* patient, const char* med_id, int quantity) 
{
    if (patient == NULL || med_id == NULL) return;

    // 1. 制造一张新的处方单 (单向节点)
    PrescriptionNode* new_script = (PrescriptionNode*)malloc(sizeof(PrescriptionNode));
    if (!new_script) return;
    strcpy(new_script->med_id, med_id);
    new_script->quantity = quantity;
    new_script->next = NULL;

    // 2. 塞进患者口袋 (单向链表尾插法)
    if (patient->script_head == NULL)
     {
        // 口袋是空的，这是第一张处方
        patient->script_head = new_script;
    } 
    else
     {
        // 口袋里有药了，顺着找，放到最后面
        PrescriptionNode* curr = patient->script_head;
        while (curr->next != NULL) 
        {
            curr = curr->next;
        }
        curr->next = new_script;
    }
    
    // 3. 统计数量加一
    patient->script_count++;
}

// ==========================================
// ==========================================
//六、接诊记录链表操作
// ---------------------------------------------------------
// 功能 1：初始化带头结点的接诊记录链表
// ---------------------------------------------------------
ConsultRecordNode* init_consult_record_list()
{
    ConsultRecordNode* head = (ConsultRecordNode*)malloc(sizeof(ConsultRecordNode));
    if (head == NULL)
    {
        printf("🔥 致命错误：内存分配失败，系统无法启动！\n");
        exit(1);
    }
    
    // 给头结点打上标记，防止和真实数据混淆
    strncpy(head->record_id, "HEAD", MAX_ID_LEN - 1);
    head->record_id[MAX_ID_LEN - 1] = '\0';
    
    // 防御阵地：头结点的指针必须干干净净
    head->prev = NULL;
    head->next = NULL;
    
    return head;
}

// ---------------------------------------------------------
// 功能 2：创建一个干净的接诊记录节点
// ---------------------------------------------------------
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
)
{
    ConsultRecordNode* node = (ConsultRecordNode*)malloc(sizeof(ConsultRecordNode));
    if (node == NULL) return NULL;
    
    // 安全复制字符串，防止缓冲区溢出
    strncpy(node->record_id, record_id, MAX_ID_LEN - 1);
    node->record_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(node->patient_id, patient_id, MAX_ID_LEN - 1);
    node->patient_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(node->doctor_id, doctor_id, MAX_ID_LEN - 1);
    node->doctor_id[MAX_ID_LEN - 1] = '\0';
    
    if (appointment_id != NULL)
    {
        strncpy(node->appointment_id, appointment_id, MAX_ID_LEN - 1);
    }
    else
    {
        node->appointment_id[0] = '\0';
    }
    node->appointment_id[MAX_ID_LEN - 1] = '\0';
    
    if (consult_time != NULL)
    {
        strncpy(node->consult_time, consult_time, MAX_NAME_LEN - 1);
    }
    else
    {
        node->consult_time[0] = '\0';
    }
    node->consult_time[MAX_NAME_LEN - 1] = '\0';
    
    if (diagnosis_text != NULL)
    {
        strncpy(node->diagnosis_text, diagnosis_text, MAX_RECORD_LEN - 1);
    }
    else
    {
        node->diagnosis_text[0] = '\0';
    }
    node->diagnosis_text[MAX_RECORD_LEN - 1] = '\0';
    
    if (treatment_advice != NULL)
    {
        strncpy(node->treatment_advice, treatment_advice, MAX_RECORD_LEN - 1);
    }
    else
    {
        node->treatment_advice[0] = '\0';
    }
    node->treatment_advice[MAX_RECORD_LEN - 1] = '\0';
    
    node->decision = decision;
    node->pre_status = pre_status;
    node->post_status = post_status;
    node->star_rating = 0; // 初始化为未评价
    node->feedback[0] = '\0'; // 初始化为空字符串
    
    node->prev = NULL;
    node->next = NULL;
    
    return node;
}

// ---------------------------------------------------------
// 功能 3：尾部安全插入法
// ---------------------------------------------------------
void insert_consult_record_tail(ConsultRecordNode* head, ConsultRecordNode* new_node)
{
    if (head == NULL || new_node == NULL) return;
    
    ConsultRecordNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node;
    new_node->prev = curr;
}

// ---------------------------------------------------------
// 功能 4：按记录编号查找接诊记录
// ---------------------------------------------------------
ConsultRecordNode* find_consult_record_by_id(ConsultRecordNode* head, const char* target_record_id)
{
    if (head == NULL || target_record_id == NULL) return NULL;
    ConsultRecordNode* curr = head->next;
    while (curr != NULL)
    {
        if (strcmp(curr->record_id, target_record_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

// ==========================================
// 七、检查项目字典链表操作
// ==========================================

// 初始化带头结点的检查项目链表
CheckItemNode* init_check_item_list()
{
    CheckItemNode* head = (CheckItemNode*)malloc(sizeof(CheckItemNode));
    if (head == NULL) exit(1);
    strncpy(head->item_id, "HEAD", MAX_ID_LEN - 1);
    head->item_id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

// 创建检查项目节点
CheckItemNode* create_check_item_node(const char* item_id, const char* item_name, const char* dept, double price, MedicareType m_type)
{
    CheckItemNode* new_node = (CheckItemNode*)malloc(sizeof(CheckItemNode));
    if (new_node == NULL) return NULL;
    
    strncpy(new_node->item_id, item_id, MAX_ID_LEN - 1);
    new_node->item_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->item_name, item_name, MAX_NAME_LEN - 1);
    new_node->item_name[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(new_node->dept, dept, MAX_NAME_LEN - 1);
    new_node->dept[MAX_NAME_LEN - 1] = '\0';
    
    new_node->price = price;
    new_node->m_type = m_type;
    new_node->prev = NULL;
    new_node->next = NULL;
    
    return new_node;
}

// 尾部插入检查项目
void insert_check_item_tail(CheckItemNode* head, CheckItemNode* new_node)
{
    if (head == NULL || new_node == NULL) return;
    CheckItemNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node;
    new_node->prev = curr;
}

// 根据检查项目编号查找
CheckItemNode* find_check_item_by_id(CheckItemNode* head, const char* target_item_id)
{
    if (head == NULL || target_item_id == NULL) return NULL;
    CheckItemNode* curr = head->next;
    while (curr != NULL)
    {
        if (strcmp(curr->item_id, target_item_id) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

// 根据科室查找检查项目
int find_check_items_by_dept(CheckItemNode* head, const char* dept, CheckItemNode** result_list)
{
    if (head == NULL || dept == NULL || result_list == NULL) return 0;
    
    int count = 0;
    CheckItemNode* curr = head->next;
    
    while (curr != NULL && count < 100)
    {
        if (strcmp(curr->dept, dept) == 0)
        {
            result_list[count++] = curr;
        }
        curr = curr->next;
    }
    
    return count;
}

// ==========================================
// 八、检查记录链表操作
// ==========================================

// 初始化带头结点的检查记录链表
CheckRecordNode* init_check_record_list()
{
    CheckRecordNode* head = (CheckRecordNode*)malloc(sizeof(CheckRecordNode));
    if (head == NULL) exit(1);
    strncpy(head->record_id, "HEAD", MAX_ID_LEN - 1);
    head->record_id[MAX_ID_LEN - 1] = '\0';
    head->prev = NULL; head->next = NULL;
    return head;
}

// 创建检查记录节点
CheckRecordNode* create_check_record_node(
    const char* record_id,
    const char* patient_id,
    const char* item_id,
    const char* item_name,
    const char* dept,
    const char* check_time,
    const char* result,
    int is_completed,
    int is_paid)
{
    CheckRecordNode* new_node = (CheckRecordNode*)malloc(sizeof(CheckRecordNode));
    if (new_node == NULL) return NULL;
    
    strncpy(new_node->record_id, record_id, MAX_ID_LEN - 1);
    new_node->record_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->patient_id, patient_id, MAX_ID_LEN - 1);
    new_node->patient_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->item_id, item_id, MAX_ID_LEN - 1);
    new_node->item_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(new_node->item_name, item_name, MAX_NAME_LEN - 1);
    new_node->item_name[MAX_NAME_LEN - 1] = '\0';
    
    strncpy(new_node->dept, dept, MAX_NAME_LEN - 1);
    new_node->dept[MAX_NAME_LEN - 1] = '\0';
    
    if (check_time != NULL)
    {
        strncpy(new_node->check_time, check_time, MAX_NAME_LEN - 1);
    }
    else
    {
        new_node->check_time[0] = '\0';
    }
    new_node->check_time[MAX_NAME_LEN - 1] = '\0';
    
    if (result != NULL)
    {
        strncpy(new_node->result, result, MAX_RECORD_LEN - 1);
    }
    else
    {
        new_node->result[0] = '\0';
    }
    new_node->result[MAX_RECORD_LEN - 1] = '\0';
    
    new_node->is_completed = is_completed;
    new_node->is_paid = is_paid;
    new_node->prev = NULL;
    new_node->next = NULL;
    
    return new_node;
}

// 尾部插入检查记录
void insert_check_record_tail(CheckRecordNode* head, CheckRecordNode* new_node)
{
    if (head == NULL || new_node == NULL) return;
    CheckRecordNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node;
    new_node->prev = curr;
}

// 根据患者编号查找检查记录数量
int get_check_records_by_patient(CheckRecordNode* head, const char* patient_id, CheckRecordNode** result_list)
{
    if (head == NULL || patient_id == NULL || result_list == NULL) return 0;
    
    int count = 0;
    CheckRecordNode* curr = head->next;
    
    while (curr != NULL && count < 100)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            result_list[count++] = curr;
        }
        curr = curr->next;
    }
    
    return count;
}

// 更新检查结果
int update_check_result(CheckRecordNode* head, const char* record_id, const char* result)
{
    if (head == NULL || record_id == NULL) return 0;
    
    CheckRecordNode* curr = head->next;
    while (curr != NULL)
    {
        if (strcmp(curr->record_id, record_id) == 0)
        {
            strncpy(curr->result, result, MAX_RECORD_LEN - 1);
            curr->result[MAX_RECORD_LEN - 1] = '\0';
            curr->is_completed = 1;
            return 1;
        }
        curr = curr->next;
    }
    
    return 0;
}

// ==========================================
// 九、安全预警队列操作
// ==========================================

// 初始化带头结点的预警队列
AlertNode* init_alert_list()
{
    AlertNode* head = (AlertNode*)malloc(sizeof(AlertNode));
    if (head == NULL)
    {
        printf("🔥 致命错误：内存分配失败，系统无法启动！\n");
        exit(1);
    }
    
    // 给头结点打上标记，防止和真实数据混淆
    strncpy(head->message, "HEAD", 255);
    head->message[255] = '\0';
    head->time = 0;
    
    // 防御阵地：头结点的指针必须干干净净
    head->prev = NULL;
    head->next = NULL;
    
    return head;
}

// 将新预警插入链表尾部
void push_system_alert(const char* msg)
{
    if (msg == NULL) return;
    
    // 如果预警队列未初始化，先初始化
    if (g_alert_list == NULL)
    {
        g_alert_list = init_alert_list();
    }
    
    // 创建新的预警节点
    AlertNode* new_node = (AlertNode*)malloc(sizeof(AlertNode));
    if (new_node == NULL)
    {
        printf("⚠️ 内存分配失败，预警信息无法添加！\n");
        return;
    }
    
    // 安全复制预警信息
    strncpy(new_node->message, msg, 255);
    new_node->message[255] = '\0';
    new_node->time = time(NULL);
    new_node->prev = NULL;
    new_node->next = NULL;
    
    // 尾插法插入预警节点
    AlertNode* curr = g_alert_list;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    
    curr->next = new_node;
    new_node->prev = curr;
}

// ==========================================
// 十、投诉工单链表操作
// ==========================================

// 初始化带头结点的投诉工单链表
ComplaintNode* init_complaint_list()
{
    ComplaintNode* head = (ComplaintNode*)malloc(sizeof(ComplaintNode));
    if (head == NULL)
    {
        printf("🔥 致命错误：内存分配失败，系统无法启动！\n");
        exit(1);
    }
    
    // 给头结点打上标记，防止和真实数据混淆
    strncpy(head->complaint_id, "HEAD", MAX_ID_LEN - 1);
    head->complaint_id[MAX_ID_LEN - 1] = '\0';
    strncpy(head->patient_id, "SYSTEM", MAX_ID_LEN - 1);
    head->patient_id[MAX_ID_LEN - 1] = '\0';
    
    // 防御阵地：头结点的指针必须干干净净
    head->prev = NULL;
    head->next = NULL;
    
    return head;
}

// 创建投诉工单节点
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
)
{
    ComplaintNode* node = (ComplaintNode*)malloc(sizeof(ComplaintNode));
    if (node == NULL) return NULL;
    
    // 安全复制字符串，防止缓冲区溢出
    strncpy(node->complaint_id, complaint_id, MAX_ID_LEN - 1);
    node->complaint_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(node->patient_id, patient_id, MAX_ID_LEN - 1);
    node->patient_id[MAX_ID_LEN - 1] = '\0';
    
    node->target_type = target_type;
    
    strncpy(node->target_id, target_id, MAX_ID_LEN - 1);
    node->target_id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(node->target_name, target_name, MAX_NAME_LEN - 1);
    node->target_name[MAX_NAME_LEN - 1] = '\0';
    
    if (content != NULL)
    {
        strncpy(node->content, content, MAX_RECORD_LEN - 1);
    }
    else
    {
        node->content[0] = '\0';
    }
    node->content[MAX_RECORD_LEN - 1] = '\0';
    
    node->status = status;
    
    if (response != NULL)
    {
        strncpy(node->response, response, MAX_RECORD_LEN - 1);
    }
    else
    {
        node->response[0] = '\0';
    }
    node->response[MAX_RECORD_LEN - 1] = '\0';
    
    if (submit_time != NULL)
    {
        strncpy(node->submit_time, submit_time, MAX_NAME_LEN - 1);
    }
    else
    {
        node->submit_time[0] = '\0';
    }
    node->submit_time[MAX_NAME_LEN - 1] = '\0';
    
    node->prev = NULL;
    node->next = NULL;
    
    return node;
}

// 尾插法插入投诉工单节点
void insert_complaint_tail(ComplaintNode* head, ComplaintNode* new_node)
{
    if (head == NULL || new_node == NULL) return;
    
    ComplaintNode* curr = head;
    while (curr->next != NULL) curr = curr->next;
    curr->next = new_node;
    new_node->prev = curr;
}

// End of Selection

