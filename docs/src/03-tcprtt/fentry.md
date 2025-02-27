# fentry 實作說明

`void tcp_rcv_established(struct sock *sk, struct sk_buff *skb);`

上面是附著的核心函式。當連線建立以後，每次用戶傳輸都會呼叫此函式。 fentry 的函式簽名會與附著的函式相同，並且可以讀取引數，例如 `sk`、`skb`，就可以藉此得到所需資訊。

## tcprtt.bpf.c 示範程式碼

```c
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_endian.h>

// for the definition of the type shared between user and kernel
#include "tcprtt.h"

#include "bpf_tracing_net.h"
    
// TODO: define ring buffer

SEC("fentry/tcp_rcv_established")
int BPF_PROG(tcp_rcv, struct sock *sk /*, optional */)
{
    // handler ipv4 only
    if (sk->__sk_common.skc_family != AF_INET)
        return 0;
    
    // TODO:
    // 蒐集 ip, port...
    // ring buffer 發送蒐集的資料
    return 0;
}
```

使用 `BPF_PROG()` 定義 fentry 函式， `tcp_rcv` 是實際函式的名稱，後面則是參數，要依序對應附著的核心函式的參數。

## 蒐集資料

- pid, command：使用 `bpf_get_current_pid_tgid()`、`bpf_get_current_comm()`，用法參考 minimal 和 bootstrap
- ip, port 和 rtt：需要從 `struct sock *sk` 取得

### ip 和 port

被封裝在 `__sk_common`  中

```c
struct sock {
    struct sock_common    __sk_common;
    ...
};
```

類別其實都是整數，然後 `skc_rcv_saddr` 是來源 ip ，`skc_num` 是來源 port 。 只有 `skc_num` 是以 host endian ，其他皆為 network endian

```c
/**
 *	struct sock_common - minimal network layer representation of sockets
 *	@skc_daddr: Foreign IPv4 addr
 *	@skc_rcv_saddr: Bound local IPv4 addr
 *	@skc_dport: placeholder for inet_dport/tw_dport
 *	@skc_num: placeholder for inet_num/tw_num
 *	@skc_family: network address family
 *  ...
 */
 
struct sock_common {
    __be32	skc_daddr;
    __be32	skc_rcv_saddr;
    ...
}
```

核心程式用 host endian 紀錄 port ，network endian 紀錄 ip，可以利用 `bpf_ntohs()`

### rtt

使用 bpf_tracing_net.h 中的 `tcp_sk()`，傳入 `sk` 呼叫得到 `struct tcp_sock *` 。

```c
struct tcp_sock {
    u32	srtt_us;	/* smoothed round trip time << 3 in usecs */`
    ...
};
```

將 `srtt_us` 右移三位元就是 rtt 。因為 `struct tcp_sock` 是核心記憶體，得用 `BPF_CORE_READ` 讀取。

## 測試範例

在本機可以透過命令列工具簡單製造 tcp 連線

```shell
# server
$ python3 -m http.server

# client
$ curl 0.0.0.0:8000
```

比對輸出紀錄的 ip、port ，確認運作正常
