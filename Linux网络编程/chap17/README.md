# 使用 VSCODE 远程实现 Linux 内核代码编译以及代码跳转
以及最终操作

- kali 开启 ssh
```bash
sudo vim /etc/ssh/sshd_config
# 修改
# PermitRootLogin yes
# PasswordAuthentication yes
/etc/init.d/ssh start
```

- apt 包更新
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install clangd
# 安装 make 所需依赖
sudo apt-get install lld clang llvm libelf-dev libssl-dev bc bison
```

- 安装 linux 头文件
```bash
sudo apt-get install --reinstall linux-headers-6.12.13-amd64
reboot
uname -r
# 6.12.13-amd64
```

- 安装 Linux 源码编译内核
```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.12.13.tar.gz
tar -zxvf linux-6.12.13.tar.gz
cd linux-6.12.13
make LLVM=1 O=./output defconfig
make LLVM=1 O=./output -j12

```


- vscode 安装扩展 clangd, 修改 PATH 为 `which clangd` 结果
- 加入 Arguments
```bash
–background-index
–compile-commands-dir=/home/kali/Desktop/test/linux-6.12.13
-j=4
--all-scopes-completion
–completion-style=detailed
```
- 加入fallbackFlags
```bash
-I/home/kali/Desktop/test/linux-6.12.13/include
-I/home/kali/Desktop/test/linux-6.12.13/arch/arm/include
```

- 生成compile_commands.json文件
```bash
cd linux-6.12.13
python ./scripts/clang-tools/gen_compile_commands.py -d ./output
reboot # 重启后可以实现 ctrl + 鼠标左键 跳转
```

包含 5 个文件: 
- test_demo.c 测试代码
- hook_demo.c 钩子处理代码例子
- nf_sockopte.h 结构体等
- user_client.c 用户层控制程序
- Makefile


使用
```bash
gcc user_client.c -o user
make 
make load

sudo ./user

make unload
make clean
```