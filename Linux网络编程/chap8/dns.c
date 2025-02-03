#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>

// 非线程安全函数

void show(struct hostent *ht, char host[]){
    if(ht){
        int i = 0;
        printf("get the host: %s addr\n", host);
        printf("name: %s\n", ht->h_name);
        
        // 协议族
        printf("type: %s\n", ht->h_addrtype == AF_INET?"AF_INET":"AF_INET6");
        // IP地址长度
        printf("length: %d\n", ht->h_length);

        // 打印IP地址
        for(i=0;;i++){
            if(ht->h_addr_list[i] != NULL){
                printf("IP: %s\n", inet_ntoa(*(struct in_addr*)ht->h_addr_list[i]));
            }else{
                break; 
            }
        }

        for(i=0;;i++){
            if(ht->h_aliases[i] != NULL){
                printf("alias %d: %s\n", i, ht->h_aliases[i]);
            }else{
                break;
            }
        }
    }
}

// 不可重入测试
void test(){
    struct hostent *ht = NULL;

    char host[] = "www.sina.com.cn";
    char host1[] = "www.sohu.com";
    struct hostent *ht1 = NULL, *ht2 = NULL;

    ht1 = gethostbyname(host);
    ht2 = gethostbyname(host1);
    int j = 0;
    for(j = 0; j < 2; j++){
        if(j == 0){
            ht = ht2;
            show(ht, host);
        }
        else{
            ht = ht1;
            show(ht, host1);
        }
        
    }
}

int main(int argc, char *argv[]){
    // char host[] = "www.sina.com.cn";
    // struct hostent *ht = NULL;

    // ht = gethostbyname(host);

    // show(ht, host);
    printf("------------------__TEST__------------------\n");
    test();
    return 0;
}
