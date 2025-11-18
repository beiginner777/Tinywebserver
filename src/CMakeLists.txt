# 将当前目录的所有文件放在变量中
aux_source_directory(. SRC_LIST)
# 生成可执行文件
add_executable(sever ${SRC_LIST})