#!/bin/bash

# chmod +x check.sh

# 定义样例数量
SAMPLE_COUNT=5

# 检查参数
if [ "$#" -eq 0 ]; then
    set -- "std" $(seq 1 $SAMPLE_COUNT)
elif [[ "$1" =~ ^(std|pro)$ ]]; then
    if [ "$#" -eq 1 ]; then
        set -- "$1" $(seq 1 $SAMPLE_COUNT)
    fi
else
    set -- "std" "$@"
fi

# 进入 sample 目录
cd sample

# 高级测试
if [ "$1" == "pro" ]; then
    # 保存 main.c
    mv ../main.c custom.c

    # 循环处理指定的文件
    shift
    for i in "$@"; do
        # 检查参数是否为整数
        if ! [[ "$i" =~ ^[0-9]+$ ]]; then
            echo "Error: Argument must be an integer."
            exit 1
        fi

        if [ "$i" -lt 1 ] || [ "$i" -gt $SAMPLE_COUNT ]; then
            echo "Error: Number must be between 1 and $SAMPLE_COUNT."
            exit 1
        fi

        # 使用 use.sh 替换 main.c
        pushd .. > /dev/null
        ./use.sh $i
        popd > /dev/null

        # 定义所有的测试
        tests=("test_address" "test_memory" "test_undefined" "test_leak" "test_thread")

        # 初始化通过的测试数量为 0
        passed_tests=0

        # 运行测试
        for test in "${tests[@]}"; do
            pushd .. > /dev/null
            make $test 2> sample/$i.err | sed "s/\x1b\[[0-9;]*m//g" > sample/$i.out
            popd > /dev/null
            diff $i.out $i.std > $i.diff
            rm $i.out

            # 如果 .diff 文件不为空或 .err 文件不为空，则表示测试失败
            if [ -s $i.diff ] || [ -s sample/$i.err ]; then
                echo -e "\033[31mSample-$i '$test' failed.\033[0m"
            else
                # 如果测试通过，增加通过的测试数量
                ((passed_tests++))
            fi

            rm $i.diff $i.err
        done

        # 根据通过的测试数量和总测试数量，输出相应的消息
        if [ $passed_tests -eq ${#tests[@]} ]; then
            echo "Sample-$i passed all ${#tests[@]} tests."
        elif [ $passed_tests -gt 0 ]; then
            echo -e "\033[31mSample-$i passed $passed_tests of ${#tests[@]} tests.\033[0m"
        fi

    done

    # 恢复 main.c
    mv custom.c ../main.c
else
    # 基础测试
    shift
    for i in "$@"; do
        # 检查参数是否为整数
        if ! [[ "$i" =~ ^[0-9]+$ ]]; then
            echo "Error: Argument must be an integer."
            exit 1
        fi

        if [ "$i" -lt 1 ] || [ "$i" -gt $SAMPLE_COUNT ]; then
            echo "Error: Number must be between 1 and $SAMPLE_COUNT."
            exit 1
        fi

        gcc -g -std=c17 -O2 -I ../include $i.c ../fs/ramfs.c ../sh/shell.c -o $i
        ./$i 2>$i.err | sed "s/\x1b\[[0-9;]*m//g" > $i.out
        diff $i.out $i.std > $i.diff
        rm $i $i.out

        if [ -s $i.diff ] || [ -s $i.err ]; then
            echo -e "\033[31mSample-$i failed.\033[0m"
        else
            echo "Sample-$i passed."
            rm $i.diff $i.err
        fi

    done
fi

# 返回到原来的目录
cd ..