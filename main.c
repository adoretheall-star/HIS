// ==========================================
// 文件名: main.c
// 描述: 智慧医疗信息管理系统 - 主入口
// 作者:周宇轩
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "global.h"
#include "list_ops.h"
// 这里未来会引入你们自己写的头文件
// #include "global.h"     // 全局常量与结构体定义
// #include "data_io.h"    // 负责读写 txt 文件的模块
// #include "auth.h"       // 负责登录与权限的模块
// #include "utils.h"      // 存放 get_safe_int 终端输入拦截器等工具函数
int main() 
{
    // ---------------------------------------------------------
    // 第一阶段：系统初始化 (加载数据)
    // ---------------------------------------------------------
    printf("======================================================\n");
    printf("     🚀 正在启动智慧医疗信息管理系统 (HIS) ...\n");
    printf("======================================================\n");
    
    // 这里未来调用 data_io.c 里的函数，把 txt 数据全部读进双向链表
    // printf("-> 正在加载患者数据...\n");
    // init_patient_list(); 
    // printf("-> 正在加载医生与科室数据...\n");
    // init_doctor_list();
    // printf("-> 正在加载药品库存数据...\n");
    // init_medicine_list();
    
    printf("\n✅ 系统初始化完成，数据加载成功！\n");
    // 模拟一个加载停顿，让控制台看起来很高级 (Windows下用Sleep, Linux下用sleep)
    // Sleep(1000); 

    // ---------------------------------------------------------
    // 第二阶段：主事件循环 (防崩溃核心)
    // ---------------------------------------------------------
    printf("⚙️ 正在点火启动底层双向链表引擎...\n"
);
    
    // 1. 初始化所有链表头节点
    g_patient_list = init_patient_list();
    g_doctor_list = init_doctor_list();
    g_medicine_list = init_medicine_list();
    g_ward_list = init_ward_list();
    g_account_list = init_account_list();

    // 2. 注入一组极端测试数据
    insert_patient_tail(g_patient_list, create_patient_node(
"P-001", "张三", 19, "110101199001011234"
));
    insert_doctor_tail(g_doctor_list, create_doctor_node(
"D-001", "李大夫", "外科"
));
    insert_medicine_tail(g_medicine_list, create_medicine_node(
"M-001", "阿莫西林", 15.5, 100
, MEDICARE_CLASS_A));
    insert_ward_tail(g_ward_list, create_ward_node(
"W-101"
));
    insert_account_tail(g_account_list, create_account_node(
"admin", "123456", "超级管理员"
, ROLE_ADMIN));

    // 3. 打印核验
    printf("✅ 引擎全部就绪！\n"
);
    printf(" -> 测试患者: %s\n"
, g_patient_list->next->name);
    printf(" -> 测试医生: %s (%s)\n"
, g_doctor_list->next->name, g_doctor_list->next->department);
    printf(" -> 测试药品: %s (库存:%d)\n"
, g_medicine_list->next->name, g_medicine_list->next->stock);
    printf(" -> 测试床位: %s\n"
, g_ward_list->next->bed_id);
    printf(" -> 测试超管: %s (权限级别:%d)\n\n"
, g_account_list->next->real_name, g_account_list->next->role);
// 测试查找引擎
    PatientNode* search_result = find_patient_by_id(g_patient_list, "P-001");
    if (search_result != NULL) {
        printf("🔍 雷达搜索成功！找到目标：患者姓名 [%s]，当前余额 [%.2f]\n\n", search_result->name, search_result->balance);
    } else {
        printf("❌ 雷达搜索失败，查无此人！\n\n");
    }
    // 🚀 时停魔法：按任意键后才清屏进入菜单！
    system(
"pause"
);
    int running = 1; // 控制系统运行状态的标志位

    while (running) 
    {
        // 每次循环清空屏幕，保持 UI 整洁 (Windows用cls, Mac/Linux用clear)
        system("cls"); 
        
       printf("\n======================================================\n");
        printf("               🏥 社区智慧医疗管理系统\n");
        printf("======================================================\n");
        printf("  [1] 🔑 内部登录 (医生/护士/药房专员/管理员)\n");
        printf("  [2] 👨‍⚕️ 快捷挂号 (智能分诊通道)\n");
        printf("  [3] 🔍 患者自助 (查病历 / 医保账单结算)  <-- 新增的患者视角\n");
        printf("  [4] 📊 医疗大屏 (内部数据可视化展示)\n");
        printf("  [0] 🚪 安全退出系统\n");
        printf("------------------------------------------------------\n");

        int choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice) {
            case 1:
                printf("\n[跳转] -> 进入内部人员登录模块...\n");
                // system_login(); 
                break;
            case 2:
                printf("\n[跳转] -> 进入快捷挂号通道...\n");
                // quick_register();
                break;
            case 3:
                // 🌟 这里就是专门为患者视角设计的通道！
                printf("\n[跳转] -> 进入患者自助查询终端...\n");
                // patient_self_service(); 
                // 在这个函数里，患者输入姓名或编号，系统遍历处方链表打印结算单
                break;
            case 4:
                printf("\n[跳转] -> 加载数据可视化大屏...\n");
                // show_dashboard();
                break;
            case 0:
                // 准备退出主循环
                printf("\n正在准备退出系统...\n");
                running = 0; 
                break;
            default:
                // 处理 0-3 之外的非法数字
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }

    // ---------------------------------------------------------
    // 第三阶段：系统清理与安全退出 (数据持久化)
    // ---------------------------------------------------------
    printf("\n======================================================\n");
    printf("     💾 正在保存数据到本地文件 (XOR 加密处理)...\n");
    
    // 这里未来调用 data_io.c 里的函数，把当前双向链表的数据覆盖写入 txt
    // save_patient_list();
    // save_doctor_list();
    // save_medicine_list();

    printf("     🧹 正在释放链表内存，清空回收站...\n");
    // 销毁所有双向链表，防止内存泄漏 (非常重要，代码检查必看)
    // destroy_all_lists(); 

    printf("\n✅ 数据已安全保存，感谢使用！再见！\n");
    printf("======================================================\n");

    return 0;
}

