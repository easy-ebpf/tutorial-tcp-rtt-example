# 範例: minimal程式及其Makefile

核心監測到用戶使用系統呼叫 `write()` 時輸出訊息，用戶附著完核心程式後會不斷使用 `write()` 

## minimal 核心程式

```c
// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */

// 這裡不符常規，用 "linux/bpf.h" 取代 "vmlinux.h"
// 需要定義在 bpf header 前面，因為 header 需要 kernel 類別
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// 部分 bpf api 需要 license
// SEC("license") 指定 symbol's section 
char LICENSE[] SEC("license") = "Dual BSD/GPL";

// supported since Linux 5.5
int my_pid = 0;

// define program type and attachment point
SEC("tp/syscalls/sys_enter_write")
int handle_tp(void *ctx)
{
    // return u64. [63: 32] is tgid, [31: 0] is pid
    // tgid is pid and pid is tid in kernel terminology
    int pid = bpf_get_current_pid_tgid() >> 32;

    if (pid != my_pid)
        return 0;

    // number of arguments is limited
    bpf_printk("BPF triggered from PID %d.\n", pid);

    return 0;
}
```

依據程式類型，函式有對應的簽名。 `handle_tp` 屬於 tracepoint ，參數只有一個指標，類別則是 tracepoint 名稱加上前綴 `trace_event_raw_` ，定義在 `vmlinux.h`

## minimal 用戶程式
### minimal.skel.h

```c
...
    
// kernel 程式對應的結構體
// 在呼叫 api 會使用 maps/progs/links
// bss/data/rodata 用戶可以直接存取
struct minimal_bpf {
	struct bpf_object_skeleton *skeleton;
	struct bpf_object *obj;
	struct {
		struct bpf_map *bss;
	} maps;
	struct {
		struct bpf_program *handle_tp;
	} progs;
	struct {
		struct bpf_link *handle_tp;
	} links;
	struct minimal_bpf__bss {
		int my_pid;
	} *bss;
};
```

skeleton 提供用戶的 api ，簡單的 eBPF 用戶程式可以不用自己呼叫 libbpf ，只需要使用 skeleton api

```c
// setup
static inline struct minimal_bpf *minimal_bpf__open(void) { ... }
static inline int minimal_bpf__load(struct minimal_bpf *obj) { ... }
static inline struct minimal_bpf *minimal_bpf__open_and_load(void) { ... }
static inline int minimal_bpf__attach(struct minimal_bpf *obj) { ... }

// teardown
static inline void minimal_bpf__destroy(struct minimal_bpf *obj) { ... }
static inline void minimal_bpf__detach(struct minimal_bpf *obj) { ... }
```

### minimal.c

```c
int main(int argc, char **argv)
{
    struct minimal_bpf *skel = minimal_bpf__open();
    skel->bss->my_pid = getpid();   // set global variable
    err = minimal_bpf__load(skel);  // Load & verify BPF programs
    err = minimal_bpf__attach(skel);

    for (;;) {
        fprintf(stderr, ".");
        sleep(1);
    }

    minimal_bpf__destroy(skel);
    ...
}
```

- 全域變數必須在程式加載前設定
- tracepoints, kprobes 和特定類型的 eBPF 程式 libbpf 可以查看 `SEC()` 進行自動附著

## eBPF 編譯流程
![截圖 2024-11-08 下午3.50.40](https://hackmd.io/_uploads/SkVxXBs-yg.png)

## makefile
### 基本規則

makefile 是自動化建置工具，透過撰寫文字檔定義檔案的相依和生成規則，將程式碼編譯成執行檔。

```makefile
# 檔名為 makefile 或 Makefile

目標文件 ...：依賴項目 ...  # 多個項目透過空格分開
    終端命令  # 開頭一定要是 TAB
    ...
```

在 makefile 檔案所在的目錄執行 `make <目標文件>` 生成特定目標， `make` 則是生成第一個規則的目標。

特殊符號說明：
- `$^`： 所有依賴項目
- `$<`： 第一個依賴項目
- `$@`： 目標文件

### 示範 makefile

```makefile
# generate vmlinux header
vmlinux.h:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h 

# build bpf kernel object file
minimal.bpf.o: minimal.bpf.c vmlinux.h
	clang -g -O2 -target bpf -c $< -o $@

# generate skeleton
minimal.skel.h: minimal.bpf.o
	bpftool gen skeleton $< > $@

# build app
minimal: minimal.c minimal.skel.h
	clang -g minimal.c -lbpf -lelf -lz -o $@

```

- `-O2`, `-target bpf` 是必要的選項
- `-lbpf -lelf -lz` 執行檔動態連結函式庫
- 執行檔不用連結核心目標檔

## 除錯相關

- 需要提升權限執行程式，也就是使用 `sudo ./<executable>` 執行
- 查看 eBPF 輸出訊息

```shell
$ sudo cat /sys/kernel/debug/tracing/trace_pipe
    <...>-3840345 [010] d... 3220701.101143: bpf_trace_printk: BPF triggered from PID 3840345.
    <...>-3840345 [010] d... 3220702.101265: bpf_trace_printk: BPF triggered from PID 3840345.
```

- 查看當前附著的程式

```shell
$ sudo bpftool perf show
pid 232272  fd 17: prog_id 394  kprobe  func do_execve  offset 0
pid 232272  fd 19: prog_id 396  tracepoint  sys_enter_execve
```
