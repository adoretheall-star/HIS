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
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "utils.h"
#include "global.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "admin_service.h"

static void generate_medicine_id(char* new_id)
{
    int max_no = 0;
    int current_no = 0;
    MedicineNode* curr = NULL;

    if (new_id == NULL)
    {
        return;
    }

    if (g_medicine_list != NULL)
    {
        curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (strncmp(curr->id, "M-", 2) == 0)
            {
                current_no = atoi(curr->id + 2);
                if (current_no > max_no)
                {
                    max_no = current_no;
                }
            }
            curr = curr->next;
        }
    }

    snprintf(new_id, MAX_ID_LEN, "M-%03d", max_no + 1);
}



static int contains_keyword(const char* source, const char* keyword)
{
    if (is_blank_string(source) || is_blank_string(keyword))
    {
        return 0;
    }

    return strstr(source, keyword) != NULL;
}

static int contains_keyword_case_insensitive(const char* source, const char* keyword)
{
    size_t source_len;
    size_t keyword_len;
    size_t i;
    size_t j;

    if (is_blank_string(source) || is_blank_string(keyword))
    {
        return 0;
    }

    source_len = strlen(source);
    keyword_len = strlen(keyword);

    if (keyword_len > source_len)
    {
        return 0;
    }

    for (i = 0; i <= source_len - keyword_len; i++)
    {
        for (j = 0; j < keyword_len; j++)
        {
            if (tolower((unsigned char)source[i + j]) != tolower((unsigned char)keyword[j]))
            {
                break;
            }
        }
        if (j == keyword_len)
        {
            return 1;
        }
    }

    return 0;
}

static int contains_keyword_by_order(const char* source, const char* keyword)
{
    const char* s;
    const char* k;

    if (is_blank_string(source) || is_blank_string(keyword))
    {
        return 0;
    }

    s = source;
    k = keyword;

    while (*k)
    {
        if (!*s)
        {
            return 0;
        }
        if (tolower((unsigned char)*s) == tolower((unsigned char)*k))
        {
            k++;
        }
        s++;
    }

    return 1;
}

static int is_medicare_type_valid(MedicareType type)
{
    return type == MEDICARE_NONE ||
           type == MEDICARE_CLASS_A ||
           type == MEDICARE_CLASS_B;
}



static int parse_date_string(const char* date_str, struct tm* out_tm)
{
    int year;
    int month;
    int day;

    if (out_tm == NULL || !is_valid_date_string(date_str))
    {
        return 0;
    }

    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    memset(out_tm, 0, sizeof(struct tm));
    out_tm->tm_year = year - 1900;
    out_tm->tm_mon = month - 1;
    out_tm->tm_mday = day;
    out_tm->tm_hour = 12;
    out_tm->tm_min = 0;
    out_tm->tm_sec = 0;
    out_tm->tm_isdst = -1;
    return 1;
}

static int days_between_dates(const char* start_date, const char* end_date)
{
    struct tm start_tm;
    struct tm end_tm;
    time_t start_time;
    time_t end_time;
    double seconds;

    if (!parse_date_string(start_date, &start_tm) ||
        !parse_date_string(end_date, &end_tm))
    {
        return 999999;
    }

    start_time = mktime(&start_tm);
    end_time = mktime(&end_tm);
    if (start_time == (time_t)-1 || end_time == (time_t)-1)
    {
        return 999999;
    }

    seconds = difftime(end_time, start_time);
    return (int)(seconds / (60 * 60 * 24));
}

static const char* get_medicare_type_text(MedicareType type)
{
    switch (type)
    {
        case MEDICARE_NONE:
            return "非医保";
        case MEDICARE_CLASS_A:
            return "甲类医保";
        case MEDICARE_CLASS_B:
            return "乙类医保";
        default:
            return "未知类型";
    }
}



/* 
  * 重复判定规则： 
  * 仅当商品名和通用名同时相同，才判定为重复药品。 
  * 别名不参与重复判定。 
  */ 
static int is_duplicate_medicine(const char* name, const char* generic_name)
{
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL)
    {
        return 0;
    }

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->name, name) == 0 && 
            strcmp(curr->generic_name, generic_name) == 0)
        {
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

// 检查药品名称是否已存在（用于提前检测重复）
int is_medicine_name_exists(const char* name)
{
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL || is_blank_string(name))
    {
        return 0;
    }

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->name, name) == 0)
        {
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

static int is_medicine_referenced_by_active_prescription(const char* med_id)
{
    PatientNode* patient_curr = NULL;

    if (is_blank_string(med_id) || g_patient_list == NULL)
    {
        return 0;
    }

    patient_curr = g_patient_list->next;
    while (patient_curr != NULL)
    {
        PrescriptionNode* script_curr = patient_curr->script_head;

        if (patient_curr->status == STATUS_COMPLETED || patient_curr->status == STATUS_NO_SHOW)
        {
            patient_curr = patient_curr->next;
            continue;
        }

        while (script_curr != NULL)
        {
            if (strcmp(script_curr->med_id, med_id) == 0)
            {
                return 1;
            }
            script_curr = script_curr->next;
        }

        patient_curr = patient_curr->next;
    }

    return 0;
}

static void print_medicine_info(const MedicineNode* med)
{
    if (med == NULL)
    {
        return;
    }

    printf("药品编号：%s\n", med->id);
    printf("商品名：%s\n", med->name);
    printf("通用名：%s\n", med->generic_name);
    printf("别名：%s\n", med->alias[0] == '\0' ? "无" : med->alias);
    printf("单价：%.2f\n", med->price);
    printf("库存：%d\n", med->stock);
    printf("医保类型：%s\n", get_medicare_type_text(med->m_type));
    printf("效期：%s\n", med->expiry_date);
    printf("------------------------------------------------------\n");
}

static void print_low_stock_info(const MedicineNode* med)
{
    if (med == NULL)
    {
        return;
    }

    printf("药品编号：%s\n", med->id);
    printf("商品名：%s\n", med->name);
    printf("通用名：%s\n", med->generic_name);
    printf("当前库存：%d\n", med->stock);
    printf("效期：%s\n", med->expiry_date);
    printf("------------------------------------------------------\n");
}

void show_all_medicines()
{
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        printf("\n======================================================\n");
        printf("                    药品信息列表\n");
        printf("======================================================\n");
        printf("当前暂无药品数据\n");
        return;
    }

    printf("\n=======================================================================================================================\n");
    printf("                    药品信息列表\n");
    printf("======================================================\n");
    // 列宽设置（按显示宽度）
    const int ID_WIDTH = 10;
    const int NAME_WIDTH = 20;
    const int GEN_NAME_WIDTH = 38;
    const int PRICE_WIDTH = 10;
    const int STOCK_WIDTH = 8;
    const int TYPE_WIDTH = 10;
    const int EXPIRY_WIDTH = 12;

    // 打印表头
    print_truncated_col("药品编号", ID_WIDTH);
    print_truncated_col("商品名", NAME_WIDTH);
    print_truncated_col("通用名", GEN_NAME_WIDTH);
    print_truncated_col("单价(元)", PRICE_WIDTH);
    print_truncated_col("库存", STOCK_WIDTH);
    print_truncated_col("医保类型", TYPE_WIDTH);
    print_truncated_col("效期", EXPIRY_WIDTH);
    printf("\n");

    // 打印分隔线
    int total_width = ID_WIDTH + NAME_WIDTH + GEN_NAME_WIDTH + PRICE_WIDTH + STOCK_WIDTH + TYPE_WIDTH + EXPIRY_WIDTH;
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        const char* m_type_name = NULL;
        switch (curr->m_type)
        {
            case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
            case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
            case MEDICARE_NONE:    m_type_name = "自费"; break;
            default:               m_type_name = "未知";
        }

        // 打印数据行
        print_truncated_col(curr->id, ID_WIDTH);
        print_truncated_col(curr->name, NAME_WIDTH);
        print_truncated_col(curr->generic_name, GEN_NAME_WIDTH);
        char price_str[32];
        snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
        print_truncated_col(price_str, PRICE_WIDTH);
        char stock_str[16];
        snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
        print_truncated_col(stock_str, STOCK_WIDTH);
        print_truncated_col(m_type_name, TYPE_WIDTH);
        print_truncated_col(curr->expiry_date, EXPIRY_WIDTH);
        printf("\n");
        curr = curr->next;
    }
}

// 检查是否有药品匹配关键词（返回是否找到结果，不打印任何内容）
int has_medicine_match(const char* keyword)
{
    int found = 0;
    MedicineNode* curr = NULL;

    if (is_blank_string(keyword) || g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        return 0;
    }

    // 准备关键词的大写版本用于拼音首字母匹配
    char keyword_upper[128];
    to_upper_string(keyword, keyword_upper, sizeof(keyword_upper));

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        // 获取拼音首字母
        char name_initials[128];
        char alias_initials[128];
        char generic_initials[256];
        get_pinyin_initials_utf8(curr->name, name_initials, sizeof(name_initials));
        get_pinyin_initials_utf8(curr->alias, alias_initials, sizeof(alias_initials));
        get_pinyin_initials_utf8(curr->generic_name, generic_initials, sizeof(generic_initials));

        if (strcmp(curr->id, keyword) == 0 || 
            contains_keyword(curr->name, keyword) || 
            contains_keyword(curr->generic_name, keyword) || 
            contains_keyword(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->id, keyword) ||
            contains_keyword_case_insensitive(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->name, keyword) ||
            contains_keyword_case_insensitive(curr->generic_name, keyword) ||
            contains_keyword_by_order(curr->alias, keyword) ||
            contains_ignore_case(name_initials, keyword_upper) ||
            contains_ignore_case(alias_initials, keyword_upper) ||
            contains_ignore_case(generic_initials, keyword_upper)) 
        {
            found = 1;
            break;
        }
        curr = curr->next;
    }

    return found;
}

int search_medicine_by_keyword(const char* keyword)
{
    int found = 0;
    MedicineNode* curr = NULL;

    if (is_blank_string(keyword))
    {
        printf("提示：查询关键字不能为空。\n");
        return 0;
    }

    if (g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        printf("未找到匹配药品\n");
        return 0;
    }

    printf("\n================ 药品查询结果 ================\n");

    // 准备关键词的大写版本用于拼音首字母匹配
    char keyword_upper[128];
    to_upper_string(keyword, keyword_upper, sizeof(keyword_upper));

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        // 获取拼音首字母
        char name_initials[128];
        char alias_initials[128];
        char generic_initials[256];
        get_pinyin_initials_utf8(curr->name, name_initials, sizeof(name_initials));
        get_pinyin_initials_utf8(curr->alias, alias_initials, sizeof(alias_initials));
        get_pinyin_initials_utf8(curr->generic_name, generic_initials, sizeof(generic_initials));

        if (strcmp(curr->id, keyword) == 0 || 
            contains_keyword(curr->name, keyword) || 
            contains_keyword(curr->generic_name, keyword) || 
            contains_keyword(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->id, keyword) ||
            contains_keyword_case_insensitive(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->name, keyword) ||
            contains_keyword_case_insensitive(curr->generic_name, keyword) ||
            contains_keyword_by_order(curr->alias, keyword) ||
            contains_ignore_case(name_initials, keyword_upper) ||
            contains_ignore_case(alias_initials, keyword_upper) ||
            contains_ignore_case(generic_initials, keyword_upper)) 
        {
            print_medicine_info(curr);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found)
    {
        printf("未找到匹配药品\n");
    }

    return found;
}

void show_low_stock_medicines_with_title(int threshold, int show_title)
{
    int found = 0;
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法执行低库存检查。\n");
        return;
    }

    if (threshold <= 0)
    {
        printf("提示：低库存阈值必须大于 0。\n");
        return;
    }

    if (show_title)
    {
        printf("\n======================================================\n");
        printf("                    低库存药品列表\n");
        printf("======================================================\n");
    }
    // 列宽设置（按显示宽度）
    const int ID_WIDTH = 10;
    const int NAME_WIDTH = 20;
    const int GEN_NAME_WIDTH = 38;
    const int PRICE_WIDTH = 10;
    const int STOCK_WIDTH = 8;
    const int TYPE_WIDTH = 10;
    const int EXPIRY_WIDTH = 12;

    // 打印表头
    print_truncated_col("药品编号", ID_WIDTH);
    print_truncated_col("商品名", NAME_WIDTH);
    print_truncated_col("通用名", GEN_NAME_WIDTH);
    print_truncated_col("单价(元)", PRICE_WIDTH);
    print_truncated_col("库存", STOCK_WIDTH);
    print_truncated_col("医保类型", TYPE_WIDTH);
    print_truncated_col("效期", EXPIRY_WIDTH);
    printf("\n");

    // 打印分隔线
    int total_width = ID_WIDTH + NAME_WIDTH + GEN_NAME_WIDTH + PRICE_WIDTH + STOCK_WIDTH + TYPE_WIDTH + EXPIRY_WIDTH;
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (curr->stock <= threshold)
        {
            const char* m_type_name = NULL;
            switch (curr->m_type)
            {
                case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
                case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
                case MEDICARE_NONE:    m_type_name = "自费"; break;
                default:               m_type_name = "未知";
            }
            // 打印数据行
            print_truncated_col(curr->id, ID_WIDTH);
            print_truncated_col(curr->name, NAME_WIDTH);
            print_truncated_col(curr->generic_name, GEN_NAME_WIDTH);
            char price_str[32];
            snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
            print_truncated_col(price_str, PRICE_WIDTH);
            char stock_str[16];
            snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
            print_truncated_col(stock_str, STOCK_WIDTH);
            print_truncated_col(m_type_name, TYPE_WIDTH);
            print_truncated_col(curr->expiry_date, EXPIRY_WIDTH);
            printf("\n");
            found = 1;
        }
        curr = curr->next;
    }

    if (!found)
    {
        printf("当前无低库存药品\n");
    }
}

void show_low_stock_medicines(int threshold)
{
    show_low_stock_medicines_with_title(threshold, 1);
}

void show_expiring_medicines_with_title(const char* today, int days_threshold, int show_title)
{
    int found = 0;
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法执行近效期检查。\n");
        return;
    }

    if (today == NULL || !is_valid_date_string(today))
    {
        printf("提示：当前日期格式非法，请使用 YYYY-MM-DD。\n");
        return;
    }

    if (days_threshold <= 0)
    {
        printf("提示：近效期天数阈值必须大于 0。\n");
        return;
    }

    if (show_title)
    {
        printf("\n======================================================\n");
        printf("                    近效期药品列表\n");
        printf("======================================================\n");
    }
    // 列宽设置（按显示宽度）
    const int ID_WIDTH = 10;
    const int NAME_WIDTH = 20;
    const int GEN_NAME_WIDTH = 38;
    const int PRICE_WIDTH = 10;
    const int STOCK_WIDTH = 8;
    const int TYPE_WIDTH = 10;
    const int EXPIRY_WIDTH = 12;
    const int DAYS_WIDTH = 12;

    // 打印表头
    print_truncated_col("药品编号", ID_WIDTH);
    print_truncated_col("商品名", NAME_WIDTH);
    print_truncated_col("通用名", GEN_NAME_WIDTH);
    print_truncated_col("单价(元)", PRICE_WIDTH);
    print_truncated_col("库存", STOCK_WIDTH);
    print_truncated_col("医保类型", TYPE_WIDTH);
    print_truncated_col("效期", EXPIRY_WIDTH);
    print_truncated_col("剩余天数", DAYS_WIDTH);
    printf("\n");

    // 打印分隔线
    int total_width = ID_WIDTH + NAME_WIDTH + GEN_NAME_WIDTH + PRICE_WIDTH + STOCK_WIDTH + TYPE_WIDTH + EXPIRY_WIDTH + DAYS_WIDTH;
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        int diff_days;

        if (!is_valid_date_string(curr->expiry_date))
        {
            curr = curr->next;
            continue;
        }

        diff_days = days_between_dates(today, curr->expiry_date);
        if (diff_days >= 0 && diff_days <= days_threshold)
        {
            const char* m_type_name = NULL;
            switch (curr->m_type)
            {
                case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
                case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
                case MEDICARE_NONE:    m_type_name = "自费"; break;
                default:               m_type_name = "未知";
            }
            // 打印数据行
            print_truncated_col(curr->id, ID_WIDTH);
            print_truncated_col(curr->name, NAME_WIDTH);
            print_truncated_col(curr->generic_name, GEN_NAME_WIDTH);
            char price_str[32];
            snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
            print_truncated_col(price_str, PRICE_WIDTH);
            char stock_str[16];
            snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
            print_truncated_col(stock_str, STOCK_WIDTH);
            print_truncated_col(m_type_name, TYPE_WIDTH);
            print_truncated_col(curr->expiry_date, EXPIRY_WIDTH);
            char days_str[16];
            snprintf(days_str, sizeof(days_str), "%d天", diff_days);
            print_truncated_col(days_str, DAYS_WIDTH);
            printf("\n");
            found = 1;
        }

        curr = curr->next;
    }

    if (!found)
    {
        printf("当前无近效期药品\n");
    }
}

void show_expiring_medicines(const char* today, int days_threshold)
{
    show_expiring_medicines_with_title(today, days_threshold, 1);
}

// 显示综合库存预警（近效期+低库存，分区显示）
void show_comprehensive_stock_alert(int low_stock_threshold, const char* today, int expiring_days_threshold)
{
    int low_stock_count = 0;
    int expiring_count = 0;
    MedicineNode* curr = NULL;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法执行综合预警检查。\n");
        return;
    }

    // ================ 近效期药品 ================
    printf("\n============= 近效期药品 ==============\n");

    // 列宽设置（按显示宽度）
    const int ID_WIDTH = 10;
    const int NAME_WIDTH = 20;
    const int GEN_NAME_WIDTH = 38;
    const int PRICE_WIDTH = 10;
    const int STOCK_WIDTH = 8;
    const int TYPE_WIDTH = 10;
    const int EXPIRY_WIDTH = 12;
    const int DAYS_WIDTH = 12;

    // 打印表头
    print_truncated_col("药品编号", ID_WIDTH);
    print_truncated_col("商品名", NAME_WIDTH);
    print_truncated_col("通用名", GEN_NAME_WIDTH);
    print_truncated_col("单价(元)", PRICE_WIDTH);
    print_truncated_col("库存", STOCK_WIDTH);
    print_truncated_col("医保类型", TYPE_WIDTH);
    print_truncated_col("效期", EXPIRY_WIDTH);
    print_truncated_col("剩余天数", DAYS_WIDTH);
    printf("\n");

    // 打印分隔线
    int total_width = ID_WIDTH + NAME_WIDTH + GEN_NAME_WIDTH + PRICE_WIDTH + STOCK_WIDTH + TYPE_WIDTH + EXPIRY_WIDTH + DAYS_WIDTH;
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 查找并打印近效期药品
    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        int diff_days;
        if (is_valid_date_string(curr->expiry_date))
        {
            diff_days = days_between_dates(today, curr->expiry_date);
            if (diff_days >= 0 && diff_days <= expiring_days_threshold)
            {
                const char* m_type_name = NULL;
                switch (curr->m_type)
                {
                    case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
                    case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
                    case MEDICARE_NONE:    m_type_name = "自费"; break;
                    default:               m_type_name = "未知";
                }
                // 打印数据行
                print_truncated_col(curr->id, ID_WIDTH);
                print_truncated_col(curr->name, NAME_WIDTH);
                print_truncated_col(curr->generic_name, GEN_NAME_WIDTH);
                char price_str[32];
                snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                print_truncated_col(price_str, PRICE_WIDTH);
                char stock_str[16];
                snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                print_truncated_col(stock_str, STOCK_WIDTH);
                print_truncated_col(m_type_name, TYPE_WIDTH);
                print_truncated_col(curr->expiry_date, EXPIRY_WIDTH);
                char days_str[16];
                snprintf(days_str, sizeof(days_str), "%d天", diff_days);
                print_truncated_col(days_str, DAYS_WIDTH);
                printf("\n");
                expiring_count++;
            }
        }
        curr = curr->next;
    }

    if (expiring_count == 0)
    {
        printf("当前无近效期药品\n");
    }
    else
    {
        printf("\n共找到 %d 个近效期药品\n", expiring_count);
    }

    // ================ 低库存药品 ================
    printf("\n============= 低库存药品 ==============\n");

    // 打印表头
    print_truncated_col("药品编号", ID_WIDTH);
    print_truncated_col("商品名", NAME_WIDTH);
    print_truncated_col("通用名", GEN_NAME_WIDTH);
    print_truncated_col("单价(元)", PRICE_WIDTH);
    print_truncated_col("库存", STOCK_WIDTH);
    print_truncated_col("医保类型", TYPE_WIDTH);
    print_truncated_col("效期", EXPIRY_WIDTH);
    printf("\n");

    // 打印分隔线
    total_width = ID_WIDTH + NAME_WIDTH + GEN_NAME_WIDTH + PRICE_WIDTH + STOCK_WIDTH + TYPE_WIDTH + EXPIRY_WIDTH;
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 查找并打印低库存药品
    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (curr->stock <= low_stock_threshold)
        {
            const char* m_type_name = NULL;
            switch (curr->m_type)
            {
                case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
                case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
                case MEDICARE_NONE:    m_type_name = "自费"; break;
                default:               m_type_name = "未知";
            }
            // 打印数据行
            print_truncated_col(curr->id, ID_WIDTH);
            print_truncated_col(curr->name, NAME_WIDTH);
            print_truncated_col(curr->generic_name, GEN_NAME_WIDTH);
            char price_str[32];
            snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
            print_truncated_col(price_str, PRICE_WIDTH);
            char stock_str[16];
            snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
            print_truncated_col(stock_str, STOCK_WIDTH);
            print_truncated_col(m_type_name, TYPE_WIDTH);
            print_truncated_col(curr->expiry_date, EXPIRY_WIDTH);
            printf("\n");
            low_stock_count++;
        }
        curr = curr->next;
    }

    if (low_stock_count == 0)
    {
        printf("当前无低库存药品\n");
    }
    else
    {
        printf("\n共找到 %d 个低库存药品\n", low_stock_count);
    }
}

/* 
 * 【功能作者】陈苗苗 55251313 
 * 【功能说明】药品资源维护 - 药品新增，支持商品名、通用名、价格、库存、医保类型和效期管理。
 */
MedicineNode* register_medicine(
    const char* name,
    const char* alias,
    const char* generic_name,
    double price,
    int stock,
    MedicareType m_type,
    const char* expiry_date
)
{
    char new_id[MAX_ID_LEN];
    const char* safe_alias = NULL;
    MedicineNode* new_node = NULL;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法新增药品。\n");
        return NULL;
    }

    if (is_blank_string(name))
    {
        printf("提示：药品商品名不能为空。\n");
        return NULL;
    }

    if (is_blank_string(generic_name))
    {
        printf("提示：药品通用名不能为空。\n");
        return NULL;
    }

    if (price <= 0)
    {
        printf("提示：药品单价必须大于 0。\n");
        return NULL;
    }

    if (stock < 0)
    {
        printf("提示：药品库存不能小于 0。\n");
        return NULL;
    }

    if (!is_medicare_type_valid(m_type))
    {
        printf("提示：医保类型非法，无法新增药品。\n");
        return NULL;
    }

    if (!is_valid_date_string(expiry_date))
    {
        printf("提示：药品效期非法，请使用有效日期 YYYY-MM-DD。\n");
        return NULL;
    }

    if (!is_future_date(expiry_date))
    {
        printf("提示：药品效期必须晚于今天，无法新增药品。\n");
        return NULL;
    }

    if (is_duplicate_medicine(name, generic_name))
    {
        printf("提示：疑似重复药品，新增失败。\n");
        return NULL;
    }

    safe_alias = is_blank_string(alias) ? "" : alias;

    generate_medicine_id(new_id);
    new_node = create_medicine_node(
        new_id,
        name,
        safe_alias,
        generic_name,
        price,
        stock,
        m_type,
        expiry_date
    );

    if (new_node == NULL)
    {
        printf("提示：药品节点创建失败。\n");
        return NULL;
    }

    insert_medicine_tail(g_medicine_list, new_node);

    printf("药品新增成功！\n");
    printf("药品编号：%s\n", new_node->id);
    printf("商品名：%s\n", new_node->name);
    printf("通用名：%s\n", new_node->generic_name);
    printf("别名：%s\n", new_node->alias[0] == '\0' ? "无" : new_node->alias);

    return new_node;
}

/* 
 * 【功能作者】陈苗苗 55251313 
 * 【功能说明】药品资源维护 - 库存维护，支持药品库存数量的更新。
 */
int update_medicine_stock(const char* med_id, int new_stock)
{
    MedicineNode* target = NULL;
    int old_stock;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法修改库存。\n");
        return 0;
    }

    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        return 0;
    }

    if (new_stock < 0)
    {
        printf("提示：库存数量不能小于 0。\n");
        return 0;
    }

    target = find_medicine_by_id(g_medicine_list, med_id);
    if (target == NULL)
    {
        printf("提示：未找到对应药品，库存修改失败。\n");
        return 0;
    }

    old_stock = target->stock;

    printf("药品编号：%s\n", target->id);
    printf("商品名：%s\n", target->name);
    printf("旧库存：%d\n", old_stock);

    if (new_stock == old_stock)
    {
        printf("新库存：%d\n", new_stock);
        printf("提示：库存数量未发生变化，未做任何修改。\n");
        return 1;
    }

    target->stock = new_stock;

    printf("新库存：%d\n", target->stock);
    printf("提示：药品库存更新成功。\n");

    return 1;
}

/* 
 * 【功能作者】陈苗苗 55251313 
 * 【功能说明】药品资源维护 - 药品信息修改，支持商品名、别名、通用名、价格、效期的部分或全部修改。
 */
/* 
  * 本函数用于部分修改药品基础信息。 
  * 各字段修改规则如下： 
  *   - new_name (商品名): 空白字符串表示不修改，非空白字符串则更新。 
  *   - new_alias (别名): NULL 表示不修改，空字符串 "" 表示清空别名，普通字符串则更新。 
  *   - new_generic_name (通用名): 空白字符串表示不修改，非空白字符串则更新。 
  *   - new_price (单价): 小于等于 0 表示不修改，大于 0 则更新。 
  *   - new_expiry_date (效期): 空白字符串表示不修改，非空白字符串必须通过 is_valid_date_string() 校验，合法才更新。 
  * 至少有一项真正发生修改才返回成功，否则提示“未修改任何信息”。 
  */ 
int update_medicine_basic_info(
    const char* med_id,
    const char* new_name,
    const char* new_alias,
    const char* new_generic_name,
    double new_price,
    const char* new_expiry_date
)
{
    MedicineNode* target = NULL;
    MedicineNode* curr_check = NULL;
    int modified = 0;

    // --- 1. 预先计算“最终准备写入的新值” ---
    char final_name[MAX_MED_NAME_LEN];
    char final_alias[MAX_ALIAS_LEN];
    char final_generic_name[MAX_GENERIC_NAME_LEN];
    char final_expiry_date[MAX_DATE_LEN];
    double final_price;

    // 用于保存旧值以便打印对比
    char old_name[MAX_MED_NAME_LEN];
    char old_alias[MAX_ALIAS_LEN];
    char old_generic_name[MAX_GENERIC_NAME_LEN];
    char old_expiry_date[MAX_DATE_LEN];
    double old_price;

    // --- 2. 执行所有校验（阶段一：基本条件校验） ---
    // 2.1. g_medicine_list 未初始化 -> 返回 0 并提示
    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法修改基础信息。\n");
        return 0;
    }

    // 2.2. med_id 为空白字符串 -> 返回 0 并提示
    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        return 0;
    }

    // 2.3. 用 find_medicine_by_id 查找药品，找不到则返回 0 并提示
    target = find_medicine_by_id(g_medicine_list, med_id);
    if (target == NULL)
    {
        printf("提示：未找到对应药品，基础信息修改失败。\n");
        return 0;
    }

    // 保存修改前的原始值 (用于后续对比和回滚，虽然这里是先校验后写入，但依然需要旧值进行比较)
    safe_copy_string(old_name, MAX_MED_NAME_LEN, target->name);
    safe_copy_string(old_alias, MAX_ALIAS_LEN, target->alias);
    safe_copy_string(old_generic_name, MAX_GENERIC_NAME_LEN, target->generic_name);
    safe_copy_string(old_expiry_date, MAX_DATE_LEN, target->expiry_date);
    old_price = target->price;

    // 根据输入参数和旧值，计算“最终准备写入的新值”
    // 商品名：new_name 非空白时用新值，否则沿用旧值
    if (new_name != NULL && !is_blank_string(new_name))
    {
        safe_copy_string(final_name, MAX_MED_NAME_LEN, new_name);
    }
    else
    {
        safe_copy_string(final_name, MAX_MED_NAME_LEN, target->name);
    }

    // 通用名：new_generic_name 非空白时用新值，否则沿用旧值
    if (new_generic_name != NULL && !is_blank_string(new_generic_name))
    {
        safe_copy_string(final_generic_name, MAX_GENERIC_NAME_LEN, new_generic_name);
    }
    else
    {
        safe_copy_string(final_generic_name, MAX_GENERIC_NAME_LEN, target->generic_name);
    }

    // 别名：new_alias == NULL 表示沿用旧值；new_alias == "" 表示清空；其他字符串表示更新
    if (new_alias == NULL) // NULL 表示不修改，沿用旧值
    {
        safe_copy_string(final_alias, MAX_ALIAS_LEN, target->alias);
    }
    else // 非 NULL，表示要修改别名
    {
        safe_copy_string(final_alias, MAX_ALIAS_LEN, new_alias);
    }

    // 单价：new_price > 0 时用新值，否则沿用旧值
    if (new_price > 0)
    {
        final_price = new_price;
    }
    else
    {
        final_price = target->price;
    }

    // 效期：new_expiry_date 非空白时用新值，否则沿用旧值
    if (new_expiry_date != NULL && !is_blank_string(new_expiry_date))
    {
        safe_copy_string(final_expiry_date, MAX_DATE_LEN, new_expiry_date);
    }
    else
    {
        safe_copy_string(final_expiry_date, MAX_DATE_LEN, target->expiry_date);
    }

    // --- 3. 执行所有校验（阶段二：业务逻辑校验） ---

    // 3.1. 如果 new_expiry_date 非空白，则必须通过 is_valid_date_string()
    if (!is_blank_string(new_expiry_date) && !is_valid_date_string(final_expiry_date))
    {
        printf("提示：药品效期格式非法，请使用 YYYY-MM-DD。\n");
        return 0; // 效期非法，修改失败
    }

    // 3.2. 如果 new_expiry_date 非空白，效期必须晚于当前日期
    if (!is_blank_string(new_expiry_date) && !is_future_date(final_expiry_date))
    {
        printf("提示：药品效期必须晚于当前日期，修改失败。\n");
        return 0; // 效期不是未来日期，修改失败
    }

    // 3.3. 修改后的 商品名 + 通用名 组合不能与其他药品重复（排除当前药品自己）
    curr_check = g_medicine_list->next;
    while (curr_check != NULL)
    {
        // 跳过当前正在修改的药品
        if (strcmp(curr_check->id, med_id) == 0)
        {
            curr_check = curr_check->next;
            continue;
        }

        // 检查商品名和通用名组合是否重复
        if (strcmp(curr_check->name, final_name) == 0 &&
            strcmp(curr_check->generic_name, final_generic_name) == 0)
        {
            printf("提示：商品名和通用名组合重复，修改失败。\n");
            return 0; // 发现重复，修改失败
        }
        curr_check = curr_check->next;
    }

    // --- 4. 检查是否有实际修改，并统一写回 target 各字段 ---
    modified = 0; // 标志位，表示是否有字段被修改

    // 比较 final_name 和 target->name
    if (strcmp(target->name, final_name) != 0)
    {
        safe_copy_string(target->name, MAX_MED_NAME_LEN, final_name);
        modified = 1;
    }

    // 比较 final_alias 和 target->alias
    if (strcmp(target->alias, final_alias) != 0)
    {
        safe_copy_string(target->alias, MAX_ALIAS_LEN, final_alias);
        modified = 1;
    }

    // 比较 final_generic_name 和 target->generic_name
    if (strcmp(target->generic_name, final_generic_name) != 0)
    {
        safe_copy_string(target->generic_name, MAX_GENERIC_NAME_LEN, final_generic_name);
        modified = 1;
    }

    // 比较 final_price 和 target->price
    if (target->price != final_price)
    {
        target->price = final_price;
        modified = 1;
    }

    // 比较 final_expiry_date 和 target->expiry_date
    if (strcmp(target->expiry_date, final_expiry_date) != 0)
    {
        safe_copy_string(target->expiry_date, MAX_DATE_LEN, final_expiry_date);
        modified = 1;
    }

    // 如果没有任何字段被修改，提示“未修改任何信息”，返回 0
    if (!modified)
    {
        printf("提示：未修改任何信息。\n");
        return 0;
    }

    // --- 5. 更新成功后打印修改前后的对比 ---
    printf("药品基础信息更新成功：\n");
    printf("药品编号：%s\n", target->id);
    printf("商品名：%s -> %s\n", old_name, target->name);
    printf("通用名：%s -> %s\n", old_generic_name, target->generic_name);
    printf("别名：%s -> %s\n",
        old_alias[0] == '\0' ? "无" : old_alias,
        target->alias[0] == '\0' ? "无" : target->alias);
    printf("单价：%.2f -> %.2f\n", old_price, target->price);
    printf("效期：%s -> %s\n", old_expiry_date, target->expiry_date);

    return 1;
}

// -----------------------------------------------------------------------------
// 软删除药品到回收站
// -----------------------------------------------------------------------------
int soft_remove_medicine_to_recycle(
    const char* med_id,
    const char* deleted_by,
    const char* reason
)
{
    MedicineNode* target = NULL;
    RecycleNode* recycle_node = NULL;
    char recycle_id[MAX_ID_LEN];

    // 校验链表是否初始化
    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化。\n");
        return 0;
    }

    if (g_recycle_list == NULL)
    {
        printf("提示：回收站链表尚未初始化。\n");
        return 0;
    }

    // 校验药品编号
    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        return 0;
    }

    // 查找药品
    target = find_medicine_by_id(g_medicine_list, med_id);
    if (target == NULL)
    {
        printf("提示：未找到对应药品，无法下架。\n");
        return 0;
    }

    // 校验是否被未完成处方引用
    if (is_medicine_referenced_by_active_prescription(med_id))
    {
        printf("提示：当前药品仍被未完成处方引用，暂不能下架。\n");
        printf("请先完成相关患者的缴费/发药流程，或移除处方后再重试。\n");
        return 0;
    }

    // 显示药品信息确认
    printf("\n即将下架的药品信息：\n");
    printf("药品编号：%s\n", target->id);
    printf("商品名：%s\n", target->name);
    printf("别名：%s\n", target->alias);
    printf("通用名：%s\n", target->generic_name);
    printf("单价：%.2f\n", target->price);
    printf("库存：%d\n", target->stock);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("医保类型：甲类\n"); break;
        case MEDICARE_CLASS_B: printf("医保类型：乙类\n"); break;
        case MEDICARE_NONE:    printf("医保类型：自费\n"); break;
        default:               printf("医保类型：未知\n"); break;
    }
    printf("有效期：%s\n", target->expiry_date);
    printf("\n是否确认下架该药品并转入回收站？Y=确认，N=取消\n");
    
    char confirm[10];
    get_safe_string("请输入选择：", confirm, sizeof(confirm));
    
    // 确认检查
    if (confirm[0] != 'Y' && confirm[0] != 'y')
    {
        printf("提示：已取消下架操作。\n");
        return 0;
    }

    // 生成回收站编号
    generate_recycle_id(recycle_id);

    // 创建回收站节点
    recycle_node = create_recycle_medicine_node(recycle_id, target, deleted_by, reason);
    if (recycle_node == NULL)
    {
        printf("提示：创建回收站记录失败。\n");
        return 0;
    }

    // 插入回收站
    insert_recycle_tail(g_recycle_list, recycle_node);

    // 从药品链表中断链
    if (target->prev != NULL)
    {
        target->prev->next = target->next;
    }
    if (target->next != NULL)
    {
        target->next->prev = target->prev;
    }

    // 释放原药品节点
    free(target);

    // 添加操作日志
    add_log("软删除药品", med_id, "药品已下架并转入回收站");

    // 打印提示信息
    printf("提示：药品已下架并转入回收站，可由管理员在回收站中恢复。\n");

    return 1;
}

// -----------------------------------------------------------------------------
// 下架药品（新增）
// -----------------------------------------------------------------------------

int remove_medicine(const char* med_id)
{
    return soft_remove_medicine_to_recycle(med_id, "admin", "管理员下架药品");
}
