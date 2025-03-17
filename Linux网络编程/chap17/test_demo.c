#include <linux/init.h> // 初始化宏 __init, __exit
#include <linux/kern_levels.h>
#include <linux/kernel.h> // 内核日志函数 printk
#include <linux/module.h> // 模块
/*
        说明:
                - sudo apt install --reinstall linux-headers-xxx.xxx 并 reboot
   后将会更新内核版本: uname -r 查看
                - 即下载当前内核版本匹配的头文件包, make 将进行编译
                - sudo insmod xxx.ko 加载内核
                - sudo rmmod xxx 卸载内核
                - dmesg 查看内核打印信息
                - modinfo xxx.ko 查看模块描述信息
                - lsmod 查看已加载的内核模块信息以及依赖关系
                - make 编译
                - make clean 可以删除除 c 程序及 Makefile 的其余文件
*/
MODULE_LICENSE("Dual BSD/GPL"); // 版权声明

// 初始化模块 static 防止名字空间污染
static int __init initialize(void) {
  printk(KERN_ALERT "kernel module init\n");

  // 错误处理
  int ret = 0;
  char *name = (char *)kmalloc(GFP_KERNEL,
                               sizeof(char) * 80); // 申请内存, 类型为GFP_KERNEL
  if (name == NULL) {                              // 申请失败
    ret = 1;
    goto ERROR_name;
  }

  char *address = (char *)kmalloc(GFP_KERNEL, sizeof(char) * 80);
  if (address == NULL) {
    ret = 2;
    goto ERROR_address;
  }

  char *age = (unsigned char *)kmalloc(GFP_KERNEL, sizeof(short));
  if (age == NULL) {
    ret = 3;
    goto ERROR_age;
  }

  return 0;

ERROR_age:
  kfree(address);
ERROR_address:
  kfree(name);
ERROR_name:
  return ret;
}

// 清理模块
static void __exit demo_exit(void) {
  printk(KERN_ALERT "kernel module exit\n");
}

// 模块初始化
module_init(initialize);

// 模块退出
module_exit(demo_exit);

// 信息声明
MODULE_AUTHOR("Jb");
MODULE_DESCRIPTION("chap17 test_demo");
MODULE_VERSION("0.0.1");
MODULE_ALIAS("chapter kernel module");
