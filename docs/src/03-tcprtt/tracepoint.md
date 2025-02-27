# tracepoint 實作說明

![tcp](https://hackmd.io/_uploads/Sy-DmkxWyx.png)

三次交握時， socket 的狀態會改變。兩次“狀態改變”相隔的時間相當於 rtt ，也就是“從 **SYN_SENT** 到 **ESTABLISHED** 的時間”和“從 **SYN_RECV** 到 **ESTABLISHED** 的時間”，可以自行計算。

**inet_sock_set_state** 這個 tracepoint 在每次 socket 狀態切換時被觸發，再透過 map 紀錄同個 socket 上次觸發的時間，就可以在狀態變成 **ESTABLISHED** 的時候計算 rtt 。另外，**TCP_CLOSE** 代表連線關閉，可以刪除 map 中的紀錄。

## tcprtt_tp.bpf.c 示範程式碼

```c
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#include "tcprtt.h"

#include "bpf_tracing_net.h"

// TODO: define ring buffer
// TODO: define hash map

SEC("tracepoint/sock/inet_sock_set_state")
int handle_set_state(struct trace_event_raw_inet_sock_set_state *ctx)
{
    // handle ipv4 only
    if (ctx->family != AF_INET)
        return 0;
    
    // TODO:
    // if oldstate, newstate are desired states:
    //     蒐集 ip, port，計算 rtt
    //     ring buffer 發送資料

    // if newstate == TCP_CLOSE:
    //     刪除 map 紀錄
    // else
    //     在 map 紀錄時間
    return 0;
}
```

## 搜集資料

- ip 和 port：從 `struct trace_event_raw_inet_sock_set_state *ctx` 取得
- rtt：`bpf_ktime_get_ns()` 可以取得當前時間，map 紀錄上次觸發的時間，兩者相減得到 rtt

### 參數類別說明

```c
struct trace_event_raw_inet_sock_set_state {
    ...
    const void *skaddr;
    int oldstate;
    int newstate;
    __u16 sport;
    __u16 dport;
    __u8 saddr[4];
    __u8 daddr[4];
};
```

- `skaddr`: 儲存 `struct sock` 的位址，可作為 hash map 的鍵
- `old_state`, `newstate`: socket 狀態，例如： **TCP_ESTABLISHED**, **TCP_SYN_SENT** ，定義在 vmlinux.h
- `saddr`, `daddr`: ip，網路位元組順序，用 `bpf_core_read(dst, sz, src)` 讀取
- `sport`, `dport`: port，本機位元組順序

