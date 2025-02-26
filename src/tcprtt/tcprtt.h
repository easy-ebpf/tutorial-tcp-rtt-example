#ifndef TCPRTT_H_
#define TCPRTT_H_

struct event {
    unsigned int saddr;
    unsigned int daddr;
    unsigned short sport;
    unsigned short dport;
    unsigned int rtt;
    int pid;
    char comm[16];
};

#endif
