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
#define MAX_GENERIC_NAME_LEN 50
#define MAX_DATE_LEN 20

// ==========================================
// 2. 全局枚举定义 
// ==========================================
typedef enum //权限管理
{
    ROLE_ADMIN = 1,   //管理者   
    ROLE_NURSE = 2,   //护士   
    ROLE_DOCTOR = 3,  //医生   
    ROLE_PATIENT = 4, //患者   
    ROLE_PHARMACIST = 5 //药房管理人员 
} RoleType;

typedef enum //就医状态
{
    STATUS_PENDING = 1,     // 待诊
    STATUS_EXAMINING = 2,    // 检查中
    STATUS_RECHECK_PENDING = 3, // 检查后待复诊
    STATUS_UNPAID = 4,       // 已看诊待缴费
    STATUS_WAIT_MED = 5,     // 已缴费待取药
    STATUS_HOSPITALIZED = 6, // 住院中
    STATUS_COMPLETED = 7,    // 就诊结束
    STATUS_NO_SHOW = 8       // 过号作废/本次挂号失效
} MedStatus;

typedef enum //医保类型
{
    MEDICARE_NONE = 0,    // 无医保 (报销 0%)
    MEDICARE_CLASS_A = 1, // 甲类医保 (报销 80%)
    MEDICARE_CLASS_B = 2  // 乙类医保 (报销 50%)
} MedicareType;

typedef enum //预约状态
{
    RESERVED = 1,   // 已预约
    CHECKED_IN = 2, // 已签到
    CANCELLED = 3,  // 已取消
    MISSED = 4      // 已过号
} AppointmentStatus;

// ==========================================
// 3. 附属结构体定义
// ==========================================
// 处方明细 (挂载在患者档案下)
typedef struct PrescriptionNode
{
    char med_id[MAX_ID_LEN]; // 药品编号
    int quantity;            // 开药数量
    struct PrescriptionNode* next;// 指向该患者开的下一种药
} PrescriptionNode;

// ==========================================
// 4. 核心双向链表结构体定义
// ==========================================

// 【实体 7：接诊记录双向链表】
typedef struct ConsultRecordNode
{
    char record_id[MAX_ID_LEN];       // 接诊记录编号
    char patient_id[MAX_ID_LEN];      // 患者编号
    char doctor_id[MAX_ID_LEN];       // 医生编号
    char appointment_id[MAX_ID_LEN];  // 对应预约编号（可选）
    char consult_time[MAX_NAME_LEN];  // 接诊时间（字符串占位）
    char diagnosis_text[MAX_RECORD_LEN]; // 本次诊断结论
    char treatment_advice[MAX_RECORD_LEN]; // 本次处理意见
    int decision;                     // 本次诊疗决策
    MedStatus pre_status;             // 接诊前状态
    MedStatus post_status;            // 接诊后状态
    int star_rating;                  // 满意度星级 (0表示未评价, 1-5表示星级)
    char feedback[MAX_RECORD_LEN];    // 文字评价内容
    
    struct ConsultRecordNode* prev;   // 前驱指针
    struct ConsultRecordNode* next;   // 后继指针
} ConsultRecordNode;

// 【实体 8：检查项目字典】
typedef struct CheckItemNode
{
    char item_id[MAX_ID_LEN];         // 检查项目编号
    char item_name[MAX_NAME_LEN];     // 检查项目名称
    char dept[MAX_NAME_LEN];          // 所属检查科室
    double price;                     // 检查费用
    MedicareType m_type;              // 检查项目的医保类型
    
    struct CheckItemNode* prev;       // 前驱指针
    struct CheckItemNode* next;       // 后继指针
} CheckItemNode;

// 【实体 9：检查记录双向链表】
typedef struct CheckRecordNode
{
    char record_id[MAX_ID_LEN];       // 检查记录编号
    char patient_id[MAX_ID_LEN];      // 患者编号
    char item_id[MAX_ID_LEN];         // 检查项目编号
    char item_name[MAX_NAME_LEN];     // 检查项目名称
    char dept[MAX_NAME_LEN];          // 检查科室
    char check_time[MAX_NAME_LEN];    // 检查时间
    char result[MAX_RECORD_LEN];      // 检查结果
    int is_completed;                 // 是否完成检查 (0:未完成, 1:已完成)
    int is_paid;                      // 0:未缴费, 1:已缴费
    
    struct CheckRecordNode* prev;     // 前驱指针
    struct CheckRecordNode* next;     // 后继指针
} CheckRecordNode;

// 【实体 1：患者与就诊档案双向链表】
typedef struct PatientNode
 {
    //静态基础信息
    char id[MAX_ID_LEN];       //患者唯一编号    
    char name[MAX_NAME_LEN];   //患者姓名      
    int age;                   //患者年龄
    char id_card[MAX_ID_LEN];  //身份证号
    MedicareType m_type;       //个人医保类型
    //高级业务字段 
    char symptom[MAX_SYMPTOM_LEN];//症状描述（供智能分诊系统使用）  
    char target_dept[MAX_NAME_LEN]; //患者挂号科室
    char doctor_id[MAX_ID_LEN];    //接诊医生的编号
    char card_id[MAX_NAME_LEN];    //虚拟诊疗卡号  
    double balance;              //账户余额
    MedStatus status;            //就医状态（对应状态流转功能）
    char diagnosis_text[MAX_RECORD_LEN]; //诊断文本
    char treatment_advice[MAX_RECORD_LEN]; //治疗建议
    // 患者的处方本
    PrescriptionNode* script_head; // 指向该患者开的第一种药(此处为单向链表)
    int script_count;      // 当前开了几种药 (默认 0)
    
   // 信用黑名单相关字段
    time_t missed_time_1; // 第1次爽约时间
    time_t missed_time_2; // 第2次爽约时间
    time_t missed_time_3; // 第3次爽约时间
    int missed_count;     // 累计爽约次数 (0-3)
    time_t blacklist_expire; // 黑名单到期时间戳
    int is_blacklisted; // 0:正常, 1:爽约黑名单, 2:逃单黑名单
    int is_emergency; // 0:普通, 1:急诊绿色通道患者
    time_t queue_time; // 排队签到时间戳
    int call_count; // 叫号未到次数统计
    double emergency_debt; // 记录急诊欠费金额
    time_t unpaid_time; // 进入待缴费状态的时间戳，0表示无

    struct PatientNode* prev;//前驱指针
    struct PatientNode* next; //后继指针
} PatientNode;

// 【实体 2：预约记录双向链表】
typedef struct AppointmentNode 
{
    char appointment_id[MAX_ID_LEN];      //预约编号
    char patient_id[MAX_ID_LEN];          //患者编号
    char appointment_date[MAX_NAME_LEN];  //预约日期
    char appointment_slot[MAX_NAME_LEN];  //预约时段
    char appoint_doctor[MAX_NAME_LEN];    //预约医生
    char appoint_dept[MAX_NAME_LEN];      //预约科室
    AppointmentStatus appointment_status; //预约状态
    double reg_fee;                       // 本次挂号费
    int fee_paid;                         // 0:未收费, 1:已收费
    int is_walk_in;                       // 0:预约号, 1:现场号

    struct AppointmentNode* prev;//前驱指针
    struct AppointmentNode* next; //后继指针
} AppointmentNode;

// 【实体 3：医生与科室双向链表】
typedef struct DoctorNode 
{
    char id[MAX_ID_LEN];      //医生工号     
    char name[MAX_NAME_LEN];  //医生姓名      
    char department[MAX_NAME_LEN]; //所属科室  
    int queue_length;      //当前医生门诊排队患者人数
    int is_on_duty;        // 1 值班中，0 未值班

    struct DoctorNode* prev;//前驱指针
    struct DoctorNode* next; //后继指针
} DoctorNode;

// 【实体 4：药品库房双向链表】
typedef struct MedicineNode 
{
    char id[MAX_ID_LEN];                     //药品编号
    char name[MAX_MED_NAME_LEN];             //药品商品名
    char alias[MAX_ALIAS_LEN];               //药品别名
    char generic_name[MAX_GENERIC_NAME_LEN]; //药品通用名
    double price;                            //单价
    int stock;                               //当前实际库存
    MedicareType m_type;                     //药品所属的医保类型（决定打几折）
    char expiry_date[MAX_DATE_LEN];          //效期 YYYY-MM-DD

    struct MedicineNode* prev;//前驱指针
    struct MedicineNode* next; //后继指针
} MedicineNode;

// 【实体 5：病房与床位双向链表】
typedef struct WardNode 
{
    char bed_id[MAX_ID_LEN];       // 床位编号 (如: W-101)
    int is_occupied;               // 0: 空闲, 1: 占用
    char patient_id[MAX_ID_LEN];   // 住在上面的患者编号 (空闲时清空)
    
    struct WardNode* prev;//前驱指针
    struct WardNode* next; //后继指针
} WardNode;
// 【实体 6：系统账号与权限控制双向链表 (解决所有员工的登录问题)】
typedef struct AccountNode {
    char username[MAX_ID_LEN];     // 登录账号 (工号)
    char password[MAX_ID_LEN];     // 登录密码 (阶段一可以先存明文，阶段二再加异或加密)
    char real_name[MAX_NAME_LEN];  // 真实姓名
    RoleType role;                 
// 核心！决定了他登录后能看到哪个菜单
    int error_count;             // 连续输错密码的次数
    time_t lock_time;            // 触发锁定的时间戳
    int is_on_duty;                // 1 值班中，0 未值班

    struct AccountNode* prev;
    struct AccountNode* next;
} AccountNode;
// 【实体 10：安全预警队列】
typedef struct AlertNode {
    char message[256];             // 预警信息
    time_t time;                   // 预警时间
    struct AlertNode* prev;        // 前驱指针
    struct AlertNode* next;        // 后继指针
} AlertNode;

// 【实体 11：投诉工单双向链表】
typedef struct ComplaintNode {
    char complaint_id[MAX_ID_LEN];    // 工单编号，格式 CP-XXX
    char patient_id[MAX_ID_LEN];      // 投诉人编号
    int target_type;                  // 投诉类型：1=医生, 2=护士/前台, 3=药师
    char target_id[MAX_ID_LEN];       // 被投诉人工号
    char target_name[MAX_NAME_LEN];   // 被投诉人真实姓名
    char content[MAX_RECORD_LEN];     // 投诉内容
    int status;                       // 0: 待处理, 1: 已回复
    char response[MAX_RECORD_LEN];    // 管理员处理意见
    char submit_time[MAX_NAME_LEN];   // 提交时间，字符串格式
    
    struct ComplaintNode* next;       // 后继指针
    struct ComplaintNode* prev;       // 前驱指针

} ComplaintNode;
// 【实体 7：日志记录单向链表】
typedef struct LogNode {
    char timestamp[20];         // 操作时间
    char operation[50];        // 操作类型
    char target[50];           // 目标对象
    char description[200];      // 简短说明
    struct LogNode* next;       // 指向下一条日志
} LogNode;

// ==========================================
// 6. 全局头结点声明 (外部文件通过 extern 共享)
// ==========================================
extern PatientNode* g_patient_list;
extern AppointmentNode* g_appointment_list;
extern DoctorNode* g_doctor_list;
extern MedicineNode* g_medicine_list;
extern WardNode* g_ward_list; 
extern AccountNode* g_account_list;
extern ConsultRecordNode* g_consult_record_list;
extern CheckItemNode* g_check_item_list;     // 检查项目字典
extern CheckRecordNode* g_check_record_list; // 检查记录
extern AlertNode* g_alert_list;              // 安全预警队列

extern ComplaintNode* g_complaint_list;      // 投诉工单链表
extern LogNode* g_log_list;
// ==========================================
// 7. 功能：安全删除节点 (Delete)
// ==========================================
// 成功删除返回 1，找不到目标返回 0
int delete_patient_by_id(PatientNode* head, const char* target_id);
int delete_doctor_by_id(DoctorNode* head, const char* target_id);
int delete_medicine_by_id(MedicineNode* head, const char* target_id);
int delete_ward_by_id(WardNode* head, const char* target_bed_id);
int delete_account_by_username(AccountNode* head, const char* target_username);

// ==========================================
// 8. 嵌套链表挂载：给患者的“处方本”开药
// ==========================================
// 给指定的患者挂载一条开药记录
void add_prescription_to_patient(PatientNode* patient, const char* med_id, int quantity);

// 删除患者处方中的最后一种药品
void remove_last_prescription_from_patient(PatientNode* patient);
#endif 
