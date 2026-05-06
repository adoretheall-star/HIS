// ==========================================
// 文件名: global.h
// 作用: 全系统核心数据结构与全局常量定义图纸
// ==========================================

#ifndef GLOBAL_H
#define GLOBAL_H

#include <time.h>

// ==========================================
// 1. 全局宏定义 
// ==========================================
#define MAX_ID_LEN 32
#define MAX_NAME_LEN 50
#define MAX_SYMPTOM_LEN 100
#define MAX_MED_NAME_LEN 100
#define MAX_RECORD_LEN 200
#define MAX_ALIAS_LEN 50
#define MAX_GENERIC_NAME_LEN 200
#define MAX_DATE_LEN 20

// ==========================================
// 2. 全局枚举定义 
// ==========================================
typedef enum //权限管理
{
    ROLE_ADMIN = 0,   //管理者 (数据文件中为0)  
    ROLE_DOCTOR = 1,  //医生 (数据文件中为1)
    ROLE_NURSE = 2,   //护士 (数据文件中为2)
    ROLE_PHARMACIST = 3, //药房管理人员 (数据文件中为3)
    ROLE_PATIENT = 4 //患者 
} RoleType;

typedef enum //就医状态
{
    STATUS_PENDING = 1,     // 待诊
    STATUS_EXAMINING = 2,    // 检查中
    STATUS_RECHECK_PENDING = 3, // 检查后待复诊
    STATUS_UNPAID = 4,       // 已看诊待缴费
    STATUS_WAIT_MED = 5,     // 已缴费待取药
    STATUS_NEED_HOSPITALIZE = 6, // 需要住院（待登记）
    STATUS_HOSPITALIZED = 7, // 住院中
    STATUS_COMPLETED = 8,   // 就诊结束
    STATUS_NO_SHOW = 9      // 过号作废/本次挂号失效
} MedStatus;

typedef enum //医保类型
{
    MEDICARE_NONE = 0,    // 自费 (报销 0%)
    MEDICARE_CLASS_A = 1, // 甲类医保 (报销 80%)
    MEDICARE_CLASS_B = 2  // 乙类医保 (报销 50%)
} MedicareType;

typedef enum //预约状态
{
    RESERVED = 0,    // 已预约
    CHECKED_IN = 1,  // 已签到/已挂号
    CANCELLED = 2,   // 已取消
    MISSED = 3       // 已过号
} AppointmentStatus;

typedef enum //病房类型
{
    WARD_GENERAL = 1,  // 普通病房
    WARD_ICU = 2,      // ICU
    WARD_ISOLATION = 3, // 隔离病房
    WARD_SINGLE = 4    // 单人病房
} WardType;

typedef enum //回收站类型
{
    RECYCLE_MEDICINE = 1,     // 药品
    RECYCLE_CHECK_ITEM = 2,   // 检查项目
    RECYCLE_ACCOUNT = 3,      // 员工账号
    RECYCLE_WARD = 4          // 床位
} RecycleType;

// ==========================================
// 3. 全局数据结构定义 
// ==========================================

// 【实验1：患者管理】患者信息双向链表
typedef struct PatientNode
{
    char id[MAX_ID_LEN];          // 患者ID
    char id_card[MAX_ID_LEN];     // 身份证号
    char card_id[MAX_ID_LEN];     // 身份证号（用于兼容代码）
    char name[MAX_NAME_LEN];      // 患者姓名
    char gender[8];               // 性别
    char birth_date[MAX_DATE_LEN]; // 出生日期
    int age;                      // 年龄
    char phone[MAX_ID_LEN];       // 联系电话
    char address[MAX_RECORD_LEN]; // 住址
    char target_dept[MAX_NAME_LEN]; // 就诊科室
    MedicareType medicare_type;   // 医保类型
    MedicareType m_type;          // 医保类型（用于兼容代码）
    MedStatus status;             // 当前就医状态
    double emergency_debt;        // 急诊欠费金额
    double balance;               // 账户余额
    int is_emergency;             // 是否急诊患者
    int is_blacklisted;           // 是否黑名单患者
    time_t blacklist_expire;        // 黑名单过期时间
    int missed_count;             // 过号次数
    time_t missed_time_1;         // 过号时间1
    time_t missed_time_2;         // 过号时间2
    time_t missed_time_3;         // 过号时间3
    int queue_time;               // 排队时间
    int call_count;               // 呼叫次数
    struct PrescriptionNode* script_head; // 处方链表头
    int script_count;              // 处方数量
    time_t unpaid_time;            // 欠费时间
    char symptom[MAX_SYMPTOM_LEN]; // 症状描述
    char diagnosis_text[MAX_RECORD_LEN]; // 诊断结论
    char treatment_advice[MAX_RECORD_LEN]; // 处理意见
    char doctor_id[MAX_ID_LEN];   // 接诊医生ID
    
    struct PatientNode* prev;     // 前驱指针
    struct PatientNode* next;     // 后继指针
} PatientNode;

// 【实验2：预约管理】预约挂号双向链表
typedef struct AppointmentNode
{
    char id[MAX_ID_LEN];              // 预约单号
    char appointment_id[MAX_ID_LEN];  // 预约编号
    char patient_id[MAX_ID_LEN];      // 患者ID
    char doctor_id[MAX_ID_LEN];       // 医生ID
    char appoint_doctor[MAX_NAME_LEN]; // 预约医生编号
    char appoint_dept[MAX_NAME_LEN];  // 预约科室编号
    char doctor_name[MAX_NAME_LEN];   // 预约医生姓名
    char department[MAX_NAME_LEN];    // 预约科室
    char appointment_date[MAX_DATE_LEN]; // 预约日期
    char appointment_slot[MAX_NAME_LEN]; // 预约时段
    AppointmentStatus appointment_status; // 预约状态
    double reg_fee;                  // 本次挂号费
    int fee_paid;                    // 0:未收费 1:已收费
    int is_walk_in;                  // 0:预约号 1:现场号

    struct AppointmentNode* prev;    // 前驱指针
    struct AppointmentNode* next;    // 后继指针
} AppointmentNode;

// 【实验3：医生与科室双向链表】
typedef struct DoctorNode
{
    char id[MAX_ID_LEN];              // 医生工号
    char name[MAX_NAME_LEN];          // 医生姓名
    char gender[8];                   // 性别
    char department[MAX_NAME_LEN];    // 所属科室
    int queue_length;                 // 当前医生问诊排队患者人数
    int is_on_duty;                   // 1 在岗中，0 未在岗

    struct DoctorNode* prev;          // 前驱指针
    struct DoctorNode* next;          // 后继指针
} DoctorNode;

// 【实验4：药品库存双向链表】
typedef struct MedicineNode
{
    char id[MAX_ID_LEN];                     // 药品编码
    char name[MAX_MED_NAME_LEN];             // 药品商品名
    char alias[MAX_ALIAS_LEN];               // 药品别名
    char generic_name[MAX_GENERIC_NAME_LEN]; // 药品通用名
    double price;                            // 单价
    int stock;                               // 当前实际库存
    MedicareType m_type;                     // 药品所属的医保类型（决定打几折）
    char expiry_date[MAX_DATE_LEN];          // 效期 YYYY-MM-DD

    struct MedicineNode* prev;               // 前驱指针
    struct MedicineNode* next;               // 后继指针
} MedicineNode;

// 【实验5：患者与床位双向链表】
typedef struct WardNode
{
    char room_id[MAX_ID_LEN];      // 病房编号 / 房间号
    char bed_id[MAX_ID_LEN];       // 床位编号 (如 W-101)
    WardType ward_type;            // 病房类型 (普通病房/ICU)
    char dept[MAX_NAME_LEN];       // 所属科室
    int is_occupied;               // 0: 空闲, 1: 占用
    char patient_id[MAX_ID_LEN];   // 住在上面的患者编号(空闲时清空)

    struct WardNode* prev;         // 前驱指针
    struct WardNode* next;         // 后继指针
} WardNode;

// 【实验8：住院记录双向链表】
typedef struct InpatientRecord
{
    char inpatient_id[MAX_ID_LEN];     // 住院流水号
    char patient_id[MAX_ID_LEN];       // 患者编号
    char bed_id[MAX_ID_LEN];           // 床位编号
    char original_bed_id[MAX_ID_LEN];  // 初分配床位(转床后仍保留)  
    WardType ward_type;                // 病房类型
    WardType recommended_ward_type;    // 推荐病房类型
    int estimated_days;                // 预计住院天数
    int days_stayed;                   // 已住院天数
    double deposit_balance;            // 押金余额
    int is_active;                     // 1: 活跃, 0: 已出院
    long last_settlement_time;         // 最近一次日结时间戳

    struct InpatientRecord* prev;      // 前驱指针
    struct InpatientRecord* next;      // 后继指针
} InpatientRecord;

// 【实验6：系统账号与权限控制双向链表 (解决所有员工的登录问题)】
typedef struct AccountNode {
    char username[MAX_ID_LEN];     // 登录账号 (工号)
    char password[MAX_ID_LEN];     // 登录密码 (第一阶段可以先存明文，阶段二再加密或加盐)
    char real_name[MAX_NAME_LEN];  // 真实姓名
    char gender[8];                // 性别
    RoleType role;                 // 权限，决定了他登录后能看到哪个菜单
    int error_count;               // 连续输错密码的次数
    time_t lock_time;              // 触发锁定的时间戳
    int is_on_duty;                // 1 在岗中，0 未在岗

    struct AccountNode* prev;
    struct AccountNode* next;
} AccountNode;

// 接诊记录双向链表
typedef struct ConsultRecordNode {
    char record_id[MAX_ID_LEN];       // 记录编号
    char patient_id[MAX_ID_LEN];      // 患者ID
    char doctor_id[MAX_ID_LEN];       // 医生ID
    char appointment_id[MAX_ID_LEN];  // 预约单号
    char consult_time[MAX_NAME_LEN];  // 接诊时间
    char diagnosis_text[MAX_RECORD_LEN]; // 诊断内容
    char treatment_advice[MAX_RECORD_LEN]; // 治疗建议
    int decision;                     // 诊疗决策
    MedStatus pre_status;             // 诊疗前状态
    MedStatus post_status;            // 诊疗后状态
    int star_rating;                  // 星级评价
    char feedback[MAX_RECORD_LEN];    // 反馈内容

    struct ConsultRecordNode* prev;
    struct ConsultRecordNode* next;
} ConsultRecordNode;

// 检查项目字典双向链表
typedef struct CheckItemNode {
    char item_id[MAX_ID_LEN];         // 项目编号
    char item_name[MAX_MED_NAME_LEN]; // 项目名称
    char dept[MAX_NAME_LEN];          // 所属科室
    double price;                     // 价格
    MedicareType m_type;              // 医保类型

    struct CheckItemNode* prev;
    struct CheckItemNode* next;
} CheckItemNode;

// 检查记录双向链表
typedef struct CheckRecordNode {
    char record_id[MAX_ID_LEN];       // 记录编号
    char patient_id[MAX_ID_LEN];      // 患者ID
    char item_id[MAX_ID_LEN];         // 检查项目ID
    char item_name[MAX_MED_NAME_LEN]; // 检查项目名称
    char dept[MAX_NAME_LEN];          // 所属科室
    char check_time[MAX_NAME_LEN];    // 检查时间
    char result[MAX_RECORD_LEN];      // 检查结果
    int is_completed;                 // 是否完成
    int is_paid;                      // 是否已缴费

    struct CheckRecordNode* prev;
    struct CheckRecordNode* next;
} CheckRecordNode;

// 【处方记录双向链表】
typedef struct PrescriptionNode {
    char prescription_id[MAX_ID_LEN];  // 处方编号
    char med_id[MAX_ID_LEN];          // 药品ID
    char medicine_name[MAX_MED_NAME_LEN]; // 药品名称
    int quantity;                     // 数量
    double price;                     // 单价
    struct PrescriptionNode* prev;     // 前驱指针
    struct PrescriptionNode* next;     // 后继指针
} PrescriptionNode;

// 【实验10：完整预警队列】
typedef struct AlertNode {
    char message[256];             // 预警消息
    time_t time;                   // 预警时间
    struct AlertNode* prev;        // 前驱指针
    struct AlertNode* next;        // 后继指针
} AlertNode;

// 【实验11：投诉工单双向链表】
typedef struct ComplaintNode {
    char complaint_id[MAX_ID_LEN];    // 工单编号，格式 CP-XXX
    char patient_id[MAX_ID_LEN];      // 投诉人编号
    int target_type;                  // 投诉类型，1=医生, 2=护士/前台, 3=药师
    char target_id[MAX_ID_LEN];       // 被投诉人ID
    char target_name[MAX_NAME_LEN];   // 被投诉人真实姓名
    char content[MAX_RECORD_LEN];     // 投诉内容
    int status;                       // 0: 待处理, 1: 已回复
    char response[MAX_RECORD_LEN];    // 管理员处理意见
    char submit_time[MAX_NAME_LEN];   // 提交时间，字符串格式

    struct ComplaintNode* next;       // 后继指针
    struct ComplaintNode* prev;       // 前驱指针

} ComplaintNode;

// 【实验7：日志记录单向链表】
typedef struct LogNode {
    char timestamp[20];         // 操作时间
    char operation[50];         // 操作类型
    char target[50];            // 目标对象
    char description[200];      // 简要说明
    struct LogNode* next;       // 指向下一条日志
} LogNode;

// 【实验12：回收站双向链表】
typedef struct RecycleNode
{
    char recycle_id[MAX_ID_LEN];        // 回收站编号，例如 R-001        
    RecycleType type;                   // 回收对象类型
    char source_id[MAX_ID_LEN];         // 原编号，例如 M-001
    char source_name[MAX_NAME_LEN];     // 原名称
    char delete_time[20];               // 删除时间
    char deleted_by[MAX_ID_LEN];        // 操作人
    char reason[MAX_RECORD_LEN];        // 删除/下架原因

    MedicineNode medicine_backup;       // 备份的药品信息
    CheckItemNode check_item_backup;    // 备份的检查项目信息
    WardNode ward_backup;               // 备份的床位信息
    AccountNode account_backup;         // 备份的员工账号信息
    int is_restored;                    // 0=未恢复, 1=已恢复

    struct RecycleNode* prev;
    struct RecycleNode* next;
} RecycleNode;

// ==========================================
// 6. 全局链表头节点声明(外部文件通过 extern 共享)
// ==========================================
extern PatientNode* g_patient_list;
extern AppointmentNode* g_appointment_list;
extern DoctorNode* g_doctor_list;
extern MedicineNode* g_medicine_list;
extern WardNode* g_ward_list;
extern AccountNode* g_account_list;
extern AccountNode* g_current_account;
extern ConsultRecordNode* g_consult_record_list;
extern CheckItemNode* g_check_item_list;     // 检查项目字典
extern CheckRecordNode* g_check_record_list; // 检查记录
extern AlertNode* g_alert_list;              // 安全预警链表

extern ComplaintNode* g_complaint_list;      // 投诉工单链表
extern LogNode* g_log_list;
extern InpatientRecord* g_inpatient_list;
extern RecycleNode* g_recycle_list;           // 回收站链表
extern int g_demo_mode;                      // 演示模式：0=真实时间 1=强制夜间 2=强制白天

// ==========================================
// 7. 功能：实现删除节点(Delete)
// ==========================================
// 成功删除返回 1，找不到目标返回 0
int delete_patient_by_id(PatientNode* head, const char* target_id);
int delete_doctor_by_id(DoctorNode* head, const char* target_id);
int delete_medicine_by_id(MedicineNode* head, const char* target_id);
int delete_ward_by_id(WardNode* head, const char* target_bed_id);
int delete_account_by_username(AccountNode* head, const char* target_username);
int delete_account(const char* username); 

// ==========================================
// 8. 处方管理链表：给患者的"处方本"开单
// ==========================================
// 给指定的患者添加一条开药记录
void add_prescription_to_patient(PatientNode* patient, const char* med_id, int quantity);

// 删除患者处方中的最后一种药
void remove_last_prescription_from_patient(PatientNode* patient);

// ==========================================
// 9. 辅助链表结构
// ==========================================
// 患者指针链表节点
typedef struct PatientPtrNode {
    PatientNode* patient;
    struct PatientPtrNode* next;
} PatientPtrNode;

// 检查记录指针链表节点
typedef struct CheckRecordPtrNode {
    CheckRecordNode* record;
    struct CheckRecordPtrNode* next;
} CheckRecordPtrNode;

// 投诉指针链表节点
typedef struct ComplaintPtrNode {
    ComplaintNode* complaint;
    struct ComplaintPtrNode* next;
} ComplaintPtrNode;

// 科室统计链表节点
typedef struct DeptStatNode {
    char department[MAX_NAME_LEN];
    int doctor_count;
    int queue_length;
    struct DeptStatNode* next;
} DeptStatNode;

// 检查项目指针链表节点(用于按科室查找检查项目)
typedef struct CheckItemPtrNode {
    CheckItemNode* item;
    struct CheckItemPtrNode* next;
} CheckItemPtrNode;

#endif