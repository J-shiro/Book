#include <stdio.h>
#include <arpa/inet.h>
#define BITS16 16
#define BITS32 32

// 16位
typedef union {
    unsigned short int value;
    unsigned char byte[2];
}to16;

// 32位
typedef union {
    unsigned long int value;
    unsigned char byte[4];
}to32;

void showvalue(unsigned char *begin, int flag){
    int num = 0, i = 0;
    if(flag == BITS16){
        num = 2;
    }else if (flag == BITS32){
        num = 4;
    }

    for (i = 0; i < num; i++){
        printf("%x ", *(begin + i));
    }
    printf("\n");
}

int main(int argc, char *argv[]){
    to16 v16_origin, v16_turn1, v16_turn2;
    to32 v32_origin, v32_turn1, v32_turn2;

    v16_origin.value = 0xabcd;
    v16_turn1.value = htons(v16_origin.value);
    v16_turn2.value = htons(v16_turn1.value);

    v32_origin.value = 0x12345678;
    v32_turn1.value = htonl(v32_origin.value);
    v32_turn2.value = htonl(v32_turn1.value);

    // 结果打印
    printf("16 host to network byte order change:\n");
    printf("\torigin:\t");
    showvalue(v16_origin.byte, BITS16);
    printf("\t1 times:");
    showvalue(v16_turn1.byte, BITS16);
    printf("\t2 times:");
    showvalue(v16_turn2.byte, BITS16);

    printf("32 host to network byte order change:\n");
    printf("\torigin:\t");
    showvalue(v32_origin.byte, BITS32);
    printf("\t1 times:");
    showvalue(v32_turn1.byte, BITS32);
    printf("\t2 times:");
    showvalue(v32_turn2.byte, BITS32);

    return 0;
}
