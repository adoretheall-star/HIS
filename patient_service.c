// ==========================================
// 文件名: patient_service.c
// 作用: 患者相关业务服务层 / 患者数据服务层实现
// 描述: 实现患者信息管理的核心业务逻辑，被护士端和患者端共同复用
// 说明: 本模块只包含业务逻辑实现，不包含菜单和界面逻辑
// ==========================================
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "patient_service.h"

// ==========================================
// 内部辅助函数
// ==========================================

/**
 * @brief 生成下一个患者编号
 * @param new_id 存储生成的患者编号
 * @details 格式为 "P-XXX"，其中 XXX 为三位数字，从 001 开始递增
 */
static void generate_patient_id(char* new_id)
{
    int max_no = 0;
    PatientNode* curr = NULL;

    // 参数校验
    if (new_id == NULL)
    {
        return;
    }

    // 如果患者链表为空，从 P-001 开始
    if (g_patient_list == NULL)
    {
        snprintf(new_id, MAX_ID_LEN, "P-001");
        return;
    }

    // 遍历患者链表，查找最大的编号
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // 检查是否为患者编号格式 (P-XXX)
        if (strncmp(curr->id, "P-", 2) == 0)
        {
            // 提取数字部分并转换为整数
            int current_no = atoi(curr->id + 2);
            if (current_no > max_no)
            {
                max_no = current_no;
            }
        }
        curr = curr->next;
    }

    // 生成新的编号，格式为 P-XXX
    snprintf(new_id, MAX_ID_LEN, "P-%03d", max_no + 1);
}
/**
 * @brief 根据身份证号查找患者
 * @param id_card 患者身份证号
 * @return 找到返回患者节点指针，未找到返回NULL
 */
PatientNode* find_patient_by_id_card(const char* id_card)
{
    PatientNode* curr = NULL;
    
    // 参数校验
    if (g_patient_list == NULL || id_card == NULL) 
        return NULL;
    
    // 遍历患者链表查找匹配的身份证号
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->id_card, id_card) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief 安全地复制文本字段
 * @param dest 目标缓冲区
 * @param max_len 目标缓冲区最大长度
 * @param src 源文本
 * @details 使用 strncpy 确保不会发生缓冲区溢出
 */
static void copy_text_field(char* dest, int max_len, const char* src)
{
    if (dest == NULL || max_len <= 0)
    {
        return;
    }

    // 使用 strncpy 进行安全复制，确保以 '\0' 结尾
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}

/**
 * @brief 获取显示文本（处理空值）
 * @param text 原始文本
 * @return 如果文本为空返回"暂无"，否则返回原始文本
 */
static const char* get_display_text(const char* text)
{
    if (text == NULL || text[0] == '\0')
    {
        return "暂无";
    }

    return text;
}

/**
 * @brief 获取汉字的拼音首字母
 * @param c 汉字字符
 * @return 拼音首字母（大写），非汉字返回原字符
 */
static char get_pinyin_first_letter(char c)
{
    unsigned char uc = (unsigned char)c;
    
    // ASCII字符直接返回大写
    if (uc < 0x80)
    {
        if (c >= 'a' && c <= 'z')
            return c - 32;
        return c;
    }
    
    // 常见汉字拼音首字母映射表（简化版）
    if (uc >= 0xB0 && uc <= 0xF7)
    {
        // 根据汉字的GB2312编码范围判断首字母
        if (uc == 0xB0) return 'A';
        if (uc == 0xB1) return 'B';
        if (uc == 0xB2) return 'C';
        if (uc == 0xB3) return 'D';
        if (uc == 0xB4) return 'E';
        if (uc == 0xB5) return 'F';
        if (uc == 0xB6) return 'G';
        if (uc == 0xB7) return 'H';
        if (uc == 0xB8) return 'J';
        if (uc == 0xB9) return 'K';
        if (uc == 0xBA) return 'L';
        if (uc == 0xBB) return 'M';
        if (uc == 0xBC) return 'N';
        if (uc == 0xBD) return 'O';
        if (uc == 0xBE) return 'P';
        if (uc == 0xBF) return 'Q';
        if (uc == 0xC0) return 'R';
        if (uc == 0xC1) return 'S';
        if (uc == 0xC2) return 'T';
        if (uc == 0xC3) return 'W';
        if (uc == 0xC4) return 'X';
        if (uc == 0xC5) return 'Y';
        if (uc == 0xC6) return 'Z';
    }
    
    return c;
}

/**
 * @brief 将中文姓名转换为拼音首字母
 * @param name 中文姓名
 * @param pinyin 存储拼音首字母的缓冲区
 * @param max_len 缓冲区最大长度
 */
static void name_to_pinyin(const char* name, char* pinyin, int max_len)
{
    if (name == NULL || pinyin == NULL || max_len <= 0)
        return;
    
    int i = 0;
    int pos = 0;
    
    while (name[i] != '\0' && pos < max_len - 1)
    {
        unsigned char uc = (unsigned char)name[i];
        
        if (uc < 0x80)
        {
            // ASCII字符
            pinyin[pos++] = get_pinyin_first_letter(name[i]);
            i++;
        }
        else
        {
            // 汉字（GB2312编码，2字节）
            pinyin[pos++] = get_pinyin_first_letter(name[i]);
            i += 2;
        }
    }
    
    pinyin[pos] = '\0';
}



/**
 * @brief 根据患者编号获取患者节点（带错误检查）
 * @param patient_id 患者编号
 * @param action_name 操作名称（用于错误提示）
 * @return 成功返回患者节点指针，失败返回NULL
 */
static PatientNode* get_patient_by_id_checked(const char* patient_id, const char* action_name)
{
    PatientNode* patient = NULL;

    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("提示：患者链表尚未初始化，无法%s！\n", action_name);
        return NULL;
    }

    // 检查患者编号是否为空
    if (patient_id == NULL || patient_id[0] == '\0')
    {
        printf("提示：患者编号不能为空！\n");
        return NULL;
    }

    // 查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("未找到对应患者档案！\n");
        return NULL;
    }

    return patient;
}

/**
 * @brief 根据症状智能推荐科室（智能导诊功能）
 * @param symptom 患者症状描述
 * @return 推荐的科室名称字符串
 */
const char* recommend_dept_by_symptom(const char* symptom)
{
    if (symptom == NULL || strlen(symptom) == 0)
    {
        return "全科";
    }
    
    // 将症状转换为小写进行匹配（避免大小写问题）
    char lower_symptom[MAX_SYMPTOM_LEN];
    strcpy(lower_symptom, symptom);
    for (int i = 0; lower_symptom[i]; i++)
    {
        lower_symptom[i] = tolower(lower_symptom[i]);
    }
    
    // ==============================================
    // 急诊科 - 紧急情况优先匹配
    // ==============================================
    if (strstr(lower_symptom, "紧急") != NULL ||
        strstr(lower_symptom, "急救") != NULL ||
        strstr(lower_symptom, "昏迷") != NULL ||
        strstr(lower_symptom, "休克") != NULL ||
        strstr(lower_symptom, "大出血") != NULL ||
        strstr(lower_symptom, "呼吸困难") != NULL ||
        strstr(lower_symptom, "心跳停止") != NULL ||
        strstr(lower_symptom, "心梗") != NULL ||
        strstr(lower_symptom, "中风") != NULL ||
        strstr(lower_symptom, "车祸") != NULL ||
        strstr(lower_symptom, "坠落") != NULL)
    {
        return "急诊科";
    }
    
    // ==============================================
    // 妇产科
    // ==============================================
    if (strstr(lower_symptom, "怀孕") != NULL ||
        strstr(lower_symptom, "妊娠") != NULL ||
        strstr(lower_symptom, "产检") != NULL ||
        strstr(lower_symptom, "分娩") != NULL ||
        strstr(lower_symptom, "妇科") != NULL ||
        strstr(lower_symptom, "月经") != NULL ||
        strstr(lower_symptom, "痛经") != NULL ||
        strstr(lower_symptom, "乳腺") != NULL ||
        strstr(lower_symptom, "子宫") != NULL ||
        strstr(lower_symptom, "卵巢") != NULL)
    {
        return "妇产科";
    }
    
    // ==============================================
    // 儿科
    // ==============================================
    if (strstr(lower_symptom, "儿童") != NULL ||
        strstr(lower_symptom, "小孩") != NULL ||
        strstr(lower_symptom, "宝宝") != NULL ||
        strstr(lower_symptom, "婴儿") != NULL ||
        strstr(lower_symptom, "幼儿") != NULL ||
        strstr(lower_symptom, "小儿") != NULL)
    {
        return "儿科";
    }
    
    // ==============================================
    // 骨科
    // ==============================================
    if (strstr(lower_symptom, "骨折") != NULL ||
        strstr(lower_symptom, "骨裂") != NULL ||
        strstr(lower_symptom, "关节") != NULL ||
        strstr(lower_symptom, "腰椎") != NULL ||
        strstr(lower_symptom, "颈椎") != NULL ||
        strstr(lower_symptom, "脊椎") != NULL ||
        strstr(lower_symptom, "骨质疏松") != NULL ||
        strstr(lower_symptom, "股骨头") != NULL)
    {
        return "骨科";
    }
    
    // ==============================================
    // 心血管内科
    // ==============================================
    if (strstr(lower_symptom, "心悸") != NULL ||
        strstr(lower_symptom, "胸闷") != NULL ||
        strstr(lower_symptom, "心慌") != NULL ||
        strstr(lower_symptom, "心律不齐") != NULL ||
        strstr(lower_symptom, "高血压") != NULL ||
        strstr(lower_symptom, "低血压") != NULL ||
        strstr(lower_symptom, "冠心病") != NULL ||
        strstr(lower_symptom, "心绞痛") != NULL ||
        strstr(lower_symptom, "心肌") != NULL ||
        strstr(lower_symptom, "心衰") != NULL)
    {
        return "心血管内科";
    }
    
    // ==============================================
    // 呼吸内科
    // ==============================================
    if (strstr(lower_symptom, "咳嗽") != NULL ||
        strstr(lower_symptom, "咳痰") != NULL ||
        strstr(lower_symptom, "哮喘") != NULL ||
        strstr(lower_symptom, "肺炎") != NULL ||
        strstr(lower_symptom, "支气管炎") != NULL ||
        strstr(lower_symptom, "呼吸困难") != NULL ||
        strstr(lower_symptom, "胸闷") != NULL ||
        strstr(lower_symptom, "气短") != NULL)
    {
        return "呼吸内科";
    }
    
    // ==============================================
    // 消化内科
    // ==============================================
    if (strstr(lower_symptom, "胃痛") != NULL ||
        strstr(lower_symptom, "胃胀") != NULL ||
        strstr(lower_symptom, "胃炎") != NULL ||
        strstr(lower_symptom, "胃溃疡") != NULL ||
        strstr(lower_symptom, "腹泻") != NULL ||
        strstr(lower_symptom, "便秘") != NULL ||
        strstr(lower_symptom, "便血") != NULL ||
        strstr(lower_symptom, "黄疸") != NULL ||
        strstr(lower_symptom, "肝") != NULL ||
        strstr(lower_symptom, "胆囊") != NULL ||
        strstr(lower_symptom, "胰腺") != NULL)
    {
        return "消化内科";
    }
    
    // ==============================================
    // 神经内科
    // ==============================================
    if (strstr(lower_symptom, "头痛") != NULL ||
        strstr(lower_symptom, "头晕") != NULL ||
        strstr(lower_symptom, "眩晕") != NULL ||
        strstr(lower_symptom, "失眠") != NULL ||
        strstr(lower_symptom, "癫痫") != NULL ||
        strstr(lower_symptom, "麻木") != NULL ||
        strstr(lower_symptom, "瘫痪") != NULL ||
        strstr(lower_symptom, "帕金森") != NULL ||
        strstr(lower_symptom, "神经") != NULL)
    {
        return "神经内科";
    }
    
    // ==============================================
    // 内分泌科
    // ==============================================
    if (strstr(lower_symptom, "糖尿病") != NULL ||
        strstr(lower_symptom, "甲亢") != NULL ||
        strstr(lower_symptom, "甲减") != NULL ||
        strstr(lower_symptom, "甲状腺") != NULL ||
        strstr(lower_symptom, "血糖") != NULL ||
        strstr(lower_symptom, "激素") != NULL ||
        strstr(lower_symptom, "肥胖") != NULL)
    {
        return "内分泌科";
    }
    
    // ==============================================
    // 肾内科
    // ==============================================
    if (strstr(lower_symptom, "肾炎") != NULL ||
        strstr(lower_symptom, "肾病") != NULL ||
        strstr(lower_symptom, "尿毒症") != NULL ||
        strstr(lower_symptom, "蛋白尿") != NULL ||
        strstr(lower_symptom, "血尿") != NULL ||
        strstr(lower_symptom, "尿频") != NULL ||
        strstr(lower_symptom, "尿急") != NULL ||
        strstr(lower_symptom, "尿痛") != NULL)
    {
        return "肾内科";
    }
    
    // ==============================================
    // 泌尿外科
    // ==============================================
    if (strstr(lower_symptom, "前列腺") != NULL ||
        strstr(lower_symptom, "结石") != NULL ||
        strstr(lower_symptom, "阳痿") != NULL ||
        strstr(lower_symptom, "早泄") != NULL ||
        strstr(lower_symptom, "包皮") != NULL ||
        strstr(lower_symptom, "睾丸") != NULL ||
        strstr(lower_symptom, "精索") != NULL)
    {
        return "泌尿外科";
    }
    
    // ==============================================
    // 肿瘤科
    // ==============================================
    if (strstr(lower_symptom, "肿瘤") != NULL ||
        strstr(lower_symptom, "癌症") != NULL ||
        strstr(lower_symptom, "癌") != NULL ||
        strstr(lower_symptom, "化疗") != NULL ||
        strstr(lower_symptom, "放疗") != NULL ||
        strstr(lower_symptom, "肿块") != NULL)
    {
        return "肿瘤科";
    }
    
    // ==============================================
    // 风湿免疫科
    // ==============================================
    if (strstr(lower_symptom, "风湿") != NULL ||
        strstr(lower_symptom, "类风湿") != NULL ||
        strstr(lower_symptom, "关节炎") != NULL ||
        strstr(lower_symptom, "红斑狼疮") != NULL ||
        strstr(lower_symptom, "干燥综合征") != NULL ||
        strstr(lower_symptom, "免疫") != NULL)
    {
        return "风湿免疫科";
    }
    
    // ==============================================
    // 感染科
    // ==============================================
    if (strstr(lower_symptom, "感染") != NULL ||
        strstr(lower_symptom, "病毒") != NULL ||
        strstr(lower_symptom, "细菌") != NULL ||
        strstr(lower_symptom, "肝炎") != NULL ||
        strstr(lower_symptom, "结核") != NULL ||
        strstr(lower_symptom, "艾滋病") != NULL ||
        strstr(lower_symptom, "梅毒") != NULL)
    {
        return "感染科";
    }
    
    // ==============================================
    // 精神科
    // ==============================================
    if (strstr(lower_symptom, "抑郁") != NULL ||
        strstr(lower_symptom, "焦虑") != NULL ||
        strstr(lower_symptom, "精神") != NULL ||
        strstr(lower_symptom, "失眠") != NULL ||
        strstr(lower_symptom, "心理") != NULL ||
        strstr(lower_symptom, "情绪") != NULL)
    {
        return "精神科";
    }
    
    // ==============================================
    // 康复医学科
    // ==============================================
    if (strstr(lower_symptom, "康复") != NULL ||
        strstr(lower_symptom, "理疗") != NULL ||
        strstr(lower_symptom, "针灸") != NULL ||
        strstr(lower_symptom, "推拿") != NULL ||
        strstr(lower_symptom, "按摩") != NULL ||
        strstr(lower_symptom, "康复训练") != NULL)
    {
        return "康复医学科";
    }
    
    // ==============================================
    // 普通外科
    // ==============================================
    if (strstr(lower_symptom, "阑尾炎") != NULL ||
        strstr(lower_symptom, "疝气") != NULL ||
        strstr(lower_symptom, "痔疮") != NULL ||
        strstr(lower_symptom, "脂肪瘤") != NULL ||
        strstr(lower_symptom, "脓肿") != NULL)
    {
        return "普通外科";
    }
    
    // ==============================================
    // 肝胆外科
    // ==============================================
    if (strstr(lower_symptom, "胆结石") != NULL ||
        strstr(lower_symptom, "胆囊炎") != NULL ||
        strstr(lower_symptom, "肝癌") != NULL ||
        strstr(lower_symptom, "肝硬化") != NULL ||
        strstr(lower_symptom, "胆管") != NULL)
    {
        return "肝胆外科";
    }
    
    // ==============================================
    // 心胸外科
    // ==============================================
    if (strstr(lower_symptom, "肺癌") != NULL ||
        strstr(lower_symptom, "食管癌") != NULL ||
        strstr(lower_symptom, "纵隔") != NULL ||
        strstr(lower_symptom, "胸") != NULL)
    {
        return "心胸外科";
    }
    
    // ==============================================
    // 神经外科
    // ==============================================
    if (strstr(lower_symptom, "脑") != NULL ||
        strstr(lower_symptom, "颅内") != NULL ||
        strstr(lower_symptom, "脑膜") != NULL ||
        strstr(lower_symptom, "垂体") != NULL)
    {
        return "神经外科";
    }
    
    // ==============================================
    // 眼科
    // ==============================================
    if (strstr(lower_symptom, "眼睛") != NULL ||
        strstr(lower_symptom, "视力") != NULL ||
        strstr(lower_symptom, "近视") != NULL ||
        strstr(lower_symptom, "远视") != NULL ||
        strstr(lower_symptom, "散光") != NULL ||
        strstr(lower_symptom, "眼痛") != NULL ||
        strstr(lower_symptom, "白内障") != NULL ||
        strstr(lower_symptom, "青光眼") != NULL ||
        strstr(lower_symptom, "视网膜") != NULL)
    {
        return "眼科";
    }
    
    // ==============================================
    // 耳鼻喉科
    // ==============================================
    if (strstr(lower_symptom, "耳朵") != NULL ||
        strstr(lower_symptom, "耳鸣") != NULL ||
        strstr(lower_symptom, "听力") != NULL ||
        strstr(lower_symptom, "鼻炎") != NULL ||
        strstr(lower_symptom, "鼻塞") != NULL ||
        strstr(lower_symptom, "喉咙") != NULL ||
        strstr(lower_symptom, "咽炎") != NULL ||
        strstr(lower_symptom, "扁桃体") != NULL ||
        strstr(lower_symptom, "中耳炎") != NULL)
    {
        return "耳鼻喉科";
    }
    
    // ==============================================
    // 口腔科
    // ==============================================
    if (strstr(lower_symptom, "牙痛") != NULL ||
        strstr(lower_symptom, "蛀牙") != NULL ||
        strstr(lower_symptom, "牙龈") != NULL ||
        strstr(lower_symptom, "口腔溃疡") != NULL ||
        strstr(lower_symptom, "牙齿") != NULL ||
        strstr(lower_symptom, "牙周") != NULL ||
        strstr(lower_symptom, "智齿") != NULL)
    {
        return "口腔科";
    }
    
    // ==============================================
    // 皮肤科
    // ==============================================
    if (strstr(lower_symptom, "皮肤") != NULL ||
        strstr(lower_symptom, "过敏") != NULL ||
        strstr(lower_symptom, "湿疹") != NULL ||
        strstr(lower_symptom, "皮炎") != NULL ||
        strstr(lower_symptom, "痘痘") != NULL ||
        strstr(lower_symptom, "皮疹") != NULL ||
        strstr(lower_symptom, "痤疮") != NULL ||
        strstr(lower_symptom, "荨麻疹") != NULL)
    {
        return "皮肤科";
    }
    
    // ==============================================
    // 普通内科（兜底）
    // ==============================================
    if (strstr(lower_symptom, "发热") != NULL ||
        strstr(lower_symptom, "发烧") != NULL ||
        strstr(lower_symptom, "感冒") != NULL ||
        strstr(lower_symptom, "乏力") != NULL ||
        strstr(lower_symptom, "疲劳") != NULL)
    {
        return "内科";
    }
    
    // ==============================================
    // 普通外科（兜底）
    // ==============================================
    if (strstr(lower_symptom, "外伤") != NULL ||
        strstr(lower_symptom, "摔伤") != NULL ||
        strstr(lower_symptom, "撞伤") != NULL ||
        strstr(lower_symptom, "割伤") != NULL ||
        strstr(lower_symptom, "肿胀") != NULL ||
        strstr(lower_symptom, "脱臼") != NULL)
    {
        return "外科";
    }
    
    // 默认分诊到全科
    return "全科";
}

/**
 * @brief 获取患者状态文本描述
 * @param status 患者就医状态枚举值
 * @return 状态对应的中文描述字符串
 */
const char* get_patient_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING:
            return "待诊";
        case STATUS_EXAMINING:
            return "检查中";
        case STATUS_RECHECK_PENDING:
            return "检查后待复诊";
        case STATUS_UNPAID:
            return "已看诊待缴费";
        case STATUS_WAIT_MED:
            return "已缴费待取药";
        case STATUS_HOSPITALIZED:
            return "住院中";
        case STATUS_COMPLETED:
            return "就诊结束";
        default:
            return "未知状态";
    }
}

// ==========================================
// 患者建档功能
// ==========================================

/**
 * @brief 患者建档
 * @param name 患者姓名
 * @param age 患者年龄
 * @param id_card 患者身份证号
 * @param symptom 症状描述
 * @param target_dept 目标科室
 * @return 成功返回患者节点指针，失败返回NULL
 */
PatientNode* register_patient(
    const char* name,
    int age,
    const char* id_card,
    const char* symptom,
    const char* target_dept
)
{
    char new_id[MAX_ID_LEN];
    PatientNode* new_patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法建档！\n");
        return NULL;
    }
    
    // 检查患者姓名是否为空
    if (name == NULL || strlen(name) == 0)
    {
        printf("⚠️ 患者姓名不能为空！\n");
        return NULL;
    }
    
    // 检查患者姓名长度是否合法
    int name_len = strlen(name);
    if (name_len < 2 || name_len > 20)
    {
        printf("⚠️ 患者姓名长度必须在 2-20 个字符之间！\n");
        return NULL;
    }
    
    // 检查患者年龄是否合法
    if (age < 0 || age > 130)
    {
        printf("⚠️ 患者年龄必须在 0-130 岁之间！\n");
        return NULL;
    }
    
    // 检查身份证号格式是否合法
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法，建档失败！\n");
        return NULL;
    }
    
    // 检查身份证号是否已存在
    if (find_patient_by_id_card(id_card) != NULL)
    {
        printf("⚠️ 该身份证号已存在，不能重复建档！\n");
        return NULL;
    }
    
    // 智能导诊：根据症状推荐科室
    const char* recommended_dept = recommend_dept_by_symptom(symptom);
    
    // 如果用户没有指定科室，则使用推荐的科室
    char actual_dept[MAX_NAME_LEN] = "";
    if (target_dept == NULL || strlen(target_dept) == 0)
    {
        strcpy(actual_dept, recommended_dept);
    }
    else
    {
        strcpy(actual_dept, target_dept);
    }
    
    // 显示智能导诊提示
    printf("🤖 智能导诊提示：根据您的症状\"%s\"，系统推荐您前往【%s】就诊\n", 
           symptom != NULL && strlen(symptom) > 0 ? symptom : "未描述", 
           recommended_dept);
    
    // 如果用户指定的科室与推荐科室不一致，给出提示
    if (target_dept != NULL && strlen(target_dept) > 0 && strcmp(target_dept, recommended_dept) != 0)
    {
        printf("💡 您选择的科室是【%s】，系统推荐科室是【%s】\n", target_dept, recommended_dept);
    }
    
    // 生成新的患者编号
    generate_patient_id(new_id);
    
    // 创建患者节点
    new_patient = create_patient_node(new_id, name, age, id_card);
    if (new_patient == NULL)
    {
        printf("⚠️ 患者节点创建失败！\n");
        return NULL;
    }

    // 复制患者信息到节点中
    copy_text_field(new_patient->id_card, MAX_ID_LEN, id_card);
    copy_text_field(new_patient->symptom, MAX_SYMPTOM_LEN, symptom);
    copy_text_field(new_patient->target_dept, MAX_NAME_LEN, actual_dept);

    // 将患者节点插入到链表尾部
    insert_patient_tail(g_patient_list, new_patient);

    return new_patient;
}

// ==========================================
// 患者档案管理功能
// ==========================================

/**
 * @brief 显示患者档案详细信息
 * @param patient 患者节点指针
 */
void display_patient_archive(const PatientNode* patient)
{
    char masked_id[19] = "";

    // 参数校验
    if (patient == NULL)
    {
        return;
    }

    // 处理身份证号脱敏
    if (strlen(patient->id_card) > 0)
    {
        mask_id_card(patient->id_card, masked_id);
    }
    else
    {
        copy_text_field(masked_id, (int)sizeof(masked_id), get_display_text(patient->id_card));
    }

    // 显示患者档案信息
    printf("\n================ 患者档案信息 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    printf("脱敏身份证号：%s\n", masked_id);
    printf("症状：%s\n", get_display_text(patient->symptom));
    printf("目标科室：%s\n", get_display_text(patient->target_dept));
    printf("当前状态：%s\n", get_patient_status_text(patient->status));
    printf("账户余额：%.2f\n", patient->balance);
    printf("==============================================\n");
}

/**
 * @brief 根据患者编号查询档案
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;

    // 获取患者节点（带错误检查）
    patient = get_patient_by_id_checked(patient_id, "查询患者档案");
    if (patient == NULL)
    {
        return 0;
    }

    // 显示患者档案信息
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief 根据身份证号查询档案
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询档案！\n");
        return 0;
    }
    
    // 检查身份证号是否为空
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 身份证号不能为空！\n");
        return 0;
    }
    
    // 检查身份证号格式是否合法
    if (!validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法！\n");
        return 0;
    }
    
    // 根据身份证号查找患者
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者档案！\n");
        return 0;
    }
    
    // 显示患者档案信息
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief 根据姓名关键词查询档案（支持模糊查询和拼音首字母搜索）
 * @param name_keyword 姓名关键词（可以是汉字或拼音首字母）
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_name(const char* name_keyword)
{
    PatientNode* curr = NULL;
    int found = 0;
    char pinyin[MAX_NAME_LEN];
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询档案！\n");
        return 0;
    }
    
    // 检查姓名关键词是否为空
    if (name_keyword == NULL || strlen(name_keyword) == 0)
    {
        printf("⚠️ 患者姓名不能为空！\n");
        return 0;
    }
    
    // 遍历患者链表，查找匹配的姓名
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // 1. 子串匹配（支持汉字模糊查询）
        if (strstr(curr->name, name_keyword) != NULL)
        {
            display_patient_archive(curr);
            found = 1;
        }
        else
        {
            // 2. 拼音首字母匹配（支持拼音首字母搜索）
            name_to_pinyin(curr->name, pinyin, MAX_NAME_LEN);
            if (strstr(pinyin, name_keyword) != NULL)
            {
                display_patient_archive(curr);
                found = 1;
            }
        }
        curr = curr->next;
    }
    
    // 如果没有找到匹配的患者
    if (!found)
    {
        printf("⚠️ 未找到包含\"%s\"的患者档案！\n", name_keyword);
        return 0;
    }
    
    return 1;
}

// ==========================================
// 患者自助基础病历查询功能
// ==========================================

/**
 * @brief 患者自助查询基础病历信息（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_basic_patient_record(const char* patient_id, const char* id_card)
{
    char masked_id[19];
    
    // 获取患者节点（带错误检查）
    PatientNode* patient = get_patient_by_id_checked(patient_id, "查询基础病历");

    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，病历查询失败！\n");
        return 0;
    }

    // 检查身份证号格式是否合法
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("提示：身份证号格式不合法，无法进行身份核验！\n");
        return 0;
    }

    // 身份核验：检查输入的身份证号是否与患者档案中的身份证号匹配
    if (strcmp(patient->id_card, id_card) != 0)
    {
        printf("⚠️ 身份核验失败，禁止访问基础病历信息！\n");
        return 0;
    }

    // 身份证号脱敏处理
    mask_id_card(patient->id_card, masked_id);
    
    // 显示基础病历信息
    printf("\n================ 基础病历信息 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    printf("症状描述: %s\n", get_display_text(patient->symptom));
    printf("目标科室: %s\n", get_display_text(patient->target_dept));
    printf("当前就诊状态: %s\n", get_patient_status_text(patient->status));
    printf("最近一次诊断结论: %s\n",
        strlen(patient->diagnosis_text) > 0 ? patient->diagnosis_text : "暂无诊断记录");
    printf("最近一次处理意见: %s\n",
        strlen(patient->treatment_advice) > 0 ? patient->treatment_advice : "暂无处理意见");
    printf("身份证号: %s\n", masked_id);

    return 1;
}

/**
 * @brief 更新患者档案信息
 * @param patient_id 患者编号（不可修改）
 * @param name 新姓名（留空表示不修改）
 * @param age 新年龄（0表示不修改）
 * @param symptom 新症状描述（留空表示不修改）
 * @param target_dept 新目标科室（留空表示不修改）
 * @param id_card 新身份证号（留空表示不修改）
 * @param balance 新账户余额
 * @return 成功返回1，失败返回0
 */
int update_patient_archive(const char* patient_id, const char* name, int age, const char* symptom, const char* target_dept, const char* id_card, double balance)
{
    // 获取患者节点（带错误检查）
    PatientNode* patient = get_patient_by_id_checked(patient_id, "修改患者档案");

    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，更新失败！\n");
        return 0;
    }
    
    // 更新患者信息（只更新非空字段）
    if (name != NULL && strlen(name) > 0)
    {
        // 检查患者姓名长度是否合法
        int name_len = strlen(name);
        if (name_len < 2 || name_len > 20)
        {
            printf("⚠️ 患者姓名长度必须在 2-20 个字符之间！\n");
            return 0;
        }
        copy_text_field(patient->name, MAX_NAME_LEN, name);
    }
    
    if (age > 0)
    {
        // 检查患者年龄是否合法
        if (age < 0 || age > 130)
        {
            printf("⚠️ 患者年龄必须在 0-130 岁之间！\n");
            return 0;
        }
        patient->age = age;
    }
    
    if (symptom != NULL && symptom[0] != '\0')
    {
        copy_text_field(patient->symptom, MAX_SYMPTOM_LEN, symptom);
    }
    
    if (target_dept != NULL && target_dept[0] != '\0')
    {
        copy_text_field(patient->target_dept, MAX_NAME_LEN, target_dept);
    }
    
    // 更新身份证号（需要验证）
    if (id_card != NULL && strlen(id_card) > 0)
    {
        // 验证身份证号格式
        if (!validate_id_card(id_card))
        {
            printf("⚠️ 身份证号格式不合法，更新失败！\n");
            return 0;
        }
        
        // 检查新身份证号是否已被其他患者使用
        PatientNode* existing_patient = find_patient_by_id_card(id_card);
        if (existing_patient != NULL && strcmp(existing_patient->id, patient_id) != 0)
        {
            printf("⚠️ 该身份证号已被其他患者使用，更新失败！\n");
            return 0;
        }
        
        // 更新身份证号
        copy_text_field(patient->id_card, MAX_ID_LEN, id_card);
    }
    
    // 更新账户余额
    patient->balance = balance;
    
    printf("✅ 患者档案更新成功！\n");
    return 1;
}

// ==========================================
// 患者就诊概览功能
// ==========================================

/**
 * @brief 显示患者就诊概览信息
 * @param patient 患者节点指针
 */
void display_patient_visit_overview(const PatientNode* patient)
{
    // 参数校验
    if (patient == NULL)
    {
        printf("⚠️ 患者信息为空！\n");
        return;
    }
    
    // 显示患者基本信息
    printf("\n================ 患者就诊概览 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    
    // 脱敏身份证号
    if (strlen(patient->id_card) > 0)
    {
        char masked_id[MAX_ID_LEN] = {0};
        mask_id_card(patient->id_card, masked_id);
        printf("身份证号：%s\n", masked_id);
    }
    else
    {
        printf("身份证号：暂无\n");
    }
    
    // 症状描述（为空时显示"暂无"）
    if (patient->symptom[0] != '\0')
        printf("症状描述: %s\n", patient->symptom);
    else
        printf("症状描述: 暂无\n");
    
    // 目标科室（为空时显示"暂无"）
    if (patient->target_dept[0] != '\0')
        printf("目标科室: %s\n", patient->target_dept);
    else
        printf("目标科室: 暂无\n");
    
    // 当前状态和账户余额
    printf("当前状态: %s\n", get_patient_status_text(patient->status));
    printf("账户余额：%.2f\n", patient->balance);
    
    // 最近一次诊断结论
    if (patient->diagnosis_text[0] != '\0')
        printf("最近诊断结论: %s\n", patient->diagnosis_text);
    else
        printf("最近诊断结论: 暂无诊断记录\n");
    
    // 最近一次处理意见
    if (patient->treatment_advice[0] != '\0')
        printf("处理意见: %s\n", patient->treatment_advice);
    else
        printf("处理意见: 暂无处理意见\n");
    
    // 最近一次预约信息
    AppointmentNode* latest_appointment = find_latest_appointment_by_patient_id(patient->id);
    if (latest_appointment != NULL)
    {
        printf("\n最近预约信息:\n");
        printf("  预约编号：%s\n", latest_appointment->appointment_id);
        printf("  预约日期：%s\n", latest_appointment->appointment_date);
        printf("  预约时段：%s\n", latest_appointment->appointment_slot);
        
        // 显示预约医生或科室
        if (latest_appointment->appoint_doctor[0] != '\0')
            printf("  预约医生：%s\n", latest_appointment->appoint_doctor);
        else if (latest_appointment->appoint_dept[0] != '\0')
            printf("  预约科室：%s\n", latest_appointment->appoint_dept);
        else
            printf("  预约科室：暂无\n");
        printf("  预约状态：%s\n", get_appointment_status_text(latest_appointment->appointment_status));
    }
    else
    {
        printf("\n最近预约信息: 暂无预约记录\n");
    }
    
    printf("==============================================\n");
}

/**
 * @brief 根据患者编号查询就诊概览
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return 0;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }
    
    // 根据患者编号查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，查询失败！\n");
        return 0;
    }
    
    // 显示患者就诊概览信息
    display_patient_visit_overview(patient);
    return 1;
}

/**
 * @brief 根据身份证号查询就诊概览
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return 0;
    }
    
    // 检查身份证号是否为空
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 身份证号不能为空！\n");
        return 0;
    }
    
    // 检查身份证号格式是否合法
    if (!validate_id_card(id_card))
    {
        printf("提示：身份证号格式不合法，无法查询！\n");
        return 0;
    }
    
    // 根据身份证号查找患者
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，查询失败！\n");
        return 0;
    }
    
    // 显示患者就诊概览信息
    display_patient_visit_overview(patient);
    return 1;
}

/**
 * @brief 获取诊疗决策文字描述（患者视角）
 * @param decision 诊疗决策整数值
 * @return 决策对应的中文描述字符串
 */
static const char* get_decision_text_for_patient(int decision)
{
    switch (decision) {
        case 1: // 结束就诊
            return "结束就诊";
        case 2: // 开药
            return "开药";
        case 3: // 开检查
            return "开检查";
        case 4: // 办理住院
            return "办理住院";
        default:
            return "未记录决策";
    }
}

/**
 * @brief 患者自助查询自己的历史就诊记录（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_patient_consult_history_verified(const char* patient_id, const char* id_card)
{
    // 第一步：身份核验
    if (patient_id == NULL || strlen(patient_id) == 0 || id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 患者编号或身份证号不能为空！\n");
        return 0;
    }
    
    // 根据身份证号查找患者
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("⚠️ 身份核验失败，未找到对应患者！\n");
        return 0;
    }
    
    // 检查患者编号是否匹配
    if (strcmp(patient->id, patient_id) != 0)
    {
        printf("⚠️ 身份核验失败，患者编号与身份证号不匹配！\n");
        return 0;
    }
    
    printf("\n==============================================\n");
    printf("          历史就诊记录查询\n");
    printf("==============================================\n");
    printf("患者姓名: %s\n", patient->name);
    printf("患者编号: %s\n", patient->id);
    printf("==============================================\n\n");
    
    // 第二步：遍历现有接诊记录链表（最新在前）
    ConsultRecordNode* head = g_consult_record_list;
    
    // 判断空链表：头结点为空 或 没有真实节点
    if (head == NULL || head->next == NULL)
    {
        printf("暂无历史就诊记录\n");
        printf("==============================================\n");
        return 1;
    }
    
    // 找到最后一条记录（最新的记录）
    ConsultRecordNode* curr = head->next;  // 从第一个真实节点开始
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    
    int record_count = 0;
    ConsultRecordNode* temp = curr;
    
    // 从最新记录开始向前遍历，确保不会遍历到头结点
    while (temp != NULL && temp != head)
    {
        if (strcmp(temp->patient_id, patient_id) == 0)
        {
            record_count++;
            
            printf("【历史就诊记录 %d】\n", record_count);
            printf("--------------------------------------------------\n");
            printf("接诊记录编号: %s\n", temp->record_id);
            printf("接诊时间: %s\n", temp->consult_time);
            
            // 优先显示接诊医生姓名
            DoctorNode* doctor = find_doctor_by_id(g_doctor_list, temp->doctor_id);
            if (doctor != NULL)
            {
                printf("接诊医生: %s（%s）\n", doctor->name, temp->doctor_id);
            }
            else
            {
                printf("接诊医生: 未知医生（%s）\n", temp->doctor_id);
            }
            
            printf("诊断结论: %s\n", temp->diagnosis_text);
            printf("处理意见: %s\n", temp->treatment_advice);
            printf("诊疗决策: %s\n", get_decision_text_for_patient(temp->decision));
            printf("接诊前状态: %s\n", get_patient_status_text(temp->pre_status));
            printf("接诊后状态: %s\n", get_patient_status_text(temp->post_status));
            
            // 预约编号（如果有）
            if (temp->appointment_id[0] != '\0')
            {
                printf("预约编号: %s\n", temp->appointment_id);
            }
            else
            {
                printf("预约编号: 无\n");
            }
            
            // 检查结果（当前系统暂不支持单独检查结果记录，简化处理）
            printf("检查结果: 暂无检查结果（课设简化版）\n");
            
            printf("--------------------------------------------------\n\n");
        }
        
        temp = temp->prev;
    }
    
    if (record_count == 0)
    {
        printf("暂无历史就诊记录\n");
    }
    else
    {
        printf("共找到 %d 条历史就诊记录\n", record_count);
    }
    
    printf("==============================================\n");
    return 1;
}
