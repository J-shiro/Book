# 使用方式: make -> make load -> make unload -> make clean

# 目标模块名称
target = hook_demo
obj-m := $(target).o

# 当前路径
CURRENT_PATH:=$(shell pwd)

# 当前内核版本
LINUX_KERNEL:=$(shell uname -r)

# 内核源码绝对路径
LINUX_KERNEL_PATH:=/lib/modules/$(LINUX_KERNEL)/build

# 线程数
JOBS := $(shell nproc)

# 关闭 BTF 生成，避免 eBPF 依赖
KCFLAGS := -DBTF_DECLS_ONLY

# 编译
all:
	@echo "Building Kernel Module $(target) for Kenel $(LINUX_KERNEL)"
	@make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) KCFLAGS="$(KCFLAGS)" modules -j$(JOBS)

clean:
	@echo "Cleaning up build files..."
	@make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean

# 重新编译
rebuild: clean all

# 加载内核模块
load:
	@echo "Loading Kernel module $(target).ko..."
	@sudo insmod $(target).ko || dmesg | tail -20

# 卸载内核模块
unload:
	@echo "Unloading Kernel module $(target).ko..."
	@sudo rmmod $(target) || dmesg | tail -20

reload: unload load

.PHONY: all clean rebuild load unload reload