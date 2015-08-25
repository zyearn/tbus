#ifndef _GCIM_H
#define _GCIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define GCIMSIZE        4096
#define CHANNELMAGIC    0x19ab1c

struct tagGCIMMeta {
    int iShmVer;
    int iShmKey;
    int iChannelSize;
};

typedef struct tagGCIMMeta  GCIMMeta;

struct tagChannel {
    int iShmId;
    uint32_t iPair1;
    uint32_t iPair2;
    int iSendQueueSize;
    int iRecvQueueSize;
};

typedef struct tagChannel   Channel;
 
struct tagChannelMeta {
    int iSendHead;
    int iSendTail;
    int iRecvHead;
    int iRecvTail;
};

typedef struct tagChannelMeta   ChannelMeta;

int gcim_create(key_t key);
int gcim_set_addr1(uint32_t addr);
int gcim_set_addr2(uint32_t addr);
int gcim_set_send_size(int size);
int gcim_set_recv_size(int size);

int gcim_create_channel();

int gcim_detach();
int gcim_delete();

#ifdef __cplusplus
}
#endif

#endif
