#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_endian.h>

#include "tcprtt.h"

#include "bpf_tracing_net.h"

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 256 * 1024);
} rb SEC(".maps");

SEC("fentry/tcp_rcv_established")
int BPF_PROG(tcp_rcv, struct sock *sk)
{
    // handle ipv4 only
    if (sk->__sk_common.skc_family != AF_INET)
        return 0;

    struct event *e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e)
        return 0;

    // TODO: 從socket中取得對應的資料，並填入event的下面三個變數。
    e->saddr = 
    e->daddr = 
    e->sport = 
    e->dport = bpf_ntohs(sk->__sk_common.skc_dport);

	// u32	srtt_us;	/* smoothed round trip time << 3 in usecs */
    struct tcp_sock *ts = tcp_sk(sk);
    e->rtt = BPF_CORE_READ(ts, srtt_us) >> 3;

    e->pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    bpf_ringbuf_submit(e, 0);
    return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";
