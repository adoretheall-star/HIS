// ==========================================
// �ļ���: patient_service.c
// ����: �������ҵ������ / �������ݷ����ʵ��
// ����: ʵ�ֻ�����Ϣ�����ĺ���ҵ���߼�������ʿ�˺ͻ��߶˹�ͬ����
// ˵��: ��ģ��ֻ����ҵ���߼�ʵ�֣��������˵��ͽ����߼�
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

// ���ܷ���Ȩ�ع���ṹ��
typedef struct {
    const char* keyword; // ֢״�ؼ���
    int weight;          // Ȩ�� (1000:һƱ���, 100:��ָ֢��, 50:�ж�/����, 10:��֢ͨ״)
    const char* dept;    // �Ƽ�����
} SymptomRule;

/*
 * ���ܷ����������̶��㷨���򣬲�����ҵ�����ݴ洢��
 * ʵ�ʻ��ߡ�ҽ����ԤԼ��ҩƷ��ҵ�����ݾ�ʹ������������
 */
// ȫ��֢״Ȩ���ֵ䣨�𲽹�����...��
static const SymptomRule g_symptom_dict[] = {
    // ?? 1. ��Σ�أ�1000�֣�һƱ��������
    {"����", 1000, "�����"}, {"����", 1000, "�����"}, {"����", 1000, "�����"}, {"�ݿ�", 1000, "�����"},
    {"���Ѫ", 1000, "�����"}, {"����ֹͣ", 1000, "�����"}, {"����", 1000, "�����"}, {"��ͨ�¹�", 1000, "�����"},
    {"׹��", 1000, "�����"}, {"����", 1000, "�����"}, {"����", 1000, "�����"}, {"�ж�", 1000, "�����"},
    {"�������ݿ�", 1000, "�����"}, {"������ʹ", 1000, "�����"},

    // ?? 2. ��ָ֢��100�֣�
    {"�Ĺ�", 100, "��Ѫ���ڿ�"}, {"�ļ�����", 100, "��Ѫ���ڿ�"}, {"��˥", 100, "��Ѫ���ڿ�"}, {"�Ĺ��ܲ�ȫ", 100, "��Ѫ���ڿ�"},
    {"�з�", 100, "���ڿ�"}, {"����Ѫ", 100, "���ڿ�"}, {"�Գ�Ѫ", 100, "���ڿ�"}, {"�Թ���", 100, "���ڿ�"},
    {"�Թ���", 100, "���ڿ�"}, {"̱��", 100, "���ڿ�"}, {"���", 100, "���ڿ�"},
    {"��֢", 100, "���ڿ�"}, {"��˥", 100, "���ڿ�"}, {"������˥", 100, "���ڿ�"}, {"������˥", 100, "���ڿ�"},
    {"����", 100, "�ǿ�"}, {"������", 100, "������"}, {"����", 100, "������"}, {"����", 100, "������"},
    {"���̲�", 100, "��Ⱦ��"}, {"÷��", 100, "��Ⱦ��"}, {"�ܲ�", 100, "��Ⱦ��"}, {"����ʪ��", 100, "��Ⱦ��"},
    {"�������", 100, "�����"}, {"����", 100, "�����"},
    {"����", 100, "������"}, {"��֢", 100, "������"}, {"��", 100, "������"}, {"����", 100, "������"},
    {"����", 100, "������"}, {"����", 100, "������"}, {"ת��", 100, "������"}, {"����", 100, "������"},
    {"��β��", 100, "��ͨ���"}, {"�ΰ�", 100, "�������"}, {"ʳ�ܰ�", 100, "�������"}, {"�ΰ�", 100, "�ε����"},

    // ?? 3. ����/�ж�֢״��50�֣�
    {"��������", 50, "�����"}, {"��������", 50, "�����ڿ�"}, {"��������", 50, "�����"}, {"��������", 50, "�����ڿ�"},
    {"��ʹ", 50, "�����"}, {"��ʹ", 50, "��Ѫ���ڿ�"}, {"��ʹ", 50, "�����ڿ�"},
    {"����", 50, "��Ѫ���ڿ�"}, {"����", 50, "�����ڿ�"}, {"��ʹ", 50, "�����"}, {"��ʹ", 50, "�����ڿ�"},
    {"ͷ��", 50, "���ڿ�"}, {"ѣ��", 50, "���ڿ�"}, {"����", 50, "�����"}, {"����", 50, "Ƥ����"},
    {"����", 50, "�ǿ�"}, {"�Ѿ�", 50, "�ǿ�"}, {"�Ѿ�", 50, "���"}, {"ˤ��", 50, "�����"}, {"ˤ��", 50, "���"},
    {"��������", 50, "�����"}, {"��������", 50, "��Ѫ���ڿ�"}, {"��Ѫѹ", 50, "��Ѫ���ڿ�"}, {"��Ѫѹ", 50, "��Ѫ���ڿ�"},
    {"���Ĳ�", 50, "��Ѫ���ڿ�"}, {"�Ľ�ʹ", 50, "��Ѫ���ڿ�"}, {"���ɲ���", 50, "��Ѫ���ڿ�"}, {"����ʧ��", 50, "��Ѫ���ڿ�"},
    {"����", 50, "��Ѫ���ڿ�"}, {"�粫", 50, "��Ѫ���ڿ�"}, {"�ļ���", 50, "��Ѫ���ڿ�"}, {"�ļ���", 50, "��Ѫ���ڿ�"},
    {"�����Ĥ", 50, "��Ѫ���ڿ�"}, {"�ļ�", 50, "��Ѫ���ڿ�"},
    {"����", 50, "�����ڿ�"}, {"����", 50, "�����ڿ�"}, {"֧������", 50, "�����ڿ�"}, {"����֧������", 50, "�����ڿ�"},
    {"֧��������", 50, "�����ڿ�"}, {"��Ѫ", 50, "�����ڿ�"}, {"�ν��", 50, "�����ڿ�"}, {"���", 50, "��Ⱦ��"},
    {"�ν��", 50, "�����ڿ�"}, {"������", 50, "�����ڿ�"}, {"�����", 50, "�����ڿ�"}, {"���Ĳ�", 50, "�����ڿ�"},
    {"θ����", 50, "�����ڿ�"}, {"θʹ", 50, "�����ڿ�"}, {"ʮ��ָ������", 50, "�����ڿ�"}, {"�᳦��", 50, "�����ڿ�"}, {"���׼��ۺ���", 50, "�����ڿ�"},
    {"��Ѫ", 50, "�����ڿ�"}, {"����", 50, "�����ڿ�"}, {"����", 50, "�����ڿ�"}, {"����", 50, "��Ⱦ��"},
    {"�׸�", 50, "��Ⱦ��"}, {"�Ҹ�", 50, "��Ⱦ��"}, {"����", 50, "��Ⱦ��"}, {"��Ӳ��", 50, "�����ڿ�"},
    {"��Ӳ��", 50, "�ε����"}, {"������", 50, "�����ڿ�"}, {"������", 50, "�ε����"}, {"����ʯ", 50, "�����ڿ�"},
    {"����ʯ", 50, "�ε����"}, {"������", 50, "�����ڿ�"},
    {"����", 50, "���ڿ�"}, {"��������", 50, "���ڿ�"}, {"��������", 50, "���ڿ�"}, {"����", 50, "���ڿ�"},
    {"�����ۺ���", 50, "���ڿ�"}, {"������", 50, "���ڿ�"}, {"Ѫ��", 50, "���ڿ�"}, {"����ˮ", 50, "���ڿ�"},
    {"����ʯ", 50, "���ڿ�"}, {"����ʯ", 50, "�������"}, {"��ʯ", 50, "�������"}, {"����ܽ�ʯ", 50, "�������"},
    {"���׽�ʯ", 50, "�������"}, {"��·��ʯ", 50, "�������"},
    {"����", 50, "�ڷ��ڿ�"}, {"�׿�", 50, "�ڷ��ڿ�"}, {"�׼�", 50, "�ڷ��ڿ�"}, {"��״��", 50, "�ڷ��ڿ�"},
    {"��״����", 50, "�ڷ��ڿ�"}, {"��״�ٽ��", 50, "�ڷ��ڿ�"}, {"Ѫ��", 50, "�ڷ��ڿ�"}, {"��Ѫ��", 50, "�ڷ��ڿ�"},
    {"��Ѫ��", 50, "�ڷ��ڿ�"},
    {"����ɭ", 50, "���ڿ�"}, {"����", 50, "���ڿ�"}, {"��Ĥ��", 50, "���ڿ�"}, {"��Ĥ", 50, "�����"},
    {"������", 50, "���ڿ�"}, {"­��", 50, "�����"},
    {"����Ǵ�", 50, "��ʪ���߿�"}, {"�����ۺ���", 50, "��ʪ���߿�"}, {"ǿֱ�Լ�����", 50, "��ʪ���߿�"},
    {"ʹ��", 50, "��ʪ���߿�"}, {"��м���ؽ���", 50, "��ʪ���߿�"},
    {"����֢", 50, "�����"}, {"����֢", 50, "�����"}, {"���", 50, "�����"}, {"����֢", 50, "�����"},
    {"ǿ��֢", 50, "�����"}, {"�־�֢", 50, "�����"},
    {"������", 50, "�ۿ�"}, {"�����", 50, "�ۿ�"}, {"����Ĥ", 50, "�ۿ�"}, {"�ж���", 50, "���Ǻ���"},

    // ?? 4. ��ͨ/����֢״��10�֣�
    {"����", 10, "������"}, {"����", 10, "������"}, {"����", 10, "������"}, {"����", 10, "������"}, {"�¾�", 10, "������"},
    {"ʹ��", 10, "������"}, {"����", 10, "������"}, {"�ӹ�", 10, "������"}, {"�ѳ�", 10, "������"}, {"����", 10, "������"},
    {"����", 10, "������"}, {"��ǻ", 10, "������"}, {"����", 10, "������"}, {"����", 10, "������"}, {"����", 10, "������"},
    {"��ͯ", 10, "����"}, {"С��", 10, "����"}, {"����", 10, "����"}, {"Ӥ��", 10, "����"}, {"�׶�", 10, "����"},
    {"С��", 10, "����"}, {"������", 10, "����"}, {"��ͯ", 10, "����"},
    {"�ؽ�", 10, "�ǿ�"}, {"��׵", 10, "�ǿ�"}, {"��׵", 10, "�ǿ�"}, {"��׵", 10, "�ǿ�"}, {"��������", 10, "�ǿ�"},
    {"�ɹ�ͷ", 10, "�ǿ�"}, {"�ǹؽ���", 10, "�ǿ�"}, {"��ʪ�Թؽ���", 10, "�ǿ�"}, {"���ʪ�ؽ���", 10, "�ǿ�"},
    {"������", 10, "�ǿ�"}, {"��Ĥ��", 10, "�ǿ�"}, {"Ť��", 10, "�ǿ�"}, {"����", 10, "�ǿ�"}, {"������", 10, "�ǿ�"},
    {"��׵��", 10, "�ǿ�"}, {"��׵����ͻ��", 10, "�ǿ�"},
    {"�ļ�", 10, "��Ѫ���ڿ�"}, {"�Ļ�", 10, "��Ѫ���ڿ�"}, {"����", 10, "�����ڿ�"}, {"��̵", 10, "�����ڿ�"},
    {"����", 10, "�����ڿ�"},
    {"θʹ", 10, "�����ڿ�"}, {"θ��", 10, "�����ڿ�"}, {"θ��", 10, "�����ڿ�"}, {"����θ��", 10, "�����ڿ�"},
    {"����θ��", 10, "�����ڿ�"}, {"��к", 10, "�����ڿ�"}, {"����", 10, "�����ڿ�"}, {"��", 10, "�����ڿ�"},
    {"����", 10, "�����ڿ�"}, {"����", 10, "�����ڿ�"}, {"����", 10, "�����ڿ�"}, {"Ż��", 10, "�����ڿ�"},
    {"ʳ������", 10, "�����ڿ�"}, {"��������", 10, "�����ڿ�"},
    {"ͷʹ", 10, "���ڿ�"}, {"ʧ��", 10, "���ڿ�"}, {"��ľ", 10, "���ڿ�"}, {"��", 10, "���ڿ�"},
    {"����", 10, "���ڿ�"}, {"��ʹ", 10, "���ڿ�"}, {"������ʹ", 10, "���ڿ�"}, {"������ʹ", 10, "���ڿ�"},
    {"�������½�", 10, "���ڿ�"}, {"ע����������", 10, "���ڿ�"},
    {"����", 10, "�ڷ��ڿ�"}, {"����", 10, "�ڷ��ڿ�"}, {"����", 10, "�ڷ��ڿ�"}, {"����", 10, "�ڷ��ڿ�"},
    {"����", 10, "�ڷ��ڿ�"}, {"��ʳ", 10, "�ڷ��ڿ�"}, {"�ڿ�", 10, "�ڷ��ڿ�"}, {"������", 10, "�ڷ��ڿ�"}, {"����", 10, "�ڷ��ڿ�"},
    {"��Ƶ", 10, "�������"}, {"��", 10, "�������"}, {"��ʹ", 10, "�������"}, {"ǰ����", 10, "�������"},
    {"ǰ������", 10, "�������"}, {"ǰ��������", 10, "�������"}, {"����", 10, "�������"}, {"��й", 10, "�������"},
    {"��Ƥ", 10, "�������"}, {"����", 10, "�������"}, {"غ��", 10, "�������"}, {"غ����", 10, "�������"},
    {"����", 10, "�������"}, {"������������", 10, "�������"}, {"����", 10, "�������"}, {"������", 10, "�������"},
    {"�����", 10, "�������"}, {"urinary tract infection", 10, "�������"},
    {"�׿�", 10, "������"}, {"���", 10, "������"}, {"��", 10, "������"}, {"����", 10, "������"},
    {"��ʪ", 10, "��ʪ���߿�"}, {"���ʪ", 10, "��ʪ���߿�"}, {"�ؽ���", 10, "��ʪ���߿�"}, {"����", 10, "��ʪ���߿�"},
    {"����ϵͳ", 10, "��ʪ���߿�"},
    {"��Ⱦ", 10, "��Ⱦ��"}, {"����", 10, "��Ⱦ��"}, {"ϸ��", 10, "��Ⱦ��"}, {"֧ԭ��", 10, "��Ⱦ��"},
    {"��ԭ��", 10, "��Ⱦ��"}, {"�����Ⱦ", 10, "��Ⱦ��"},
    {"����", 10, "�����"}, {"����", 10, "�����"}, {"����", 10, "�����"}, {"����", 10, "�����"},
    {"������ѯ", 10, "�����"}, {"����", 10, "�����"}, {"��������", 10, "�����"}, {"��˥��", 10, "�����"}, {"ʧ��֢", 10, "�����"},
    {"����", 10, "����ҽѧ��"}, {"����", 10, "����ҽѧ��"}, {"���", 10, "����ҽѧ��"}, {"����", 10, "����ҽѧ��"},
    {"��Ħ", 10, "����ҽѧ��"}, {"����ѵ��", 10, "����ҽѧ��"}, {"��������", 10, "����ҽѧ��"}, {"��������", 10, "����ҽѧ��"},
    {"��������", 10, "����ҽѧ��"}, {"��ҵ����", 10, "����ҽѧ��"}, {"�˶�����", 10, "����ҽѧ��"},
    {"����", 10, "��ͨ���"}, {"�̴�", 10, "��ͨ���"}, {"֬����", 10, "��ͨ���"}, {"ŧ��", 10, "��ͨ���"},
    {"����", 10, "�ε����"}, {"�ݸ�", 10, "�������"}, {"��", 10, "�������"}, {"��", 10, "�����"},
    {"�۾�", 10, "�ۿ�"}, {"����", 10, "�ۿ�"}, {"����", 10, "�ۿ�"}, {"Զ��", 10, "�ۿ�"}, {"ɢ��", 10, "�ۿ�"}, {"��ʹ", 10, "�ۿ�"},
    {"����", 10, "���Ǻ���"}, {"����", 10, "���Ǻ���"}, {"����", 10, "���Ǻ���"}, {"����", 10, "���Ǻ���"},
    {"����", 10, "���Ǻ���"}, {"����", 10, "���Ǻ���"}, {"����", 10, "���Ǻ���"}, {"������", 10, "���Ǻ���"},
    {"��ʹ", 10, "��ǻ��"}, {"����", 10, "��ǻ��"}, {"����", 10, "��ǻ��"}, {"��ǻ����", 10, "��ǻ��"},
    {"����", 10, "��ǻ��"}, {"����", 10, "��ǻ��"}, {"�ǳ�", 10, "��ǻ��"},
    {"Ƥ��", 10, "Ƥ����"}, {"ʪ��", 10, "Ƥ����"}, {"Ƥ��", 10, "Ƥ����"}, {"����", 10, "Ƥ����"}, {"Ƥ��", 10, "Ƥ����"},
    {"�", 10, "Ƥ����"}, {"ݡ����", 10, "Ƥ����"},
    {"����", 10, "�ڿ�"}, {"����", 10, "�ڿ�"}, {"��ð", 10, "�ڿ�"}, {"����", 10, "�ڿ�"}, {"ƣ��", 10, "�ڿ�"},
    {"����", 10, "���"}, {"ײ��", 10, "���"}, {"����", 10, "���"}, {"����", 10, "���"}
    // �ݲ����ӽ������������С���㣬�ȴ���������ע��
};
static const int g_dict_size = sizeof(g_symptom_dict) / sizeof(g_symptom_dict[0]);

// ==========================================
// �ڲ���������
// ==========================================

/**
 * @brief ������һ�����߱��
 * @param new_id �洢���ɵĻ��߱��
 * @details ��ʽΪ "P-XXX"������ XXX Ϊ��λ���֣��� 001 ��ʼ����
 */
static void generate_patient_id(char* new_id)
{
    int max_no = 0;
    PatientNode* curr = NULL;

    // ����У��
    if (new_id == NULL)
    {
        return;
    }

    // �����������Ϊ�գ��� P-001 ��ʼ
    if (g_patient_list == NULL)
    {
        snprintf(new_id, MAX_ID_LEN, "P-001");
        return;
    }

    // ���������������������ı��
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // ����Ƿ�Ϊ���߱�Ÿ�ʽ (P-XXX)
        if (strncmp(curr->id, "P-", 2) == 0)
        {
            // ��ȡ���ֲ��ֲ�ת��Ϊ����
            int current_no = atoi(curr->id + 2);
            if (current_no > max_no)
            {
                max_no = current_no;
            }
        }
        curr = curr->next;
    }

    // �����µı�ţ���ʽΪ P-XXX
    snprintf(new_id, MAX_ID_LEN, "P-%03d", max_no + 1);
}
/**
 * @brief ��������֤�Ų��һ���
 * @param id_card ��������֤��
 * @return �ҵ����ػ��߽ڵ�ָ�룬δ�ҵ�����NULL
 */
PatientNode* find_patient_by_id_card(const char* id_card)
{
    PatientNode* curr = NULL;
    
    // ����У��
    if (g_patient_list == NULL || id_card == NULL) 
        return NULL;
    
    // ����������������ƥ�������֤��
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
 * @brief ���ݻ��߱�Ų��Ҽ���¼
 * @param head ����¼����ͷָ��
 * @param patient_id ���߱��
 * @return �ҵ����ؼ���¼�ڵ�ָ�룬δ�ҵ�����NULL
 */
CheckRecordNode* find_check_records_by_patient_id(CheckRecordNode* head, const char* patient_id)
{
    if (head == NULL || patient_id == NULL) return NULL;
    CheckRecordNode* curr = head->next; // ����ͷ���
    while (curr != NULL && curr != head) // ѭ����������
    {
        if (strcmp(curr->patient_id, patient_id) == 0) return curr; // �ҵ�ƥ��ļ���¼
        curr = curr->next;
    }
    return NULL; // û���ҵ�ƥ��ļ���¼
}

/**
 * @brief ��ȫ�ظ����ı��ֶ�
 * @param dest Ŀ�껺����
 * @param max_len Ŀ�껺������󳤶�
 * @param src Դ�ı�
 * @details ʹ�� strncpy ȷ�����ᷢ�����������
 */
static void copy_text_field(char* dest, int max_len, const char* src)
{
    if (dest == NULL || max_len <= 0)
    {
        return;
    }

    // ʹ�� strncpy ���а�ȫ���ƣ�ȷ���� '\0' ��β
    strncpy(dest, src, (size_t)max_len - 1);
    dest[max_len - 1] = '\0';
}

/**
 * @brief ��ȡ��ʾ�ı���������ֵ��
 * @param text ԭʼ�ı�
 * @return ����ı�Ϊ�շ���"����"�����򷵻�ԭʼ�ı�
 */
static const char* get_display_text(const char* text)
{
    if (text == NULL || text[0] == '\0')
    {
        return "����";
    }

    return text;
}

/**
 * @brief ��ȡ���ֵ�ƴ������ĸ
 * @param c �����ַ�
 * @return ƴ������ĸ����д�����Ǻ��ַ���ԭ�ַ�
 */
static int utf8_char_len(unsigned char first_byte)
{
    if (first_byte < 0x80) return 1;
    if (first_byte < 0xC0) return 1;
    if (first_byte < 0xE0) return 2;
    if (first_byte < 0xF0) return 3;
    return 4;
}

static char get_chinese_pinyin_initial_utf8(const char* ch, int len)
{
    if (len < 3) return 0;

    if (memcmp(ch, "\xE5\xBC\xA0", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE7\x8E\x8B", 3) == 0) return 'W';
    if (memcmp(ch, "\xE6\x9D\x8E", 3) == 0) return 'L';
    if (memcmp(ch, "\xE5\x88\x98", 3) == 0) return 'L';
    if (memcmp(ch, "\xE9\x99\x88", 3) == 0) return 'C';
    if (memcmp(ch, "\xE6\x9D\xA8", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE9\xBB\x84", 3) == 0) return 'H';
    if (memcmp(ch, "\xE8\xB5\xB5", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE5\x91\xA8", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE5\x90\xB4", 3) == 0) return 'W';
    if (memcmp(ch, "\xE9\x83\x91", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE5\x86\xAF", 3) == 0) return 'F';
    if (memcmp(ch, "\xE8\x92\x8B", 3) == 0) return 'J';
    if (memcmp(ch, "\xE6\xB2\x88", 3) == 0) return 'S';
    if (memcmp(ch, "\xE9\x9F\xA9", 3) == 0) return 'H';
    if (memcmp(ch, "\xE6\x9C\xB1", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE7\xA7\xA6", 3) == 0) return 'Q';
    if (memcmp(ch, "\xE8\xAE\xB8", 3) == 0) return 'X';
    if (memcmp(ch, "\xE4\xBD\x95", 3) == 0) return 'H';
    if (memcmp(ch, "\xE5\x90\x95", 3) == 0) return 'L';
    if (memcmp(ch, "\xE6\x96\xBD", 3) == 0) return 'S';
    if (memcmp(ch, "\xE6\x96\x87", 3) == 0) return 'W';
    if (memcmp(ch, "\xE5\xAE\x8B", 3) == 0) return 'S';
    if (memcmp(ch, "\xE5\x94\x90", 3) == 0) return 'T';
    if (memcmp(ch, "\xE4\xBC\x9F", 3) == 0) return 'W';
    if (memcmp(ch, "\xE5\xA8\x9C", 3) == 0) return 'N';
    if (memcmp(ch, "\xE5\xBC\xBA", 3) == 0) return 'Q';
    if (memcmp(ch, "\xE6\x95\x8F", 3) == 0) return 'M';
    if (memcmp(ch, "\xE9\x9D\x99", 3) == 0) return 'J';
    if (memcmp(ch, "\xE7\xA3\x8A", 3) == 0) return 'L';
    if (memcmp(ch, "\xE5\x86\x9B", 3) == 0) return 'J';
    if (memcmp(ch, "\xE6\xB4\x8B", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE5\x8B\x87", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE5\xBB\xBA", 3) == 0) return 'J';
    if (memcmp(ch, "\xE5\x9B\xBD", 3) == 0) return 'G';
    if (memcmp(ch, "\xE6\xAC\xA3", 3) == 0) return 'X';
    if (memcmp(ch, "\xE6\x80\xA1", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE6\xB5\xA9", 3) == 0) return 'H';
    if (memcmp(ch, "\xE7\x84\xB6", 3) == 0) return 'R';
    if (memcmp(ch, "\xE9\x9B\xA8", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE6\xA1\x90", 3) == 0) return 'T';
    if (memcmp(ch, "\xE5\xAD\x90", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE8\xBD\xA9", 3) == 0) return 'X';
    if (memcmp(ch, "\xE6\xA2\x93", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE6\xB6\xB5", 3) == 0) return 'H';
    if (memcmp(ch, "\xE8\xAF\x97", 3) == 0) return 'S';
    if (memcmp(ch, "\xE6\xB3\xBD", 3) == 0) return 'Z';
    if (memcmp(ch, "\xE5\xAE\x87", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE4\xBD\xB3", 3) == 0) return 'J';
    if (memcmp(ch, "\xE7\x90\xAA", 3) == 0) return 'Q';
    if (memcmp(ch, "\xE8\xB1\xAA", 3) == 0) return 'H';
    if (memcmp(ch, "\xE4\xB8\x80", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE8\xAF\xBA", 3) == 0) return 'N';
    if (memcmp(ch, "\xE8\x89\xBA", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE9\xA6\xA8", 3) == 0) return 'X';
    if (memcmp(ch, "\xE6\x80\x9D", 3) == 0) return 'S';
    if (memcmp(ch, "\xE8\xBF\x9C", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE8\xAF\xAD", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE8\x90\x8C", 3) == 0) return 'M';
    if (memcmp(ch, "\xE8\x8A\xB3", 3) == 0) return 'F';
    if (memcmp(ch, "\xE9\x80\x83", 3) == 0) return 'T';
    if (memcmp(ch, "\xE5\x8D\x95", 3) == 0) return 'D';
    if (memcmp(ch, "\xE7\x88\xBD", 3) == 0) return 'S';
    if (memcmp(ch, "\xE7\xBA\xA6", 3) == 0) return 'Y';
    if (memcmp(ch, "\xE5\x8D\xB3", 3) == 0) return 'J';
    if (memcmp(ch, "\xE5\xB0\x86", 3) == 0) return 'J';
    if (memcmp(ch, "\xE8\xA7\xA3", 3) == 0) return 'J';
    if (memcmp(ch, "\xE7\xA6\x81", 3) == 0) return 'J';
    return 0;
}

int can_patient_make_appointment(
    PatientNode* patient,
    const char* current_symptom,
    const char* appoint_doctor,
    const char* appoint_dept
)
{
    (void)current_symptom;
    (void)appoint_doctor;
    (void)appoint_dept;

    if (patient == NULL)
    {
        printf("?? ������Ϣ�����ڣ��޷�ԤԼ��\n");
        return 0;
    }

    if (patient->is_blacklisted != 0)
    {
        printf("?? �û��ߵ�ǰ����ˬԼ/��������¼���ݲ�����ԤԼ���������뵽�ֳ��ҺŻ���ϵ����̨������\n");
        return 0;
    }

    if (patient->emergency_debt > 0)
    {
        printf("?? �û��ߴ��ڼ���Ƿ�� %.2f Ԫ���ݲ�����ԤԼ�����Ȳ���Ƿ�ѻ��ֳ�������\n", patient->emergency_debt);
        return 0;
    }

    if (patient->status == STATUS_NO_SHOW)
    {
        printf("?? �û��߱���ԤԼ�ѹ���/ˬԼ���ݲ������ٴ�ԤԼ����������ѡ���ֳ��Һš�\n");
        return 0;
    }

    if (patient->status == STATUS_PENDING ||
        patient->status == STATUS_EXAMINING ||
        patient->status == STATUS_RECHECK_PENDING ||
        patient->status == STATUS_UNPAID ||
        patient->status == STATUS_WAIT_MED ||
        patient->status == STATUS_NEED_HOSPITALIZE ||
        patient->status == STATUS_HOSPITALIZED)
    {
        printf("?? �û��ߵ�ǰ����δ��ɾ������̣������ظ�ԤԼ��\n");
        printf("��ǰ״̬��%s\n", get_patient_status_text(patient->status));
        return 0;
    }

    return 1;
}

int can_patient_walk_in_register(
    PatientNode* patient,
    const char* current_symptom,
    const char* appoint_doctor,
    const char* appoint_dept
)
{
    int is_emergency = 0;
    const char* symptom_to_check = NULL;

    if (patient == NULL)
    {
        printf("?? ������Ϣ�����ڣ��޷��ֳ��Һš�\n");
        return 0;
    }

    symptom_to_check = (!is_blank_string(current_symptom)) ? current_symptom : patient->symptom;
    is_emergency = is_emergency_request_by_existing_triage(symptom_to_check, appoint_doctor, appoint_dept);

    if (patient->status == STATUS_NO_SHOW)
    {
        printf("��ʾ���û��ߴ��ڹ���/ˬԼ��¼�����ѵ��ֳ��������ֳ��Һ������Ŷӡ�\n");
    }

    if (patient->is_blacklisted != 0)
    {
        if (is_emergency)
        {
            printf("?? ϵͳ��⵽����֢״����ǰ�����䴦�ں�����/ԤԼ����״̬����������������ɫͨ�����������о��Ρ�\n");
        }
        else
        {
            printf("?? �û��ߵ�ǰ���ں�����״̬��������ϵ����̨������\n");
            return 0;
        }
    }

    if (patient->emergency_debt > 0)
    {
        if (is_emergency)
        {
            printf("?? ϵͳ��⵽����֢״�����ߴ��ڼ���Ƿ�� %.2f Ԫ����������������ɫͨ�����������о��Ρ�\n", patient->emergency_debt);
        }
        else
        {
            printf("?? �û��ߴ��ڼ���Ƿ�� %.2f Ԫ����ͨ�ֳ��Һ�ǰ���Ȳ���Ƿ�ѡ�\n", patient->emergency_debt);
            return 0;
        }
    }

    if (patient->status == STATUS_PENDING ||
        patient->status == STATUS_EXAMINING ||
        patient->status == STATUS_RECHECK_PENDING ||
        patient->status == STATUS_UNPAID ||
        patient->status == STATUS_WAIT_MED ||
        patient->status == STATUS_NEED_HOSPITALIZE ||
        patient->status == STATUS_HOSPITALIZED)
    {
        if (is_emergency)
        {
            printf("?? �û��ߵ�ǰ����δ��ɾ������̣�����ǰ֢״���ϼ�����ɫͨ����������ҽ����Ա��ʵ�����ȴ�����\n");
            return 1;
        }
        else
        {
            printf("?? �û��ߵ�ǰ����δ��ɾ������̣������ظ��ֳ��Һš�\n");
            printf("��ǰ״̬��%s\n", get_patient_status_text(patient->status));
            return 0;
        }
    }

    return 1;
}

static void name_to_pinyin(const char* name, char* pinyin, int max_len)
{
    if (name == NULL || pinyin == NULL || max_len <= 0)
        return;
    
    int i = 0;
    int pos = 0;
    
    while (name[i] != '\0' && pos < max_len - 1)
    {
        unsigned char uc = (unsigned char)name[i];
        int clen = utf8_char_len(uc);
        
        if (clen == 1)
        {
            if (uc >= 'a' && uc <= 'z')
                pinyin[pos++] = (char)(uc - 32);
            else if (uc >= 'A' && uc <= 'Z')
                pinyin[pos++] = (char)uc;
            i++;
        }
        else
        {
            char initial = get_chinese_pinyin_initial_utf8(&name[i], clen);
            if (initial != 0)
                pinyin[pos++] = initial;
            i += clen;
        }
    }
    
    pinyin[pos] = '\0';
}



/**
 * @brief ���ݻ��߱�Ż�ȡ���߽ڵ㣨�������飩
 * @param patient_id ���߱��
 * @param action_name �������ƣ����ڴ�����ʾ��
 * @return �ɹ����ػ��߽ڵ�ָ�룬ʧ�ܷ���NULL
 */
static PatientNode* get_patient_by_id_checked(const char* patient_id, const char* action_name)
{
    PatientNode* patient = NULL;

    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("��ʾ������������δ��ʼ�����޷�%s��\n", action_name);
        return NULL;
    }

    // ��黼�߱���Ƿ�Ϊ��
    if (patient_id == NULL || patient_id[0] == '\0')
    {
        printf("��ʾ�����߱�Ų���Ϊ�գ�\n");
        return NULL;
    }

    // ���һ���
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("δ�ҵ���Ӧ���ߵ�����\n");
        return NULL;
    }

    return patient;
}

/**
 * @brief ����֢״�����Ƽ����ң�����Ȩ�ػ�����һƱ����㷨��
 * @param symptom ����֢״����
 * @return �Ƽ��Ŀ��������ַ���
 * 
 * �㷨˵����
 * 1. һƱ�������Σ��֢״��Ȩ�ء�1000��ֱ�ӷ��ؼ����
 * 2. Ȩ�ػ��֣�����֢״�ؼ���ƥ�䣬Ϊ��Ӧ�����ۼ�Ȩ�ط���
 * 3. ƽ���ٲã�ֻ��Ψһ��߷ֿ��Ҳŷ��أ����򷵻�ȫ��
 */
const char* recommend_dept_by_symptom(const char* symptom)
{
    // ���У�飺֢״Ϊ�ջ򳤶�Ϊ0ʱ������ȫ��
    if (symptom == NULL || strlen(symptom) == 0)
    {
        return "ȫ��";
    }
    
    // ��֢״ת��ΪСд�������Сдƥ������
    char lower_symptom[MAX_SYMPTOM_LEN];
    strncpy(lower_symptom, symptom, MAX_SYMPTOM_LEN - 1);
    lower_symptom[MAX_SYMPTOM_LEN - 1] = '\0';
    for (int i = 0; lower_symptom[i]; i++)
    {
        lower_symptom[i] = (char)tolower((unsigned char)lower_symptom[i]);
    }
    
    /*
     * ���ܷ�����ʱ�Ʒְ壺������һ��֢״���������еĿ��ҵ÷�ͳ�ƣ�
     * ���洢���ߡ�ҽ����ҩƷ��ԤԼ��ҵ�����ݡ�
     */
    // ������ҼƷְ壺��¼ÿ�����ҵ��ۼ�Ȩ�ط���
    struct {
        const char* dept;
        int score;
    } score_board[50] = {0};
    int board_size = 0;
    
    // ����֢״Ȩ���ֵ䣬���йؼ���ƥ��
    for (int i = 0; i < g_dict_size; i++)
    {
        const SymptomRule* rule = &g_symptom_dict[i];
        
        // ���֢״�������Ƿ������ǰ�ؼ���
        if (strstr(lower_symptom, rule->keyword) != NULL)
        {
            // ��һƱ�������Σ��֢״��Ȩ�ء�1000��ֱ�ӷ��ؼ����
            if (rule->weight >= 1000)
            {
                return "�����";
            }
            
            // �������ۼӡ����ҿ����Ƿ����ڼƷְ���
            int found = 0;
            for (int j = 0; j < board_size; j++)
            {
                if (strcmp(score_board[j].dept, rule->dept) == 0)
                {
                    // �����Ѵ��ڣ��ۼ�Ȩ�ط���
                    score_board[j].score += rule->weight;
                    found = 1;
                    break;
                }
            }
            
            // ���Ҳ����ڣ�������¼
            if (!found && board_size < 50)
            {
                score_board[board_size].dept = rule->dept;
                score_board[board_size].score = rule->weight;
                board_size++;
            }
        }
    }
    
    // �Ʒְ�Ϊ�գ�����ȫ��
    if (board_size == 0)
    {
        return "ȫ��";
    }
    
    // ��ƽ���ٲá��ҳ�������ߵĿ���
    int max_score = score_board[0].score;
    const char* best_dept = score_board[0].dept;
    int max_count = 1;
    
    // �����Ʒְ壬������߷ֲ�ͳ����߷ֿ�������
    for (int i = 1; i < board_size; i++)
    {
        if (score_board[i].score > max_score)
        {
            // ���ָ��߷֣�������߷ֺ���ѿ���
            max_score = score_board[i].score;
            best_dept = score_board[i].dept;
            max_count = 1;
        }
        else if (score_board[i].score == max_score)
        {
            // ������ͬ��߷֣����Ӽ���
            max_count++;
        }
    }
    
    // �ٲù�����߷�Ϊ0����ڶ����߷ֿ��ң�����ȫ��
    if (max_score == 0 || max_count > 1)
    {
        return "ȫ��";
    }
    
    // Ψһ��߷ֿ��ң����ظÿ���
    return best_dept;
}

/**
 * @brief ��ȡ����״̬�ı�����
 * @param status ���߾�ҽ״̬ö��ֵ
 * @return ״̬��Ӧ�����������ַ���
 */
const char* get_patient_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING:
            return "����";
        case STATUS_EXAMINING:
            return "�����";
        case STATUS_RECHECK_PENDING:
            return "���������";
        case STATUS_UNPAID:
            return "�ѿ�����ɷ�";
        case STATUS_WAIT_MED:
            return "�ѽɷѴ�ȡҩ";
        case STATUS_NEED_HOSPITALIZE:
            return "��סԺ���Ǽ�";
        case STATUS_HOSPITALIZED:
            return "סԺ��";
        case STATUS_COMPLETED:
            return "�������";
        case STATUS_NO_SHOW:
            return "��������";
        default:
            return "δ֪״̬";
    }
}

// ==========================================
// ���߽�������
// ==========================================

/**
 * @brief ���߽���
 * @param name ��������
 * @param age ��������
 * @param id_card ��������֤��
 * @param symptom ֢״����
 * @param target_dept Ŀ�����
 * @return �ɹ����ػ��߽ڵ�ָ�룬ʧ�ܷ���NULL
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
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷�������\n");
        return NULL;
    }
    
    // ��黼�������Ƿ�Ϊ��
    if (name == NULL || strlen(name) == 0)
    {
        printf("?? ������������Ϊ�գ�\n");
        return NULL;
    }
    
    // ��黼�����������Ƿ�Ϸ�
    int name_len = (int)strlen(name);
    if (name_len < 2 || name_len > 20)
    {
        printf("?? �����������ȱ����� 2-20 ���ַ�֮�䣡\n");
        return NULL;
    }
    
    // ��黼�������Ƿ�Ϸ�
    if (age < 0 || age > 130)
    {
        printf("?? ������������� 0-130 ��֮�䣡\n");
        return NULL;
    }
    
    // �������֤�Ÿ�ʽ�Ƿ�Ϸ�
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("?? ����֤�Ÿ�ʽ���Ϸ�������ʧ�ܣ�\n");
        return NULL;
    }
    
    // �������֤���Ƿ��Ѵ���
    PatientNode* existing_patient = find_patient_by_id_card(id_card);
    if (existing_patient != NULL)
    {
        // �����ϵ����Ƿ��ں������У����������ظ�����
        if (existing_patient->is_blacklisted != 0)
        {
            printf("?? ������֤�Ŷ�Ӧ�Ļ������ں������У���ʹ��ԭ�������������ܴ����µ�����\n");
        }
        else
        {
            printf("?? ������֤���Ѵ��ڣ������ظ�������\n");
        }
        return NULL;
    }
    
    // ʱ���۵���ҹ��ģʽ�ӹ�
    const char* recommended_dept;
    char actual_dept[MAX_NAME_LEN] = "";
    
    // ����ͳһ�������ܵ��������ʵ��֢״�ж�
    recommended_dept = recommend_dept_by_symptom(symptom);
    
    // ������Ȩ�ж�������֢״��
    if (strcmp(recommended_dept, "�����") == 0)
    {
        printf("?? ֢״���ϼ����������ѱ��ΪΣ�ػ��ߣ�\n");
    }
    
    if (is_night_shift())
    {
        printf("?? ҹ��ģʽ������������ͨ�����ѹرգ�ϵͳ��Ϊ���Զ����䵽����ơ�\n");
        // ǿ�ƽ�Ŀ���������Ϊ�����
        if (target_dept != NULL && strlen(target_dept) > 0 && strcmp(target_dept, "�����") != 0)
        {
            printf("?? ��ѡ��Ŀ��ҡ�%s����ҹ���ѹرգ�ϵͳ��Ϊ������������ơ�\n", target_dept);
        }
        // ֻ�޸�ʵ�ʿ��ң����޸��Ƽ�����
        strcpy(actual_dept, "�����");
        
        // ��ʾ���ܵ�����ʾ
        printf("?? ���ܵ�����ʾ����������֢״\"%s\"��ϵͳ�Ƽ���ǰ����%s������\n", 
               symptom != NULL && strlen(symptom) > 0 ? symptom : "δ����", 
               recommended_dept);
        
        // ��ʾ�û�ʵ�ʾ������
        printf("?? ����ҹ��ģʽ�������ڡ�����ơ�����\n");
    }
    else
    {
        // ����û�û��ָ�����ң���ʹ���Ƽ��Ŀ���
        if (target_dept == NULL || strlen(target_dept) == 0)
        {
            strcpy(actual_dept, recommended_dept);
        }
        else
        {
            strcpy(actual_dept, target_dept);
        }
        
        // ��ʾ���ܵ�����ʾ
        printf("?? ���ܵ�����ʾ����������֢״\"%s\"��ϵͳ�Ƽ���ǰ����%s������\n", 
               symptom != NULL && strlen(symptom) > 0 ? symptom : "δ����", 
               recommended_dept);
        
        // ����û�ָ���Ŀ������Ƽ����Ҳ�һ�£�������ʾ
        if (target_dept != NULL && strlen(target_dept) > 0 && strcmp(target_dept, recommended_dept) != 0)
        {
            printf("?? ��ѡ��Ŀ����ǡ�%s����ϵͳ�Ƽ������ǡ�%s��\n", target_dept, recommended_dept);
        }
    }
    
    // �����µĻ��߱��
    generate_patient_id(new_id);
    
    // �������߽ڵ�
    new_patient = create_patient_node(new_id, name, age, gender, id_card);
    if (new_patient == NULL)
    {
        printf("?? ���߽ڵ㴴��ʧ�ܣ�\n");
        return NULL;
    }

    // ���ƻ�����Ϣ���ڵ���
    copy_text_field(new_patient->id_card, MAX_ID_LEN, id_card);
    copy_text_field(new_patient->symptom, MAX_SYMPTOM_LEN, symptom);
    copy_text_field(new_patient->target_dept, MAX_NAME_LEN, actual_dept);
    
    // �����ж�������Ƽ������Ǽ���ƣ����Ϊ���ﻼ��
    if (strcmp(recommended_dept, "�����") == 0)
    {
        new_patient->is_emergency = 1;
        printf("?? �û���֢״���ϼ����������ѱ��Ϊ������ɫͨ�����ߣ�\n");
    }
    // �����߽ڵ���뵽����β��
    insert_patient_tail(g_patient_list, new_patient);

    new_patient->status = STATUS_COMPLETED;

    return new_patient;
}

// ==========================================
// ���ߵ�����������
// ==========================================

/**
 * @brief ��ʾ���ߵ�����ϸ��Ϣ
 * @param patient ���߽ڵ�ָ��
 */
void display_patient_archive(const PatientNode* patient)
{
    char masked_id[19] = "";

    // ����У��
    if (patient == NULL)
    {
        return;
    }

    // ��������֤������
    if (strlen(patient->id_card) > 0)
    {
        mask_id_card(patient->id_card, masked_id);
    }
    else
    {
        copy_text_field(masked_id, (int)sizeof(masked_id), get_display_text(patient->id_card));
    }

    // ��ʾ���ߵ�����Ϣ
    printf("\n================ ���ߵ�����Ϣ ================\n");
    printf("���߱�ţ�%s\n", patient->id);
    printf("������%s\n", patient->name);
    printf("�Ա�%s\n", strlen(patient->gender) > 0 ? patient->gender : "δ����");
    printf("���䣺%d\n", patient->age);
    printf("��������֤�ţ�%s\n", masked_id);
    printf("֢״��%s\n", get_display_text(patient->symptom));
    printf("Ŀ����ң�%s\n", get_display_text(patient->target_dept));
    printf("��ǰ״̬��%s\n", get_patient_status_text(patient->status));
    printf("�˻���%.2f\n", patient->balance);
    printf("==============================================\n");
}

/**
 * @brief ���ݻ��߱�Ų�ѯ����
 * @param patient_id ���߱��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_archive_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;

    // ��ȡ���߽ڵ㣨�������飩
    patient = get_patient_by_id_checked(patient_id, "��ѯ���ߵ���");
    if (patient == NULL)
    {
        return 0;
    }

    // ��鲢���Ϲ��ڵĴ��ɷѶ���
    check_and_void_expired_orders(patient);

    // ��ʾ���ߵ�����Ϣ
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief ��������֤�Ų�ѯ����
 * @param id_card ��������֤��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_archive_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷���ѯ������\n");
        return 0;
    }
    
    // �������֤���Ƿ�Ϊ��
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("?? ����֤�Ų���Ϊ�գ�\n");
        return 0;
    }
    
    // �������֤�Ÿ�ʽ�Ƿ�Ϸ�
    if (!validate_id_card(id_card))
    {
        printf("?? ����֤�Ÿ�ʽ���Ϸ���\n");
        return 0;
    }
    
    // ��������֤�Ų��һ���
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߵ�����\n");
        return 0;
    }
    
    // ��鲢���Ϲ��ڵĴ��ɷѶ���
    check_and_void_expired_orders(patient);
    
    // ��ʾ���ߵ�����Ϣ
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief ���������ؼ��ʲ�ѯ������֧��ģ����ѯ��ƴ������ĸ������
 * @param name_keyword �����ؼ��ʣ������Ǻ��ֻ�ƴ������ĸ��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_archive_by_name(const char* name_keyword)
{
    PatientNode* curr = NULL;
    int found = 0;
    char pinyin[MAX_NAME_LEN];
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷���ѯ������\n");
        return 0;
    }
    
    // ��������ؼ����Ƿ�Ϊ��
    if (name_keyword == NULL || strlen(name_keyword) == 0)
    {
        printf("?? ������������Ϊ�գ�\n");
        return 0;
    }
    
    // ������������������ƥ�������
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // 1. �Ӵ�ƥ�䣨֧�ֺ���ģ����ѯ��
        if (strstr(curr->name, name_keyword) != NULL)
        {
            display_patient_archive(curr);
            found = 1;
        }
        else
        {
            // 2. ƴ������ĸƥ�䣨֧��ƴ������ĸ��������Сд�����У�
            name_to_pinyin(curr->name, pinyin, MAX_NAME_LEN);
            
            // ���û�����ת��Ϊ��д����ƥ��
            char keyword_upper[MAX_NAME_LEN];
            int k;
            for (k = 0; name_keyword[k] != '\0' && k < MAX_NAME_LEN - 1; k++)
            {
                if (name_keyword[k] >= 'a' && name_keyword[k] <= 'z')
                    keyword_upper[k] = name_keyword[k] - 32;
                else
                    keyword_upper[k] = name_keyword[k];
            }
            keyword_upper[k] = '\0';
            
            if (strstr(pinyin, keyword_upper) != NULL)
            {
                display_patient_archive(curr);
                found = 1;
            }
        }
        curr = curr->next;
    }
    
    // ���û���ҵ�ƥ��Ļ���
    if (!found)
    {
        printf("?? δ�ҵ�����\"%s\"�Ļ��ߵ�����\n", name_keyword);
        return 0;
    }
    
    return 1;
}

// ==========================================
// ������������������ѯ����
// ==========================================

/**
 * @brief ����������ѯ����������Ϣ����Ҫ���ݺ��飩
 * @param patient_id ���߱��
 * @param id_card ��������֤�ţ��������ݺ��飩
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_basic_patient_record(const char* patient_id, const char* id_card)
{
    char masked_id[19];
    
    // ��ȡ���߽ڵ㣨�������飩
    PatientNode* patient = get_patient_by_id_checked(patient_id, "��ѯ��������");

    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߣ�������ѯʧ�ܣ�\n");
        return 0;
    }

    // �������֤�Ÿ�ʽ�Ƿ�Ϸ�
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("��ʾ������֤�Ÿ�ʽ���Ϸ����޷��������ݺ��飡\n");
        return 0;
    }

    // ���ݺ��飺������������֤���Ƿ��뻼�ߵ����е�����֤��ƥ��
    if (strcmp(patient->id_card, id_card) != 0)
    {
        printf("?? ���ݺ���ʧ�ܣ���ֹ���ʻ���������Ϣ��\n");
        return 0;
    }

    // ����֤����������
    mask_id_card(patient->id_card, masked_id);
    
    // ��ʾ����������Ϣ
    printf("\n================ ����������Ϣ ================\n");
    printf("���߱�ţ�%s\n", patient->id);
    printf("������%s\n", patient->name);
    printf("�Ա�%s\n", patient->gender);
    printf("���䣺%d\n", patient->age);
    printf("֢״����: %s\n", get_display_text(patient->symptom));
    printf("Ŀ�����: %s\n", get_display_text(patient->target_dept));
    printf("��ǰ����״̬: %s\n", get_patient_status_text(patient->status));
    printf("���һ����Ͻ���: %s\n",
        strlen(patient->diagnosis_text) > 0 ? patient->diagnosis_text : "������ϼ�¼");
    printf("���һ�δ������: %s\n",
        strlen(patient->treatment_advice) > 0 ? patient->treatment_advice : "���޴������");
    printf("����֤��: %s\n", masked_id);

    return 1;
}

/**
 * @brief ���»��ߵ�����Ϣ
 * @param patient_id ���߱�ţ������޸ģ�
 * @param name �����������ձ�ʾ���޸ģ�
 * @param age �����䣨0��ʾ���޸ģ�
 * @param symptom ��֢״���������ձ�ʾ���޸ģ�
 * @param target_dept ��Ŀ����ң����ձ�ʾ���޸ģ�
 * @param id_card ������֤�ţ����ձ�ʾ���޸ģ�
 * @param balance ���˻����
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int update_patient_archive(const char* patient_id, const char* name, int age, const char* symptom, const char* target_dept, const char* id_card, double balance)
{
    // ��ȡ���߽ڵ㣨�������飩
    PatientNode* patient = get_patient_by_id_checked(patient_id, "�޸Ļ��ߵ���");

    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߣ�����ʧ�ܣ�\n");
        return 0;
    }
    
    // ���»�����Ϣ��ֻ���·ǿ��ֶΣ�
    if (name != NULL && strlen(name) > 0)
    {
        // ��黼�����������Ƿ�Ϸ�
        int name_len = (int)strlen(name);
        if (name_len < 2 || name_len > 20)
        {
            printf("?? �����������ȱ����� 2-20 ���ַ�֮�䣡\n");
            return 0;
        }
        copy_text_field(patient->name, MAX_NAME_LEN, name);
    }
    
    if (age > 0)
    {
        // ��黼�������Ƿ�Ϸ�
        if (age < 0 || age > 130)
        {
            printf("?? ������������� 0-130 ��֮�䣡\n");
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
    
    // ��������֤�ţ���Ҫ��֤��
    if (id_card != NULL && strlen(id_card) > 0)
    {
        // ��֤����֤�Ÿ�ʽ
        if (!validate_id_card(id_card))
        {
            printf("?? ����֤�Ÿ�ʽ���Ϸ�������ʧ�ܣ�\n");
            return 0;
        }
        
        // ���������֤���Ƿ��ѱ���������ʹ��
        PatientNode* existing_patient = find_patient_by_id_card(id_card);
        if (existing_patient != NULL && strcmp(existing_patient->id, patient_id) != 0)
        {
            printf("?? ������֤���ѱ���������ʹ�ã�����ʧ�ܣ�\n");
            return 0;
        }
        
        // ��������֤��
        copy_text_field(patient->id_card, MAX_ID_LEN, id_card);
    }
    
    // �����˻����
    patient->balance = balance;
    
    printf("? ���ߵ������³ɹ���\n");
    return 1;
}

// ==========================================
// ���߾����������
// ==========================================

/**
 * @brief ��ʾ���߾��������Ϣ
 * @param patient ���߽ڵ�ָ��
 */
void display_patient_visit_overview(const PatientNode* patient)
{
    // ����У��
    if (patient == NULL)
    {
        printf("?? ������ϢΪ�գ�\n");
        return;
    }
    
    // ��ʾ���߻�����Ϣ
    printf("\n================ ���߾������ ================\n");
    printf("���߱�ţ�%s\n", patient->id);
    printf("������%s\n", patient->name);
    printf("�Ա�%s\n", strlen(patient->gender) > 0 ? patient->gender : "δ����");
    printf("���䣺%d\n", patient->age);
    
    // ��������֤��
    if (strlen(patient->id_card) > 0)
    {
        char masked_id[MAX_ID_LEN] = {0};
        mask_id_card(patient->id_card, masked_id);
        printf("����֤�ţ�%s\n", masked_id);
    }
    else
    {
        printf("����֤�ţ�����\n");
    }
    
    // ֢״������Ϊ��ʱ��ʾ"����"��
    if (patient->symptom[0] != '\0')
        printf("֢״����: %s\n", patient->symptom);
    else
        printf("֢״����: ����\n");
    
    // Ŀ����ң�Ϊ��ʱ��ʾ"����"��
    if (patient->target_dept[0] != '\0')
        printf("Ŀ�����: %s\n", patient->target_dept);
    else
        printf("Ŀ�����: ����\n");
    
    // ��ǰ״̬���˻����
    printf("��ǰ״̬: %s\n", get_patient_status_text(patient->status));
    printf("�˻���%.2f\n", patient->balance);
    
    // ���һ����Ͻ���
    if (patient->diagnosis_text[0] != '\0')
        printf("�����Ͻ���: %s\n", patient->diagnosis_text);
    else
        printf("�����Ͻ���: ������ϼ�¼\n");
    
    // ���һ�δ������
    if (patient->treatment_advice[0] != '\0')
        printf("�������: %s\n", patient->treatment_advice);
    else
        printf("�������: ���޴������\n");
    
    // ���һ�ιҺ���Ϣ
    AppointmentNode* latest_appointment = find_latest_appointment_by_patient_id(patient->id);
    if (latest_appointment != NULL)
    {
        printf("\n����Һ���Ϣ:\n");
        printf("  �Һű�ţ�%s\n", latest_appointment->appointment_id);
        printf("  �Һ����ͣ�%s\n", latest_appointment->is_walk_in ? "�ֳ���" : "ԤԼ��");
        printf("  �Һ����ڣ�%s\n", latest_appointment->appointment_date);
        printf("  �Һ�ʱ�Σ�%s\n", latest_appointment->appointment_slot);
        
        // ��ʾ�Һ�ҽ�������
        if (latest_appointment->appoint_doctor[0] != '\0')
            printf("  �Һ�ҽ����%s\n", latest_appointment->appoint_doctor);
        else if (latest_appointment->appoint_dept[0] != '\0')
            printf("  �Һſ��ң�%s\n", latest_appointment->appoint_dept);
        else
            printf("  �Һſ��ң�����\n");
        printf("  �Һ�״̬��%s\n", get_appointment_display_status(latest_appointment));
    }
    else
    {
        printf("\n����Һ���Ϣ: ���޹Һż�¼\n");
    }
    
    printf("==============================================\n");
}

/**
 * @brief ���ݻ��߱�Ų�ѯ�������
 * @param patient_id ���߱��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_visit_overview_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷���ѯ��\n");
        return 0;
    }
    
    // ��黼�߱���Ƿ�Ϊ��
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return 0;
    }
    
    // ��黼�߱�Ÿ�ʽ�Ƿ�Ϸ�
    if (!validate_patient_id(patient_id))
    {
        printf("?? ���߱�Ÿ�ʽ���Ϸ�����ȷ��ʽΪ P-1001����������ԣ�\n");
        return 0;
    }
    
    // ���ݻ��߱�Ų��һ���
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߣ���ѯʧ�ܣ�\n");
        return 0;
    }
    
    // ��ʾ���߾��������Ϣ
    display_patient_visit_overview(patient);
    return 1;
}

/**
 * @brief ��������֤�Ų�ѯ�������
 * @param id_card ��������֤��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_visit_overview_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷���ѯ��\n");
        return 0;
    }
    
    // �������֤���Ƿ�Ϊ��
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("?? ����֤�Ų���Ϊ�գ�\n");
        return 0;
    }
    
    // �������֤�Ÿ�ʽ�Ƿ�Ϸ�
    if (!validate_id_card(id_card))
    {
        printf("��ʾ������֤�Ÿ�ʽ���Ϸ����޷���ѯ��\n");
        return 0;
    }
    
    // ��������֤�Ų��һ���
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߣ���ѯʧ�ܣ�\n");
        return 0;
    }
    
    // ��ʾ���߾��������Ϣ
    display_patient_visit_overview(patient);
    return 1;
}

/**
 * @brief ��ȡ���ƾ������������������ӽǣ�
 * @param decision ���ƾ�������ֵ
 * @return ���߶�Ӧ�����������ַ���
 */
static const char* get_decision_text_for_patient(int decision)
{
    switch (decision) {
        case 1: // ��������
            return "��������";
        case 2: // ��ҩ
            return "��ҩ";
        case 3: // �����
            return "�����";
        case 4: // ����סԺ
            return "����סԺ";
        default:
            return "δ��¼����";
    }
}

/**
 * @brief ����������ѯ�Լ�����ʷ�����¼����Ҫ���ݺ��飩
 * @param patient_id ���߱��
 * @param id_card ��������֤�ţ��������ݺ��飩
 * @return �ɹ�����1��ʧ�ܷ���0
 */
// ==========================================
// ҵ��������
// ==========================================

/**
 * @brief ��鲢���Ϲ��ڵĴ��ɷѶ���
 * @param patient ���߽ڵ�ָ��
 */
void check_and_void_expired_orders(PatientNode* patient)
{
    // ����У��
    if (patient == NULL)
    {
        return;
    }
    
    // ��黼��״̬�Ƿ�Ϊ���ɷ�����δ�ɷ�ʱ���
    if (patient->status == STATUS_UNPAID && patient->unpaid_time != 0)
    {
        // ��ȡ��ǰʱ��
        time_t current_time = time(NULL);
        
        // ����Ƿ񳬹�72Сʱ
        if (current_time - patient->unpaid_time > 72 * 3600)
        {
            // ������free������script_head����������нڵ�
            PrescriptionNode* curr = patient->script_head;
            while (curr != NULL)
            {
                PrescriptionNode* temp = curr;
                curr = curr->next;
                free(temp);
            }
            
            // ��մ�������ֶ�
            patient->script_head = NULL;
            patient->script_count = 0;

            // ͬ�������û�����������δ֧����鵥������ɵ�����������˵�
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
            
            // ��ӡ��ʾ��Ϣ
            printf("?? ��⵽������ʱ 72 Сʱ��δ�ɷѴ������鵥���Զ�����\n");
        }
    }
}

// ==========================================
// ����������ѯ��ʷ�����¼����
// ==========================================

/**
 * @brief ����������ѯ�Լ�����ʷ�����¼����Ҫ���ݺ��飩
 * @param patient_id ���߱��
 * @param id_card ��������֤�ţ��������ݺ��飩
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int query_patient_consult_history_verified(const char* patient_id, const char* id_card)
{
    PatientNode* patient = NULL;
    
    // ��һ�����ǿ�У�飨��ֹ��ָ�룩
    if (patient_id == NULL || strlen(patient_id) == 0 || id_card == NULL || strlen(id_card) == 0)
    {
        printf("?? ���߱�Ż�����֤�Ų���Ϊ�գ�\n");
        return 0;
    }
    
    // �ڶ�������������֤�Ų��һ���
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("?? δ�ҵ���Ӧ���ߣ���������֤���Ƿ���ȷ��\n");
        return 0;
    }
    
    // ����������黼�߱���Ƿ�ƥ��
    if (strcmp(patient->id, patient_id) != 0)
    {
        printf("?? ���߱��������֤�Ų�ƥ�䣬��������ԣ�\n");
        return 0;
    }
    
    // ���ݺ���ͨ������ʾ��Ϣ
    printf("\n==============================================\n");
    printf("          ��ʷ�����¼��ѯ\n");
    printf("==============================================\n");
    printf("��������: %s\n", patient->name);
    printf("���߱��: %s\n", patient->id);
    printf("==============================================\n\n");
    
    // ��鲢���Ϲ��ڵĴ��ɷѶ���
    check_and_void_expired_orders(patient);
    
    // ���岽���������н����¼������������ǰ��
    ConsultRecordNode* head = g_consult_record_list;
    
    // �жϿ�������ͷ���Ϊ�� �� û����ʵ�ڵ�
    if (head == NULL || head->next == NULL)
    {
        printf("������ʷ�����¼\n");
        printf("==============================================\n");
        return 1;
    }
    
    // �ҵ����һ����¼�����µļ�¼��
    ConsultRecordNode* curr = head->next;  // �ӵ�һ����ʵ�ڵ㿪ʼ
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    
    int record_count = 0;
    ConsultRecordNode* temp = curr;
    
    // �����¼�¼��ʼ��ǰ������ȷ�����������ͷ���
    while (temp != NULL && temp != head)
    {
        if (strcmp(temp->patient_id, patient_id) == 0)
        {
            record_count++;
            
            printf("����ʷ�����¼ %d��\n", record_count);
            printf("--------------------------------------------------\n");
            printf("�����¼���: %s\n", temp->record_id);
            printf("����ʱ��: %s\n", temp->consult_time);
            
            // ������ʾ����ҽ������
            DoctorNode* doctor = find_doctor_by_id(g_doctor_list, temp->doctor_id);
            if (doctor != NULL)
            {
                printf("����ҽ��: %s��%s��\n", doctor->name, temp->doctor_id);
                printf("��������: %s\n", doctor->department);
            }
            else
            {
                printf("����ҽ��: δ֪ҽ����%s��\n", temp->doctor_id);
                printf("��������: δ֪\n");
            }
            
            printf("��Ͻ���: %s\n", temp->diagnosis_text);
            printf("�������: %s\n", temp->treatment_advice);
            printf("���ƾ���: %s\n", get_decision_text_for_patient(temp->decision));
            printf("����ǰ״̬: %s\n", get_patient_status_text(temp->pre_status));
            printf("�����״̬: %s\n", get_patient_status_text(temp->post_status));
            
            // ԤԼ��ţ�����У�
            if (temp->appointment_id[0] != '\0')
            {
                printf("ԤԼ���: %s\n", temp->appointment_id);
            }
            else
            {
                printf("ԤԼ���: ��\n");
            }
            
            // �����
            // ���Ҷ�Ӧ���ߵļ���¼
            printf("�����: ");
            CheckRecordNode* check_record = find_check_records_by_patient_id(g_check_record_list, patient_id);
            if (check_record != NULL)
            {
                // ��ʾ��һ������¼�Ľ��
                printf("%s\n", check_record->result);
            }
            else
            {
                printf("���޼����\n");
            }
            
            printf("--------------------------------------------------\n\n");
        }
        
        temp = temp->prev;
    }
    
    if (record_count == 0)
    {
        printf("������ʷ�����¼\n");
    }
    else
    {
        printf("���ҵ� %d ����ʷ�����¼\n", record_count);
    }
    
    printf("==============================================\n");
    return 1;
}

/**
 * @brief ���������ӵ�
 * @param patient_id ���߱��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int handle_emergency_escape(const char* patient_id_input) {
    PatientNode* patient = NULL;
    char patient_id[MAX_ID_LEN];
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL) {
        printf("?? ����������δ��ʼ�����޷����������ӵ���\n");
        return 0;
    }
    
    // ��ʾ��Ϣ
    printf("\n��ʾ������ '0' ���Ի�����һ�������� '00' �����˳�����\n");
    
    // ���ƻ��߱������
    if (patient_id_input != NULL) {
        strncpy(patient_id, patient_id_input, MAX_ID_LEN - 1);
        patient_id[MAX_ID_LEN - 1] = '\0';
    } else {
        // ���뻼�߱��
        while (1) {
            printf("�����뻼�߱��: ");
            char patient_id_input[MAX_ID_LEN];
            get_safe_string("", patient_id_input, MAX_ID_LEN);
            strncpy(patient_id, patient_id_input, MAX_ID_LEN - 1);
            patient_id[MAX_ID_LEN - 1] = '\0';
            
            // �����˻��˳�
            if (strcmp(patient_id, "0") == 0) {
                printf("�ѻ��˵���һ��\n");
                return 0;
            } else if (strcmp(patient_id, "00") == 0) {
                printf("���˳�����\n");
                return 0;
            }
            
            // ��黼�߱�Ÿ�ʽ�Ƿ�Ϸ�
            if (validate_patient_id(patient_id)) {
                // ���һ���
                patient = find_patient_by_id(g_patient_list, patient_id);
                if (patient != NULL) {
                    // ����Ƿ�Ϊ���ﻼ���Ҵ��ڴ��ɷ�״̬
                    if (patient->is_emergency == 1 && patient->status == STATUS_UNPAID) {
                        break;
                    } else {
                        if (patient->is_emergency != 1) {
                            printf("?? �û��߲��Ǽ�����ɫͨ�����ߣ��޷�ִ�м����ӵ�������\n");
                        } else if (patient->status != STATUS_UNPAID) {
                            printf("?? �û��ߵ�ǰ״̬���Ǵ��ɷѣ��޷�ִ�м����ӵ�������\n");
                        }
                        printf("������������������Ļ��߱�ţ������� '0' ���ˣ�'00' �˳�\n");
                    }
                } else {
                    printf("?? ���޴��ˣ�δ�ҵ���Ӧ���ߣ�\n");
                    printf("���������뻼�߱�ţ������� '0' ���ˣ�'00' �˳�\n");
                }
            } else {
                printf("?? ���߱�Ÿ�ʽ���Ϸ�����ȷ��ʽΪ P-1001����������ԣ�\n");
                printf("���������뻼�߱�ţ������� '0' ���ˣ�'00' �˳�\n");
            }
        }
    }
    
    // �����ӵ����
    double debt_amount;
    while (1) {
        printf("������û��ߵļ���Ƿ�ѽ��: ");
        char input[50];
        get_safe_string("", input, 50);
        
        // �����˻��˳�
        if (strcmp(input, "0") == 0) {
            printf("�ѻ��˵���һ��\n");
            return 0;
        } else if (strcmp(input, "00") == 0) {
            printf("���˳�����\n");
            return 0;
        }
        
        // ת��Ϊ����
        if (sscanf(input, "%lf", &debt_amount) == 1 && debt_amount > 0) {
            break;
        } else {
            printf("?? Ƿ�ѽ��������0�����������룡\n");
            printf("����������Ƿ�ѽ������� '0' ���ˣ�'00' �˳�\n");
        }
    }
    
    // ��¼Ƿ�ѽ�ִ����������
    patient->emergency_debt = debt_amount;
    patient->is_blacklisted = 2;
    printf("? [��߼��𾯱�] �û������ü�����ɫͨ���ӵ������������ú�������\n");
    printf("? �ӵ����: %.2f Ԫ\n", debt_amount);
    printf("? �û��߽����ñ���ֹ�Һź�ԤԼ���񣬳��ǲ���Ƿ�ѣ�\n");
    return 1;
}
/**
 * @brief ����Ƿ�Ѳ�����������
 * @param patient_id ���߱��
 * @return �ɹ�����1��ʧ�ܷ���0
 */
int settle_blacklisted_debt(const char* patient_id)
{
    PatientNode* patient = NULL;
    
    // ��黼�������Ƿ��ʼ��
    if (g_patient_list == NULL)
    {
        printf("?? ����������δ��ʼ�����޷�����Ƿ�Ѻ�����\n");
        return 0;
    }
    
    // ��黼�߱���Ƿ�Ϊ��
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return 0;
    }
    
    // ��黼�߱�Ÿ�ʽ�Ƿ�Ϸ�
    if (!validate_patient_id(patient_id))
    {
        printf("?? ���߱�Ÿ�ʽ���Ϸ�����ȷ��ʽΪ P-1001����������ԣ�\n");
        return 0;
    }
    
    // ���һ���
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("?? ���޴��ˣ�δ�ҵ���Ӧ���ߣ�\n");
        return 0;
    }
    
    // ȷ�ϻ����Ƿ����ӵ���������
    if (patient->is_blacklisted != 2)
    {
        printf("?? �û��߲����ӵ��������У����������\n");
        return 0;
    }
    
    // ��ӡ����Ƿ�ѽ��
    printf("======================================================\n");
    printf("                ?? Ƿ�Ѻ���ȷ��\n");
    printf("======================================================\n");
    printf("���߱��: %s\n", patient->id);
    printf("��������: %s\n", patient->name);
    printf("����֤��: %s\n", patient->id_card);
    printf("Ƿ�ѽ��: %.2f Ԫ\n", patient->emergency_debt);
    printf("------------------------------------------------------\n");
    
    // ����ʵ�ʽɷѽ��
    double actual_payment;
    while (1)
    {
        printf("������û���ʵ�ʽ��ɵĽ�Ԫ��: ");
        char input[50];
        get_safe_string("", input, 50);
        
        // �����˻��˳�
        if (strcmp(input, "0") == 0)
        {
            printf("��ȡ��������\n");
            return 0;
        }
        
        // ת��Ϊ����
        if (sscanf(input, "%lf", &actual_payment) == 1 && actual_payment >= 0)
        {
            break;
        }
        else
        {
            printf("?? ��������Ч�Ľ����֣���\n");
        }
    }
    
    // �ȶԽɷѽ����Ƿ�ѽ��
    if (actual_payment < patient->emergency_debt)
    {
        printf("?? ʵ�ʽɷѽ�%.2f Ԫ��С��Ƿ�ѽ�%.2f Ԫ��������ʧ�ܣ�\n", actual_payment, patient->emergency_debt);
        printf("�����ѻ��߲���Ƿ�������������\n");
        return 0;
    }
    
    // ִ�к�������
    patient->emergency_debt = 0.0; // ����Ƿ��
    patient->is_blacklisted = 0; // �Ƴ�������
    patient->status = STATUS_COMPLETED; // ����״̬Ϊ�������
    
    // ��ӡ�ɹ���ʾ
    printf("? �����ɹ����û������Ƴ����������ָ�������ҽȨ�ޡ�\n");
    printf("ʵ�ʽɷѽ��: %.2f Ԫ\n", actual_payment);
    printf("======================================================\n");
    
    return 1;
}

// ==========================================
// �������ģ��
// ==========================================
int process_patient_payment(const char* patient_id)
{
    PatientNode* patient = get_patient_by_id_checked(patient_id, "�����ɷ�");
    if (patient == NULL) return 0;

    // 1. ״̬У��
    if (patient->status != STATUS_UNPAID)
    {
        printf("?? ��ǰ״̬Ϊ[%s]��������нɷѲ�����\n", get_patient_status_text(patient->status));
        return 0;
    }

    double total_amount = 0.0;
    double actual_pay = 0.0;
    double medicare_cover = 0.0;
    int index = 1;

    printf("\n================ ?? �˵���ϸ ================\n");

    // 2. ������������������������
    PrescriptionNode* p_curr = patient->script_head;
    while (p_curr != NULL)
    {
        // ͨ��ҩƷIDȥȫ��ҩƷ�⣨˫���������в�۸�
        MedicineNode* med = find_medicine_by_id(g_medicine_list, p_curr->med_id);
        if (med != NULL)
        {
            double item_total = med->price * p_curr->quantity;
            double item_actual = item_total;

            // ҽ�������߼���������ҽ������ҩƷ֧�ֱ���
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

            printf("[%d] ҩƷ: %s | ����: %.2f | ����: %d | С��: %.2f | ҽ����: %.2f\n",
                   index++, med->name, med->price, p_curr->quantity, item_total, item_actual);
        }
        p_curr = p_curr->next;
    }

    // 3. ��������¼��������
    CheckRecordNode* cr_curr = g_check_record_list->next;
    while (cr_curr != NULL)
    {
        if (strcmp(cr_curr->patient_id, patient_id) == 0 && cr_curr->is_paid == 0)
        {
            // ͨ�������ĿIDȥȫ�ּ����Ŀ���в�۸��ҽ������
            CheckItemNode* check_item = find_check_item_by_id(g_check_item_list, cr_curr->item_id);
            if (check_item != NULL)
            {
                double item_total = check_item->price;
                double item_actual = item_total;

                // ����ҽ�������߼�
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

                printf("[%d] ���: %s | ����: %.2f | С��: %.2f | ҽ����: %.2f\n",
                       index++, check_item->item_name, check_item->price, item_total, item_actual);
            }
        }
        cr_curr = cr_curr->next;
    }

    medicare_cover = total_amount - actual_pay;

    printf("------------------------------------------\n");
    printf("�ܼƽ��: %.2f Ԫ\n", total_amount);
    printf("ҽ��ͳ��: %.2f Ԫ\n", medicare_cover);
    printf("�����Ը�: %.2f Ԫ\n", actual_pay);
    printf("��ǰ���: %.2f Ԫ\n", patient->balance);
    printf("==========================================\n");

    // 4. �۷ѷ��У��
    if (patient->balance < actual_pay)
    {
        printf("? ���㣡���� %.2f Ԫ�����ȵ���ֵ���ڳ�ֵ��\n", actual_pay - patient->balance);
        return 0;
    }

    // 5. ִ�пۿ�
    patient->balance -= actual_pay;
    patient->unpaid_time = 0;

    // 6. ��Ǽ���¼Ϊ�ѽɷ�
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

    // 7. ����״̬��ת����ѭ�ٴ��������ԭ��
    if (has_paid_check > 0)
    {
        patient->status = STATUS_EXAMINING;
        printf("? �ɷѳɹ����ѿ۳� %.2f Ԫ����ǰʣ����� %.2f Ԫ��\n", actual_pay, patient->balance);
        printf("?? [״̬����] �������ԭ�򣺵�ǰ״̬��תΪ ����� (STATUS_EXAMINING) -> ?? ������ȱ��ֿո�������ǰ���������ҡ��Ŷ�����飡\n");
        if (patient->script_head != NULL)
        {
            printf("?? ��ʾ�������»��д�ȡҩƷ�����ڼ��յ��ڻ������ǰ��ҩ��ȡҩ��\n");
        }
    }
    else if (patient->script_head != NULL)
    {
        patient->status = STATUS_WAIT_MED;
        printf("? �ɷѳɹ����ѿ۳� %.2f Ԫ����ǰʣ����� %.2f Ԫ��\n", actual_pay, patient->balance);
        printf("?? ���Ʋ�����ҩ�����Ŷ�ȡҩ��\n");
    }

    return 1;
}

// ==========================================
// �������������ģ��
// ==========================================
int submit_patient_evaluation(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return 0;
    }

    if (g_consult_record_list == NULL)
    {
        printf("?? �����¼������δ��ʼ����\n");
        return 0;
    }

    // �������һ���������δ���۵ľ����¼
    ConsultRecordNode* last_completed_record = NULL;
    ConsultRecordNode* curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0 && curr->post_status == STATUS_COMPLETED && curr->star_rating == 0)
        {
            last_completed_record = curr;
        }
        curr = curr->next;
    }

    if (last_completed_record == NULL)
    {
        // ����Ƿ�������ɵ������۵ļ�¼
        int has_completed_record = 0;
        curr = g_consult_record_list->next;
        while (curr != NULL)
        {
            if (strcmp(curr->patient_id, patient_id) == 0 && curr->post_status == STATUS_COMPLETED)
            {
                has_completed_record = 1;
                break;
            }
            curr = curr->next;
        }
        
        if (has_completed_record)
        {
            printf("?? ������������ɾ����¼�������ۣ������ظ����ۣ�\n");
        }
        else
        {
            printf("?? ����ǰû������ɵľ����¼���޷��������ۣ�\n");
        }
        return 0;
    }

    // չʾҽ����Ϣ�;���ʱ��
    DoctorNode* doctor = find_doctor_by_id(g_doctor_list, last_completed_record->doctor_id);
    printf("\n================ ��������� ================\n");
    printf("�����¼��ţ�%s\n", last_completed_record->record_id);
    printf("����ҽ����%s����ţ�%s��\n", doctor ? doctor->name : "δ֪", last_completed_record->doctor_id);
    printf("����ʱ�䣺%s\n", last_completed_record->consult_time[0] != '\0' ? last_completed_record->consult_time : "δ֪");
    printf("========================================\n");

    // �����Ǽ����ۣ�1-5��
    int star_rating;
    while (1)
    {
        char star_input[10];
        get_safe_string("������������Ǽ� (1-5������ 0 ���ˣ����� 00 �˳�): ", star_input, sizeof(star_input));
        
        // ����Ƿ��˳�
        if (strcmp(star_input, "00") == 0)
        {
            printf("����ȡ����\n");
            return 0;
        }
        
        // ����Ƿ����
        if (strcmp(star_input, "0") == 0)
        {
            return 0; // ���˵���һ��
        }
        
        // ת��Ϊ����
        star_rating = atoi(star_input);
        if (star_rating >= 1 && star_rating <= 5)
        {
            break; // ��ʽ��ȷ���˳�ѭ��
        }
        else
        {
            printf("?? �Ǽ����۱����� 1-5 ֮�䣬���������룡\n");
        }
    }

    // ������������
    char feedback[MAX_RECORD_LEN];
    while (1)
    {
        get_safe_string("�������������ۣ�ѡ����� 0 ���ˣ����� 00 �˳���: ", feedback, MAX_RECORD_LEN);
        
        // ����Ƿ��˳�
        if (strcmp(feedback, "00") == 0)
        {
            printf("����ȡ����\n");
            return 0;
        }
        
        // ����Ƿ����
        if (strcmp(feedback, "0") == 0)
        {
            // ���������Ǽ�����
            while (1)
            {
                char star_input[10];
                get_safe_string("������������Ǽ� (1-5������ 0 ���ˣ����� 00 �˳�): ", star_input, sizeof(star_input));
                
                // ����Ƿ��˳�
                if (strcmp(star_input, "00") == 0)
                {
                    printf("����ȡ����\n");
                    return 0;
                }
                
                // ����Ƿ����
                if (strcmp(star_input, "0") == 0)
                {
                    return 0; // ���˵���һ��
                }
                
                // ת��Ϊ����
                star_rating = atoi(star_input);
                if (star_rating >= 1 && star_rating <= 5)
                {
                    break; // ��ʽ��ȷ���˳�ѭ��
                }
                else
                {
                    printf("?? �Ǽ����۱����� 1-5 ֮�䣬���������룡\n");
                }
            }
            continue; // �ص�������������ѭ��
        }
        
        break; // ������ɣ��˳�ѭ��
    }

    // ������������
    last_completed_record->star_rating = star_rating;
    if (strlen(feedback) > 0)
    {
        strncpy(last_completed_record->feedback, feedback, MAX_RECORD_LEN - 1);
        last_completed_record->feedback[MAX_RECORD_LEN - 1] = '\0';
    }

    printf("\n? ���۳ɹ�����л���ķ�����\n");
    return 1;
}

// ==========================================
// ����Ͷ�߹���ģ��
// ==========================================
int submit_new_complaint(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return 0;
    }

    if (g_account_list == NULL)
    {
        printf("?? �˺�������δ��ʼ����\n");
        return 0;
    }

    // �û���ѡ��Ͷ������
    int target_type;
    while (1)
    {
        printf("\n��ѡ��Ͷ�����ͣ�\n");
        printf("  [1] ��ҽ��\n");
        printf("  [2] �Ի�ʿ/ǰ̨\n");
        printf("  [3] ��ҩʦ\n");
        printf("  [0] ����\n");
        printf("  [00] �˳�\n");
        
        char type_input[10];
        get_safe_string("?? ������ѡ��: ", type_input, sizeof(type_input));
        
        // ����Ƿ��˳�
        if (strcmp(type_input, "00") == 0)
        {
            printf("����ȡ����\n");
            return 0;
        }
        
        // ����Ƿ����
        if (strcmp(type_input, "0") == 0)
        {
            return 0; // ���˵���һ��
        }
        
        // ת��Ϊ����
        target_type = atoi(type_input);
        if (target_type >= 1 && target_type <= 3)
        {
            break; // ��ʽ��ȷ���˳�ѭ��
        }
        else
        {
            printf("?? ��Ч��ѡ�����������룡\n");
        }
    }

    // ����ȫ���˺���������ӡ��Ӧ��ɫ��Ա��
    printf("\n================ ��Ͷ����Ա�б� ================\n");
    AccountNode* curr = g_account_list->next;
    int count = 0;
    while (curr != NULL)
    {
        if ((target_type == 1 && curr->role == ROLE_DOCTOR) ||
            (target_type == 2 && curr->role == ROLE_NURSE) ||
            (target_type == 3 && curr->role == ROLE_PHARMACIST))
        {
            count++;
            printf("[%d] �˺ţ�%s | ������%s | �Ա�%s\n", count, curr->username, curr->real_name, 
                   strlen(curr->gender) > 0 ? curr->gender : "δ����");
        }
        curr = curr->next;
    }

    if (count == 0)
    {
        printf("?? ��ǰû�п�Ͷ�ߵ������Ա��\n");
        return 0;
    }

    // �û����뱻Ͷ�����˺�
    char target_id[MAX_ID_LEN];
    int valid_target = 0;
    char target_name[MAX_NAME_LEN];
    
    while (1)
    {
        get_safe_string("�����뱻Ͷ���˵��˺ţ����� 0 ���ˣ����� 00 �˳���: ", target_id, MAX_ID_LEN);
        
        // ����Ƿ��˳�
        if (strcmp(target_id, "00") == 0)
        {
            printf("����ȡ����\n");
            return 0;
        }
        
        // ����Ƿ����
        if (strcmp(target_id, "0") == 0)
        {
            // ����ѡ��Ͷ������
            while (1)
            {
                printf("\n��ѡ��Ͷ�����ͣ�\n");
                printf("  [1] ��ҽ��\n");
                printf("  [2] �Ի�ʿ/ǰ̨\n");
                printf("  [3] ��ҩʦ\n");
                printf("  [0] ����\n");
                printf("  [00] �˳�\n");
                
                char type_input[10];
                get_safe_string("?? ������ѡ��: ", type_input, sizeof(type_input));
                
                // ����Ƿ��˳�
                if (strcmp(type_input, "00") == 0)
                {
                    printf("����ȡ����\n");
                    return 0;
                }
                
                // ����Ƿ����
                if (strcmp(type_input, "0") == 0)
                {
                    return 0; // ���˵���һ��
                }
                
                // ת��Ϊ����
                target_type = atoi(type_input);
                if (target_type >= 1 && target_type <= 3)
                {
                    break; // ��ʽ��ȷ���˳�ѭ��
                }
                else
                {
                    printf("?? ��Ч��ѡ�����������룡\n");
                }
            }
            
            // ���´�ӡ��Ͷ����Ա�б�
            printf("\n================ ��Ͷ����Ա�б� ================\n");
            curr = g_account_list->next;
            count = 0;
            while (curr != NULL)
            {
                if ((target_type == 1 && curr->role == ROLE_DOCTOR) ||
                    (target_type == 2 && curr->role == ROLE_NURSE) ||
                    (target_type == 3 && curr->role == ROLE_PHARMACIST))
                {
                    count++;
                    printf("[%d] �˺ţ�%s | ������%s | �Ա�%s\n", count, curr->username, curr->real_name, 
                           strlen(curr->gender) > 0 ? curr->gender : "δ����");
                }
                curr = curr->next;
            }
            
            if (count == 0)
            {
                printf("?? ��ǰû�п�Ͷ�ߵ������Ա��\n");
                return 0;
            }
            
            continue; // �ص���Ͷ�����˺�����ѭ��
        }
        
        // У�鱻Ͷ�����Ƿ����
        valid_target = 0;
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
        
        if (valid_target)
        {
            break; // ��Ͷ������Ч���˳�ѭ��
        }
        else
        {
            printf("?? ������˺Ų����ڻ�������ѡͶ�����ͣ����������룡\n");
        }
    }

    // ¼������Ͷ������
    char content[MAX_RECORD_LEN];
    while (1)
    {
        get_safe_string("������Ͷ�����ݣ����� 0 ���ˣ����� 00 �˳���: ", content, MAX_RECORD_LEN);
        
        // ����Ƿ��˳�
        if (strcmp(content, "00") == 0)
        {
            printf("����ȡ����\n");
            return 0;
        }
        
        // ����Ƿ����
        if (strcmp(content, "0") == 0)
        {
            // �������뱻Ͷ�����˺�
            valid_target = 0;
            while (1)
            {
                get_safe_string("�����뱻Ͷ���˵��˺ţ����� 0 ���ˣ����� 00 �˳���: ", target_id, MAX_ID_LEN);
                
                // ����Ƿ��˳�
                if (strcmp(target_id, "00") == 0)
                {
                    printf("����ȡ����\n");
                    return 0;
                }
                
                // ����Ƿ����
                if (strcmp(target_id, "0") == 0)
                {
                    // ����ѡ��Ͷ������
                    while (1)
                    {
                        printf("\n��ѡ��Ͷ�����ͣ�\n");
                        printf("  [1] ��ҽ��\n");
                        printf("  [2] �Ի�ʿ/ǰ̨\n");
                        printf("  [3] ��ҩʦ\n");
                        printf("  [0] ����\n");
                        printf("  [00] �˳�\n");
                        
                        char type_input[10];
                        get_safe_string("?? ������ѡ��: ", type_input, sizeof(type_input));
                        
                        // ����Ƿ��˳�
                        if (strcmp(type_input, "00") == 0)
                        {
                            printf("����ȡ����\n");
                            return 0;
                        }
                        
                        // ����Ƿ����
                        if (strcmp(type_input, "0") == 0)
                        {
                            return 0; // ���˵���һ��
                        }
                        
                        // ת��Ϊ����
                        target_type = atoi(type_input);
                        if (target_type >= 1 && target_type <= 3)
                        {
                            break; // ��ʽ��ȷ���˳�ѭ��
                        }
                        else
                        {
                            printf("?? ��Ч��ѡ�����������룡\n");
                        }
                    }
                    
                    // ���´�ӡ��Ͷ����Ա�б�
                    printf("\n================ ��Ͷ����Ա�б� ================\n");
                    curr = g_account_list->next;
                    count = 0;
                    while (curr != NULL)
                    {
                        if ((target_type == 1 && curr->role == ROLE_DOCTOR) ||
                            (target_type == 2 && curr->role == ROLE_NURSE) ||
                            (target_type == 3 && curr->role == ROLE_PHARMACIST))
                        {
                            count++;
                            printf("[%d] �˺ţ�%s | ������%s | �Ա�%s\n", count, curr->username, curr->real_name, 
                                   strlen(curr->gender) > 0 ? curr->gender : "δ����");
                        }
                        curr = curr->next;
                    }
                    
                    if (count == 0)
                    {
                        printf("?? ��ǰû�п�Ͷ�ߵ������Ա��\n");
                        return 0;
                    }
                    
                    continue; // �ص���Ͷ�����˺�����ѭ��
                }
                
                // У�鱻Ͷ�����Ƿ����
                valid_target = 0;
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
                
                if (valid_target)
                {
                    break; // ��Ͷ������Ч���˳�ѭ��
                }
                else
                {
                    printf("?? ������˺Ų����ڻ�������ѡͶ�����ͣ����������룡\n");
                }
            }
            
            continue; // �ص�Ͷ����������ѭ��
        }
        
        // ���Ͷ�������Ƿ�Ϊ��
        if (strlen(content) > 0)
        {
            break; // Ͷ�����ݲ�Ϊ�գ��˳�ѭ��
        }
        else
        {
            printf("?? Ͷ�����ݲ���Ϊ�գ����������룡\n");
        }
    }

    // �Զ����ɹ������
    char complaint_id[MAX_ID_LEN];
    int complaint_count = 0;
    ComplaintNode* complaint_curr = g_complaint_list->next;
    while (complaint_curr != NULL)
    {
        complaint_count++;
        complaint_curr = complaint_curr->next;
    }
    snprintf(complaint_id, sizeof(complaint_id), "CP-%03d", complaint_count + 1);

    // ���ɵ�ǰϵͳʱ��
    char submit_time[MAX_NAME_LEN];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(submit_time, MAX_NAME_LEN, "%Y-%m-%d %H:%M:%S", t);

    // ����Ͷ�߹����ڵ㲢��������
    ComplaintNode* new_complaint = create_complaint_node(
        complaint_id,
        patient_id,
        target_type,
        target_id,
        target_name,
        content,
        0, // ��ʼ״̬Ϊ������
        NULL,
        submit_time
    );

    if (new_complaint == NULL)
    {
        printf("?? �ڴ����ʧ�ܣ�Ͷ�߹�������ʧ�ܣ�\n");
        return 0;
    }

    insert_complaint_tail(g_complaint_list, new_complaint);

    printf("\n? Ͷ�߹����ύ�ɹ���\n");
    printf("������ţ�%s\n", complaint_id);
    printf("�ύʱ�䣺%s\n", submit_time);
    printf("���ǻᾡ�촦������Ͷ�ߣ���л���ķ�����\n");

    return 1;
}

// ==========================================
// ���߾�������ʱ����ģ��
// ==========================================
void show_patient_visit_timeline(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return;
    }
    
    // ���һ���
    PatientNode* patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("? δ�ҵ��û��ߣ����黼�߱�š�\n");
        return;
    }
    
    // ��ʾ���߻�����Ϣ
    printf("\n================ ?? ���߻�����Ϣ ================\n");
    printf("���߱�ţ�%s\n", patient->id);
    printf("������%s\n", patient->name);
    printf("�Ա�%s\n", patient->gender);
    printf("���䣺%d\n", patient->age);
    printf("Ŀ����ң�%s\n", patient->target_dept);
    printf("ҽ����ţ�%s\n", patient->doctor_id);
    printf("��ǰ״̬��%s\n", get_patient_status_text(patient->status));
    printf("�˻���%.2f Ԫ\n", patient->balance);
    
    // ��ѯ�û�������ԤԼ/�Һż�¼
    AppointmentNode* latest_appt = NULL;
    if (g_appointment_list != NULL)
    {
        AppointmentNode* appt_curr = g_appointment_list->next;
        while (appt_curr != NULL)
        {
            if (strcmp(appt_curr->patient_id, patient_id) == 0)
            {
                latest_appt = appt_curr;
            }
            appt_curr = appt_curr->next;
        }
    }

    // ��ʾʱ����
    printf("\n=============== ���߾�������ʱ���� ===============\n");

    // ����1�����߽�����ʼ����ɣ�
    printf("[��] 1. ���߽���\n");

    // ����2������ is_walk_in ��̬��ʾ
    if (latest_appt != NULL)
    {
        if (latest_appt->is_walk_in == 1)
        {
            printf("[��] 2. �ֳ��Һ�\n");
        }
        else
        {
            printf("[��] 2. ԤԼ�Ǽ�\n");
        }
    }
    else
    {
        printf("[ ] 2. ����ԤԼ/�Һż�¼\n");
    }

    // ����3������ is_walk_in �� appointment_status ��̬��ʾ
    if (latest_appt != NULL)
    {
        if (latest_appt->appointment_status == CANCELLED)
        {
            printf("[��] 3. ԤԼ/�Һż�¼��ȡ����δ����������\n");
        }
        else if (latest_appt->appointment_status == MISSED)
        {
            printf("[��] 3. ԤԼ/�Һ��ѹ��ţ����μ�¼ʧЧ\n");
        }
        else if (latest_appt->is_walk_in == 1)
        {
            if (latest_appt->appointment_status == CHECKED_IN)
            {
                printf("[��] 3. �ֳ����ѹҺţ�ֱ�ӽ���������\n");
            }
            else
            {
                printf("[~] 3. �ֳ����Ѵ������ȴ���Ӵ���\n");
            }
        }
        else
        {
            if (latest_appt->appointment_status == CHECKED_IN)
            {
                printf("[��] 3. ԤԼ����ǩ��������������\n");
            }
            else
            {
                printf("[ ] 3. ԤԼ���ѵǼǣ��ȴ���Ժǩ��\n");
            }
        }
    }
    else
    {
        printf("[ ] 3. ��δ����������\n");
    }

    // ����4��ҽ������
    if (patient->status == STATUS_EXAMINING || patient->status == STATUS_RECHECK_PENDING ||
        patient->status == STATUS_UNPAID || patient->status == STATUS_WAIT_MED ||
        patient->status == STATUS_NEED_HOSPITALIZE || patient->status == STATUS_HOSPITALIZED ||
        patient->status == STATUS_COMPLETED)
    {
        printf("[��] 4. ҽ������\n");
    }
    else
    {
        printf("[ ] 4. ҽ������\n");
    }

    // ����5�����/����
    if (patient->status == STATUS_RECHECK_PENDING)
    {
        printf("[��] 5. ��� / �������У�\n");
    }
    else if (patient->status == STATUS_EXAMINING)
    {
        printf("[��] 5. ��� / ��������\n");
    }
    else if (patient->status == STATUS_UNPAID || patient->status == STATUS_WAIT_MED ||
             patient->status == STATUS_NEED_HOSPITALIZE || patient->status == STATUS_HOSPITALIZED ||
             patient->status == STATUS_COMPLETED)
    {
        printf("[��] 5. ��� / ����\n");
    }
    else
    {
        printf("[ ] 5. ��� / ����\n");
    }

    // ����6���ɷѴ���
    if (patient->status == STATUS_WAIT_MED || patient->status == STATUS_NEED_HOSPITALIZE ||
        patient->status == STATUS_HOSPITALIZED || patient->status == STATUS_COMPLETED)
    {
        printf("[��] 6. �ɷѴ���\n");
    }
    else
    {
        printf("[ ] 6. �ɷѴ���\n");
    }

    // ����7��ȡҩ/סԺ
    if (patient->status == STATUS_NEED_HOSPITALIZE || patient->status == STATUS_HOSPITALIZED)
    {
        printf("[��] 7. ȡҩ / סԺ��סԺ�У�\n");
    }
    else if (patient->status == STATUS_COMPLETED)
    {
        printf("[��] 7. ȡҩ / סԺ����ȡҩ��\n");
    }
    else if (patient->status == STATUS_WAIT_MED)
    {
        printf("[��] 7. ȡҩ / סԺ����ȡҩ��\n");
    }
    else
    {
        printf("[ ] 7. ȡҩ / סԺ\n");
    }

    // ����8���������
    if (patient->status == STATUS_COMPLETED)
    {
        printf("[��] 8. �������\n");
    }
    else if (patient->status == STATUS_NO_SHOW)
    {
        printf("[��] 8. ����������ѹ������ϣ�\n");
    }
    else
    {
        printf("[ ] 8. �������\n");
    }

    // ��ǰ����˵��
    printf("--------------------------------------------------------\n");
    printf("��ǰ���ȣ�");
    switch (patient->status)
    {
        case STATUS_PENDING:
            printf("�����ѽ��������ڵȴ�ҽ������\n");
            break;
        case STATUS_EXAMINING:
            printf("ҽ���ѽ���������ڽ��м��\n");
            break;
        case STATUS_RECHECK_PENDING:
            printf("�������ɣ����ߵȴ�ҽ������\n");
            break;
        case STATUS_UNPAID:
            printf("ҽ���Ѹ���������ڵȴ��ɷ�\n");
            break;
        case STATUS_WAIT_MED:
            printf("�ɷ�����ɣ��������ڵȴ�ȡҩ\n");
            break;
        case STATUS_NEED_HOSPITALIZE:
            printf("ҽ������סԺ���������ڰ���סԺ�Ǽ�\n");
            break;
        case STATUS_HOSPITALIZED:
            printf("�����Ѱ���סԺ������סԺ������\n");
            break;
        case STATUS_COMPLETED:
            printf("���ξ���������ȫ�����\n");
            break;
        case STATUS_NO_SHOW:
            printf("���ιҺ��ѹ�������\n");
            break;
        default:
            printf("״̬δ֪\n");
            break;
    }
    printf("====================================================\n");

    // ����ԤԼ/�Һ���Ϣ
    if (latest_appt != NULL)
    {
        printf("\n������ԤԼ/�Һ���Ϣ��\n");
        printf("��Դ���ͣ�%s\n", latest_appt->is_walk_in == 1 ? "�ֳ���" : "ԤԼ��");
        printf("��ţ�%s\n", latest_appt->appointment_id);
        printf("���ڣ�%s\n", latest_appt->appointment_date);
        printf("ʱ�Σ�%s\n", latest_appt->appointment_slot);
        printf("���ң�%s\n", latest_appt->department);
        printf("ҽ����%s\n", latest_appt->doctor_name);

        if (latest_appt->appointment_status == CANCELLED)
        {
            printf("״̬����ȡ��\n");
            printf("��ǰ˵������ԤԼ/�Һż�¼��ȡ��������Ϊ��ʷ��¼չʾ��\n");
        }
        else if (latest_appt->appointment_status == MISSED)
        {
            printf("״̬���ѹ���\n");
            printf("��ǰ˵������ԤԼ/�Һż�¼�ѹ��ţ����μ�¼��ʧЧ��\n");
        }
        else if (latest_appt->is_walk_in == 1)
        {
            if (latest_appt->appointment_status == CHECKED_IN)
            {
                printf("״̬���ѹҺ�\n");
                printf("��ǰ˵�����������ֳ��ҺŲ����������У��������ǩ����\n");
            }
            else
            {
                printf("״̬����ԤԼ\n");
                printf("��ǰ˵�����ֳ����Ѵ���������Ӵ�����\n");
            }
        }
        else
        {
            if (latest_appt->appointment_status == CHECKED_IN)
            {
                printf("״̬����ǩ��\n");
                printf("��ǰ˵�������������ԤԼǩ�������������С�\n");
            }
            else
            {
                printf("״̬����ԤԼ\n");
                printf("��ǰ˵�������������ԤԼ�Ǽǣ����ھ��ﵱ�쵽Ժǩ����\n");
            }
        }
    }
    else
    {
        printf("\n������ԤԼ/�Һ���Ϣ��\n");
        printf("����ԤԼ/�Һż�¼��\n");
    }

    // �ײ�˵��
    printf("\n˵������ʱ������ݻ��ߵ�ǰ��ҽ״̬��ԤԼ��¼��סԺ��¼�ۺ����ɣ�����չʾ���߱��ξ������̽��ȡ�\n");
}

void query_patient_complaints(const char* patient_id)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("?? ���߱�Ų���Ϊ�գ�\n");
        return;
    }

    if (g_complaint_list == NULL)
    {
        printf("?? Ͷ�߹���������δ��ʼ����\n");
        return;
    }

    // �ռ��û��ߵ�����Ͷ�߼�¼
    ComplaintPtrNode* complaint_list = NULL;
    ComplaintPtrNode* tail = NULL;
    int complaint_count = 0;
    ComplaintNode* curr = g_complaint_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            // �����µ�Ͷ��ָ��ڵ�
            ComplaintPtrNode* new_node = (ComplaintPtrNode*)malloc(sizeof(ComplaintPtrNode));
            if (new_node == NULL)
            {
                continue;
            }
            new_node->complaint = curr;
            new_node->next = NULL;

            // ���ӵ�����
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
        printf("\n?? ����ǰû�з�����κ�Ͷ�ߣ�\n");
        return;
    }

    // �����ӡ��������ǰ��
    printf("\n================ Ͷ�߼�¼��ѯ ================\n");
    
    // �����������Ȳ������ӡ
    ComplaintPtrNode* prev = NULL;
    ComplaintPtrNode* curr_ptr = complaint_list;
    ComplaintPtrNode* next = NULL;
    
    // ��ת����
    while (curr_ptr != NULL)
    {
        next = curr_ptr->next;
        curr_ptr->next = prev;
        prev = curr_ptr;
        curr_ptr = next;
    }
    complaint_list = prev;
    
    // ��ӡ��ת�������
    curr_ptr = complaint_list;
    int index = 1;
    while (curr_ptr != NULL)
    {
        ComplaintNode* complaint = curr_ptr->complaint;
        printf("\n��Ͷ�߹��� %d��\n", index++);
        printf("������ţ�%s\n", complaint->complaint_id);
        printf("�ύʱ�䣺%s\n", complaint->submit_time);
        
        // ��ӡͶ������
        printf("Ͷ�����ͣ�");
        switch (complaint->target_type)
        {
            case 1: printf("��ҽ��\n"); break;
            case 2: printf("�Ի�ʿ/ǰ̨\n"); break;
            case 3: printf("��ҩʦ\n"); break;
            default: printf("δ֪\n"); break;
        }
        
        printf("��Ͷ���ˣ�%s���˺ţ�%s��\n", complaint->target_name, complaint->target_id);
        printf("Ͷ�����ݣ�%s\n", complaint->content);
        
        // ��ӡ����״̬
        printf("����״̬��");
        if (complaint->status == 0)
        {
            printf("�����У������ĵȴ�\n");
        }
        else
        {
            printf("�ѻظ�\n");
            printf("���������%s\n", complaint->response);
        }
        printf("----------------------------------------\n");
        curr_ptr = curr_ptr->next;
    }
    printf("========================================\n");
    
    // �ͷ�����
    curr_ptr = complaint_list;
    while (curr_ptr != NULL)
    {
        ComplaintPtrNode* temp = curr_ptr;
        curr_ptr = curr_ptr->next;
        free(temp);
    }
}

void show_patient_queue_progress_self_service(void)
{
    char patient_id[MAX_ID_LEN] = "";
    char id_card[MAX_ID_LEN] = "";
    
    system("cls");
    printf("========================================================\n");
    printf("              ?? ��ѯ�ҵ��Ŷӽ���\n");
    printf("========================================================\n");
    
    get_safe_string("�����뻼�߱�ţ�", patient_id, MAX_ID_LEN);
    
    if (strcmp(patient_id, "0") == 0 || strcmp(patient_id, "B") == 0)
    {
        printf("������ȡ����\n");
        system("pause");
        return;
    }
    
    PatientNode* patient = find_patient_by_id(g_patient_list, patient_id);
    
    if (patient == NULL)
    {
        printf("\n? δ�ҵ��û��ߣ����黼�߱�š�\n");
        system("pause");
        return;
    }
    
    get_safe_string("����������֤�ţ�", id_card, MAX_ID_LEN);
    
    if (strcmp(id_card, patient->id_card) != 0)
    {
        printf("\n? ����У��ʧ�ܣ��޷���ѯ�Ŷӽ��ȡ�\n");
        system("pause");
        return;
    }
    
    // ����У��ͨ������ʾ�Ŷӽ���
    system("cls");
    printf("========================================================\n");
    printf("              ?? �ҵ��Ŷӽ���\n");
    printf("========================================================\n");
    printf("���߱�ţ�%s\n", patient->id);
    printf("����������%s\n", patient->name);
    printf("��ǰ״̬��%s\n", get_patient_status_text(patient->status));
    printf("Ŀ����ң�%s\n", patient->target_dept);
    printf("����ҽ����%s\n", patient->doctor_id);
    printf("\n");
    
    if (patient->status == STATUS_PENDING)
    {
        if (strlen(patient->doctor_id) > 0)
        {
            // ����ǰ����������
            int waiting_count = 0;
            PatientNode* curr = g_patient_list->next;
            int found_current = 0;
            
            while (curr != NULL && !found_current)
            {
                if (strcmp(curr->id, patient->id) == 0)
                {
                    found_current = 1;
                }
                else if (strcmp(curr->doctor_id, patient->doctor_id) == 0 && curr->status == STATUS_PENDING)
                {
                    waiting_count++;
                }
                curr = curr->next;
            }
            
            printf("��������ȡ�\n");
            printf("ǰ������������%d ��\n", waiting_count);
            
            // ����Ԥ�Ƶȴ�ʱ��
            int total_minutes = waiting_count * 5;
            if (total_minutes >= 60)
            {
                int hours = total_minutes / 60;
                int minutes = total_minutes % 60;
                printf("Ԥ�Ƶȴ�ʱ�䣺Լ %d Сʱ %d ����\n", hours, minutes);
            }
            else
            {
                printf("Ԥ�Ƶȴ�ʱ�䣺Լ %d ����\n", total_minutes);
            }
            
            printf("\n��ʾ��Ԥ�Ƶȴ�ʱ�䰴ÿ������ƽ�� 5 ���ӹ��㣬�����ο���\n");
        }
        else
        {
            printf("��������ȡ�\n");
            printf("��δ�������ҽ������ȴ���ʿ���š�\n");
        }
    }
    else
    {
        // ���ݲ�ͬ״̬��ʾ��ʾ
        switch (patient->status)
        {
            case STATUS_EXAMINING:
                printf("��ʾ������ǰ���ڼ���У�����ȴ�ҽ�����\n");
                break;
            case STATUS_RECHECK_PENDING:
                printf("��ʾ������ǰ����������ȴ�ҽ�����ﰲ�š�\n");
                break;
            case STATUS_UNPAID:
                printf("��ʾ������ǰ�ѿ�����ɷѣ�������ɽɷѡ�\n");
                break;
            case STATUS_WAIT_MED:
                printf("��ʾ������ǰ�ѽɷѴ�ȡҩ����ǰ��ҩ��ȡҩ��\n");
                break;
            case STATUS_NEED_HOSPITALIZE:
                printf("��ʾ������ǰ��סԺ���Ǽǣ���ǰ��סԺ�ǼǴ�������\n");
                break;
            case STATUS_HOSPITALIZED:
                printf("��ʾ������ǰסԺ�С�\n");
                break;
            case STATUS_COMPLETED:
                printf("��ʾ�������ξ����ѽ�����\n");
                break;
            case STATUS_NO_SHOW:
                printf("��ʾ�������ιҺ��ѹ������ϣ������¹ҺŻ���ѯ����̨��\n");
                break;
            default:
                printf("��ʾ����ǰ״̬δ֪��\n");
                break;
        }
    }
    
    printf("========================================================\n");
    system("pause");
}

int is_emergency_request_by_existing_triage(
    const char* symptom,
    const char* appoint_doctor,
    const char* appoint_dept
)
{
    if (!is_blank_string(symptom))
    {
        const char* recommended_dept = recommend_dept_by_symptom(symptom);
        if (recommended_dept != NULL && strcmp(recommended_dept, "�����") == 0)
        {
            return 1;
        }
    }

    if (!is_blank_string(appoint_dept) && strcmp(appoint_dept, "�����") == 0)
    {
        return 1;
    }

    if (!is_blank_string(appoint_doctor) && g_doctor_list != NULL)
    {
        DoctorNode* doctor = find_doctor_by_id(g_doctor_list, appoint_doctor);
        if (doctor != NULL && strcmp(doctor->department, "�����") == 0)
        {
            return 1;
        }
    }

    return 0;
}

int patient_recharge_balance(const char* patient_id, double amount)
{
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("[WARN] 患者编号不能为空！\n");
        return 0;
    }

    if (amount <= 0)
    {
        printf("[WARN] 充值金额必须大于0！\n");
        return 0;
    }

    if (g_patient_list == NULL)
    {
        printf("[WARN] 患者链表尚未初始化！\n");
        return 0;
    }

    PatientNode* patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("[WARN] 未找到对应患者，充值失败！\n");
        return 0;
    }

    patient->balance += amount;

    printf("\n[OK] 充值成功！\n");
    printf("充值金额：%.2f 元\n", amount);
    printf("当前余额：%.2f 元\n", patient->balance);

    return 1;
}

