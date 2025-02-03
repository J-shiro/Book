// 使用协议族函数

#include<netdb.h>
#include<stdio.h>

// 显示协议项目的函数
void display_protocol(struct protoent *pt){
    int i = 0;
    if(pt){
        printf("protocol name: %s, ", pt->p_name);
        if(pt->p_aliases){
            printf("alias name: ");
            while(pt->p_aliases[i]){
                printf("%s ", pt->p_aliases[i]);
                i++;
            }
        }
        printf(", value: %d \n", pt->p_proto); // 协议值
    }
}

int main(int argc, char *argv[]){
    int i = 0;

    // 第一个const表示指针指向的字符是不可修改的，修饰char
    // 第二个const表示指针本身不可改变，修饰指针*
    // 此为指针数组，指针指向char字符串
    const char *const protocol_name[] = { // 要查询的协议名称
        "ip",
        "icmp",
        "tcp",
        "udp",
        "rdp",
        "ipv6",
        NULL
    };

    setprotoent(1); // 使用getprotobyname时不关闭文件/etc/protocols
    while(protocol_name[i] != NULL){
        struct protoent *pt = getprotobyname((const char *)&protocol_name[i][0]);

        if(pt){
            display_protocol(pt);
        }
        i++;
    };
    endprotoent(); // 关闭/etc/protocols
    return 0;
}