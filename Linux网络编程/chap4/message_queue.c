#include <sys/types.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include <sys/ipc.h>

#define K 1024
#define WRITELEN (128 * K)

void msg_show_attr(int msg_id, struct msqid_ds msg_info)
{
    // print messaage attribute
    int ret = -1;
    sleep(1);
    ret = msgctl(msg_id, IPC_STAT, &msg_info);
    if (-1 == ret)
    {
        printf("get the message information fail\n");
        return;
    }
    printf("\n");
    printf("now the queue bytes count: %ld\n", msg_info.msg_cbytes);
    printf("queue message count: %ld\n", msg_info.msg_qnum);
    printf("queue max bytes count: %ld\n", msg_info.msg_qbytes);
    printf("last send message process id: %d\n", msg_info.msg_lspid);
    printf("last recv message process id: %d\n", msg_info.msg_lrpid);
    printf("last send message time: %s\n", ctime(&(msg_info.msg_stime)));
    printf("last recv message time: %s\n", ctime(&(msg_info.msg_rtime)));
    printf("last change time: %s\n", ctime(&(msg_info.msg_ctime)));
    printf("message UID: %d\n", msg_info.msg_perm.uid);
    printf("message GID: %d\n", msg_info.msg_perm.gid);
    printf("\n-------------\n");
}

int main(void)
{
    int ret = -1;
    int msg_flags, msg_id;
    key_t key;
    struct msgmbuf
    {
        long mtype;
        char mtext[10];
    };
    struct msqid_ds msg_info;
    struct msgmbuf msg_mbuf;

    int msg_sflags, msg_rflags;
    char *msgpath = "msg/";

    // get the key
    key = ftok(msgpath, 'b');
    if (key != -1)
    {
        printf(" success create KEY\n");
    }
    else
    {
        printf(" fail create KEY\n");
    }

    // try to remove already exist queue
    msg_id = msgget(key, 0);
    if (msg_id != -1)
    {
        printf("message queue already exists, remove it\n");
        if (msgctl(msg_id, IPC_RMID, NULL) == -1)
        {
            perror("msgctl remove failed");
            return -1;
        }
    }
    // create the message queue
    msg_flags = IPC_CREAT | IPC_EXCL;
    msg_id = msgget(key, msg_flags | 0x0666); // create message
    if (-1 == msg_id)
    {
        printf("message create fail\n");
        return 0;
    }
    msg_show_attr(msg_id, msg_info);

    // send message
    msg_sflags = IPC_NOWAIT;
    msg_mbuf.mtype = 10;
    memcpy(msg_mbuf.mtext, "test", sizeof("test"));
    // msg_mbuf.mtext[sizeof(msg_mbuf.mtext) - 1] = '\0';
    ret = msgsnd(msg_id, &msg_mbuf, sizeof("test"), msg_sflags);

    if (-1 == ret)
    {
        printf("sending message fail\n");
    }
    msg_show_attr(msg_id, msg_info);

    // recv message
    msg_rflags = IPC_NOWAIT | MSG_NOERROR;
    ret = msgrcv(msg_id, &msg_mbuf, sizeof(msg_mbuf.mtext), 10, msg_rflags);
    if (-1 == ret)
    {
        printf("recv message fail\n");
    }
    else
    {
        printf("recv message success, length: %d\n", ret);
    }
    msg_show_attr(msg_id, msg_info);

    // set the message attribute
    msg_info.msg_perm.uid = 8;
    msg_info.msg_perm.gid = 8;
    msg_info.msg_qbytes = 12345;

    ret = msgctl(msg_id, IPC_SET, &msg_info);
    if (-1 == ret)
    {
        printf("set the attribute fail\n");
        return 0;
    }
    msg_show_attr(msg_id, msg_info);

    // delete message queue
    ret = msgctl(msg_id, IPC_RMID, NULL);
    if (-1 == ret)
    {
        printf("delete message fail\n");
        return 0;
    }
    return 0;
}
