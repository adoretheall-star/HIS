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
#include "appointment.h"
#include "utils.h"
#include "patient_service.h"

// 智能分诊权重规则结构体
typedef struct {
    const char* keyword; // 症状关键词
    int weight;          // 权重 (1000:一票否决, 100:重症指向, 50:中度/交叉, 10:普通症状)
    const char* dept;    // 推荐科室
} SymptomRule;

// 全量症状权重字典（逐步构建中...）
static const SymptomRule g_symptom_dict[] = {
    // 🚨 1. 极危重（1000分，一票否决进急诊）
    {"紧急", 1000, "急诊科"}, {"急救", 1000, "急诊科"}, {"昏迷", 1000, "急诊科"}, {"休克", 1000, "急诊科"},
    {"大出血", 1000, "急诊科"}, {"心跳停止", 1000, "急诊科"}, {"车祸", 1000, "急诊科"}, {"交通事故", 1000, "急诊科"},
    {"坠落", 1000, "急诊科"}, {"烫伤", 1000, "急诊科"}, {"烧伤", 1000, "急诊科"}, {"中毒", 1000, "急诊科"},
    {"过敏性休克", 1000, "急诊科"}, {"剧烈疼痛", 1000, "急诊科"},

    // ⚠️ 2. 重症指向（100分）
    {"心梗", 100, "心血管内科"}, {"心肌梗死", 100, "心血管内科"}, {"心衰", 100, "心血管内科"}, {"心功能不全", 100, "心血管内科"},
    {"中风", 100, "神经内科"}, {"脑溢血", 100, "神经内科"}, {"脑出血", 100, "神经内科"}, {"脑梗塞", 100, "神经内科"},
    {"脑梗死", 100, "神经内科"}, {"瘫痪", 100, "神经内科"}, {"癫痫", 100, "神经内科"},
    {"尿毒症", 100, "肾内科"}, {"肾衰", 100, "肾内科"}, {"急性肾衰", 100, "肾内科"}, {"慢性肾衰", 100, "肾内科"},
    {"骨折", 100, "骨科"}, {"宫外孕", 100, "妇产科"}, {"分娩", 100, "妇产科"}, {"流产", 100, "妇产科"},
    {"艾滋病", 100, "感染科"}, {"梅毒", 100, "感染科"}, {"淋病", 100, "感染科"}, {"尖锐湿疣", 100, "感染科"},
    {"精神分裂", 100, "精神科"}, {"精神病", 100, "精神科"},
    {"肿瘤", 100, "肿瘤科"}, {"癌症", 100, "肿瘤科"}, {"癌", 100, "肿瘤科"}, {"化疗", 100, "肿瘤科"},
    {"放疗", 100, "肿瘤科"}, {"恶性", 100, "肿瘤科"}, {"转移", 100, "肿瘤科"}, {"复发", 100, "肿瘤科"},
    {"阑尾炎", 100, "普通外科"}, {"肺癌", 100, "心胸外科"}, {"食管癌", 100, "心胸外科"}, {"肝癌", 100, "肝胆外科"},

    // 🩺 3. 交叉/中度症状（50分）
    {"呼吸困难", 50, "急诊科"}, {"呼吸困难", 50, "呼吸内科"}, {"呼吸急促", 50, "急诊科"}, {"呼吸急促", 50, "呼吸内科"},
    {"胸痛", 50, "急诊科"}, {"胸痛", 50, "心血管内科"}, {"胸痛", 50, "呼吸内科"},
    {"胸闷", 50, "心血管内科"}, {"胸闷", 50, "呼吸内科"}, {"腹痛", 50, "急诊科"}, {"腹痛", 50, "消化内科"},
    {"头晕", 50, "神经内科"}, {"眩晕", 50, "神经内科"}, {"过敏", 50, "急诊科"}, {"过敏", 50, "皮肤科"},
    {"骨裂", 50, "骨科"}, {"脱臼", 50, "骨科"}, {"脱臼", 50, "外科"}, {"摔伤", 50, "急诊科"}, {"摔伤", 50, "外科"},
    {"心跳加速", 50, "急诊科"}, {"心跳加速", 50, "心血管内科"}, {"高血压", 50, "心血管内科"}, {"低血压", 50, "心血管内科"},
    {"冠心病", 50, "心血管内科"}, {"心绞痛", 50, "心血管内科"}, {"心律不齐", 50, "心血管内科"}, {"心律失常", 50, "心血管内科"},
    {"房颤", 50, "心血管内科"}, {"早搏", 50, "心血管内科"}, {"心肌病", 50, "心血管内科"}, {"心肌炎", 50, "心血管内科"},
    {"心脏瓣膜", 50, "心血管内科"}, {"心肌", 50, "心血管内科"},
    {"哮喘", 50, "呼吸内科"}, {"肺炎", 50, "呼吸内科"}, {"支气管炎", 50, "呼吸内科"}, {"慢性支气管炎", 50, "呼吸内科"},
    {"支气管扩张", 50, "呼吸内科"}, {"咯血", 50, "呼吸内科"}, {"肺结核", 50, "呼吸内科"}, {"结核", 50, "感染科"},
    {"肺结节", 50, "呼吸内科"}, {"肺气肿", 50, "呼吸内科"}, {"慢阻肺", 50, "呼吸内科"}, {"肺心病", 50, "呼吸内科"},
    {"胃溃疡", 50, "消化内科"}, {"胃痛", 50, "消化内科"}, {"十二指肠溃疡", 50, "消化内科"}, {"结肠炎", 50, "消化内科"}, {"肠易激综合征", 50, "消化内科"},
    {"便血", 50, "消化内科"}, {"黄疸", 50, "消化内科"}, {"肝炎", 50, "消化内科"}, {"肝炎", 50, "感染科"},
    {"甲肝", 50, "感染科"}, {"乙肝", 50, "感染科"}, {"丙肝", 50, "感染科"}, {"肝硬化", 50, "消化内科"},
    {"肝硬化", 50, "肝胆外科"}, {"胆囊炎", 50, "消化内科"}, {"胆囊炎", 50, "肝胆外科"}, {"胆结石", 50, "消化内科"},
    {"胆结石", 50, "肝胆外科"}, {"胰腺炎", 50, "消化内科"},
    {"肾炎", 50, "肾内科"}, {"急性肾炎", 50, "肾内科"}, {"慢性肾炎", 50, "肾内科"}, {"肾病", 50, "肾内科"},
    {"肾病综合征", 50, "肾内科"}, {"蛋白尿", 50, "肾内科"}, {"血尿", 50, "肾内科"}, {"肾积水", 50, "肾内科"},
    {"肾结石", 50, "肾内科"}, {"肾结石", 50, "泌尿外科"}, {"结石", 50, "泌尿外科"}, {"输尿管结石", 50, "泌尿外科"},
    {"膀胱结石", 50, "泌尿外科"}, {"尿路结石", 50, "泌尿外科"},
    {"糖尿病", 50, "内分泌科"}, {"甲亢", 50, "内分泌科"}, {"甲减", 50, "内分泌科"}, {"甲状腺", 50, "内分泌科"},
    {"甲状腺炎", 50, "内分泌科"}, {"甲状腺结节", 50, "内分泌科"}, {"血糖", 50, "内分泌科"}, {"高血糖", 50, "内分泌科"},
    {"低血糖", 50, "内分泌科"},
    {"帕金森", 50, "神经内科"}, {"脑炎", 50, "神经内科"}, {"脑膜炎", 50, "神经内科"}, {"脑膜", 50, "神经外科"},
    {"脑肿瘤", 50, "神经内科"}, {"颅内", 50, "神经外科"},
    {"红斑狼疮", 50, "风湿免疫科"}, {"干燥综合征", 50, "风湿免疫科"}, {"强直性脊柱炎", 50, "风湿免疫科"},
    {"痛风", 50, "风湿免疫科"}, {"银屑病关节炎", 50, "风湿免疫科"},
    {"抑郁症", 50, "精神科"}, {"焦虑症", 50, "精神科"}, {"躁狂", 50, "精神科"}, {"躁郁症", 50, "精神科"},
    {"强迫症", 50, "精神科"}, {"恐惧症", 50, "精神科"},
    {"白内障", 50, "眼科"}, {"青光眼", 50, "眼科"}, {"视网膜", 50, "眼科"}, {"中耳炎", 50, "耳鼻喉科"},

    // 💊 4. 普通/基础症状（10分）
    {"怀孕", 10, "妇产科"}, {"妊娠", 10, "妇产科"}, {"产检", 10, "妇产科"}, {"妇科", 10, "妇产科"}, {"月经", 10, "妇产科"},
    {"痛经", 10, "妇产科"}, {"乳腺", 10, "妇产科"}, {"子宫", 10, "妇产科"}, {"卵巢", 10, "妇产科"}, {"宫颈", 10, "妇产科"},
    {"阴道", 10, "妇产科"}, {"盆腔", 10, "妇产科"}, {"不孕", 10, "妇产科"}, {"不育", 10, "妇产科"}, {"产后", 10, "妇产科"},
    {"儿童", 10, "儿科"}, {"小孩", 10, "儿科"}, {"宝宝", 10, "儿科"}, {"婴儿", 10, "儿科"}, {"幼儿", 10, "儿科"},
    {"小儿", 10, "儿科"}, {"新生儿", 10, "儿科"}, {"孩童", 10, "儿科"},
    {"关节", 10, "骨科"}, {"腰椎", 10, "骨科"}, {"颈椎", 10, "骨科"}, {"脊椎", 10, "骨科"}, {"骨质疏松", 10, "骨科"},
    {"股骨头", 10, "骨科"}, {"骨关节炎", 10, "骨科"}, {"风湿性关节炎", 10, "骨科"}, {"类风湿关节炎", 10, "骨科"},
    {"腱鞘炎", 10, "骨科"}, {"滑膜炎", 10, "骨科"}, {"扭伤", 10, "骨科"}, {"拉伤", 10, "骨科"}, {"肩周炎", 10, "骨科"},
    {"颈椎病", 10, "骨科"}, {"腰椎间盘突出", 10, "骨科"},
    {"心悸", 10, "心血管内科"}, {"心慌", 10, "心血管内科"}, {"咳嗽", 10, "呼吸内科"}, {"咳痰", 10, "呼吸内科"},
    {"气短", 10, "呼吸内科"},
    {"胃痛", 10, "消化内科"}, {"胃胀", 10, "消化内科"}, {"胃炎", 10, "消化内科"}, {"急性胃炎", 10, "消化内科"},
    {"慢性胃炎", 10, "消化内科"}, {"腹泻", 10, "消化内科"}, {"便秘", 10, "消化内科"}, {"肝", 10, "消化内科"},
    {"胆囊", 10, "消化内科"}, {"胰腺", 10, "消化内科"}, {"恶心", 10, "消化内科"}, {"呕吐", 10, "消化内科"},
    {"食欲不振", 10, "消化内科"}, {"消化不良", 10, "消化内科"},
    {"头痛", 10, "神经内科"}, {"失眠", 10, "神经内科"}, {"麻木", 10, "神经内科"}, {"神经", 10, "神经内科"},
    {"神经炎", 10, "神经内科"}, {"神经痛", 10, "神经内科"}, {"三叉神经痛", 10, "神经内科"}, {"坐骨神经痛", 10, "神经内科"},
    {"记忆力下降", 10, "神经内科"}, {"注意力不集中", 10, "神经内科"},
    {"激素", 10, "内分泌科"}, {"肥胖", 10, "内分泌科"}, {"消瘦", 10, "内分泌科"}, {"多饮", 10, "内分泌科"},
    {"多尿", 10, "内分泌科"}, {"多食", 10, "内分泌科"}, {"口渴", 10, "内分泌科"}, {"肾上腺", 10, "内分泌科"}, {"垂体", 10, "内分泌科"},
    {"尿频", 10, "泌尿外科"}, {"尿急", 10, "泌尿外科"}, {"尿痛", 10, "泌尿外科"}, {"前列腺", 10, "泌尿外科"},
    {"前列腺炎", 10, "泌尿外科"}, {"前列腺增生", 10, "泌尿外科"}, {"阳痿", 10, "泌尿外科"}, {"早泄", 10, "泌尿外科"},
    {"包皮", 10, "泌尿外科"}, {"包茎", 10, "泌尿外科"}, {"睾丸", 10, "泌尿外科"}, {"睾丸炎", 10, "泌尿外科"},
    {"精索", 10, "泌尿外科"}, {"精索静脉曲张", 10, "泌尿外科"}, {"膀胱", 10, "泌尿外科"}, {"膀胱炎", 10, "泌尿外科"},
    {"尿道炎", 10, "泌尿外科"}, {"urinary tract infection", 10, "泌尿外科"},
    {"肿块", 10, "肿瘤科"}, {"结节", 10, "肿瘤科"}, {"瘤", 10, "肿瘤科"}, {"良性", 10, "肿瘤科"},
    {"风湿", 10, "风湿免疫科"}, {"类风湿", 10, "风湿免疫科"}, {"关节炎", 10, "风湿免疫科"}, {"免疫", 10, "风湿免疫科"},
    {"免疫系统", 10, "风湿免疫科"},
    {"感染", 10, "感染科"}, {"病毒", 10, "感染科"}, {"细菌", 10, "感染科"}, {"支原体", 10, "感染科"},
    {"衣原体", 10, "感染科"}, {"真菌感染", 10, "感染科"},
    {"抑郁", 10, "精神科"}, {"焦虑", 10, "精神科"}, {"精神", 10, "精神科"}, {"心理", 10, "精神科"},
    {"心理咨询", 10, "精神科"}, {"情绪", 10, "精神科"}, {"情绪波动", 10, "精神科"}, {"神经衰弱", 10, "精神科"}, {"失眠症", 10, "精神科"},
    {"康复", 10, "康复医学科"}, {"理疗", 10, "康复医学科"}, {"针灸", 10, "康复医学科"}, {"推拿", 10, "康复医学科"},
    {"按摩", 10, "康复医学科"}, {"康复训练", 10, "康复医学科"}, {"康复治疗", 10, "康复医学科"}, {"物理治疗", 10, "康复医学科"},
    {"言语治疗", 10, "康复医学科"}, {"作业治疗", 10, "康复医学科"}, {"运动治疗", 10, "康复医学科"},
    {"疝气", 10, "普通外科"}, {"痔疮", 10, "普通外科"}, {"脂肪瘤", 10, "普通外科"}, {"脓肿", 10, "普通外科"},
    {"胆管", 10, "肝胆外科"}, {"纵隔", 10, "心胸外科"}, {"胸", 10, "心胸外科"}, {"脑", 10, "神经外科"},
    {"眼睛", 10, "眼科"}, {"视力", 10, "眼科"}, {"近视", 10, "眼科"}, {"远视", 10, "眼科"}, {"散光", 10, "眼科"}, {"眼痛", 10, "眼科"},
    {"耳朵", 10, "耳鼻喉科"}, {"耳鸣", 10, "耳鼻喉科"}, {"听力", 10, "耳鼻喉科"}, {"鼻炎", 10, "耳鼻喉科"},
    {"鼻塞", 10, "耳鼻喉科"}, {"喉咙", 10, "耳鼻喉科"}, {"咽炎", 10, "耳鼻喉科"}, {"扁桃体", 10, "耳鼻喉科"},
    {"牙痛", 10, "口腔科"}, {"蛀牙", 10, "口腔科"}, {"牙龈", 10, "口腔科"}, {"口腔溃疡", 10, "口腔科"},
    {"牙齿", 10, "口腔科"}, {"牙周", 10, "口腔科"}, {"智齿", 10, "口腔科"},
    {"皮肤", 10, "皮肤科"}, {"湿疹", 10, "皮肤科"}, {"皮炎", 10, "皮肤科"}, {"痘痘", 10, "皮肤科"}, {"皮疹", 10, "皮肤科"},
    {"痤疮", 10, "皮肤科"}, {"荨麻疹", 10, "皮肤科"},
    {"发热", 10, "内科"}, {"发烧", 10, "内科"}, {"感冒", 10, "内科"}, {"乏力", 10, "内科"}, {"疲劳", 10, "内科"},
    {"外伤", 10, "外科"}, {"撞伤", 10, "外科"}, {"割伤", 10, "外科"}, {"肿胀", 10, "外科"}
    // 暂不添加结束符和数组大小计算，等待后续数据注入
};
static const int g_dict_size = sizeof(g_symptom_dict) / sizeof(g_symptom_dict[0]);

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
 * @brief 根据症状描述推荐科室（基于权重积分与一票否决算法）
 * @param symptom 患者症状描述
 * @return 推荐的科室名称字符串
 * 
 * 算法说明：
 * 1. 一票否决：极危重症状（权重≥1000）直接返回急诊科
 * 2. 权重积分：根据症状关键词匹配，为对应科室累加权重分数
 * 3. 平局仲裁：只有唯一最高分科室才返回，否则返回全科
 */
const char* recommend_dept_by_symptom(const char* symptom)
{
    // 入参校验：症状为空或长度为0时，返回全科
    if (symptom == NULL || strlen(symptom) == 0)
    {
        return "全科";
    }
    
    // 将症状转换为小写，避免大小写匹配问题
    char lower_symptom[MAX_SYMPTOM_LEN];
    strncpy(lower_symptom, symptom, MAX_SYMPTOM_LEN - 1);
    lower_symptom[MAX_SYMPTOM_LEN - 1] = '\0';
    for (int i = 0; lower_symptom[i]; i++)
    {
        lower_symptom[i] = tolower(lower_symptom[i]);
    }
    
    // 定义科室计分板：记录每个科室的累计权重分数
    struct {
        const char* dept;
        int score;
    } score_board[50] = {0};
    int board_size = 0;
    
    // 遍历症状权重字典，进行关键词匹配
    for (int i = 0; i < g_dict_size; i++)
    {
        const SymptomRule* rule = &g_symptom_dict[i];
        
        // 检查症状描述中是否包含当前关键词
        if (strstr(lower_symptom, rule->keyword) != NULL)
        {
            // 【一票否决】极危重症状（权重≥1000）直接返回急诊科
            if (rule->weight >= 1000)
            {
                return "急诊科";
            }
            
            // 【分数累加】查找科室是否已在计分板中
            int found = 0;
            for (int j = 0; j < board_size; j++)
            {
                if (strcmp(score_board[j].dept, rule->dept) == 0)
                {
                    // 科室已存在，累加权重分数
                    score_board[j].score += rule->weight;
                    found = 1;
                    break;
                }
            }
            
            // 科室不存在，新增记录
            if (!found && board_size < 50)
            {
                score_board[board_size].dept = rule->dept;
                score_board[board_size].score = rule->weight;
                board_size++;
            }
        }
    }
    
    // 计分板为空，返回全科
    if (board_size == 0)
    {
        return "全科";
    }
    
    // 【平局仲裁】找出分数最高的科室
    int max_score = score_board[0].score;
    const char* best_dept = score_board[0].dept;
    int max_count = 1;
    
    // 遍历计分板，查找最高分并统计最高分科室数量
    for (int i = 1; i < board_size; i++)
    {
        if (score_board[i].score > max_score)
        {
            // 发现更高分，更新最高分和最佳科室
            max_score = score_board[i].score;
            best_dept = score_board[i].dept;
            max_count = 1;
        }
        else if (score_board[i].score == max_score)
        {
            // 发现相同最高分，增加计数
            max_count++;
        }
    }
    
    // 仲裁规则：最高分为0或存在多个最高分科室，返回全科
    if (max_score == 0 || max_count > 1)
    {
        return "全科";
    }
    
    // 唯一最高分科室，返回该科室
    return best_dept;
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
        case STATUS_NO_SHOW:
            return "过号作废";
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
    const char* gender,
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
    PatientNode* existing_patient = find_patient_by_id_card(id_card);
    if (existing_patient != NULL)
    {
        // 无论老档案是否在黑名单中，都不允许重复建档
        if (existing_patient->is_blacklisted != 0)
        {
            printf("⚠️ 该身份证号对应的患者已在黑名单中，请使用原档案处理，不能创建新档案！\n");
        }
        else
        {
            printf("⚠️ 该身份证号已存在，不能重复建档！\n");
        }
        return NULL;
    }
    
    // 时空折叠：夜间模式接管
    const char* recommended_dept;
    char actual_dept[MAX_NAME_LEN] = "";
    
    // 首先统一调用智能导诊，给出真实的症状判定
    recommended_dept = recommend_dept_by_symptom(symptom);
    
    // 生命特权判定（仅看症状）
    if (strcmp(recommended_dept, "急诊科") == 0)
    {
        printf("🚨 症状符合急诊特征，已标记为危重患者！\n");
    }
    
    if (is_night_shift())
    {
        printf("🌙 夜间模式启动！所有普通门诊已关闭，系统将为您自动分配到急诊科。\n");
        // 强制将目标科室设置为急诊科
        if (target_dept != NULL && strlen(target_dept) > 0 && strcmp(target_dept, "急诊科") != 0)
        {
            printf("💡 您选择的科室【%s】在夜间已关闭，系统已为您调整至急诊科。\n", target_dept);
        }
        // 只修改实际科室，不修改推荐科室
        strcpy(actual_dept, "急诊科");
        
        // 显示智能导诊提示
        printf("🤖 智能导诊提示：根据您的症状\"%s\"，系统推荐您前往【%s】就诊\n", 
               symptom != NULL && strlen(symptom) > 0 ? symptom : "未描述", 
               recommended_dept);
        
        // 提示用户实际就诊科室
        printf("💡 由于夜间模式，您将在【急诊科】就诊\n");
    }
    else
    {
        // 如果用户没有指定科室，则使用推荐的科室
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
    }
    
    // 生成新的患者编号
    generate_patient_id(new_id);
    
    // 创建患者节点
    new_patient = create_patient_node(new_id, name, age, gender, id_card);
    if (new_patient == NULL)
    {
        printf("⚠️ 患者节点创建失败！\n");
        return NULL;
    }

    // 复制患者信息到节点中
    copy_text_field(new_patient->id_card, MAX_ID_LEN, id_card);
    copy_text_field(new_patient->symptom, MAX_SYMPTOM_LEN, symptom);
    copy_text_field(new_patient->target_dept, MAX_NAME_LEN, actual_dept);
    
    // 急诊判定：如果推荐科室是急诊科，标记为急诊患者
    if (strcmp(recommended_dept, "急诊科") == 0)
    {
        new_patient->is_emergency = 1;
        printf("🚨 该患者症状符合急诊特征，已标记为急诊绿色通道患者！\n");
    }
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
    printf("性别：%s\n", strlen(patient->gender) > 0 ? patient->gender : "未设置");
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

    // 检查并作废过期的待缴费订单
    check_and_void_expired_orders(patient);

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
    
    // 检查并作废过期的待缴费订单
    check_and_void_expired_orders(patient);
    
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
    printf("性别：%s\n", strlen(patient->gender) > 0 ? patient->gender : "未设置");
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
        printf("  预约状态：%s\n", get_appointment_display_status(latest_appointment));
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
// ==========================================
// 业务辅助功能
// ==========================================

/**
 * @brief 检查并作废过期的待缴费订单
 * @param patient 患者节点指针
 */
void check_and_void_expired_orders(PatientNode* patient)
{
    // 参数校验
    if (patient == NULL)
    {
        return;
    }
    
    // 检查患者状态是否为待缴费且有未缴费时间戳
    if (patient->status == STATUS_UNPAID && patient->unpaid_time != 0)
    {
        // 获取当前时间
        time_t current_time = time(NULL);
        
        // 检查是否超过72小时
        if (current_time - patient->unpaid_time > 72 * 3600)
        {
            // 遍历并free掉患者script_head链表里的所有节点
            PrescriptionNode* curr = patient->script_head;
            while (curr != NULL)
            {
                PrescriptionNode* temp = curr;
                curr = curr->next;
                free(temp);
            }
            
            // 清空处方相关字段
            patient->script_head = NULL;
            patient->script_count = 0;

            // 同步清理该患者名下所有未支付检查单，避免旧单串入后续新账单
            if (g_check_record_list != NULL)
            {
                CheckRecordNode* check_curr = g_check_record_list->next;
                while (check_curr != NULL)
                {
                    CheckRecordNode* next_node = check_curr->next;
                    if (strcmp(check_curr->patient_id, patient->id) == 0 && check_curr->is_paid == 0)
                    {
                        if (check_curr->prev != NULL)
                        {
                            check_curr->prev->next = check_curr->next;
                        }
                        if (check_curr->next != NULL)
                        {
                            check_curr->next->prev = check_curr->prev;
                        }
                        free(check_curr);
                    }
                    check_curr = next_node;
                }
            }

            patient->unpaid_time = 0;
            patient->status = STATUS_COMPLETED;
            
            // 打印提示信息
            printf("⚠️ 检测到订单超时 72 小时，未缴费处方与检查单已自动作废\n");
        }
    }
}

// ==========================================
// 患者自助查询历史就诊记录功能
// ==========================================

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
    
    // 检查并作废过期的待缴费订单
    check_and_void_expired_orders(patient);
    
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

/**
 * @brief 处理急诊逃单
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int handle_emergency_escape(const char* patient_id)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法处理急诊逃单！\n");
        return 0;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }
    
    // 查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者！\n");
        return 0;
    }
    
    // 检查是否为急诊患者且处于待缴费状态
    if (patient->is_emergency == 1 && patient->status == STATUS_UNPAID)
    {
        // 要求输入逃单金额
        double debt_amount;
        printf("请输入该患者的急诊欠费金额: ");
        scanf("%lf", &debt_amount);
        while (getchar() != '\n'); // 清空输入缓冲区
        
        if (debt_amount <= 0)
        {
            printf("⚠️ 欠费金额必须大于0！\n");
            return 0;
        }
        
        // 记录欠费金额并执行永久拉黑
        patient->emergency_debt = debt_amount;
        patient->is_blacklisted = 2;
        printf("⛔ [最高级别警报] 该患者利用急诊绿色通道逃单，已列入永久黑名单！\n");
        printf("⛔ 逃单金额: %.2f 元\n", debt_amount);
        printf("⛔ 该患者将永久被禁止挂号和预约服务，除非补缴欠费！\n");
        return 1;
    }
    else
    {
        if (patient->is_emergency != 1)
        {
            printf("⚠️ 该患者不是急诊绿色通道患者，无法执行急诊逃单处理！\n");
        }
        else if (patient->status != STATUS_UNPAID)
        {
            printf("⚠️ 该患者当前状态不是待缴费，无法执行急诊逃单处理！\n");
        }
        return 0;
    }
}

/**
 * @brief 补缴欠费并核销黑名单
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int settle_blacklisted_debt(const char* patient_id)
{
    PatientNode* patient = NULL;
    char confirm[10];
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法处理欠费核销！\n");
        return 0;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }
    
    // 查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者！\n");
        return 0;
    }
    
    // 确认患者是否在逃单黑名单中
    if (patient->is_blacklisted != 2)
    {
        printf("⚠️ 该患者不在逃单黑名单中，无需核销！\n");
        return 0;
    }
    
    // 打印患者欠费金额
    printf("======================================================\n");
    printf("                💰 欠费核销确认\n");
    printf("======================================================\n");
    printf("患者编号: %s\n", patient->id);
    printf("患者姓名: %s\n", patient->name);
    printf("身份证号: %s\n", patient->id_card);
    printf("欠费金额: %.2f 元\n", patient->emergency_debt);
    printf("------------------------------------------------------\n");
    printf("确认该患者已补缴全部欠费？(y/n): ");
    
    // 读取确认输入
    fgets(confirm, sizeof(confirm), stdin);
    // 移除换行符
    confirm[strcspn(confirm, "\n")] = 0;
    
    // 检查确认输入
    if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0)
    {
        printf("⚠️ 核销操作已取消！\n");
        return 0;
    }
    
    // 执行核销操作
    patient->emergency_debt = 0.0; // 清零欠费
    patient->is_blacklisted = 0; // 移出黑名单
    patient->status = STATUS_COMPLETED; // 重置状态为就诊结束
    
    // 打印成功提示
    printf("✅ 信用修复成功！该患者已移出黑名单，恢复正常就医权限。\n");
    printf("======================================================\n");
    
    return 1;
}

// ==========================================
// 财务结算模块
// ==========================================
int process_patient_payment(const char* patient_id)
{
    PatientNode* patient = get_patient_by_id_checked(patient_id, "自助缴费");
    if (patient == NULL) return 0;

    // 1. 状态校验
    if (patient->status != STATUS_UNPAID)
    {
        printf("⚠️ 当前状态为[%s]，无需进行缴费操作！\n", get_patient_status_text(patient->status));
        return 0;
    }

    double total_amount = 0.0;
    double actual_pay = 0.0;
    double medicare_cover = 0.0;
    int index = 1;

    printf("\n================ 🏥 账单明细 ================\n");

    // 2. 遍历处方单向链表进行算账
    PrescriptionNode* p_curr = patient->script_head;
    while (p_curr != NULL)
    {
        // 通过药品ID去全局药品库（双向链表）中查价格
        MedicineNode* med = find_medicine_by_id(g_medicine_list, p_curr->med_id);
        if (med != NULL)
        {
            double item_total = med->price * p_curr->quantity;
            double item_actual = item_total;

            // 医保报销逻辑：患者有医保，且药品支持报销
            if (patient->m_type != MEDICARE_NONE)
            {
                if (med->m_type == MEDICARE_CLASS_A)
                {
                    item_actual = item_total * 0.2;
                }
                else if (med->m_type == MEDICARE_CLASS_B)
                {
                    item_actual = item_total * 0.5;
                }
            }

            total_amount += item_total;
            actual_pay += item_actual;

            printf("[%d] 药品: %s | 单价: %.2f | 数量: %d | 小计: %.2f | 医保后: %.2f\n",
                   index++, med->name, med->price, p_curr->quantity, item_total, item_actual);
        }
        p_curr = p_curr->next;
    }

    // 3. 遍历检查记录进行算账
    CheckRecordNode* cr_curr = g_check_record_list->next;
    while (cr_curr != NULL)
    {
        if (strcmp(cr_curr->patient_id, patient_id) == 0 && cr_curr->is_paid == 0)
        {
            // 通过检查项目ID去全局检查项目库中查价格和医保类型
            CheckItemNode* check_item = find_check_item_by_id(g_check_item_list, cr_curr->item_id);
            if (check_item != NULL)
            {
                double item_total = check_item->price;
                double item_actual = item_total;

                // 检查费医保核算逻辑
                if (patient->m_type != MEDICARE_NONE)
                {
                    if (check_item->m_type == MEDICARE_CLASS_A)
                    {
                        item_actual = item_total * 0.2;
                    }
                    else if (check_item->m_type == MEDICARE_CLASS_B)
                    {
                        item_actual = item_total * 0.5;
                    }
                }

                total_amount += item_total;
                actual_pay += item_actual;

                printf("[%d] 检查: %s | 单价: %.2f | 小计: %.2f | 医保后: %.2f\n",
                       index++, check_item->item_name, check_item->price, item_total, item_actual);
            }
        }
        cr_curr = cr_curr->next;
    }

    medicare_cover = total_amount - actual_pay;

    printf("------------------------------------------\n");
    printf("总计金额: %.2f 元\n", total_amount);
    printf("医保统筹: %.2f 元\n", medicare_cover);
    printf("个人自付: %.2f 元\n", actual_pay);
    printf("当前余额: %.2f 元\n", patient->balance);
    printf("==========================================\n");

    // 4. 扣费风控校验
    if (patient->balance < actual_pay)
    {
        printf("❌ 余额不足！还差 %.2f 元，请先到充值窗口充值。\n", actual_pay - patient->balance);
        return 0;
    }

    // 5. 执行扣款
    patient->balance -= actual_pay;
    patient->unpaid_time = 0;

    // 6. 标记检查记录为已缴费
    int has_paid_check = 0;
    cr_curr = g_check_record_list->next;
    while (cr_curr != NULL)
    {
        if (strcmp(cr_curr->patient_id, patient_id) == 0 && cr_curr->is_paid == 0)
        {
            cr_curr->is_paid = 1;
            has_paid_check = 1;
        }
        cr_curr = cr_curr->next;
    }

    // 7. 智能状态流转（遵循临床检查优先原则）
    if (has_paid_check > 0)
    {
        patient->status = STATUS_EXAMINING;
        printf("✅ 缴费成功！已扣除 %.2f 元，当前剩余余额 %.2f 元。\n", actual_pay, patient->balance);
        printf("🔄 [状态更新] 检查优先原则：当前状态流转为 检查中 (STATUS_EXAMINING) -> 📢 导诊：请先保持空腹，优先前往【检查科室】排队做检查！\n");
        if (patient->script_head != NULL)
        {
            printf("📢 提示：您名下还有待取药品，请在检查空档期或结束后前往药房取药。\n");
        }
    }
    else if (patient->script_head != NULL)
    {
        patient->status = STATUS_WAIT_MED;
        printf("✅ 缴费成功！已扣除 %.2f 元，当前剩余余额 %.2f 元。\n", actual_pay, patient->balance);
        printf("📢 请移步至【药房】排队取药。\n");
    }

    return 1;
}

// ==========================================
// 患者满意度评价模块
// ==========================================
int submit_patient_evaluation(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (g_consult_record_list == NULL)
    {
        printf("⚠️ 接诊记录链表尚未初始化！\n");
        return 0;
    }

    // 查找最后一条已完成的就诊记录
    ConsultRecordNode* last_completed_record = NULL;
    ConsultRecordNode* curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0 && curr->post_status == STATUS_COMPLETED)
        {
            last_completed_record = curr;
        }
        curr = curr->next;
    }

    if (last_completed_record == NULL)
    {
        printf("⚠️ 您当前没有已完成的就诊记录，无法进行评价！\n");
        return 0;
    }

    // 检查是否已评价
    if (last_completed_record->star_rating > 0)
    {
        printf("⚠️ 您已对本次就诊（记录编号：%s）进行过评价，不能重复评价！\n", last_completed_record->record_id);
        return 0;
    }

    // 展示医生信息和就诊时间
    DoctorNode* doctor = find_doctor_by_id(g_doctor_list, last_completed_record->doctor_id);
    printf("\n================ 满意度评价 ================\n");
    printf("就诊记录编号：%s\n", last_completed_record->record_id);
    printf("就诊医生：%s（编号：%s）\n", doctor ? doctor->name : "未知", last_completed_record->doctor_id);
    printf("就诊时间：%s\n", last_completed_record->consult_time[0] != '\0' ? last_completed_record->consult_time : "未知");
    printf("========================================\n");

    // 输入星级评价（1-5）
    int star_rating;
    do
    {
        star_rating = get_safe_int("请输入满意度星级 (1-5): ");
        if (star_rating < 1 || star_rating > 5)
        {
            printf("⚠️ 星级评价必须在 1-5 之间，请重新输入！\n");
        }
    } while (star_rating < 1 || star_rating > 5);

    // 输入文字评价
    char feedback[MAX_RECORD_LEN];
    get_safe_string("请输入文字评价（选填）: ", feedback, MAX_RECORD_LEN);

    // 保存评价数据
    last_completed_record->star_rating = star_rating;
    if (strlen(feedback) > 0)
    {
        strncpy(last_completed_record->feedback, feedback, MAX_RECORD_LEN - 1);
        last_completed_record->feedback[MAX_RECORD_LEN - 1] = '\0';
    }

    printf("\n✅ 评价成功！感谢您的反馈！\n");
    return 1;
}

// ==========================================
// 患者投诉管理模块
// ==========================================
int submit_new_complaint(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (g_account_list == NULL)
    {
        printf("⚠️ 账号链表尚未初始化！\n");
        return 0;
    }

    // 让患者选择投诉类型
    int target_type;
    do
    {
        printf("\n请选择投诉类型：\n");
        printf("  [1] 对医生\n");
        printf("  [2] 对护士/前台\n");
        printf("  [3] 对药师\n");
        target_type = get_safe_int("👉 请输入选择: ");
        if (target_type < 1 || target_type > 3)
        {
            printf("⚠️ 无效的选择，请重新输入！\n");
        }
    } while (target_type < 1 || target_type > 3);

    // 遍历全局账号链表，打印对应角色的员工
    printf("\n================ 可投诉人员列表 ================\n");
    AccountNode* curr = g_account_list->next;
    int count = 0;
    while (curr != NULL)
    {
        if ((target_type == 1 && curr->role == ROLE_DOCTOR) ||
            (target_type == 2 && curr->role == ROLE_NURSE) ||
            (target_type == 3 && curr->role == ROLE_PHARMACIST))
        {
            count++;
            printf("[%d] 账号：%s | 姓名：%s | 性别：%s\n", count, curr->username, curr->real_name, 
                   strlen(curr->gender) > 0 ? curr->gender : "未设置");
        }
        curr = curr->next;
    }

    if (count == 0)
    {
        printf("⚠️ 当前没有可投诉的相关人员！\n");
        return 0;
    }

    // 用户输入被投诉人账号
    char target_id[MAX_ID_LEN];
    int valid_target = 0;
    char target_name[MAX_NAME_LEN];
    
    do
    {
        get_safe_string("请输入被投诉人的账号: ", target_id, MAX_ID_LEN);
        
        // 校验被投诉人是否存在
        curr = g_account_list->next;
        while (curr != NULL)
        {
            if (strcmp(curr->username, target_id) == 0 &&
                ((target_type == 1 && curr->role == ROLE_DOCTOR) ||
                 (target_type == 2 && curr->role == ROLE_NURSE) ||
                 (target_type == 3 && curr->role == ROLE_PHARMACIST)))
            {
                valid_target = 1;
                strcpy(target_name, curr->real_name);
                break;
            }
            curr = curr->next;
        }
        
        if (!valid_target)
        {
            printf("⚠️ 输入的账号不存在或不属于所选投诉类型，请重新输入！\n");
        }
    } while (!valid_target);

    // 录入文字投诉内容
    char content[MAX_RECORD_LEN];
    get_safe_string("请输入投诉内容: ", content, MAX_RECORD_LEN);

    // 自动生成工单编号
    char complaint_id[MAX_ID_LEN];
    int complaint_count = 0;
    ComplaintNode* complaint_curr = g_complaint_list->next;
    while (complaint_curr != NULL)
    {
        complaint_count++;
        complaint_curr = complaint_curr->next;
    }
    snprintf(complaint_id, sizeof(complaint_id), "CP-%03d", complaint_count + 1);

    // 生成当前系统时间
    char submit_time[MAX_NAME_LEN];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(submit_time, MAX_NAME_LEN, "%Y-%m-%d %H:%M:%S", t);

    // 创建投诉工单节点并插入链表
    ComplaintNode* new_complaint = create_complaint_node(
        complaint_id,
        patient_id,
        target_type,
        target_id,
        target_name,
        content,
        0, // 初始状态为待处理
        NULL,
        submit_time
    );

    if (new_complaint == NULL)
    {
        printf("⚠️ 内存分配失败，投诉工单创建失败！\n");
        return 0;
    }

    insert_complaint_tail(g_complaint_list, new_complaint);

    printf("\n✅ 投诉工单提交成功！\n");
    printf("工单编号：%s\n", complaint_id);
    printf("提交时间：%s\n", submit_time);
    printf("我们会尽快处理您的投诉，感谢您的反馈！\n");

    return 1;
}

void query_patient_complaints(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return;
    }

    if (g_complaint_list == NULL)
    {
        printf("⚠️ 投诉工单链表尚未初始化！\n");
        return;
    }

    // 收集该患者的所有投诉记录
    ComplaintPtrNode* complaint_list = NULL;
    ComplaintPtrNode* tail = NULL;
    int complaint_count = 0;
    ComplaintNode* curr = g_complaint_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            // 创建新的投诉指针节点
            ComplaintPtrNode* new_node = (ComplaintPtrNode*)malloc(sizeof(ComplaintPtrNode));
            if (new_node == NULL)
            {
                continue;
            }
            new_node->complaint = curr;
            new_node->next = NULL;

            // 添加到链表
            if (complaint_list == NULL)
            {
                complaint_list = new_node;
                tail = new_node;
            }
            else
            {
                tail->next = new_node;
                tail = new_node;
            }
            complaint_count++;
        }
        curr = curr->next;
    }

    if (complaint_count == 0)
    {
        printf("\n⚠️ 您当前没有发起过任何投诉！\n");
        return;
    }

    // 逆序打印（最新在前）
    printf("\n================ 投诉记录查询 ================\n");
    
    // 计算链表长度并逆序打印
    ComplaintPtrNode* prev = NULL;
    ComplaintPtrNode* curr_ptr = complaint_list;
    ComplaintPtrNode* next = NULL;
    
    // 反转链表
    while (curr_ptr != NULL)
    {
        next = curr_ptr->next;
        curr_ptr->next = prev;
        prev = curr_ptr;
        curr_ptr = next;
    }
    complaint_list = prev;
    
    // 打印反转后的链表
    curr_ptr = complaint_list;
    int index = 1;
    while (curr_ptr != NULL)
    {
        ComplaintNode* complaint = curr_ptr->complaint;
        printf("\n【投诉工单 %d】\n", index++);
        printf("工单编号：%s\n", complaint->complaint_id);
        printf("提交时间：%s\n", complaint->submit_time);
        
        // 打印投诉类型
        printf("投诉类型：");
        switch (complaint->target_type)
        {
            case 1: printf("对医生\n"); break;
            case 2: printf("对护士/前台\n"); break;
            case 3: printf("对药师\n"); break;
            default: printf("未知\n"); break;
        }
        
        printf("被投诉人：%s（账号：%s）\n", complaint->target_name, complaint->target_id);
        printf("投诉内容：%s\n", complaint->content);
        
        // 打印处理状态
        printf("处理状态：");
        if (complaint->status == 0)
        {
            printf("处理中，请耐心等待\n");
        }
        else
        {
            printf("已回复\n");
            printf("处理意见：%s\n", complaint->response);
        }
        printf("----------------------------------------\n");
        curr_ptr = curr_ptr->next;
    }
    printf("========================================\n");
    
    // 释放链表
    curr_ptr = complaint_list;
    while (curr_ptr != NULL)
    {
        ComplaintPtrNode* temp = curr_ptr;
        curr_ptr = curr_ptr->next;
        free(temp);
    }
}



