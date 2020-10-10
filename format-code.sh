#!/bin/bash
# 依赖当前目录的.clang-format
# 对src目录做format
#find ./src -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" -o -name "*.hpp" | xargs clang-format -i

if [ -f "pre-commit" ];then
    cp pre-commit ./.git/hooks/pre-commit
fi

find . -name '*.cpp' -o -name '*.h' -o -name "*.hpp$" -o -name "*.cxx$" -o -name "*.c$" -o -name "*.cc$"|xargs -i clang-format-4.0 -i -style=file {}
