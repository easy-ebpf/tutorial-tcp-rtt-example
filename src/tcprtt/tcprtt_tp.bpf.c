#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#include "tcprtt.h"

#include "bpf_tracing_net.h"

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 256 * 1024);
} rb SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 8 * 1024);
	__type(key, struct sock *);
	__type(value, u64);
} record SEC(".maps");

SEC("tracepoint/sock/inet_sock_set_state")
int handle_set_state(struct trace_event_raw_inet_sock_set_state *ctx)
{
	// handle ipv4 only
	if (ctx->family != AF_INET)
		return 0;

	u64 curr = bpf_ktime_get_ns();
	struct sock *sk = (struct sock *)ctx->skaddr;

	// TODO: 加入下面三種state的判斷: TCP_ESTABLISHED, TCP_SYN_RECV, TCP_SYN_SENT
	if (ctx->newstate == /* TODO */ &&
		(ctx->oldstate == /* TODO */ || ctx->oldstate == /* TODO */)) {
		u64 *prev = bpf_map_lookup_elem(&record, &sk);
		if (prev) {
			struct event *e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
			if (e) {
				bpf_core_read(&e->saddr, sizeof(e->saddr), &ctx->saddr); 
				// 用跟前一行同樣的方式取得下面三樣資料(從context ctx到event e)
				// daddr, sport, dport

				e->rtt = (curr - *prev) / 1000;

				e->pid = bpf_get_current_pid_tgid() >> 32;
				bpf_get_current_comm(&e->comm, sizeof(e->comm));
				bpf_ringbuf_submit(e, 0);
			}
		}
	}

	if (ctx->newstate == TCP_CLOSE) {
		bpf_map_delete_elem(&record, &sk);
	} else {
		bpf_map_update_elem(&record, &sk, &curr, BPF_ANY);
	}
	return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";
