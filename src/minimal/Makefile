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
	clang -g $< -lbpf -lelf -lz -o $@
