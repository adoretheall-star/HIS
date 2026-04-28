#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> // 鐢ㄤ簬 isnan, isinf 鍑芥暟
#include "admin_service.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "patient_service.h"
#include "utils.h"

// 鐥呮埧璐圭巼瀹氫箟
#define GENERAL_WARD_RATE 120.0  // 鏅€氱梾鎴挎瘡澶╄垂鐢?
#define ICU_WARD_RATE 500.0      // ICU姣忓ぉ璐圭敤
#define ISOLATION_WARD_RATE 300.0  // 闅旂鐥呮埧姣忓ぉ璐圭敤
#define SINGLE_WARD_RATE 400.0    // 鍗曚汉鐥呮埧姣忓ぉ璐圭敤

// 褰撳墠鐧诲綍鐢ㄦ埛
AccountNode* g_current_account = NULL;
DoctorNode* g_current_doctor = NULL;
PatientNode* g_current_patient = NULL;

// 澶栭儴鍑芥暟澹版槑
extern void query_patient_complaints(const char* patient_id);

// 澶у皬鍐欎笉鏁忔劅鐨勫瓧绗︿覆姣旇緝鍑芥暟
int my_strcasecmp(const char* s1, const char* s2)
{
    if (s1 == NULL || s2 == NULL)
    {
        return s1 - s2;
    }
    
    while (*s1 && *s2)
    {
        char c1 = *s1;
        char c2 = *s2;
        
        // 杞崲涓哄皬鍐欒繘琛屾瘮杈?
        if (c1 >= 'A' && c1 <= 'Z')
        {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z')
        {
            c2 += 'a' - 'A';
        }
        
        if (c1 != c2)
        {
            return c1 - c2;
        }
        
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

// 鑾峰彇鐥呮埧绫诲瀷鐨勮垂鐢?
static double get_ward_rate(WardType ward_type)
{
    switch (ward_type) {
        case WARD_TYPE_ICU:
            return ICU_WARD_RATE;
        case WARD_TYPE_ISOLATION:
            return ISOLATION_WARD_RATE;
        case WARD_TYPE_SINGLE:
            return SINGLE_WARD_RATE;
        case WARD_TYPE_GENERAL:
        default:
            return GENERAL_WARD_RATE;
    }
}

// 鍓嶇疆澹版槑
static void show_expiring_medicine_warning(void);
static int days_between_dates(const char* start_date, const char* end_date);

// 妫€鏌ヨ嵂鍝佹槸鍚﹁繎鏁堟湡
static int is_medicine_expiring_soon(const char* expiry_date)
{
    if (expiry_date == NULL)
    {
        return 0;
    }
    
    // 绠€鍗曠殑杩戞晥鏈熸鏌ワ細鍋囪褰撳墠鏃ユ湡涓?2024-01-01锛屾鏌ユ槸鍚﹀湪 30 澶╁唴杩囨湡
    // 瀹為檯椤圭洰涓簲璇ヤ娇鐢ㄥ綋鍓嶇郴缁熸棩鏈熻繘琛屾瘮杈?
    const char* current_date = "2024-01-01";
    int days_diff = days_between_dates(current_date, expiry_date);
    
    return (days_diff >= 0 && days_diff <= 30);
}

// 杩戞晥鏈熻嵂鍝侀璀?
static void show_expiring_medicine_warning(void)
{
    int count = 0;
    int total_width = 80;
    
    // 妫€鏌ヨ繎鏁堟湡鑽搧
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (is_medicine_expiring_soon(curr->expiry_date))
            {
                count++;
            }
            curr = curr->next;
        }
    }
    
    // 杈撳嚭缁熻淇℃伅
    printf("\n======================================================\n");
    printf("                  杩戞晥鏈熻嵂鍝侀璀n");
    printf("======================================================\n");
    
    if (count > 0)
    {
        printf("鈿狅笍 鍙戠幇 %d 绉嶈繎鏁堟湡鑽搧锛岃鍙婃椂澶勭悊锛乗n", count);
    }
    else
    {
        printf("鉁?娌℃湁鍙戠幇杩戞晥鏈熻嵂鍝併€俓n");
    }
    
    printf("======================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 瑙ｆ瀽鏃ユ湡瀛楃涓蹭负 tm 缁撴瀯
static int parse_date_string(const char* date_str, struct tm* tm_out);

// 璁＄畻涓や釜鏃ユ湡涔嬮棿鐨勫ぉ鏁板樊
static int days_between_dates(const char* start_date, const char* end_date);

// 璁＄畻瀛楃涓茬殑鏄剧ず瀹藉害锛堜腑鏂囩畻2涓搴︼級
static int get_display_width(const char* str)
{
    if (str == NULL) return 0;
    
    int width = 0;
    const unsigned char* p = (const unsigned char*)str;
    
    while (*p != '\0')
    {
        if (*p < 128)
        {
            // ASCII 瀛楃锛屽搴?1
            width++;
        }
        else
        {
            // 闈?ASCII 瀛楃锛堝涓枃锛夛紝瀹藉害 2
            width += 2;
            // 璺宠繃鍚庣画瀛楄妭锛圲TF-8 缂栫爜锛?
            if (*p >= 0xE0) p += 2; // 3瀛楄妭UTF-8
            else p += 1; // 2瀛楄妭UTF-8
        }
        p++;
    }
    
    return width;
}

// 鎸夋寚瀹氬搴︽墦鍗版枃鏈苟琛ョ┖鏍?
static void print_padded_text(const char* str, int target_width)
{
    if (str == NULL) str = "";

    int current_width = get_display_width(str);
    printf("%s", str);

    // 璁＄畻闇€瑕佽ˉ鐨勭┖鏍兼暟
    int spaces = target_width - current_width;
    if (spaces > 0)
    {
        for (int i = 0; i < spaces; i++)
        {
            printf(" ");
        }
    }
}

// 鏍规嵁鐥呮埧绫诲瀷浼扮畻鏃ヨ垂鐢?
static double estimate_daily_cost_by_ward_type(WardType ward_type)
{
    switch (ward_type)
    {
        case WARD_TYPE_GENERAL:
            return 200.0;
        case WARD_TYPE_ICU:
            return 1500.0;
        default:
            return 200.0;
    }
}

// 鏌ョ湅鎵€鏈夊憳宸ヨ处鍙?
void show_all_accounts(void)
{
    AccountNode* curr = NULL;
    int count = 0;
    
    // 绗竴姝ワ細缁熻姣忎竴鍒楃殑鏈€澶ф樉绀哄搴?
    int username_width = get_display_width("鐧诲綍璐﹀彿");
    int real_name_width = get_display_width("鐪熷疄濮撳悕");
    int role_width = get_display_width("瑙掕壊");
    
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        curr = g_account_list->next;
        while (curr != NULL)
        {
            const char* role_name = NULL;
            switch (curr->role)
            {
                case ROLE_ADMIN:
                    role_name = "绠＄悊鍛?;
                    break;
                case ROLE_NURSE:
                    role_name = "鎶ゅ＋";
                    break;
                case ROLE_DOCTOR:
                    role_name = "鍖荤敓";
                    break;
                case ROLE_PHARMACIST:
                    role_name = "鑽笀";
                    break;
                default:
                    role_name = "鏈煡";
                    break;
            }
            
            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_role_width = get_display_width(role_name);
            
            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_role_width > role_width)
                role_width = current_role_width;
            
            curr = curr->next;
        }
    }
    
    // 绗簩姝ワ細鎸夊姩鎬佸垪瀹借緭鍑鸿〃鏍?
    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤鍛樺伐璐﹀彿鏁版嵁\n");
        return;
    }
    
    // 璁＄畻鎬诲搴﹀苟杈撳嚭杈规
    int total_width = username_width + real_name_width + role_width + 8; // 8鏄垪闂磋窛鎬诲拰
    
    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 杈撳嚭鏍囬
    int title_width = get_display_width("鍛樺伐鍒楄〃");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("鍛樺伐鍒楄〃");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 杈撳嚭琛ㄥご
    print_padded_text("鐧诲綍璐﹀彿", username_width);
    printf("    "); // 4涓┖鏍肩殑鍒楅棿璺?
    print_padded_text("鐪熷疄濮撳悕", real_name_width);
    printf("    "); // 4涓┖鏍肩殑鍒楅棿璺?
    print_padded_text("瑙掕壊", role_width);
    printf("\n");
    
    // 杈撳嚭琛ㄥご涓嬫í绾?
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 杈撳嚭鏁版嵁琛?
    curr = g_account_list->next;
    while (curr != NULL)
    {
        const char* role_name = NULL;
        switch (curr->role)
        {
            case ROLE_ADMIN:
                role_name = "绠＄悊鍛?;
                break;
            case ROLE_NURSE:
                role_name = "鎶ゅ＋";
                break;
            case ROLE_DOCTOR:
                role_name = "鍖荤敓";
                break;
            case ROLE_PHARMACIST:
                role_name = "鑽笀";
                break;
            default:
                role_name = "鏈煡";
                break;
        }
        
        print_padded_text(curr->username, username_width);
        printf("    "); // 4涓┖鏍肩殑鍒楅棿璺?
        print_padded_text(curr->real_name, real_name_width);
        printf("    "); // 4涓┖鏍肩殑鍒楅棿璺?
        print_padded_text(role_name, role_width);
        printf("\n");
        
        count++;
        curr = curr->next;
    }
    
    // 杈撳嚭搴曢儴妯嚎
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 杈撳嚭鍛樺伐鎬绘暟
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "鍛樺伐鎬绘暟锛?d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 娉ㄥ唽鏂板憳宸ヨ处鍙?
int register_account(const char* username, const char* password, const char* real_name, const char* gender, RoleType role)
{
    if (username == NULL || password == NULL || real_name == NULL)
    {
        return 0;
    }

    // 妫€鏌ヨ处鍙锋槸鍚﹀凡瀛樺湪
    if (find_account_by_username(g_account_list, username) != NULL)
    {
        return 0;
    }

    // 鍒涘缓鏂拌处鍙疯妭鐐?
    AccountNode* new_node = (AccountNode*)malloc(sizeof(AccountNode));
    if (new_node == NULL)
    {
        return 0;
    }

    strncpy(new_node->username, username, sizeof(new_node->username) - 1);
    new_node->username[sizeof(new_node->username) - 1] = '\0';
    strncpy(new_node->password, password, sizeof(new_node->password) - 1);
    new_node->password[sizeof(new_node->password) - 1] = '\0';
    strncpy(new_node->real_name, real_name, sizeof(new_node->real_name) - 1);
    new_node->real_name[sizeof(new_node->real_name) - 1] = '\0';
    new_node->role = role;
    new_node->next = NULL;

    // 娣诲姞鍒伴摼琛?
    if (g_account_list == NULL)
    {
        g_account_list = (AccountNode*)malloc(sizeof(AccountNode));
        if (g_account_list == NULL)
        {
            free(new_node);
            return 0;
        }
        g_account_list->next = new_node;
    }
    else
    {
        AccountNode* curr = g_account_list;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = new_node;
    }

    return 1;
}

// 楠岃瘉璐﹀彿
int verify_account(const char* username, const char* password, int* role)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        return 0;
    }

    if (strcmp(account->password, password) != 0)
    {
        return 0;
    }

    if (role != NULL)
    {
        *role = account->role;
    }

    return 1;
}

// 淇敼鍛樺伐璧勬枡
int update_account_basic_info(const char* username, const char* new_real_name, const char* new_password, RoleType new_role)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        return 0;
    }

    if (new_real_name != NULL)
    {
        strncpy(account->real_name, new_real_name, sizeof(account->real_name) - 1);
        account->real_name[sizeof(account->real_name) - 1] = '\0';
    }

    if (new_password != NULL)
    {
        strncpy(account->password, new_password, sizeof(account->password) - 1);
        account->password[sizeof(account->password) - 1] = '\0';
    }

    account->role = new_role;

    return 1;
}

// 鍒犻櫎鍛樺伐璐﹀彿
int delete_account(const char* username)
{
    if (username == NULL || g_account_list == NULL || g_account_list->next == NULL)
    {
        return 0;
    }

    AccountNode* prev = g_account_list;
    AccountNode* curr = g_account_list->next;

    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0)
        {
            prev->next = curr->next;
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }

    return 0;
}

// 鏌ョ湅鎵€鏈夊尰鐢熷強鍏跺€肩彮鐘舵€?
void show_all_doctors_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤鍖荤敓鏁版嵁\n");
        return;
    }

    // 绗竴姝ワ細缁熻姣忎竴鍒楃殑鏈€澶ф樉绀哄搴?
    int username_width = get_display_width("鐧诲綍璐﹀彿");
    int real_name_width = get_display_width("鐪熷疄濮撳悕");
    int duty_width = get_display_width("鍊肩彮鐘舵€?);

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_DOCTOR)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 绗簩姝ワ細鎸夊姩鎬佸垪瀹借緭鍑鸿〃鏍?
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭鏍囬
    int title_width = get_display_width("鍖荤敓鍊肩彮鐘舵€?);
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("鍖荤敓鍊肩彮鐘舵€?);
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭琛ㄥご
    print_padded_text("鐧诲綍璐﹀彿", username_width);
    printf("    ");
    print_padded_text("鐪熷疄濮撳悕", real_name_width);
    printf("    ");
    print_padded_text("鍊肩彮鐘舵€?, duty_width);
    printf("\n");

    // 杈撳嚭琛ㄥご涓嬫í绾?
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭鏁版嵁琛?
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_DOCTOR)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 杈撳嚭搴曢儴妯嚎
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭缁熻淇℃伅
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "鍖荤敓鎬绘暟锛?d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 鏌ョ湅鎵€鏈夋姢澹強鍏跺€肩彮鐘舵€?
void show_all_nurses_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤鎶ゅ＋鏁版嵁\n");
        return;
    }

    // 绗竴姝ワ細缁熻姣忎竴鍒楃殑鏈€澶ф樉绀哄搴?
    int username_width = get_display_width("鐧诲綍璐﹀彿");
    int real_name_width = get_display_width("鐪熷疄濮撳悕");
    int duty_width = get_display_width("鍊肩彮鐘舵€?);

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_NURSE)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 绗簩姝ワ細鎸夊姩鎬佸垪瀹借緭鍑鸿〃鏍?
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭鏍囬
    int title_width = get_display_width("鎶ゅ＋鍊肩彮鐘舵€?);
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("鎶ゅ＋鍊肩彮鐘舵€?);
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭琛ㄥご
    print_padded_text("鐧诲綍璐﹀彿", username_width);
    printf("    ");
    print_padded_text("鐪熷疄濮撳悕", real_name_width);
    printf("    ");
    print_padded_text("鍊肩彮鐘舵€?, duty_width);
    printf("\n");

    // 杈撳嚭琛ㄥご涓嬫í绾?
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭鏁版嵁琛?
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_NURSE)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 杈撳嚭搴曢儴妯嚎
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭缁熻淇℃伅
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "鎶ゅ＋鎬绘暟锛?d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 鏌ョ湅鎵€鏈夎嵂甯堝強鍏跺€肩彮鐘舵€?
void show_all_pharmacists_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤鑽笀鏁版嵁\n");
        return;
    }

    // 绗竴姝ワ細缁熻姣忎竴鍒楃殑鏈€澶ф樉绀哄搴?
    int username_width = get_display_width("鐧诲綍璐﹀彿");
    int real_name_width = get_display_width("鐪熷疄濮撳悕");
    int duty_width = get_display_width("鍊肩彮鐘舵€?);

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_PHARMACIST)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 绗簩姝ワ細鎸夊姩鎬佸垪瀹借緭鍑鸿〃鏍?
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭鏍囬
    int title_width = get_display_width("鑽笀鍊肩彮鐘舵€?);
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("鑽笀鍊肩彮鐘舵€?);
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 杈撳嚭琛ㄥご
    print_padded_text("鐧诲綍璐﹀彿", username_width);
    printf("    ");
    print_padded_text("鐪熷疄濮撳悕", real_name_width);
    printf("    ");
    print_padded_text("鍊肩彮鐘舵€?, duty_width);
    printf("\n");

    // 杈撳嚭琛ㄥご涓嬫í绾?
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭鏁版嵁琛?
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_PHARMACIST)
        {
            const char* duty_status = curr->is_on_duty ? "鍦ㄥ矖" : "浼戞伅";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 杈撳嚭搴曢儴妯嚎
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 杈撳嚭缁熻淇℃伅
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "鑽笀鎬绘暟锛?d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 鏇存柊鍖荤敓鍊肩彮鐘舵€?
int update_doctor_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_DOCTOR)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 鏇存柊鎶ゅ＋鍊肩彮鐘舵€?
int update_nurse_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_NURSE)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 鏇存柊鑽笀鍊肩彮鐘舵€?
int update_pharmacist_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_PHARMACIST)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 鏄剧ず绠＄悊鍛樻搷浣滈潰鏉?
void show_admin_dashboard(void)
{
    int patient_count = 0;
    int medicine_count = 0;
    int waiting_dispense_count = 0;
    int low_stock_count = 0;
    int expiring_medicine_count = 0;
    int doctor_count = 0;
    int nurse_count = 0;
    int pharmacist_count = 0;

    // 缁熻鎮ｈ€呮暟閲忓拰寰呭彂鑽偅鑰呮暟閲?
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            // STATUS_WAIT_MED = 5 琛ㄧず"宸茬即璐瑰緟鍙栬嵂"
            if (curr->status == STATUS_WAIT_MED)
            {
                waiting_dispense_count++;
            }
            curr = curr->next;
        }
    }

    // 缁熻鑽搧鏁伴噺銆佷綆搴撳瓨鏁伴噺鍜岃繎鏁堟湡鑽搧鏁伴噺
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            medicine_count++;
            if (curr->stock < 10)
            {
                low_stock_count++;
            }
            
            // 妫€鏌ユ槸鍚︿负杩戞晥鏈熻嵂鍝侊紙30澶╁唴杩囨湡锛?
            if (strlen(curr->expiry_date) > 0)
            {
                time_t now = time(NULL);
                struct tm now_tm;
                localtime_s(&now_tm, &now);
                
                char current_date[11];
                snprintf(current_date, sizeof(current_date), "%d-%02d-%02d", 
                         now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
                
                int days_left = days_between_dates(current_date, curr->expiry_date);
                if (days_left > 0 && days_left <= 30)
                {
                    expiring_medicine_count++;
                }
            }
            
            curr = curr->next;
        }
    }

    // 缁熻鍖荤敓銆佹姢澹拰鑽笀鏁伴噺
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        AccountNode* curr = g_account_list->next;
        while (curr != NULL)
        {
            switch (curr->role)
            {
                case ROLE_DOCTOR:
                    doctor_count++;
                    break;
                case ROLE_NURSE:
                    nurse_count++;
                    break;
                case ROLE_PHARMACIST:
                    pharmacist_count++;
                    break;
                default:
                    break;
            }
            curr = curr->next;
        }
    }

    printf("\n==============================================================\n");
    printf("                      绠＄悊缁熻闈㈡澘\n");
    printf("==============================================================\n");
    printf("鎮ｈ€呮€绘暟锛?d\n", patient_count);
    printf("褰撳墠寰呭彂鑽偅鑰呮暟閲忥細%d\n", waiting_dispense_count);
    printf("鑽搧鎬绘暟锛?d\n", medicine_count);
    printf("浣庡簱瀛樿嵂鍝佹暟閲忥細%d\n", low_stock_count);
    printf("杩戞晥鏈熻嵂鍝佹暟閲忥細%d\n", expiring_medicine_count);
    printf("鍖荤敓鎬绘暟锛?d\n", doctor_count);
    printf("鎶ゅ＋鎬绘暟锛?d\n", nurse_count);
    printf("鑽笀鎬绘暟锛?d\n", pharmacist_count);
    printf("==============================================================\n");
}

// 瑙ｆ瀽鏃ユ湡瀛楃涓蹭负 tm 缁撴瀯
static int parse_date_string(const char* date_str, struct tm* tm_out)
{
    if (date_str == NULL || tm_out == NULL)
    {
        return 0;
    }

    int year, month, day;
    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        return 0;
    }

    memset(tm_out, 0, sizeof(struct tm));
    tm_out->tm_year = year - 1900;
    tm_out->tm_mon = month - 1;
    tm_out->tm_mday = day;
    tm_out->tm_hour = 12; // 閬垮厤澶忎护鏃堕棶棰?

    return 1;
}

// 璁＄畻涓や釜鏃ユ湡涔嬮棿鐨勫ぉ鏁板樊
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

// 浣庡簱瀛樿嵂鍝侀璀?
static void show_low_stock_warning(void)
{
    int count = 0;
    
    // 绗竴姝ワ細缁熻姣忎竴鍒楃殑鏈€澶ф樉绀哄搴?
    int id_width = get_display_width("鑽搧缂栧彿");
    int name_width = get_display_width("鍟嗗搧鍚?);
    int generic_name_width = get_display_width("閫氱敤鍚?);
    int alias_width = get_display_width("鍒悕");
    int stock_width = get_display_width("褰撳墠搴撳瓨");
    int price_width = get_display_width("鍗曚环");
    int medicare_width = get_display_width("鍖讳繚绫诲瀷");
    int expiry_width = get_display_width("鏁堟湡");
    
    // 缁熻浣庡簱瀛樿嵂鍝佸苟璁＄畻鍒楀
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (curr->stock < 10)
            {
                count++;
                
                const char* medicare_name = NULL;
                switch (curr->m_type)
                {
                    case MEDICARE_NONE:
                        medicare_name = "鏃犲尰淇?;
                        break;
                    case MEDICARE_CLASS_A:
                        medicare_name = "鐢茬被鍖讳繚";
                        break;
                    case MEDICARE_CLASS_B:
                        medicare_name = "涔欑被鍖讳繚";
                        break;
                    default:
                        medicare_name = "鏈煡";
                        break;
                }
                
                int current_id_width = get_display_width(curr->id);
                int current_name_width = get_display_width(curr->name);
                int current_generic_name_width = get_display_width(curr->generic_name);
                int current_alias_width = get_display_width(curr->alias);
                int current_medicare_width = get_display_width(medicare_name);
                int current_expiry_width = get_display_width(curr->expiry_date);
                
                // 澶勭悊搴撳瓨鍜屽崟浠风殑瀹藉害
                char stock_str[20];
                char price_str[20];
                snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                int current_stock_width = get_display_width(stock_str);
                int current_price_width = get_display_width(price_str);
                
                if (current_id_width > id_width)
                    id_width = current_id_width;
                if (current_name_width > name_width)
                    name_width = current_name_width;
                if (current_generic_name_width > generic_name_width)
                    generic_name_width = current_generic_name_width;
                if (current_alias_width > alias_width)
                    alias_width = current_alias_width;
                if (current_stock_width > stock_width)
                    stock_width = current_stock_width;
                if (current_price_width > price_width)
                    price_width = current_price_width;
                if (current_medicare_width > medicare_width)
                    medicare_width = current_medicare_width;
                if (current_expiry_width > expiry_width)
                    expiry_width = current_expiry_width;
            }
            curr = curr->next;
        }
    }
    
    // 绗簩姝ワ細鎸夊姩鎬佸垪瀹借緭鍑鸿〃鏍?
    printf("\n");
    int total_width = id_width + name_width + generic_name_width + alias_width + stock_width + price_width + medicare_width + expiry_width + 28; // 28鏄垪闂磋窛鎬诲拰
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 杈撳嚭鏍囬
    int title_width = get_display_width("浣庡簱瀛樿嵂鍝侀璀?);
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("浣庡簱瀛樿嵂鍝侀璀?);
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("褰撳墠鏃犱綆搴撳瓨鑽搧棰勮\n");
    }
    else
    {
        // 杈撳嚭琛ㄥご
        print_padded_text("鑽搧缂栧彿", id_width);
        printf("    ");
        print_padded_text("鍟嗗搧鍚?, name_width);
        printf("    ");
        print_padded_text("閫氱敤鍚?, generic_name_width);
        printf("    ");
        print_padded_text("鍒悕", alias_width);
        printf("    ");
        print_padded_text("褰撳墠搴撳瓨", stock_width);
        printf("    ");
        print_padded_text("鍗曚环", price_width);
        printf("    ");
        print_padded_text("鍖讳繚绫诲瀷", medicare_width);
        printf("    ");
        print_padded_text("鏁堟湡", expiry_width);
        printf("\n");
        
        // 杈撳嚭琛ㄥご涓嬫í绾?
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 杈撳嚭鏁版嵁琛?
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            while (curr != NULL)
            {
                if (curr->stock < 10)
                {
                    const char* medicare_name = NULL;
                    switch (curr->m_type)
                    {
                        case MEDICARE_NONE:
                            medicare_name = "鏃犲尰淇?;
                            break;
                        case MEDICARE_CLASS_A:
                            medicare_name = "鐢茬被鍖讳繚";
                            break;
                        case MEDICARE_CLASS_B:
                            medicare_name = "涔欑被鍖讳繚";
                            break;
                        default:
                            medicare_name = "鏈煡";
                            break;
                    }
                    
                    print_padded_text(curr->id, id_width);
                    printf("    ");
                    print_padded_text(curr->name, name_width);
                    printf("    ");
                    print_padded_text(curr->generic_name, generic_name_width);
                    printf("    ");
                    print_padded_text(curr->alias, alias_width);
                    printf("    ");
                    char stock_str[20];
                    char price_str[20];
                    snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                    snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                    print_padded_text(stock_str, stock_width);
                    printf("    ");
                    print_padded_text(price_str, price_width);
                    printf("    ");
                    print_padded_text(medicare_name, medicare_width);
                    printf("    ");
                    print_padded_text(curr->expiry_date, expiry_width);
                    printf("\n");
                }
                curr = curr->next;
            }
        }
    }
}

// ==========================================
// 绯荤粺棰勮鏌ョ湅妯″潡锛堟暣鍚堟墍鏈?绉嶉璀︾被鍨嬶級
// ==========================================

// 棰滆壊瀹氫箟
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// 棰勮绠＄悊瀛愯彍鍗?
void admin_alert_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("                鈿狅笍  棰勮绠＄悊\n");
        printf("======================================================\n");
        printf("  [1] 鏌ョ湅浜嬩欢棰勮锛堟伓鎰忔寕鍙枫€佺埥绾︺€佹€ヨ瘖锛塡n");
        printf("  [2] 鏌ョ湅鑽搧棰勮锛堜綆搴撳瓨銆佷复鏈熻嵂锛塡n");
        printf("  [3] 鏌ョ湅浣忛櫌棰勮锛堟娂閲戜笉瓒炽€佹瑺璐癸級\n");
        printf("  [4] 鏌ョ湅搴婁綅璧勬簮棰勮\n");
        printf("  [5] 鏌ョ湅鎵€鏈夐璀︼紙鏁村悎瑙嗗浘锛塡n");
        printf("  [0] 杩斿洖涓婁竴绾n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("馃憠 璇疯緭鍏ユ搷浣滅紪鍙? "))
        {
            case 1:
                show_event_alerts();
                system("pause");
                break;
            case 2:
                show_medicine_alerts();
                system("pause");
                break;
            case 3:
                show_inpatient_alerts();
                system("pause");
                break;
            case 4:
                show_bed_resource_alerts();
                system("pause");
                break;
            case 5:
                show_system_alerts();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n鈿狅笍 鏃犳晥鐨勯€夐」锛岃閲嶆柊杈撳叆锛乗n");
                system("pause");
                break;
        }
    }
}

// 鏄剧ず搴婁綅璧勬簮棰勮
void show_bed_resource_alerts(void)
{
    int total_beds = 0;
    int occupied_beds = 0;
    int free_beds = 0;
    int general_occupied = 0, general_free = 0;
    int special_occupied = 0, special_free = 0;
    int icu_occupied = 0, icu_free = 0;
    
    printf("\n======================================================\n");
    printf("                搴婁綅璧勬簮棰勮\n");
    printf("======================================================\n");
    
    // 缁熻搴婁綅浣跨敤鎯呭喌
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* ward_curr = g_ward_list->next;
        while (ward_curr != NULL)
        {
            total_beds++;
            if (ward_curr->is_occupied)
            {
                occupied_beds++;
            }
            
            if (ward_curr->ward_type == WARD_TYPE_GENERAL)
            {
                if (ward_curr->is_occupied)
                    general_occupied++;
                else
                    general_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_SINGLE)
            {
                if (ward_curr->is_occupied)
                    special_occupied++;
                else
                    special_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_ICU)
            {
                if (ward_curr->is_occupied)
                    icu_occupied++;
                else
                    icu_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_ISOLATION)
            {
                // 浼犳煋鐥呴殧绂荤梾鎴垮崟鐙粺璁?
                if (ward_curr->is_occupied)
                    occupied_beds++;
            }
            
            ward_curr = ward_curr->next;
        }
        
        free_beds = total_beds - occupied_beds;
        
        // 鏄剧ず搴婁綅璧勬簮棰勮鎽樿
        printf("銆愬簥浣嶈祫婧愰璀︽憳瑕併€慭n");
        printf("------------------------------------------------------\n");
        printf("鎬诲簥浣嶆暟锛?d\n", total_beds);
        printf("宸插崰鐢ㄥ簥浣嶏細%d\n", occupied_beds);
        printf("绌洪棽搴婁綅锛? RED "%d" RESET "\n", free_beds);
        printf("搴婁綅浣跨敤鐜囷細%.2f%%\n", total_beds > 0 ? (float)occupied_beds / total_beds * 100 : 0);
        printf("------------------------------------------------------\n");
        
        // 鏄剧ず鍚勭被鍨嬬梾鎴垮簥浣嶆儏鍐?
        printf("銆愬悇绫诲瀷鐥呮埧搴婁綅鎯呭喌銆慭n");
        printf("------------------------------------------------------\n");
        printf("鏅€氱梾鎴匡細宸插崰鐢?%d, 绌洪棽 " RED "%d" RESET "\n", general_occupied, general_free);
        printf("鐗规畩鐥呮埧锛氬凡鍗犵敤 %d, 绌洪棽 " RED "%d" RESET "\n", special_occupied, special_free);
        printf("ICU锛氬凡鍗犵敤 %d, 绌洪棽 " RED "%d" RESET "\n", icu_occupied, icu_free);
        printf("------------------------------------------------------\n");
        
        // 鏄剧ず搴婁綅璧勬簮棰勮
        if (free_beds == 0)
        {
            printf("" RED "鈿狅笍 棰勮锛氭墍鏈夊簥浣嶅凡婊★紝璇峰強鏃跺鐞嗭紒" RESET "\n");
        }
        else if (free_beds < 5)
        {
            printf("" RED "鈿狅笍 棰勮锛氬簥浣嶇揣寮狅紝浠呭墿 %d 涓┖闂插簥浣嶏紒" RESET "\n", free_beds);
        }
        else
        {
            printf("鉁?搴婁綅璧勬簮鍏呰冻\n");
        }
    }
    else
    {
        printf("褰撳墠鏃犵梾鎴挎暟鎹甛n");
    }
    
    printf("======================================================\n");
}

// 鏄剧ず浜嬩欢棰勮锛堟伓鎰忔寕鍙枫€佺埥绾︺€佹€ヨ瘖锛?
void show_event_alerts(void)
{
    printf("\n======================================================\n");
    printf("                浜嬩欢棰勮鏌ョ湅\n");
    printf("======================================================\n");
    
    if (g_alert_list != NULL && g_alert_list->next != NULL)
    {
        AlertNode* curr = g_alert_list->next;
        int count = 0;
        while (curr != NULL)
        {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&curr->time));
            printf("[%s] " RED "%s" RESET "\n", time_str, curr->message);
            curr = curr->next;
            count++;
        }
        printf("鍏辨壘鍒?%d 鏉′簨浠堕璀n", count);
    }
    else
    {
        printf("褰撳墠鏃犱簨浠堕璀n");
    }
    
    printf("======================================================\n");
}

// 鏄剧ず鑽搧棰勮锛堜綆搴撳瓨銆佷复鏈熻嵂锛?
void show_medicine_alerts(void)
{
    int low_stock_count = 0;
    int expiring_count = 0;
    
    printf("\n======================================================\n");
    printf("                鑽搧棰勮鏌ョ湅\n");
    printf("======================================================\n");
    
    // 鏄剧ず浣庡簱瀛樿嵂鍝侀璀?
    printf("銆愪綆搴撳瓨鑽搧棰勮銆慭n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        while (med_curr != NULL)
        {
            if (med_curr->stock < 5)
            {
                printf("鑽搧缂栧彿锛?s\n", med_curr->id);
                printf("鍟嗗搧鍚嶏細%s\n", med_curr->name);
                printf("褰撳墠搴撳瓨锛? RED "%d" RESET "\n", med_curr->stock);
                printf("鏁堟湡锛?s\n", med_curr->expiry_date);
                printf("------------------------------------------------------\n");
                low_stock_count++;
            }
            med_curr = med_curr->next;
        }
        if (low_stock_count == 0)
        {
            printf("褰撳墠鏃犱綆搴撳瓨鑽搧棰勮\n");
        }
        printf("鍏辨壘鍒?%d 绉嶄綆搴撳瓨鑽搧\n", low_stock_count);
    }
    else
    {
        printf("褰撳墠鏃犺嵂鍝佹暟鎹甛n");
    }
    printf("------------------------------------------------------\n");
    
    // 鏄剧ず涓存湡鑽搧棰勮
    printf("銆愪复鏈熻嵂鍝侀璀︺€慭n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        char today_str[11];
        sprintf(today_str, "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        while (med_curr != NULL)
        {
            int diff_days = days_between_dates(today_str, med_curr->expiry_date);
            if (diff_days >= 0 && diff_days <= 30)
            {
                printf("鑽搧缂栧彿锛?s\n", med_curr->id);
                printf("鍟嗗搧鍚嶏細%s\n", med_curr->name);
                printf("鏁堟湡锛?s\n", med_curr->expiry_date);
                printf("璺濈杩囨湡锛? RED "%d澶? RESET "\n", diff_days);
                printf("------------------------------------------------------\n");
                expiring_count++;
            }
            med_curr = med_curr->next;
        }
        if (expiring_count == 0)
        {
            printf("褰撳墠鏃犱复鏈熻嵂鍝侀璀n");
        }
        printf("鍏辨壘鍒?%d 绉嶄复鏈熻嵂鍝乗n", expiring_count);
    }
    else
    {
        printf("褰撳墠鏃犺嵂鍝佹暟鎹甛n");
    }
    
    printf("======================================================\n");
}

// 鏄剧ず浣忛櫌棰勮锛堟娂閲戜笉瓒炽€佹瑺璐癸級
void show_inpatient_alerts(void)
{
    int deposit_warning_count = 0;
    
    printf("\n======================================================\n");
    printf("                浣忛櫌棰勮鏌ョ湅\n");
    printf("======================================================\n");
    
    // 鏄剧ず鎶奸噾涓嶈冻棰勮
    printf("銆愭娂閲戜笉瓒抽璀︺€慭n");
    printf("------------------------------------------------------\n");
    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* inpatient_curr = g_inpatient_list->next;
        PatientNode* patient = NULL;
        
        while (inpatient_curr != NULL)
        {
            if (inpatient_curr->is_active)
            {
                patient = find_patient_by_id(g_patient_list, inpatient_curr->patient_id);
                if (patient != NULL)
                {
                    double daily_rate = 0.0;
                    const char* warn_msg = "";
                    int show_warn = 0;
                    
                    if (strlen(inpatient_curr->bed_id) > 0)
                    {
                        daily_rate = get_ward_rate(inpatient_curr->ward_type);
                        if (inpatient_curr->deposit_balance < daily_rate * 3)
                        {
                            warn_msg = "鎶奸噾涓嶈冻";
                            show_warn = 1;
                        }
                    }
                    else if (strlen(inpatient_curr->bed_id) == 0)
                    {
                        double warning_threshold = get_ward_rate(inpatient_curr->recommended_ward_type) * 3;
                        if (inpatient_curr->deposit_balance < warning_threshold)
                        {
                            warn_msg = "鎶奸噾杈冨皯";
                            show_warn = 1;
                        }
                    }
                    
                    if (show_warn)
                    {
                        printf("鎮ｈ€呯紪鍙凤細%s\n", inpatient_curr->patient_id);
                        printf("鎮ｈ€呭鍚嶏細%s\n", patient->name);
                        printf("鎶奸噾浣欓锛? RED "%.2f 鍏? RESET "\n", inpatient_curr->deposit_balance);
                        printf("棰勮鐘舵€侊細" RED "%s" RESET "\n", warn_msg);
                        printf("------------------------------------------------------\n");
                        deposit_warning_count++;
                    }
                }
            }
            inpatient_curr = inpatient_curr->next;
        }
        if (deposit_warning_count == 0)
        {
            printf("褰撳墠鏃犳娂閲戜笉瓒抽璀n");
        }
        printf("鍏辨壘鍒?%d 浣嶆偅鑰呮娂閲戜笉瓒砛n", deposit_warning_count);
    }
    else
    {
        printf("褰撳墠鏃犱綇闄㈡暟鎹甛n");
    }
    printf("------------------------------------------------------\n");
    
    // 鏄剧ず娆犺垂鎮ｈ€呴璀︼紙寰呭惎鐢級
    printf("銆愭瑺璐规偅鑰呴璀︺€慭n");
    printf("------------------------------------------------------\n");
    printf("褰撳墠鐗堟湰娆犺垂鎮ｈ€呴璀﹀姛鑳藉緟鏀惰垂/缁撶畻妯″潡瀹屽叏鎺ュ叆鍚庡惎鐢ㄣ€俓n");
    
    printf("======================================================\n");
}

// 鏄剧ず绯荤粺棰勮鏌ョ湅
void show_system_alerts(void)
{
    int has_alerts = 0;
    
    printf("\n======================================================\n");
    printf("                  绯荤粺棰勮鏌ョ湅\n");
    printf("======================================================\n");
    
    // 1. 鏄剧ず鎭舵剰鎸傚彿鎷︽埅銆佺埥绾︽嫤鎴€佹€ヨ瘖寮哄埗鏀捐锛堟潵鑷璀﹂槦鍒楋級
    printf("銆愪簨浠堕璀︺€慭n");
    printf("------------------------------------------------------\n");
    if (g_alert_list != NULL && g_alert_list->next != NULL)
    {
        AlertNode* curr = g_alert_list->next;
        int count = 0;
        while (curr != NULL)
        {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&curr->time));
            printf("[%s] " RED "%s" RESET "\n", time_str, curr->message);
            curr = curr->next;
            count++;
            has_alerts = 1;
        }
        printf("鍏辨壘鍒?%d 鏉′簨浠堕璀n", count);
    }
    else
    {
        printf("褰撳墠鏃犱簨浠堕璀n");
    }
    printf("------------------------------------------------------\n");
    
    // 2. 鏄剧ず浣庡簱瀛樿嵂鍝侀璀?
    printf("銆愪綆搴撳瓨鑽搧棰勮銆慭n");
    printf("------------------------------------------------------\n");
    int low_stock_count = 0;
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        while (med_curr != NULL)
        {
            if (med_curr->stock < 5)
            {
                printf("鑽搧缂栧彿锛?s\n", med_curr->id);
                printf("鍟嗗搧鍚嶏細%s\n", med_curr->name);
                printf("褰撳墠搴撳瓨锛? RED "%d" RESET "\n", med_curr->stock);
                printf("鏁堟湡锛?s\n", med_curr->expiry_date);
                printf("------------------------------------------------------\n");
                low_stock_count++;
                has_alerts = 1;
            }
            med_curr = med_curr->next;
        }
        if (low_stock_count == 0)
        {
            printf("褰撳墠鏃犱綆搴撳瓨鑽搧棰勮\n");
        }
        printf("鍏辨壘鍒?%d 绉嶄綆搴撳瓨鑽搧\n", low_stock_count);
    }
    else
    {
        printf("褰撳墠鏃犺嵂鍝佹暟鎹甛n");
    }
    printf("------------------------------------------------------\n");
    
    // 3. 鏄剧ず涓存湡鑽搧棰勮
    printf("銆愪复鏈熻嵂鍝侀璀︺€慭n");
    printf("------------------------------------------------------\n");
    int expiring_count = 0;
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        char today_str[11];
        sprintf(today_str, "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        while (med_curr != NULL)
        {
            int diff_days = days_between_dates(today_str, med_curr->expiry_date);
            if (diff_days >= 0 && diff_days <= 30)
            {
                printf("鑽搧缂栧彿锛?s\n", med_curr->id);
                printf("鍟嗗搧鍚嶏細%s\n", med_curr->name);
                printf("鏁堟湡锛?s\n", med_curr->expiry_date);
                printf("璺濈杩囨湡锛? RED "%d澶? RESET "\n", diff_days);
                printf("------------------------------------------------------\n");
                expiring_count++;
                has_alerts = 1;
            }
            med_curr = med_curr->next;
        }
        if (expiring_count == 0)
        {
            printf("褰撳墠鏃犱复鏈熻嵂鍝侀璀n");
        }
        printf("鍏辨壘鍒?%d 绉嶄复鏈熻嵂鍝乗n", expiring_count);
    }
    else
    {
        printf("褰撳墠鏃犺嵂鍝佹暟鎹甛n");
    }
    printf("------------------------------------------------------\n");
    
    // 4. 鏄剧ず鎶奸噾涓嶈冻棰勮
    printf("銆愭娂閲戜笉瓒抽璀︺€慭n");
    printf("------------------------------------------------------\n");
    int deposit_warning_count = 0;
    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* inpatient_curr = g_inpatient_list->next;
        PatientNode* patient = NULL;
        
        while (inpatient_curr != NULL)
        {
            if (inpatient_curr->is_active)
            {
                patient = find_patient_by_id(g_patient_list, inpatient_curr->patient_id);
                if (patient != NULL)
                {
                    double daily_rate = 0.0;
                    const char* warn_msg = "";
                    int show_warn = 0;
                    
                    if (strlen(inpatient_curr->bed_id) > 0)
                    {
                        daily_rate = get_ward_rate(inpatient_curr->ward_type);
                        if (inpatient_curr->deposit_balance < daily_rate * 3)
                        {
                            warn_msg = "鎶奸噾涓嶈冻";
                            show_warn = 1;
                        }
                    }
                    else if (strlen(inpatient_curr->bed_id) == 0)
                    {
                        double warning_threshold = get_ward_rate(inpatient_curr->recommended_ward_type) * 3;
                        if (inpatient_curr->deposit_balance < warning_threshold)
                        {
                            warn_msg = "鎶奸噾杈冨皯";
                            show_warn = 1;
                        }
                    }
                    
                    if (show_warn)
                    {
                        printf("鎮ｈ€呯紪鍙凤細%s\n", inpatient_curr->patient_id);
                        printf("鎮ｈ€呭鍚嶏細%s\n", patient->name);
                        printf("鎶奸噾浣欓锛? RED "%.2f 鍏? RESET "\n", inpatient_curr->deposit_balance);
                        printf("棰勮鐘舵€侊細" RED "%s" RESET "\n", warn_msg);
                        printf("------------------------------------------------------\n");
                        deposit_warning_count++;
                        has_alerts = 1;
                    }
                }
            }
            inpatient_curr = inpatient_curr->next;
        }
        if (deposit_warning_count == 0)
        {
            printf("褰撳墠鏃犳娂閲戜笉瓒抽璀n");
        }
        printf("鍏辨壘鍒?%d 浣嶆偅鑰呮娂閲戜笉瓒砛n", deposit_warning_count);
    }
    else
    {
        printf("褰撳墠鏃犱綇闄㈡暟鎹甛n");
    }
    printf("------------------------------------------------------\n");
    
    // 5. 鏄剧ず娆犺垂鎮ｈ€呴璀︼紙寰呭惎鐢級
    printf("銆愭瑺璐规偅鑰呴璀︺€慭n");
    printf("------------------------------------------------------\n");
    printf("褰撳墠鐗堟湰娆犺垂鎮ｈ€呴璀﹀姛鑳藉緟鏀惰垂/缁撶畻妯″潡瀹屽叏鎺ュ叆鍚庡惎鐢ㄣ€俓n");
    printf("------------------------------------------------------\n");
    
    printf("======================================================\n");
    if (!has_alerts)
    {
        printf("鉁?褰撳墠鏃犻璀︼紝绯荤粺杩愯姝ｅ父\n");
    }
    else
    {
        printf("" RED "鈿狅笍 鍙戠幇棰勮淇℃伅锛岃鍙婃椂澶勭悊" RESET "\n");
    }
    printf("======================================================\n");
}

// ==========================================
// 鎶曡瘔绠＄悊妯″潡
// ==========================================

// 绠＄悊鍛樻姇璇夌鐞嗗瓙鑿滃崟
void admin_complaint_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               馃摑 鎶曡瘔绠＄悊\n");
        printf("======================================================\n");
        printf("  [1] 鏌ョ湅宸插鐞嗘姇璇塡n");
        printf("  [2] 澶勭悊鎶曡瘔\n");
        printf("  [3] 鎸夋偅鑰呯紪鍙锋煡璇㈡姇璇夊巻鍙瞈n");
        printf("  [4] 鎸夋姇璇夌紪鍙锋煡璇㈡姇璇夎鎯匼n");
        printf("  [0] 杩斿洖涓婁竴绾n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("馃憠 璇疯緭鍏ユ搷浣滅紪鍙? "))
        {
            case 1:
                show_all_complaints();
                system("pause");
                break;
            case 2:
                handle_complaint_response();
                system("pause");
                break;
            case 3:
                query_patient_complaints_by_id();
                system("pause");
                break;
            case 4:
                query_complaint_by_id();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n鈿狅笍 鏃犳晥鐨勯€夐」锛岃閲嶆柊杈撳叆锛乗n");
                system("pause");
                break;
        }
    }
}

// 鏄剧ず宸插鐞嗘姇璇?
void show_all_complaints()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤鎶曡瘔璁板綍锛乗n");
        return;
    }
    
    printf("\n======================================================\n");
    printf("                    宸插鐞嗘姇璇夊垪琛╘n");
    printf("======================================================\n");
    
    ComplaintNode* curr = g_complaint_list->next;
    int index = 1;
    int has_processed = 0;
    
    while (curr != NULL)
    {
        if (curr->status == 1)
        {
            has_processed = 1;
            printf("\n銆愭姇璇夊伐鍗?%d銆慭n", index++);
            printf("宸ュ崟缂栧彿锛?s\n", curr->complaint_id);
            printf("鎮ｈ€呯紪鍙凤細%s\n", curr->patient_id);
            printf("鎻愪氦鏃堕棿锛?s\n", curr->submit_time);
            
            printf("鎶曡瘔绫诲瀷锛?);
            switch (curr->target_type)
            {
                case 1: printf("瀵瑰尰鐢焅n"); break;
                case 2: printf("瀵规姢澹?鍓嶅彴\n"); break;
                case 3: printf("瀵硅嵂甯圽n"); break;
                default: printf("鏈煡\n"); break;
            }
            
            printf("琚姇璇変汉锛?s锛堣处鍙凤細%s锛塡n", curr->target_name, curr->target_id);
            printf("鎶曡瘔鍐呭锛?s\n", curr->content);
            
            printf("澶勭悊鐘舵€侊細宸插洖澶峔n");
            printf("澶勭悊鎰忚锛?s\n", curr->response);
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (!has_processed)
    {
        printf("\n鈿狅笍 褰撳墠娌℃湁宸插鐞嗙殑鎶曡瘔锛乗n");
    }
    
    printf("======================================================\n");
}

// 澶勭悊鎶曡瘔
void handle_complaint_response()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤鎶曡瘔璁板綍锛乗n");
        return;
    }
    
    // 鍏堟樉绀哄緟澶勭悊鐨勬姇璇?
    printf("\n======================================================\n");
    printf("                    寰呭鐞嗘姇璇塡n");
    printf("======================================================\n");
    
    ComplaintNode* curr = g_complaint_list->next;
    int index = 1;
    int has_pending = 0;
    
    while (curr != NULL)
    {
        if (curr->status == 0)
        {
            has_pending = 1;
            printf("\n銆愭姇璇夊伐鍗?%d銆慭n", index++);
            printf("宸ュ崟缂栧彿锛?s\n", curr->complaint_id);
            printf("鎮ｈ€呯紪鍙凤細%s\n", curr->patient_id);
            printf("鎶曡瘔绫诲瀷锛?);
            switch (curr->target_type)
            {
                case 1: printf("瀵瑰尰鐢焅n"); break;
                case 2: printf("瀵规姢澹?鍓嶅彴\n"); break;
                case 3: printf("瀵硅嵂甯圽n"); break;
                default: printf("鏈煡\n"); break;
            }
            printf("琚姇璇変汉锛?s锛堣处鍙凤細%s锛塡n", curr->target_name, curr->target_id);
            printf("鎶曡瘔鍐呭锛?s\n", curr->content);
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (!has_pending)
    {
        printf("\n鉁?褰撳墠娌℃湁寰呭鐞嗙殑鎶曡瘔锛乗n");
        return;
    }
    
    // 杈撳叆瑕佸鐞嗙殑鎶曡瘔缂栧彿
    char complaint_id[MAX_ID_LEN];
    get_safe_string("璇疯緭鍏ヨ澶勭悊鐨勬姇璇夊伐鍗曠紪鍙? ", complaint_id, MAX_ID_LEN);
    
    // 鏌ユ壘鎶曡瘔
    curr = g_complaint_list->next;
    ComplaintNode* target_complaint = NULL;
    
    while (curr != NULL)
    {
        if (strcmp(curr->complaint_id, complaint_id) == 0)
        {
            target_complaint = curr;
            break;
        }
        curr = curr->next;
    }
    
    if (target_complaint == NULL)
    {
        printf("\n鈿狅笍 鏈壘鍒拌鎶曡瘔宸ュ崟锛乗n");
        return;
    }
    
    if (target_complaint->status != 0)
    {
        printf("\n鈿狅笍 璇ユ姇璇夊伐鍗曞凡缁忓鐞嗚繃浜嗭紒\n");
        return;
    }
    
    // 杈撳叆澶勭悊鎰忚
    char response[MAX_RECORD_LEN];
    get_safe_string("璇疯緭鍏ュ鐞嗘剰瑙? ", response, MAX_RECORD_LEN);
    
    // 鏇存柊鎶曡瘔鐘舵€佸拰鍥炲
    target_complaint->status = 1;
    strncpy(target_complaint->response, response, MAX_RECORD_LEN - 1);
    target_complaint->response[MAX_RECORD_LEN - 1] = '\0';
    
    printf("\n鉁?鎶曡瘔澶勭悊鎴愬姛锛乗n");
    printf("宸ュ崟缂栧彿锛?s\n", target_complaint->complaint_id);
    printf("澶勭悊鎰忚锛?s\n", target_complaint->response);
}

// 鎸夋偅鑰呯紪鍙锋煡璇㈡姇璇夊巻鍙?
void query_patient_complaints_by_id()
{
    char patient_id[MAX_ID_LEN];
    get_safe_string("璇疯緭鍏ユ偅鑰呯紪鍙? ", patient_id, MAX_ID_LEN);
    
    // 璋冪敤鐜版湁鐨?query_patient_complaints 鍑芥暟
    query_patient_complaints(patient_id);
}

// 鎸夋姇璇夌紪鍙锋煡璇㈡姇璇夎鎯?
void query_complaint_by_id()
{
    char complaint_id[MAX_ID_LEN];
    get_safe_string("璇疯緭鍏ユ姇璇夌紪鍙? ", complaint_id, MAX_ID_LEN);
    
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤鎶曡瘔璁板綍锛乗n");
        return;
    }
    
    // 鏌ユ壘鎶曡瘔
    ComplaintNode* curr = g_complaint_list->next;
    ComplaintNode* target_complaint = NULL;
    
    while (curr != NULL)
    {
        if (strcmp(curr->complaint_id, complaint_id) == 0)
        {
            target_complaint = curr;
            break;
        }
        curr = curr->next;
    }
    
    if (target_complaint == NULL)
    {
        printf("\n鈿狅笍 鏈壘鍒拌鎶曡瘔宸ュ崟锛乗n");
        return;
    }
    
    // 鏄剧ず鎶曡瘔璇︽儏
    printf("\n======================================================\n");
    printf("                    鎶曡瘔璇︽儏\n");
    printf("======================================================\n");
    printf("宸ュ崟缂栧彿锛?s\n", target_complaint->complaint_id);
    printf("鎮ｈ€呯紪鍙凤細%s\n", target_complaint->patient_id);
    printf("鎻愪氦鏃堕棿锛?s\n", target_complaint->submit_time);
    
    // 鎵撳嵃鎶曡瘔绫诲瀷
    printf("鎶曡瘔绫诲瀷锛?);
    switch (target_complaint->target_type)
    {
        case 1: printf("瀵瑰尰鐢焅n"); break;
        case 2: printf("瀵规姢澹?鍓嶅彴\n"); break;
        case 3: printf("瀵硅嵂甯圽n"); break;
        default: printf("鏈煡\n"); break;
    }
    
    printf("琚姇璇変汉锛?s锛堣处鍙凤細%s锛塡n", target_complaint->target_name, target_complaint->target_id);
    printf("鎶曡瘔鍐呭锛?s\n", target_complaint->content);
    
    // 鎵撳嵃澶勭悊鐘舵€?
    printf("澶勭悊鐘舵€侊細");
    if (target_complaint->status == 0)
    {
        printf("寰呭鐞哱n");
    }
    else
    {
        printf("宸插洖澶峔n");
        printf("澶勭悊鎰忚锛?s\n", target_complaint->response);
    }
    printf("======================================================\n");
}

// ==========================================
// 璇勪环绠＄悊妯″潡
// ==========================================

// 绠＄悊鍛樿瘎浠风鐞嗗瓙鑿滃崟
void admin_evaluation_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               猸?璇勪环绠＄悊\n");
        printf("======================================================\n");
        printf("  [1] 鏌ョ湅鎵€鏈夎瘎浠穃n");
        printf("  [2] 鏌ョ湅璇勪环缁熻\n");
        printf("  [0] 杩斿洖涓婁竴绾n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("馃憠 璇疯緭鍏ユ搷浣滅紪鍙? "))
        {
            case 1:
                show_all_evaluations();
                system("pause");
                break;
            case 2:
                show_evaluation_statistics();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n鈿狅笍 鏃犳晥鐨勯€夐」锛岃閲嶆柊杈撳叆锛乗n");
                system("pause");
                break;
        }
    }
}

// 鏄剧ず鎵€鏈夎瘎浠?
void show_all_evaluations()
{
    if (g_consult_record_list == NULL || g_consult_record_list->next == NULL)
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤璇勪环璁板綍锛乗n");
        return;
    }
    
    printf("\n======================================================\n");
    printf("                    璇勪环鍒楄〃\n");
    printf("======================================================\n");
    
    ConsultRecordNode* curr = g_consult_record_list->next;
    int index = 1;
    int evaluation_count = 0;
    
    while (curr != NULL)
    {
        if (curr->star_rating > 0)
        {
            evaluation_count++;
            printf("\n銆愯瘎浠峰伐鍗?%d銆慭n", index++);
            printf("璁板綍缂栧彿锛?s\n", curr->record_id);
            printf("鎮ｈ€呯紪鍙凤細%s\n", curr->patient_id);
            printf("灏辫瘖鍖荤敓锛?s锛堢紪鍙凤細%s锛塡n", 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id) ? 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id)->name : "鏈煡", 
                   curr->doctor_id);
            printf("灏辫瘖鏃堕棿锛?s\n", curr->consult_time);
            
            // 鎵撳嵃鏄熺骇璇勪环
            printf("婊℃剰搴﹁瘎鍒嗭細");
            for (int i = 0; i < curr->star_rating; i++)
            {
                printf("猸?);
            }
            printf(" (%d鏄?\n", curr->star_rating);
            
            // 鎵撳嵃鏂囧瓧璇勪环
            if (strlen(curr->feedback) > 0)
            {
                printf("鏂囧瓧璇勪环锛?s\n", curr->feedback);
            }
            else
            {
                printf("鏂囧瓧璇勪环锛氾紙鏃狅級\n");
            }
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (evaluation_count == 0)
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤璇勪环璁板綍锛乗n");
    }
    else
    {
        printf("\n鍏辨壘鍒?%d 鏉¤瘎浠疯褰昞n", evaluation_count);
    }
    printf("======================================================\n");
}

// 鏄剧ず璇勪环缁熻
void show_evaluation_statistics()
{
    if (g_consult_record_list == NULL)
    {
        printf("\n鈿狅笍 鎺ヨ瘖璁板綍閾捐〃灏氭湭鍒濆鍖栵紒\n");
        return;
    }
    
    int total_evaluations = 0;
    int star_counts[6] = {0}; // star_counts[0] 琛ㄧず鏈瘎浠? star_counts[1-5] 琛ㄧず鍚勬槦绾?
    float total_score = 0.0;
    
    ConsultRecordNode* curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (curr->star_rating > 0)
        {
            total_evaluations++;
            star_counts[curr->star_rating]++;
            total_score += curr->star_rating;
        }
        else
        {
            star_counts[0]++;
        }
        curr = curr->next;
    }
    
    printf("\n======================================================\n");
    printf("                    璇勪环缁熻\n");
    printf("======================================================\n");
    printf("鎬昏瘎浠锋暟閲忥細%d\n", total_evaluations);
    
    if (total_evaluations > 0)
    {
        float average_score = total_score / total_evaluations;
        printf("骞冲潎婊℃剰搴︼細%.2f 鏄焅n", average_score);
        printf("------------------------------------------------------\n");
        printf("鍚勬槦绾ц瘎浠峰垎甯冿細\n");
        printf("  猸愨瓙猸愨瓙猸?(5鏄?: %d 鏉?(%.1f%%)\n", 
               star_counts[5], (float)star_counts[5] / total_evaluations * 100);
        printf("  猸愨瓙猸愨瓙 (4鏄?: %d 鏉?(%.1f%%)\n", 
               star_counts[4], (float)star_counts[4] / total_evaluations * 100);
        printf("  猸愨瓙猸?(3鏄?: %d 鏉?(%.1f%%)\n", 
               star_counts[3], (float)star_counts[3] / total_evaluations * 100);
        printf("  猸愨瓙 (2鏄?: %d 鏉?(%.1f%%)\n", 
               star_counts[2], (float)star_counts[2] / total_evaluations * 100);
        printf("  猸?(1鏄?: %d 鏉?(%.1f%%)\n", 
               star_counts[1], (float)star_counts[1] / total_evaluations * 100);
        
        // 濂借瘎鐜囷紙4鏄熷強浠ヤ笂锛?
        int positive_count = star_counts[5] + star_counts[4];
        float positive_rate = (float)positive_count / total_evaluations * 100;
        printf("------------------------------------------------------\n");
        printf("濂借瘎鐜囷紙4鏄熷強浠ヤ笂锛夛細%.1f%%\n", positive_rate);
        
        // 宸瘎鐜囷紙2鏄熷強浠ヤ笅锛?
        int negative_count = star_counts[2] + star_counts[1];
        float negative_rate = (float)negative_count / total_evaluations * 100;
        printf("宸瘎鐜囷紙2鏄熷強浠ヤ笅锛夛細%.1f%%\n", negative_rate);
    }
    else
    {
        printf("\n鈿狅笍 褰撳墠鏆傛棤璇勪环璁板綍锛乗n");
    }
    
    printf("======================================================\n");
    
    // 杈撳嚭缁熻淇℃伅
    printf("璇勪环鎬绘暟锛?d\n", total_evaluations);
    printf("======================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

static int str_disp_width(const char* s)
{
    int width = 0;
    while (*s)
    {
        unsigned char c = (unsigned char)*s;
        if (c < 0x80)      { width += 1; s += 1; }
        else if ((c & 0xE0) == 0xC0) { width += 2; s += 2; }
        else if ((c & 0xF0) == 0xE0) { width += 2; s += 3; }
        else if ((c & 0xF8) == 0xF0) { width += 2; s += 4; }
        else                { s += 1; }
    }
    return width;
}

static void print_field(const char* s, int field_width)
{
    int dw = str_disp_width(s);
    printf("%s", s);
    int pad = field_width - dw;
    if (pad > 0) printf("%*s", pad, "");
}

// 鏄剧ず璐熻浇鐩戞帶鏌ョ湅
void show_load_monitoring(void)
{
    system("cls");
    printf("\n================================================================================\n");
    printf("                        鍏叡璐熻浇鐩戞帶\n");
    printf("================================================================================\n");

    int doctor_count = 0;
    int patient_count = 0;
    int bed_count = 0;
    int occupied_beds = 0;

    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        DoctorNode* curr = g_doctor_list->next;
        while (curr != NULL)
        {
            doctor_count++;
            curr = curr->next;
        }
    }

    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            curr = curr->next;
        }
    }

    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            bed_count++;
            if (curr->is_occupied)
                occupied_beds++;
            curr = curr->next;
        }
    }

    printf("銆愬叕鍏变俊鎭粺璁°€慭n");
    printf("--------------------------------------------------------------------------------\n");
    printf("鍖荤敓鎬绘暟锛?d\n", doctor_count);
    printf("鎮ｈ€呮€绘暟锛?d\n", patient_count);
    printf("搴婁綅鎬绘暟锛?d\n", bed_count);
    printf("宸插崰鐢ㄥ簥浣嶏細%d\n", occupied_beds);
    printf("绌洪棽搴婁綅锛?d\n", bed_count - occupied_beds);
    printf("--------------------------------------------------------------------------------\n");

    printf("鎻愮ず锛氭闈㈡澘鏁版嵁瀹炴椂鏇存柊锛屽彲浣滀负绯荤粺杩愯鐩戞帶鍙傝€冦€俓n");
    printf("================================================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 寰呭彂鑽偅鑰呴璀?
static void show_waiting_dispense_warning(void)
{
    int count = 0;
    int total_width = 0;
    
    int id_width = get_display_width("鎮ｈ€呯紪鍙?);
    int name_width = get_display_width("濮撳悕");
    int age_width = get_display_width("骞撮緞");
    int status_width = get_display_width("褰撳墠鐘舵€?);
    int dept_width = get_display_width("灏辫瘖绉戝");
    
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_WAIT_MED)
            {
                count++;
                
                const char* status_name = NULL;
                switch (curr->status)
                {
                    case STATUS_WAIT_MED:
                        status_name = "宸茬即璐瑰緟鍙栬嵂";
                        break;
                    default:
                        status_name = "鏈煡";
                        break;
                }
                
                int current_id_width = get_display_width(curr->id);
                int current_name_width = get_display_width(curr->name);
                int current_status_width = get_display_width(status_name);
                int current_dept_width = get_display_width(curr->target_dept);
                
                if (current_id_width > id_width)
                    id_width = current_id_width;
                if (current_name_width > name_width)
                    name_width = current_name_width;
                if (current_status_width > status_width)
                    status_width = current_status_width;
                if (current_dept_width > dept_width)
                    dept_width = current_dept_width;
            }
            curr = curr->next;
        }
    }

    total_width = id_width + name_width + age_width + status_width + dept_width + 12;

    printf("\n======================================================\n");
    printf("                  寰呭彇鑽偅鑰呮彁閱抃n");
    printf("======================================================\n");
    print_padded_text("鎮ｈ€呯紪鍙?, id_width);
    printf("    ");
    print_padded_text("濮撳悕", name_width);
    printf("    ");
    print_padded_text("骞撮緞", age_width);
    printf("    ");
    print_padded_text("褰撳墠鐘舵€?, status_width);
    printf("    ");
    print_padded_text("灏辫瘖绉戝", dept_width);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_WAIT_MED)
            {
                const char* status_name = NULL;
                switch (curr->status)
                {
                    case STATUS_WAIT_MED:
                        status_name = "宸茬即璐瑰緟鍙栬嵂";
                        break;
                    default:
                        status_name = "鏈煡";
                        break;
                }
                
                print_padded_text(curr->id, id_width);
                printf("    ");
                print_padded_text(curr->name, name_width);
                printf("    ");
                printf("%d", curr->age);
                printf("    ");
                print_padded_text(status_name, status_width);
                printf("    ");
                print_padded_text(curr->target_dept, dept_width);
                printf("\n");
            }
            curr = curr->next;
        }
    }
    
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "寰呭彇鑽偅鑰呮暟閲忥細%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 鍑芥暟澹版槑
static void show_bed_occupancy_warning(void);
static void show_deposit_warning(void);
static void show_arrears_warning(void);

// 鏄剧ず璧勬簮棰勮
void show_resource_warnings(void)
{
    int choice;
    
    do
    {
        int low_stock_count = 0;
        int expiring_medicine_count = 0;
        int deposit_warning_count = 0;
        int waiting_dispense_count = 0;
        int occupied_beds = 0;
        int free_beds = 0;
        int unpaid_patient_count = 0;

        // 缁熻浣庡簱瀛樿嵂鍝佸拰杩戞晥鏈熻嵂鍝佹暟閲?
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            
            time_t now = time(NULL);
            struct tm now_tm;
            localtime_s(&now_tm, &now);
            
            char current_date[11];
            snprintf(current_date, sizeof(current_date), "%d-%02d-%02d", 
                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
            
            while (curr != NULL)
            {
                if (curr->stock < 10)
                {
                    low_stock_count++;
                }
                
                // 妫€鏌ユ槸鍚︿负杩戞晥鏈熻嵂鍝侊紙30澶╁唴杩囨湡锛?
                if (strlen(curr->expiry_date) > 0)
                {
                    int days_left = days_between_dates(current_date, curr->expiry_date);
                    if (days_left > 0 && days_left <= 30)
                    {
                        expiring_medicine_count++;
                    }
                }
                
                curr = curr->next;
            }
        }

        // 缁熻鎶奸噾棰勮浣忛櫌鎮ｈ€呮暟閲?
        if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
        {
            InpatientRecord* curr = g_inpatient_list->next;
            while (curr != NULL)
            {
                // 浠呯粺璁″綋鍓嶄粛鍦ㄤ綇闄腑鐨勬偅鑰?
                if (curr->is_active)
                {
                    occupied_beds++;
                    // 鏍规嵁鐥呮埧绫诲瀷浼扮畻鏃ヨ垂鐢紝鑻ユ娂閲戜笉瓒?澶╄垂鐢ㄥ垯棰勮
                    double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                    double threshold = daily_cost * 3; // 鎶奸噾涓嶈冻3澶╄垂鐢ㄦ椂棰勮
                    if (curr->deposit_balance < threshold)
                    {
                        deposit_warning_count++;
                    }
                }
                curr = curr->next;
            }
        }

        // 缁熻寰呭彂鑽偅鑰呮暟閲?
        if (g_patient_list != NULL && g_patient_list->next != NULL)
        {
            PatientNode* curr = g_patient_list->next;
            while (curr != NULL)
            {
                if (curr->status == STATUS_UNPAID)
                {
                    waiting_dispense_count++;
                }
                curr = curr->next;
            }
        }

        // 缁熻娆犺垂/寰呯即璐规偅鑰呮暟閲?
        if (g_patient_list != NULL && g_patient_list->next != NULL)
        {
            PatientNode* curr = g_patient_list->next;
            while (curr != NULL)
            {
                if (curr->status == STATUS_UNPAID)
                {
                    unpaid_patient_count++;
                }
                curr = curr->next;
            }
        }

        // 璁＄畻绌洪棽搴婁綅鏁帮紙鍋囪鏈?00涓簥浣嶏級
        free_beds = 100 - occupied_beds;
        if (free_beds < 0) free_beds = 0;

        printf("\n==============================================================\n");
        printf("                      璧勬簮棰勮\n");
        printf("==============================================================\n");
        printf("浣庡簱瀛樿嵂鍝佹暟閲忥細%d\n", low_stock_count);
        printf("杩戞晥鏈熻嵂鍝佹暟閲忥細%d\n", expiring_medicine_count);
        printf("寰呭彂鑽偅鑰呮暟閲忥細%d\n", waiting_dispense_count);
        printf("褰撳墠宸插崰鐢ㄥ簥浣嶆暟锛?d\n", occupied_beds);
        printf("褰撳墠绌洪棽搴婁綅鏁帮細%d\n", free_beds);
        printf("鎶奸噾棰勮浣忛櫌鎮ｈ€呮暟閲忥細%d\n", deposit_warning_count);
        printf("娆犺垂 / 寰呯即璐规偅鑰呮暟閲忥細%d\n", unpaid_patient_count);
        printf("==============================================================\n");
        printf("璇烽€夋嫨瑕佹煡鐪嬬殑棰勮璇︽儏\n");
        printf("==============================================================\n");
        printf("[1] 浣庡簱瀛樿嵂鍝侀璀n");
        printf("[2] 杩戞晥鏈熻嵂鍝侀璀n");
        printf("[3] 寰呭彂鑽偅鑰呴璀n");
        printf("[4] 搴婁綅鍗犵敤棰勮\n");
        printf("[5] 浣忛櫌鎶奸噾棰勮\n");
        printf("[6] 娆犺垂 / 寰呯即璐归璀n");
        printf("[0] 杩斿洖涓婁竴绾n");
        printf("==============================================================\n");
        printf("璇疯緭鍏ラ€夋嫨锛?);
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_low_stock_warning();
                break;
            case 2:
                show_expiring_medicine_warning();
                break;
            case 3:
                show_waiting_dispense_warning();
                break;
            case 4:
                show_bed_occupancy_warning();
                break;
            case 5:
                show_deposit_warning();
                break;
            case 6:
                show_arrears_warning();
                break;
            case 0:
                // 杩斿洖涓婁竴绾?
                break;
            default:
                printf("杈撳叆閿欒锛岃閲嶆柊閫夋嫨锛乗n");
                printf("鎸変换鎰忛敭缁х画...\n");
                get_single_char("");
                break;
        }
    } while (choice != 0);
}





// 搴婁綅鍗犵敤棰勮
static void show_bed_occupancy_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("                搴婁綅鍗犵敤棰勮\n");
    printf("======================================================\n");

    int occupied_count = 0;
    int free_count = 0;

    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        
        while (curr != NULL)
        {
            if (curr->is_active)
            {
                occupied_count++;
                printf("鎮ｈ€呯紪鍙凤細%s\n", curr->patient_id);
                
                // 鏌ユ壘鎮ｈ€呭鍚?
                PatientNode* patient = find_patient_by_id(g_patient_list, curr->patient_id);
                if (patient != NULL)
                {
                    printf("鎮ｈ€呭鍚嶏細%s\n", patient->name);
                }
                else
                {
                    printf("鎮ｈ€呭鍚嶏細鏈煡\n");
                }
                
                printf("褰撳墠鐘舵€侊細宸插崰鐢╘n");
                printf("------------------------------------------------------\n");
            }
            curr = curr->next;
        }
    }

    // 璁＄畻绌洪棽搴婁綅鏁帮紙鍋囪鏈?00涓簥浣嶏級
    free_count = 100 - occupied_count;
    if (free_count < 0) free_count = 0;

    if (occupied_count == 0)
    {
        printf("褰撳墠鏆傛棤搴婁綅鍗犵敤棰勮銆俓n");
    }
    else
    {
        printf("宸插崰鐢ㄥ簥浣嶆€绘暟锛?d\n", occupied_count);
        printf("绌洪棽搴婁綅鎬绘暟锛?d\n", free_count);
    }
    
    printf("======================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 浣忛櫌鎶奸噾棰勮
static void show_deposit_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("                浣忛櫌鎶奸噾棰勮\n");
    printf("======================================================\n");

    int count = 0;

    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        
        while (curr != NULL)
        {
            if (curr->is_active)
            {
                // 鏍规嵁鐥呮埧绫诲瀷浼扮畻鏃ヨ垂鐢紝鑻ユ娂閲戜笉瓒?澶╄垂鐢ㄥ垯棰勮
                double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                double threshold = daily_cost * 3; // 鎶奸噾涓嶈冻3澶╄垂鐢ㄦ椂棰勮
                
                if (curr->deposit_balance < threshold)
                {
                    count++;
                    printf("鎮ｈ€呯紪鍙凤細%s\n", curr->patient_id);
                    
                    // 鏌ユ壘鎮ｈ€呭鍚?
                    PatientNode* patient = find_patient_by_id(g_patient_list, curr->patient_id);
                    if (patient != NULL)
                    {
                        printf("鎮ｈ€呭鍚嶏細%s\n", patient->name);
                    }
                    else
                    {
                        printf("鎮ｈ€呭鍚嶏細鏈煡\n");
                    }
                    
                    printf("褰撳墠鎶奸噾锛?.2f\n", curr->deposit_balance);
                    printf("棰勮闃堝€硷細%.2f\n", threshold);
                    printf("宸锛?.2f\n", threshold - curr->deposit_balance);
                    printf("褰撳墠浣忛櫌鐘舵€侊細浣忛櫌涓璡n");
                    printf("------------------------------------------------------\n");
                }
            }
            curr = curr->next;
        }
    }

    if (count == 0)
    {
        printf("褰撳墠鏆傛棤浣忛櫌鎶奸噾棰勮銆俓n");
    }
    else
    {
        printf("鎶奸噾棰勮浣忛櫌鎮ｈ€呮€绘暟锛?d\n", count);
    }
    
    printf("======================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 娆犺垂/寰呯即璐归璀?
static void show_arrears_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("              娆犺垂 / 寰呯即璐归璀n");
    printf("======================================================\n");

    int count = 0;

    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        
        while (curr != NULL)
        {
            if (curr->status == STATUS_UNPAID)
            {
                count++;
                printf("鎮ｈ€呯紪鍙凤細%s\n", curr->id);
                printf("鎮ｈ€呭鍚嶏細%s\n", curr->name);
                printf("褰撳墠鐘舵€侊細寰呯即璐筡n");
                printf("璐︽埛浣欓锛?.2f\n", curr->balance);
                
                // 妫€鏌ユ槸鍚︿负浣忛櫌鎮ｈ€?
                int is_inpatient = 0;
                if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
                {
                    InpatientRecord* inpatient_curr = g_inpatient_list->next;
                    while (inpatient_curr != NULL)
                    {
                        if (inpatient_curr->is_active && strcmp(inpatient_curr->patient_id, curr->id) == 0)
                        {
                            is_inpatient = 1;
                            break;
                        }
                        inpatient_curr = inpatient_curr->next;
                    }
                }
                
                printf("鏄惁浣忛櫌锛?s\n", is_inpatient ? "鏄? : "鍚?);
                printf("澶囨敞锛?s\n", curr->balance < 0 ? "娆犺垂" : "寰呯即璐?);
                printf("------------------------------------------------------\n");
            }
            curr = curr->next;
        }
    }

    if (count == 0)
    {
        printf("褰撳墠鏆傛棤娆犺垂 / 寰呯即璐归璀︺€俓n");
    }
    else
    {
        printf("娆犺垂 / 寰呯即璐规偅鑰呮€绘暟锛?d\n", count);
    }
    
    printf("======================================================\n");
    printf("鎸変换鎰忛敭杩斿洖...\n");
    get_single_char("");
}

// 鏄剧ず鍏叡鐘舵€佺粺璁℃憳瑕?
void show_public_status_statistics(void)
{
    printf("\n==============================================================\n");
    printf("                      鍏叡鐘舵€佺粺璁℃憳瑕乗n");
    printf("==============================================================\n");
    
    int patient_count = 0;
    int doctor_count = 0;
    int nurse_count = 0;
    int pharmacist_count = 0;
    int medicine_count = 0;
    
    PatientNode* patient_curr = NULL;
    DoctorNode* doctor_curr = NULL;
    AccountNode* account_curr = NULL;

    // 缁熻鎮ｈ€呮暟閲?
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            curr = curr->next;
        }
    }
    
    // 缁熻鍖绘姢浜哄憳鏁伴噺
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        AccountNode* curr = g_account_list->next;
        while (curr != NULL)
        {
            switch (curr->role)
            {
                case ROLE_DOCTOR:
                    doctor_count++;
                    break;
                case ROLE_NURSE:
                    nurse_count++;
                    break;
                case ROLE_PHARMACIST:
                    pharmacist_count++;
                    break;
                default:
                    break;
            }
            curr = curr->next;
        }
    }
    
    // 缁熻鑽搧鏁伴噺
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            medicine_count++;
            curr = curr->next;
        }
    }

    // 缁熻鎶ゅ＋鍜岃嵂甯堟€绘暟
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            if (account_curr->role == ROLE_NURSE)
            {
                nurse_count++;
            }
            else if (account_curr->role == ROLE_PHARMACIST)
            {
                pharmacist_count++;
            }
            account_curr = account_curr->next;
        }
    }

    // 鏄剧ず鍏叡鐘舵€佺粺璁℃憳瑕?
    printf("\n======================================================\n");
    printf("                  鐘舵€佺粺璁℃憳瑕乗n");
    printf("======================================================\n");
    printf("1. 鎮ｈ€呮€绘暟锛?d\n", patient_count);
    printf("2. 鍖荤敓鎬绘暟锛?d\n", doctor_count);
    printf("3. 鎶ゅ＋鎬绘暟锛?d\n", nurse_count);
    printf("4. 鑽笀鎬绘暟锛?d\n", pharmacist_count);
    printf("5. 鑽搧鎬绘暟锛?d\n", medicine_count);
    printf("======================================================\n");
    printf("鎻愮ず锛氭闈㈡澘鏁版嵁瀹炴椂鏇存柊锛屼粎渚涘弬鑰冦€俓n");
    printf("======================================================\n");
}

// 鏄剧ず浼犳煋鐥呭紓甯告彁閱?
void show_infectious_disease_alerts(void)
{
    printf("\n==============================================================\n");
    printf("                      浼犳煋鐥呭紓甯告彁閱抃n");
    printf("==============================================================\n");
    printf("褰撳墠鏃犱紶鏌撶梾寮傚父鎻愰啋\n");
    printf("==============================================================\n");
}

void handle_medicine_register(void)
{
    char name[MAX_MED_NAME_LEN] = "";
    char alias[MAX_ALIAS_LEN] = "";
    char generic_name[MAX_GENERIC_NAME_LEN] = "";
    char expiry_date[MAX_DATE_LEN] = "";
    double price = 0.0;
    int stock = 0;
    int medicare_type = 0;
    int step = 1;

    printf("\n================ 鏂板鑽搧 ================\n");
    printf("鎻愮ず锛氳緭鍏?'B' 杩斿洖涓婁竴姝ワ紝绗竴姝ヨ緭鍏?'B' 閫€鍑篭n\n");

    while (step >= 1 && step <= 7)
    {
        switch (step)
        {
            case 1:
                while (1)
                {
                    if (!get_form_string("璇疯緭鍏ュ晢鍝佸悕锛堣緭鍏?B 閫€鍑猴級: ", name, MAX_MED_NAME_LEN))
                    {
                        return;
                    }
                    if (is_blank_string(name))
                    {
                        printf("鍟嗗搧鍚嶄笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
                        continue;
                    }
                    // 鎻愬墠妫€鏌ュ晢鍝佸悕鏄惁宸插瓨鍦紝閬垮厤鐢ㄦ埛鐧藉～鍚庣画淇℃伅
                    if (is_medicine_name_exists(name))
                    {
                        printf("璀﹀憡锛氬凡瀛樺湪鍚屽悕鑽搧锛岃浣跨敤涓嶅悓鍚嶇О鎴栦慨鏀瑰凡鏈夎嵂鍝乗n");
                        continue;
                    }
                    step = 2;
                    break;
                }
                break;

            case 2:
                while (1)
                {
                    if (!get_form_string("璇疯緭鍏ュ埆鍚嶏紙鍙暀绌猴紝杈撳叆 B 杩斿洖涓婁竴姝ワ級: ", alias, MAX_ALIAS_LEN))
                    {
                        step = 1;
                        break;
                    }
                    step = 3;
                    break;
                }
                break;

            case 3:
                while (1)
                {
                    if (!get_form_string("璇疯緭鍏ラ€氱敤鍚嶏紙杈撳叆 B 杩斿洖涓婁竴姝ワ級: ", generic_name, MAX_GENERIC_NAME_LEN))
                    {
                        step = 2;
                        break;
                    }
                    if (is_blank_string(generic_name))
                    {
                        printf("閫氱敤鍚嶄笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
                        continue;
                    }
                    step = 4;
                    break;
                }
                break;

            case 4:
                while (1)
                {
                    if (!get_form_double("璇疯緭鍏ヨ嵂鍝佸崟浠凤紙杈撳叆 B 杩斿洖涓婁竴姝ワ級: ", &price, 0, "鍗曚环蹇呴』鏄ぇ浜?0 鐨勬暟瀛楋紝璇烽噸鏂拌緭鍏n"))
                    {
                        step = 3;
                        break;
                    }
                    step = 5;
                    break;
                }
                break;

            case 5:
                while (1)
                {
                    if (!get_form_int("璇疯緭鍏ュ垵濮嬪簱瀛橈紙杈撳叆 B 杩斿洖涓婁竴姝ワ級: ", &stock, 0, 999999, "搴撳瓨蹇呴』鏄ぇ浜庣瓑浜?0 鐨勬暣鏁帮紝璇烽噸鏂拌緭鍏n"))
                    {
                        step = 4;
                        break;
                    }
                    step = 6;
                    break;
                }
                break;

            case 6:
                printf("鍖讳繚绫诲瀷锛?=闈炲尰淇?1=鐢茬被鍖讳繚 2=涔欑被鍖讳繚\n");
                while (1)
                {
                    if (!get_form_int("璇疯緭鍏ュ尰淇濈被鍨嬬紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴姝ワ級: ", &medicare_type, 0, 2, "鍖讳繚绫诲瀷杈撳叆闈炴硶锛岃閲嶆柊杈撳叆\n"))
                    {
                        step = 5;
                        break;
                    }
                    step = 7;
                    break;
                }
                break;

            case 7:
                while (1)
                {
                    if (!get_form_date("璇疯緭鍏ユ晥鏈燂紙YYYY-MM-DD锛岃緭鍏?B 杩斿洖涓婁竴姝ワ級: ", expiry_date, MAX_DATE_LEN))
                    {
                        step = 6;
                        break;
                    }
                    step = 8;
                    break;
                }
                break;

            default:
                break;
        }
    }

    if (step == 8)
    {
        register_medicine(
            name,
            alias,
            generic_name,
            price,
            stock,
            (MedicareType)medicare_type,
            expiry_date
        );
    }
}

void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char new_alias[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    char new_expiry_date[MAX_DATE_LEN];
    const char* alias_ptr = NULL;
    double new_price = -1.0; // 鍒濆鍖栦负-1锛岃〃绀轰笉淇敼

    printf("\n================ 淇敼鑽搧鍩虹淇℃伅 ===============-\n");

    // 1. 鑽搧缂栧彿锛堝繀濉級
    MedicineNode* medicine_node = NULL;
    while (1)
    {
        get_safe_string("璇疯緭鍏ヨ嵂鍝佺紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", med_id, MAX_ID_LEN);
        
        // 妫€鏌ユ槸鍚﹁緭鍏/b杩斿洖涓婁竴绾?
        if (strcmp(med_id, "B") == 0 || strcmp(med_id, "b") == 0)
        {
            return;
        }
        
        // 妫€鏌ユ槸鍚︿负绌?
        if (is_blank_string(med_id))
        {
            printf("鑽搧缂栧彿涓嶈兘涓虹┖锛岃閲嶆柊杈撳叆\n");
            continue;
        }
        
        // 妫€鏌ヨ嵂鍝佹槸鍚﹀瓨鍦?
        medicine_node = find_medicine_by_id(g_medicine_list, med_id);
        if (medicine_node == NULL)
        {
            printf("鏈壘鍒扮紪鍙蜂负 %s 鐨勮嵂鍝侊紝淇敼娴佺▼缁撴潫\n", med_id);
            return;
        }
        
        break;
    }

    // 鏄剧ず褰撳墠鑽搧淇℃伅
    printf("\n------------- 褰撳墠鑽搧淇℃伅 ------------\n");
    printf("鑽搧缂栧彿锛?s\n", medicine_node->id);
    printf("鍟嗗搧鍚嶏細%s\n", medicine_node->name);
    printf("閫氱敤鍚嶏細%s\n", medicine_node->generic_name);
    printf("鍒悕锛?s\n", (medicine_node->alias[0] == '\0') ? "鏃? : medicine_node->alias);
    printf("鍗曚环锛?.2f\n", medicine_node->price);
    printf("搴撳瓨锛?d\n", medicine_node->stock);
    
    // 鏄剧ず鍖讳繚绫诲瀷
    switch (medicine_node->m_type)
    {
        case MEDICARE_NONE:
            printf("鍖讳繚绫诲瀷锛氶潪鍖讳繚\n");
            break;
        case MEDICARE_CLASS_A:
            printf("鍖讳繚绫诲瀷锛氱敳绫诲尰淇漒n");
            break;
        case MEDICARE_CLASS_B:
            printf("鍖讳繚绫诲瀷锛氫箼绫诲尰淇漒n");
            break;
        default:
            printf("鍖讳繚绫诲瀷锛氭湭鐭n");
            break;
    }
    printf("鏁堟湡锛?s\n", medicine_node->expiry_date);
    printf("----------------------------------------\n");

    // 2. 鍟嗗搧鍚嶏紙閫夊～锛岀┖鐧藉瓧绗︿覆琛ㄧず涓嶄慨鏀癸級
    get_safe_string("璇疯緭鍏ユ柊鍟嗗搧鍚嶏紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", new_name, MAX_MED_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0)
    {
        return;
    }

    // 3. 閫氱敤鍚嶏紙閫夊～锛岀┖鐧藉瓧绗︿覆琛ㄧず涓嶄慨鏀癸級
    get_safe_string("璇疯緭鍏ユ柊閫氱敤鍚嶏紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", new_generic_name, MAX_GENERIC_NAME_LEN);
    if (strcmp(new_generic_name, "B") == 0 || strcmp(new_generic_name, "b") == 0)
    {
        return;
    }

    // 4. 鍒悕锛堥€夊～锛岃緭鍏?B 杩斿洖涓婁竴绾э紝绌哄瓧绗︿覆琛ㄧず娓呯┖锛?
    get_safe_string("璇疯緭鍏ユ柊鍒悕锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э紝杈撳叆绌哄瓧绗︿覆娓呯┖鍒悕锛? ", new_alias, MAX_ALIAS_LEN);
    if (strcmp(new_alias, "B") == 0 || strcmp(new_alias, "b") == 0)
    {
        return;
    }
    if (!is_blank_string(new_alias))
    {
        alias_ptr = new_alias;
    }

    // 5. 鍗曚环锛堥€夊～锛?1 琛ㄧず涓嶄慨鏀癸級
    while (1)
    {
        char price_str[32];
        get_safe_string("璇疯緭鍏ユ柊鍗曚环锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0)
        {
            return;
        }
        if (is_blank_string(price_str))
        {
            new_price = -1.0; // 淇濇寔涓嶅彉
            break;
        }
        if (sscanf(price_str, "%lf", &new_price) != 1 || new_price <= 0)
        {
            printf("鍗曚环蹇呴』鏄ぇ浜?0 鐨勬暟瀛楋紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    // 6. 鏁堟湡锛堥€夊～锛岀┖鐧藉瓧绗︿覆琛ㄧず涓嶄慨鏀癸級
    get_safe_string("璇疯緭鍏ユ柊鏁堟湡锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", new_expiry_date, MAX_DATE_LEN);
    if (strcmp(new_expiry_date, "B") == 0 || strcmp(new_expiry_date, "b") == 0)
    {
        return;
    }

    // 璋冪敤 update_medicine_basic_info 鍑芥暟
    update_medicine_basic_info(
        med_id,
        is_blank_string(new_name) ? NULL : new_name,
        alias_ptr,
        is_blank_string(new_generic_name) ? NULL : new_generic_name,
        new_price,
        is_blank_string(new_expiry_date) ? NULL : new_expiry_date
    );
}

void handle_medicine_stock_update(void)
{
    char med_id[MAX_ID_LEN];
    int new_stock;

    printf("\n================ 淇敼鑽搧搴撳瓨 ===============-\n");
    printf("鎻愮ず锛氳緭鍏?'B' 鍙繑鍥炰笂涓€绾ц彍鍗昞n\n");

    while (1)
    {
        get_safe_string("璇疯緭鍏ヨ嵂鍝佺紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", med_id, MAX_ID_LEN);
        if (strcmp(med_id, "B") == 0 || strcmp(med_id, "b") == 0)
        {
            return;
        }
        if (is_blank_string(med_id))
        {
            printf("鑽搧缂栧彿涓嶈兘涓虹┖锛岃閲嶆柊杈撳叆\n");
            continue;
        }
        break;
    }

    while (1)
    {
        char stock_str[32];
        get_safe_string("璇疯緭鍏ユ柊搴撳瓨鏁伴噺锛堣緭鍏?B 杩斿洖涓婁竴绾э級: ", stock_str, sizeof(stock_str));
        if (strcmp(stock_str, "B") == 0 || strcmp(stock_str, "b") == 0)
        {
            return;
        }
        if (sscanf(stock_str, "%d", &new_stock) != 1 || new_stock < 0)
        {
            printf("搴撳瓨鏁伴噺蹇呴』鏄ぇ浜庣瓑浜?0 鐨勬暣鏁帮紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    update_medicine_stock(med_id, new_stock);
}

void handle_medicine_search(void)
{
    char keyword[MAX_MED_NAME_LEN];

    printf("\n================ 鏌ヨ鑽搧 ================\n");
    printf("杈撳叆鑽搧缂栧彿鎴栧叧閿瘝鏌ヨ锛岃緭鍏?Q 閫€鍑篭n\n");

    while (1)
    {
        printf("璇疯緭鍏ヨ嵂鍝佺紪鍙锋垨鍏抽敭璇? ");
        fflush(stdout);
        if (fgets(keyword, MAX_MED_NAME_LEN, stdin) == NULL) break;
        keyword[strcspn(keyword, "\n")] = '\0';

        if (keyword[0] == '\0') continue;

        if (my_strcasecmp(keyword, "Q") == 0)
        {
            printf("宸查€€鍑鸿嵂鍝佹煡璇n");
            break;
        }

        search_medicine_by_keyword(keyword);
        printf("------------------------------------------\n");
    }
}

void handle_medicine_show_all(void)
{
    show_all_medicines();
}

void handle_medicine_low_stock(void)
{
    int threshold;

    printf("\n================ 浣庡簱瀛樿嵂鍝?===============-\n");
    threshold = get_safe_int("璇疯緭鍏ヤ綆搴撳瓨闃堝€? ");
    show_low_stock_medicines(threshold);
}

void handle_medicine_expiring(void)
{
    char today[MAX_DATE_LEN];
    int days_threshold;
    time_t now;
    struct tm* local_time;

    time(&now);
    local_time = localtime(&now);
    strftime(today, sizeof(today), "%Y-%m-%d", local_time);

    printf("\n褰撳墠绯荤粺鏃ユ湡锛?s\n", today);
    days_threshold = get_safe_int("璇疯緭鍏ヨ繎鏁堟湡澶╂暟闃堝€? ");
    show_expiring_medicines(today, days_threshold);
}

void handle_medicine_dispense(void)
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 鍙戣嵂澶勭悊 ================\n");
    printf("鎻愮ず锛氳緭鍏?'0' 鍙互鍥為€€涓婁竴姝ワ紝杈撳叆 '00' 鍙互閫€鍑烘搷浣淺n");
    show_paid_patients_waiting_for_dispense();
    printf("------------------------------------------------------\n");
    
    while (1)
    {
        get_safe_string("璇疯緭鍏ヨ鍙戣嵂鐨勬偅鑰呯紪鍙? ", patient_id, MAX_ID_LEN);
        
        // 妫€鏌ユ槸鍚﹂€€鍑?
        if (strcmp(patient_id, "00") == 0)
        {
            printf("鎿嶄綔鍙栨秷锛乗n");
            return;
        }
        
        // 妫€鏌ユ槸鍚﹀洖閫€
        if (strcmp(patient_id, "0") == 0)
        {
            return;
        }
        
        // 妫€鏌ユ偅鑰呯紪鍙锋牸寮忔槸鍚﹀悎娉?
        if (validate_patient_id(patient_id))
        {
            break;
        }
        else
        {
            printf("鈿狅笍 鎮ｈ€呯紪鍙锋牸寮忎笉鍚堟硶锛屾纭牸寮忎负 P-1001锛岃閲嶆柊杈撳叆锛乗n");
            printf("鎻愮ず锛氳緭鍏?'0' 鍙互鍥為€€涓婁竴姝ワ紝杈撳叆 '00' 鍙互閫€鍑烘搷浣淺n");
        }
    }
    
    dispense_medicine_for_patient(patient_id);
}

void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];
    MedicineNode* medicine = NULL;
    int confirm;

    printf("\n================ 涓嬫灦鑽搧 ===============-\n");
    get_safe_string("璇疯緭鍏ヨ涓嬫灦鐨勮嵂鍝佺紪鍙? ", med_id, MAX_ID_LEN);

    // 鏌ユ壘鑽搧鏄惁瀛樺湪
    medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌瀵瑰簲鑽搧锛屼笅鏋跺け璐ャ€俓n");
        return;
    }

    // 鏄剧ず鑽搧鍩烘湰淇℃伅
    printf("\n褰撳墠鑽搧淇℃伅锛歕n");
    printf("鑽搧缂栧彿锛?s\n", medicine->id);
    printf("鍟嗗搧鍚嶏細%s\n", medicine->name);
    printf("閫氱敤鍚嶏細%s\n", medicine->generic_name);
    printf("鍒悕锛?s\n", medicine->alias[0] == '\0' ? "鏃? : medicine->alias);
    printf("鍗曚环锛?.2f\n", medicine->price);
    printf("搴撳瓨锛?d\n", medicine->stock);
    // 鏄剧ず鍖讳繚绫诲瀷
    switch (medicine->m_type)
    {
        case MEDICARE_NONE:
            printf("鍖讳繚绫诲瀷锛氶潪鍖讳繚\n");
            break;
        case MEDICARE_CLASS_A:
            printf("鍖讳繚绫诲瀷锛氱敳绫诲尰淇漒n");
            break;
        case MEDICARE_CLASS_B:
            printf("鍖讳繚绫诲瀷锛氫箼绫诲尰淇漒n");
            break;
        default:
            printf("鍖讳繚绫诲瀷锛氭湭鐭n");
            break;
    }
    printf("鏁堟湡锛?s\n", medicine->expiry_date);
    printf("----------------------------------------\n");

    // 浜屾纭
    printf("鏄惁纭畾涓嬫灦璇ヨ嵂鍝侊紵\n");
    printf("[1] 纭畾涓嬫灦\n");
    printf("[0] 鍙栨秷杩斿洖\n");
    confirm = get_safe_int("璇疯緭鍏ユ搷浣滅紪鍙? ");

    if (confirm != 1)
    {
        printf("鎻愮ず锛氬凡鍙栨秷涓嬫灦鎿嶄綔銆俓n");
        return;
    }

    // 鎵ц涓嬫灦鎿嶄綔
    remove_medicine(med_id);
}

// 鍒濆鍖栨棩蹇楀垪琛?
void init_log_list(void)
{
    if (g_log_list == NULL)
    {
        g_log_list = (LogNode*)malloc(sizeof(LogNode));
        if (g_log_list != NULL)
        {
            g_log_list->next = NULL;
        }
    }
}

// 娣诲姞鏃ュ織
void add_log(const char* operation, const char* target, const char* description)
{
    if (operation == NULL || target == NULL || description == NULL)
        return;
    
    // 鍒濆鍖栨棩蹇楅摼琛紙濡傛灉鏈垵濮嬪寲锛?
    if (g_log_list == NULL)
    {
        init_log_list();
    }
    
    // 鍒涘缓鏂版棩蹇楄妭鐐?
    LogNode* new_node = (LogNode*)malloc(sizeof(LogNode));
    if (new_node == NULL)
        return;
    
    // 璁剧疆鏃ュ織鏃堕棿
    time_t now = time(NULL);
    struct tm now_tm;
    localtime_s(&now_tm, &now);
    snprintf(new_node->timestamp, sizeof(new_node->timestamp), 
             "%d-%02d-%02d %02d:%02d:%02d",
             now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
             now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    
    // 璁剧疆鏃ュ織鍐呭
    strncpy(new_node->operation, operation, sizeof(new_node->operation) - 1);
    new_node->operation[sizeof(new_node->operation) - 1] = '\0';
    
    strncpy(new_node->target, target, sizeof(new_node->target) - 1);
    new_node->target[sizeof(new_node->target) - 1] = '\0';
    
    strncpy(new_node->description, description, sizeof(new_node->description) - 1);
    new_node->description[sizeof(new_node->description) - 1] = '\0';
    
    new_node->next = NULL;
    
    // 娣诲姞鍒伴摼琛ㄥ熬閮?
    LogNode* curr = g_log_list;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_node;
}

// 鏄剧ず鏃ュ織
void show_logs(void)
{
    printf("\n==============================================================\n");
    printf("                      绯荤粺鏃ュ織\n");
    printf("==============================================================\n");
    
    if (g_log_list == NULL || g_log_list->next == NULL)
    {
        printf("褰撳墠鏃犳棩蹇楄褰昞n");
        printf("==============================================================\n");
        return;
    }
    printf("\n==============================================================\n");
    printf("                        绯荤粺鏃ュ織\n");
    printf("==============================================================\n");
    print_field("鏃堕棿", 20); print_field("鎿嶄綔", 16); print_field("鐩爣", 8); printf("鎻忚堪\n");
    printf("--------------------------------------------------------------\n");

    LogNode* curr = g_log_list->next;
    int count = 0;
    
    while (curr != NULL)
    {
        printf("%-20s", curr->timestamp);
        print_field(curr->operation, 16);
        printf("%-8s %s\n", curr->target, curr->description);
        count++;
        curr = curr->next;
    }
    
    printf("--------------------------------------------------------------\n");
    printf("鏃ュ織鎬绘暟锛?d\n", count);
    printf("==============================================================\n");
}

// 澶勭悊鑽搧鍩烘湰淇℃伅鏇存柊
void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char new_alias[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    double new_price;
    char new_expiry_date[MAX_DATE_LEN];
    
    printf("\n==============================================================\n");
    printf("                      鏇存柊鑽搧鍩烘湰淇℃伅\n");
    printf("==============================================================\n");
    
    get_safe_string("璇疯緭鍏ヨ嵂鍝佺紪鍙凤細", med_id, sizeof(med_id));
    
    // 妫€鏌ヨ嵂鍝佹槸鍚﹀瓨鍦?
    MedicineNode* medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("鑽搧涓嶅瓨鍦紒\n");
        printf("==============================================================\n");
        return;
    }
    
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝佸悕绉帮細", new_name, sizeof(new_name));
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝佸埆鍚嶏細", new_alias, sizeof(new_alias));
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝侀€氱敤鍚嶏細", new_generic_name, sizeof(new_generic_name));
    new_price = get_safe_double("璇疯緭鍏ユ柊鐨勮嵂鍝佷环鏍硷細");
    get_safe_string("璇疯緭鍏ユ柊鐨勬湁鏁堟湡锛圷YYY-MM-DD锛夛細", new_expiry_date, sizeof(new_expiry_date));
    
    // 璋冪敤鏇存柊鑽搧鍩烘湰淇℃伅鍑芥暟
    int result = update_medicine_basic_info(
        med_id, new_name, new_alias, new_generic_name, new_price, new_expiry_date
    );
    
    if (result == 1)
    {
        printf("\n鑽搧鍩烘湰淇℃伅鏇存柊鎴愬姛锛乗n");
        add_log("鑽搧淇℃伅鏇存柊", med_id, "鏇存柊鑽搧鍩烘湰淇℃伅");
    }
    else
    {
        printf("\n鑽搧鍩烘湰淇℃伅鏇存柊澶辫触锛乗n");
        add_log("鑽搧淇℃伅鏇存柊", med_id, "鏇存柊鑽搧鍩烘湰淇℃伅澶辫触");
    }
    
    printf("==============================================================\n");
}

// 澶勭悊杩戞晥鏈熻嵂鍝佹鏌?
void handle_expiring_medicine_check(void)
{
    char today[MAX_DATE_LEN];
    int days_threshold;
    
    printf("\n==============================================================\n");
    printf("                      杩戞晥鏈熻嵂鍝佹鏌n");
    printf("==============================================================\n");
    
    get_safe_string("璇疯緭鍏ュ綋鍓嶆棩鏈燂紙YYYY-MM-DD锛夛細", today, sizeof(today));
    days_threshold = get_safe_int("璇疯緭鍏ラ璀﹀ぉ鏁帮細");
    
    show_expiring_medicines(today, days_threshold);
    
    add_log("杩戞晥鏈熸鏌?, today, "鎵ц杩戞晥鏈熻嵂鍝佹鏌?);
    printf("==============================================================\n");
}

// 鏄剧ず鎵€鏈夋鏌ラ」鐩瓧鍏?
void show_all_check_items(void)
{
    CheckItemNode* curr = NULL;
    int count = 0;

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤妫€鏌ラ」鐩暟鎹甛n");
        return;
    }

    printf("\n==============================================================\n");
    printf("                    妫€鏌ラ」鐩瓧鍏竆n");
    printf("==============================================================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "椤圭洰缂栧彿", "椤圭洰鍚嶇О", "鎵€灞炵瀹?, "浠锋牸(鍏?", "鍖讳繚绫诲瀷");
    printf("--------------------------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        const char* m_type_name = NULL;
        switch (curr->m_type)
        {
            case MEDICARE_CLASS_A:
                m_type_name = "鐢茬被";
                break;
            case MEDICARE_CLASS_B:
                m_type_name = "涔欑被";
                break;
            case MEDICARE_NONE:
                m_type_name = "鑷垂";
                break;
            default:
                m_type_name = "鏈煡";
                break;
        }
        printf("%-10s %-20s %-12s %10.2f %8s\n",
               curr->item_id,
               curr->item_name,
               curr->dept,
               curr->price,
               m_type_name);
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("妫€鏌ラ」鐩€绘暟锛?d\n", count);
    printf("==============================================================\n");
}

static void generate_check_item_id(char* new_id)
{
    int max_no = 0;
    int current_no = 0;
    CheckItemNode* curr = NULL;

    if (new_id == NULL) return;

    if (g_check_item_list != NULL)
    {
        curr = g_check_item_list->next;
        while (curr != NULL)
        {
            if (strncmp(curr->item_id, "C-", 2) == 0)
            {
                current_no = atoi(curr->item_id + 2);
                if (current_no > max_no) max_no = current_no;
            }
            curr = curr->next;
        }
    }

    snprintf(new_id, MAX_ID_LEN, "C-%03d", max_no + 1);
}

static int is_check_item_referenced_by_active_record(const char* item_id)
{
    if (is_blank_string(item_id) || g_check_record_list == NULL) return 0;

    CheckRecordNode* curr = g_check_record_list->next;
    while (curr != NULL)
    {
        if (curr->is_completed == 0 && strcmp(curr->item_id, item_id) == 0)
            return 1;
        curr = curr->next;
    }
    return 0;
}

void search_check_item_by_keyword(const char* keyword)
{
    int found = 0;
    CheckItemNode* curr = NULL;

    if (is_blank_string(keyword))
    {
        printf("鎻愮ず锛氭煡璇㈠叧閿瓧涓嶈兘涓虹┖銆俓n");
        return;
    }

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("鏈壘鍒板尮閰嶆鏌ラ」鐩甛n");
        return;
    }

    printf("\n================ 妫€鏌ラ」鐩煡璇㈢粨鏋?================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "椤圭洰缂栧彿", "椤圭洰鍚嶇О", "鎵€灞炵瀹?, "浠锋牸(鍏?", "鍖讳繚绫诲瀷");
    printf("------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        if (strstr(curr->item_id, keyword) != NULL ||
            strstr(curr->item_name, keyword) != NULL ||
            strstr(curr->dept, keyword) != NULL)
        {
            const char* m_type_name = NULL;
            switch (curr->m_type)
            {
                case MEDICARE_CLASS_A: m_type_name = "鐢茬被"; break;
                case MEDICARE_CLASS_B: m_type_name = "涔欑被"; break;
                case MEDICARE_NONE:    m_type_name = "鑷垂"; break;
                default:               m_type_name = "鏈煡"; break;
            }
            printf("%-10s %-20s %-12s %10.2f %8s\n",
                   curr->item_id, curr->item_name, curr->dept,
                   curr->price, m_type_name);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found) printf("鏈壘鍒板尮閰嶆鏌ラ」鐩甛n");
}

void handle_check_item_register(void)
{
    char item_name[MAX_NAME_LEN];
    char dept[MAX_NAME_LEN];
    double price;
    int medicare_type;
    char new_id[MAX_ID_LEN];

    printf("\n================ 鏂板妫€鏌ラ」鐩?===============-\n");
    printf("鎻愮ず锛氫换鎰忚緭鍏ラ」杈撳叆 'B' 鍙繑鍥炰笂涓€绾ц彍鍗昞n\n");

    while (1)
    {
        if (!get_form_string("璇疯緭鍏ユ鏌ラ」鐩悕绉帮紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", item_name, MAX_NAME_LEN))
            return;
        if (is_blank_string(item_name))
        {
            printf("妫€鏌ラ」鐩悕绉颁笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_string("璇疯緭鍏ユ墍灞炵瀹わ紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", dept, MAX_NAME_LEN))
            return;
        if (is_blank_string(dept))
        {
            printf("鎵€灞炵瀹や笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_double("璇疯緭鍏ユ鏌ヨ垂鐢紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", &price, 0, "妫€鏌ヨ垂鐢ㄥ繀椤诲ぇ浜?0锛岃閲嶆柊杈撳叆\n"))
            return;
        break;
    }

    printf("鍖讳繚绫诲瀷锛?=鑷垂 1=鐢茬被 2=涔欑被\n");
    while (1)
    {
        if (!get_form_int("璇疯緭鍏ュ尰淇濈被鍨嬬紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", &medicare_type, 0, 2, "鍖讳繚绫诲瀷杈撳叆闈炴硶锛岃閲嶆柊杈撳叆\n"))
            return;
        break;
    }

    if (g_check_item_list == NULL)
    {
        printf("鎻愮ず锛氭鏌ラ」鐩摼琛ㄥ皻鏈垵濮嬪寲锛屾棤娉曟柊澧炪€俓n");
        return;
    }

    generate_check_item_id(new_id);
    CheckItemNode* new_node = create_check_item_node(new_id, item_name, dept, price, (MedicareType)medicare_type);
    if (new_node == NULL)
    {
        printf("鎻愮ず锛氭鏌ラ」鐩妭鐐瑰垱寤哄け璐ャ€俓n");
        return;
    }

    insert_check_item_tail(g_check_item_list, new_node);
    printf("妫€鏌ラ」鐩柊澧炴垚鍔燂紒\n");
    printf("椤圭洰缂栧彿锛?s\n", new_node->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", new_node->item_name);
    printf("鎵€灞炵瀹わ細%s\n", new_node->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", new_node->price);
    printf("鍖讳繚绫诲瀷锛?s\n",
           new_node->m_type == MEDICARE_CLASS_A ? "鐢茬被" :
           new_node->m_type == MEDICARE_CLASS_B ? "涔欑被" : "鑷垂");
}

void handle_check_item_update(void)
{
    char item_id[MAX_ID_LEN];
    char new_name[MAX_NAME_LEN];
    char new_dept[MAX_NAME_LEN];
    double new_price = -1.0;
    int new_m_type = -1;

    printf("\n================ 淇敼妫€鏌ラ」鐩俊鎭?===============-\n");

    CheckItemNode* target = NULL;
    while (1)
    {
        get_safe_string("璇疯緭鍏ユ鏌ラ」鐩紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", item_id, MAX_ID_LEN);
        if (strcmp(item_id, "B") == 0 || strcmp(item_id, "b") == 0) return;
        if (is_blank_string(item_id))
        {
            printf("妫€鏌ラ」鐩紪鍙蜂笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        target = find_check_item_by_id(g_check_item_list, item_id);
        if (target == NULL)
        {
            printf("鏈壘鍒扮紪鍙蜂负 %s 鐨勬鏌ラ」鐩紝淇敼娴佺▼缁撴潫\n", item_id);
            return;
        }
        break;
    }

    printf("\n------------- 褰撳墠妫€鏌ラ」鐩俊鎭?------------\n");
    printf("椤圭洰缂栧彿锛?s\n", target->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", target->item_name);
    printf("鎵€灞炵瀹わ細%s\n", target->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("鍖讳繚绫诲瀷锛氱敳绫籠n"); break;
        case MEDICARE_CLASS_B: printf("鍖讳繚绫诲瀷锛氫箼绫籠n"); break;
        case MEDICARE_NONE:    printf("鍖讳繚绫诲瀷锛氳嚜璐筡n"); break;
    }
    printf("----------------------------------------\n");

    get_safe_string("璇疯緭鍏ユ柊椤圭洰鍚嶇О锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", new_name, MAX_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0) return;

    get_safe_string("璇疯緭鍏ユ柊鎵€灞炵瀹わ紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", new_dept, MAX_NAME_LEN);
    if (strcmp(new_dept, "B") == 0 || strcmp(new_dept, "b") == 0) return;

    while (1)
    {
        char price_str[32];
        get_safe_string("璇疯緭鍏ユ柊妫€鏌ヨ垂鐢紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0) return;
        if (is_blank_string(price_str)) break;
        char* endptr = NULL;
        new_price = strtod(price_str, &endptr);
        if (endptr == price_str || *endptr != '\0' || new_price <= 0)
        {
            printf("妫€鏌ヨ垂鐢ㄥ繀椤讳负澶т簬 0 鐨勬暟瀛楋紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    printf("鍖讳繚绫诲瀷锛?=鑷垂 1=鐢茬被 2=涔欑被锛?1 涓嶄慨鏀癸級\n");
    while (1)
    {
        char type_str[32];
        get_safe_string("璇疯緭鍏ユ柊鍖讳繚绫诲瀷锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", type_str, sizeof(type_str));
        if (strcmp(type_str, "B") == 0 || strcmp(type_str, "b") == 0) return;
        if (is_blank_string(type_str)) break;
        char* endptr = NULL;
        new_m_type = (int)strtol(type_str, &endptr, 10);
        if (endptr == type_str || *endptr != '\0' || new_m_type < 0 || new_m_type > 2)
        {
            printf("鍖讳繚绫诲瀷蹇呴』涓?0/1/2锛岃閲嶆柊杈撳叆\n");
            continue;
        }
        break;
    }

    if (!is_blank_string(new_name)) strncpy(target->item_name, new_name, MAX_NAME_LEN - 1);
    if (!is_blank_string(new_dept)) strncpy(target->dept, new_dept, MAX_NAME_LEN - 1);
    if (new_price > 0) target->price = new_price;
    if (new_m_type >= 0) target->m_type = (MedicareType)new_m_type;

    printf("\n鎻愮ず锛氭鏌ラ」鐩俊鎭慨鏀规垚鍔熴€俓n");
}

void handle_check_item_remove(void)
{
    char item_id[MAX_ID_LEN];
    CheckItemNode* target = NULL;
    int confirm;

    printf("\n================ 涓嬫灦妫€鏌ラ」鐩?===============-\n");
    get_safe_string("璇疯緭鍏ヨ涓嬫灦鐨勬鏌ラ」鐩紪鍙? ", item_id, MAX_ID_LEN);

    target = find_check_item_by_id(g_check_item_list, item_id);
    if (target == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌瀵瑰簲妫€鏌ラ」鐩紝涓嬫灦澶辫触銆俓n");
        return;
    }

    printf("\n褰撳墠妫€鏌ラ」鐩俊鎭細\n");
    printf("椤圭洰缂栧彿锛?s\n", target->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", target->item_name);
    printf("鎵€灞炵瀹わ細%s\n", target->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("鍖讳繚绫诲瀷锛氱敳绫籠n"); break;
        case MEDICARE_CLASS_B: printf("鍖讳繚绫诲瀷锛氫箼绫籠n"); break;
        case MEDICARE_NONE:    printf("鍖讳繚绫诲瀷锛氳嚜璐筡n"); break;
    }
    printf("----------------------------------------\n");

    if (is_check_item_referenced_by_active_record(item_id))
    {
        printf("鎻愮ず锛氬綋鍓嶆鏌ラ」鐩粛琚湭瀹屾垚妫€鏌ヨ褰曞紩鐢紝鏆備笉鑳戒笅鏋躲€俓n");
        printf("璇峰厛瀹屾垚鐩稿叧鎮ｈ€呯殑妫€鏌ユ祦绋嬪悗鍐嶉噸璇曘€俓n");
        return;
    }

    printf("鏄惁纭畾涓嬫灦璇ユ鏌ラ」鐩紵\n");
    printf("[1] 纭畾涓嬫灦\n");
    printf("[0] 鍙栨秷杩斿洖\n");
    confirm = get_safe_int("璇疯緭鍏ユ搷浣滅紪鍙? ");

    if (confirm != 1)
    {
        printf("鎻愮ず锛氬凡鍙栨秷涓嬫灦鎿嶄綔銆俓n");
        return;
    }

    if (delete_check_item_by_id(g_check_item_list, item_id))
        printf("鎻愮ず锛氭鏌ラ」鐩笅鏋舵垚鍔熴€俓n");
    else
        printf("鎻愮ず锛氭鏌ラ」鐩笅鏋跺け璐ャ€俓n");
}

