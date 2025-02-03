#include<stdio.h>

// union: value和byte共享同一块内存, 可通过byte访问value高低字节
typedef union {
    unsigned short int value; // 短整型变量
    unsigned char byte[2]; // 字符类型
}to; // 测试字节序类型

int main(int argc, char *argv[]){
    to typeorder; 
    typeorder.value = 0xabcd;

    // 小端序判断
    if(typeorder.byte[0] == 0xcd && typeorder.byte[1] == 0xab){
        printf("Low endian byte order"
            "byte[0]: 0x%x, byte[1]: 0x%x\n",
            typeorder.byte[0],
            typeorder.byte[1]);
    }

    // 大端序判断
    if(typeorder.byte[0] == 0xab && typeorder.byte[1] == 0xcd){
        printf("High endian byte order"
            "byte[0]: 0x%x, byte[1]: 0x%x\n",
            typeorder.byte[0],
            typeorder.byte[1]);
    }

    return 0;
}