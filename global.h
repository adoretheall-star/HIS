// ==========================================
// 文件名: global.h
// 作用: 全系统核心数据结构与全局常量定义图纸
// ==========================================

#ifndef GLOBAL_H
#define GLOBAL_H

// ==========================================
// 1. 全局宏定义 
// ==========================================
#define MAX_ID_LEN 32
#define MAX_NAME_LEN 50
#define MAX_SYMPTOM_LEN 100
#define MAX_RECORD_LEN 200

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
    STATUS_COMPLETED = 7     // 就诊结束
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

    struct DoctorNode* prev;//前驱指针
    struct DoctorNode* next; //后继指针
} DoctorNode;

// 【实体 4：药品库房双向链表】
typedef struct MedicineNode 
{
    char id[MAX_ID_LEN];       //药品编号     
    char name[MAX_NAME_LEN];   //药品名称      
    double price;          //单价
    int stock;             //当前实际库存
    MedicareType m_type;   //药品所属的医保类型（决定打几折）

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

    struct AccountNode* prev;
    struct AccountNode* next;
} AccountNode;
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
#endif 