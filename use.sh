#!/bin/bash

# chmod +x use.sh

# 定义样例数量
SAMPLE_COUNT=5

# 检查参数数量
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number>"
    exit 1
fi

# 检查参数是否为整数
if ! [[ "$1" =~ ^[0-9]+$ ]]; then
    echo "Error: Argument must be an integer."
    exit 1
fi

# 检查参数是否在 1 到 SAMPLE_COUNT 之间
if [ "$1" -lt 1 ] || [ "$1" -gt $SAMPLE_COUNT ]; then
    echo "Error: Number must be between 1 and $SAMPLE_COUNT."
    exit 1
fi

# 复制 sample 文件夹中的 c 文件到根目录，覆盖 main.c
cp sample/$1.c main.c