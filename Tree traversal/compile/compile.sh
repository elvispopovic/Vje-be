/usr/bin/clang++ -std=c++11 -Wall -g -Iinclude -c tree.cpp -o obj/debug/tree.o

/usr/bin/clang++ -fno-omit-frame-pointer -fdiagnostics-color=always obj/debug/tree.o -o bin/debug/tree