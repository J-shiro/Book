#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>

#define PORT_SERV 8888 // 服务器端口
#define BUFF_LEN 256 // 缓冲区大小
#define NUM_DATA 100
#define LENGTH 1024
static char buff[NUM_DATA][LENGTH];

static void process_data(int s, struct sockaddr *client, char *tmp_buff, int n){
    int index;
    if ( n < 4){
        printf("Received data is too short\n");
        return;
    }

    // 提取头部标记，前4字节
    index = ntohl(*((int *)tmp_buff));
    if(index < 0 || index >= NUM_DATA){
        printf("Invalid index: %d\n", index);
        return;
    }

    if(n-4 > LENGTH){
        printf("data too long for buffer: %d\n", n-4);
        return;
    }
    memcpy(buff[index], tmp_buff + 4, n-4); // 数据复制到指定位置
    printf("Data stored at index %d\n", index);

    sendto(s, tmp_buff, n, 0, client, sizeof(*client));
}

static void udpserv_echo(int s, struct sockaddr * client){
    int n; // 接收数据长度
    char tmp_buff[BUFF_LEN];
    socklen_t len; // 地址长度
    while(1){
        len = sizeof(*client);
        // 接收数据放到buff中，并获得客户端地址
        n = recvfrom(s, tmp_buff, BUFF_LEN, 0, client, &len);
        if(n == -1){
            perror("recvfrom failed");
            continue;
        }
        // 将接收到的n个字节发送回客户端
        // sendto(s, tmp_buff, n, 0, client, len);
        
        // 缓冲区溢出对策:根据接收到数据的头部标记，选择合适的缓冲区位置复制数据
        process_data(s, client, tmp_buff, n);
    }
}

int main(int argc, char * argv[]){
    int s;
    struct sockaddr_in addr_serv, addr_clie;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&addr_serv, 0, sizeof(addr_serv));

    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(PORT_SERV);

    bind(s, (struct sockaddr *)&addr_serv, sizeof(addr_serv));

    udpserv_echo(s, (struct sockaddr *)&addr_clie); // 回显处理程序

    return 0;
}

