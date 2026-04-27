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
        printf("当前暂无药品数据\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                    药品信息列表\n");
    printf("======================================================\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        print_medicine_info(curr);
        curr = curr->next;
    }
}

void search_medicine_by_keyword(const char* keyword)
{
    int found = 0;
    MedicineNode* curr = NULL;

    if (is_blank_string(keyword))
    {
        printf("提示：查询关键字不能为空。\n");
        return;
    }

    if (g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        printf("未找到匹配药品\n");
        return;
    }

    printf("\n================ 药品查询结果 ================\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->id, keyword) == 0 || 
            contains_keyword(curr->name, keyword) || 
            contains_keyword(curr->generic_name, keyword) || 
            contains_keyword(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->id, keyword) ||
            contains_keyword_case_insensitive(curr->alias, keyword) ||
            contains_keyword_case_insensitive(curr->name, keyword) ||
            contains_keyword_case_insensitive(curr->generic_name, keyword) ||
            contains_keyword_by_order(curr->alias, keyword)) 
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
}

void show_low_stock_medicines(int threshold)
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

    printf("\n======================================================\n");
    printf("                    低库存药品列表\n");
    printf("======================================================\n");

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (curr->stock < threshold)
        {
            print_low_stock_info(curr);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found)
    {
        printf("当前无低库存药品\n");
    }
}

void show_expiring_medicines(const char* today, int days_threshold)
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

    printf("\n======================================================\n");
    printf("                    近效期药品列表\n");
    printf("======================================================\n");

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
            print_medicine_info(curr);
            found = 1;
        }

        curr = curr->next;
    }

    if (!found)
    {
        printf("当前无近效期药品\n");
    }
}

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

    target->stock = new_stock;

    printf("新库存：%d\n", target->stock);
    printf("提示：药品库存更新成功。\n");

    return 1;
}

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
// 下架药品（新增）
// -----------------------------------------------------------------------------

int remove_medicine(const char* med_id)
{
    MedicineNode* target = NULL;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法下架药品。\n");
        return 0;
    }

    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        return 0;
    }

    target = find_medicine_by_id(g_medicine_list, med_id);
    if (target == NULL)
    {
        printf("提示：未找到对应药品，下架失败。\n");
        return 0;
    }

    if (is_medicine_referenced_by_active_prescription(med_id))
    {
        printf("提示：当前药品仍被未完成处方引用，暂不能下架。\n");
        printf("请先完成相关患者的缴费/发药流程，或移除处方后再重试。\n");
        return 0;
    }

    if (delete_medicine_by_id(g_medicine_list, med_id))
    {
        printf("提示：药品下架成功。\n");
        return 1;
    }
    else
    {
        printf("提示：药品下架失败。\n");
        return 0;
    }
}
