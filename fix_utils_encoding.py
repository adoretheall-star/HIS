import codecs

# 读取文件（尝试用 GBK 读取）
try:
    with codecs.open('c:\\Users\\Lenovo\\OneDrive\\Desktop\\HIS总\\HIS\\utils.c', 'r', 'gbk', errors='replace') as f:
        content = f.read()
    print("Successfully read with GBK")
except:
    try:
        with codecs.open('c:\\Users\\Lenovo\\OneDrive\\Desktop\\HIS总\\HIS\\utils.c', 'r', 'utf-8', errors='replace') as f:
            content = f.read()
        print("Successfully read with UTF-8")
    except Exception as e:
        print(f"Error reading file: {e}")
        exit(1)

# 写入修复后的文件（用 UTF-8）
with codecs.open('c:\\Users\\Lenovo\\OneDrive\\Desktop\\HIS总\\HIS\\utils.c', 'w', 'utf-8-sig') as f:
    f.write(content)

print("修复完成！")