#include <stdio.h>
#include <string.h>

#define DATA_DIR "data/"

// 简单的 trim 函数
void trim_newline(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len = strlen(str);
    }
}

// 简单的分割函数
int split_line_by_delimiter(const char* line, char delimiter, char** fields, int max_fields) {
    if (line == NULL || fields == NULL || max_fields <= 0) return 0;
    
    static char line_copy[8192];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    int count = 0;
    char* start = line_copy;
    char* p = line_copy;
    
    while (*p && count < max_fields) {
        if (*p == delimiter) {
            *p = '\0';
            fields[count++] = start;
            start = p + 1;
        }
        p++;
    }
    fields[count++] = start;
    
    return count;
}

int main() {
    // 测试文件路径
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%spatients.txt", DATA_DIR);
    printf("测试读取患者文件: %s\n", file_path);
    
    // 打开文件
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("无法打开文件: %s\n", file_path);
        return 1;
    }
    
    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        printf("无法读取表头\n");
        fclose(fp);
        return 1;
    }
    
    // 读取数据行
    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;
        
        printf("\n读取到行: %s\n", line);
        
        // 分割字段
        char* fields[30];
        int field_count = split_line_by_delimiter(line, '|', fields, 30);
        printf("字段数: %d\n", field_count);
        
        // 打印前几个字段
        for (int i = 0; i < field_count && i < 10; i++) {
            printf("字段[%d]: %s\n", i, fields[i]);
        }
        
        // 测试身份证号字段
        if (field_count >= 5) {
            printf("身份证号字段: %s\n", fields[4]);
        }
    }
    
    fclose(fp);
    return 0;
}
