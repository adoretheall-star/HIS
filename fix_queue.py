import re

with open('utils.c', 'r', encoding='utf-8') as f:
    content = f.read()

# 替换第一处：在职医生的队列人数
old1 = 'printf("%3d 人  [", queue);'
new1 = '''{ char num_buf[32];
                sprintf(num_buf, "%d人", queue);
                print_padded_text(num_buf, 8);
            }
            printf("  [");'''

content = content.replace(old1, new1)

# 替换第二处：不在岗医生的队列人数
old2 = 'printf("  0 人  [");'
new2 = '''{ char num_buf[32];
                sprintf(num_buf, "0人");
                print_padded_text(num_buf, 8);
            }
            printf("  [");'''

content = content.replace(old2, new2)

with open('utils.c', 'w', encoding='utf-8') as f:
    f.write(content)

print("修改完成！")