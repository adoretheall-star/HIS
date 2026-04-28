import re

# 读取文件内容
with open('c:\\Users\\LENOVO\\Desktop\\HIS\\main.c', 'r', encoding='utf-8') as f:
    content = f.read()

# 移除所有冲突标记
content = re.sub(r'<<<<<<< HEAD\n.*?=======\n.*?>>>>>>> C-2-1\n?', '', content, flags=re.DOTALL)

# 写回文件
with open('c:\\Users\\LENOVO\\Desktop\\HIS\\main.c', 'w', encoding='utf-8') as f:
    f.write(content)

print("冲突标记已移除")