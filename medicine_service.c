#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
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

static int is_blank_string(const char* str)
{
    int i;

    if (str == NULL)
    {
        return 1;
    }

    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isspace((unsigned char)str[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int contains_keyword(const char* source, const char* keyword)
{
    if (is_blank_string(source) || is_blank_string(keyword))
    {
        return 0;
    }

    return strstr(source, keyword) != NULL;
}

static int is_medicare_type_valid(MedicareType type)
{
    return type == MEDICARE_NONE ||
           type == MEDICARE_CLASS_A ||
           type == MEDICARE_CLASS_B;
}

static int is_leap_year(int year)
{
    return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

static int get_days_in_month(int year, int month)
{
    static const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month < 1 || month > 12)
    {
        return 0;
    }

    if (month == 2 && is_leap_year(year))
    {
        return 29;
    }

    return days_in_month[month - 1];
}

static int is_basic_date_format_valid(const char* date_str)
{
    int i;

    if (is_blank_string(date_str))
    {
        return 0;
    }

    if (strlen(date_str) != 10)
    {
        return 0;
    }

    for (i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
        {
            if (date_str[i] != '-')
            {
                return 0;
            }
        }
        else if (!isdigit((unsigned char)date_str[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int is_date_value_valid(const char* date_str)
{
    int year;
    int month;
    int day;
    int max_day;

    if (!is_basic_date_format_valid(date_str))
    {
        return 0;
    }

    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    if (year <= 0)
    {
        return 0;
    }

    if (month < 1 || month > 12)
    {
        return 0;
    }

    max_day = get_days_in_month(year, month);
    if (day < 1 || day > max_day)
    {
        return 0;
    }

    return 1;
}

static int is_valid_date_string(const char* date_str)
{
    return is_basic_date_format_valid(date_str) && is_date_value_valid(date_str);
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

static void safe_copy_string(char* dest, int max_len, const char* src)
{
    if (dest == NULL || max_len <= 0)
    {
        return;
    }

    if (src == NULL)
    {
        dest[0] = '\0';
        return;
    }

    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
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

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->id, keyword) == 0 || 
            contains_keyword(curr->name, keyword) || 
            contains_keyword(curr->generic_name, keyword) || 
            contains_keyword(curr->alias, keyword)) 
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
  * 本函数用于全量修改药品基础信息，不支持只改其中一部分字段。 
  * 调用时必须同时提供商品名、别名、通用名、单价和效期； 
  * 其中别名可以传入空字符串，表示清空别名。 
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
    char old_name[MAX_MED_NAME_LEN];
    char old_alias[MAX_ALIAS_LEN];
    char old_generic_name[MAX_GENERIC_NAME_LEN];
    char old_expiry_date[MAX_DATE_LEN];
    double old_price;

    if (g_medicine_list == NULL)
    {
        printf("提示：药品链表尚未初始化，无法修改基础信息。\n");
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
        printf("提示：未找到对应药品，基础信息修改失败。\n");
        return 0;
    }

    if (is_blank_string(new_name))
    {
        printf("提示：药品商品名不能为空。\n");
        return 0;
    }

    if (new_alias == NULL)
    {
        printf("提示：药品别名不能为空指针，如需清空别名请传空字符串。\n");
        return 0;
    }

    if (is_blank_string(new_generic_name))
    {
        printf("提示：药品通用名不能为空。\n");
        return 0;
    }

    if (new_price <= 0)
    {
        printf("提示：药品单价必须大于 0。\n");
        return 0;
    }

    if (is_blank_string(new_expiry_date))
    {
        printf("提示：药品效期不能为空。\n");
        return 0;
    }

    if (!is_valid_date_string(new_expiry_date))
    {
        printf("提示：药品效期格式非法，请使用 YYYY-MM-DD。\n");
        return 0;
    }

    safe_copy_string(old_name, MAX_MED_NAME_LEN, target->name);
    safe_copy_string(old_alias, MAX_ALIAS_LEN, target->alias);
    safe_copy_string(old_generic_name, MAX_GENERIC_NAME_LEN, target->generic_name);
    safe_copy_string(old_expiry_date, MAX_DATE_LEN, target->expiry_date);
    old_price = target->price;

    safe_copy_string(target->name, MAX_MED_NAME_LEN, new_name);
    safe_copy_string(target->alias, MAX_ALIAS_LEN, new_alias);
    safe_copy_string(target->generic_name, MAX_GENERIC_NAME_LEN, new_generic_name);
    target->price = new_price;
    safe_copy_string(target->expiry_date, MAX_DATE_LEN, new_expiry_date);

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