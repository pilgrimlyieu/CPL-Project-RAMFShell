#!/bin/bash

# chmod +x check.sh

# 进入 sample 目录
cd sample

# 循环处理 1 到 5 的文件
for i in {1..5}; do
    # 编译 c 文件
    gcc -g -std=c17 -O2 -I ../include $i.c ../fs/ramfs.c ../sh/shell.c -o $i

    # 运行编译后的程序，将输出重定向到 .out 文件，并使用 sed 命令移除 ANSI 转义序列
    ./$i | sed "s/\x1b\[[0-9;]*m//g" > $i.out

    # 使用 diff 命令比较 .out 文件和 .std 文件，将差异输出到 .diff 文件
    diff $i.out $i.std > $i.diff

    # 如果 .diff 文件为空，则表示测试通过
    if [ -s $i.diff ]; then
        echo "Test $i failed."
    else
        echo "Test $i passed."
        rm $i.diff
    fi

    # 删除编译后的程序和 .out 文件
    rm $i $i.out
done

# 返回到原来的目录
cd ..