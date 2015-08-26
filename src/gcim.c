#include "gcim.h"

static char *pGCIMAddr = NULL;
static char *pCur = NULL;
static int iGCIMShmId = 0;

int gcim_get_and_at(key_t key) {
    if (pGCIMAddr) {
        return 0;
    }
    
    iGCIMShmId = shmget(key, GCIMSIZE, 0777 | IPC_CREAT);
    if (iGCIMShmId < 0) {
        printf("error: shmget\n");
        return -1;
    }

    pGCIMAddr = (char *)shmat(iGCIMShmId, (void *)0, 0);
    if (pGCIMAddr == (char *)(-1)) {
        printf("error: shmat");
        return -1;
    }

    return 0;
}

bool gcim_is_valid() {
    if (!pGCIMAddr) {
        return false;
    }

    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    return pMeta->iMagic == MAGIC;
}

int gcim_create(key_t key) {
    printf("in gcim_create, key=%d\n", key);

    int iRet = gcim_get_and_at(key);
    if (iRet < 0) {
        printf("error: gcim_get_and_at\n");
        return -1;
    }

    pCur = pGCIMAddr + sizeof(GCIMMeta);

    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    pMeta->iMagic = MAGIC;
    pMeta->iShmVer = 0;
    pMeta->iShmKey = key;
    pMeta->iChannelSize = 0;

    return 0;
}

int gcim_set_addr1(uint32_t addr) {
    Channel *pChannel = (Channel *)pCur;

    if (!pCur) {
        printf("error: pCur is NULL");    
        return -1;
    }

    pChannel->iPair1 = addr;

   return 0;

}

int gcim_set_addr2(uint32_t addr) {
    Channel *pChannel = (Channel *)pCur;

    if (!pCur) {
        printf("error: pCur is NULL");    
        return -1;
    }

    pChannel->iPair2 = addr;

    return 0;
}

int gcim_set_send_size(int size) {
    if (size <= 0) {
        printf("error: size shoule greater than 0\n");
        return -1;
    }

    if (!pCur) {
        printf("error: pCur is NULL");    
        return -1;
    }

    Channel *pChannel = (Channel *)pCur;
    pChannel->iSendQueueSize = size;
    
    return 0;
}

int gcim_set_recv_size(int size) {
    if (size <= 0) {
        printf("error: size shoule greater than 0\n");
        return -1;
    }

    if (!pCur) {
        printf("error: pCur is NULL\n");    
        return -1;
    }

    Channel *pChannel = (Channel *)pCur;
    pChannel->iRecvQueueSize = size;
    
    return 0;
}

int gcim_create_channel() {
    printf("in create_channel: pur = %p\n", pCur);
    if (!pCur) {
        printf("error: pCur is NULL\n");
        return -1;
    }
    
    int iRet = 0;
    Channel *pChannel = (Channel *)pCur;
    int iSS = pChannel->iSendQueueSize;
    int iRS = pChannel->iRecvQueueSize;

    if (iSS + iRS <= 0) {
        printf("error: send and recv buffer too samll\n");
        return -1;
    }

    int shmid = shmget(IPC_PRIVATE, iSS+iRS+sizeof(ChannelMeta), 0666 | IPC_CREAT);
    if (shmid < 0) {
        printf("error: get private channel failed\n");
        return -1;
    }

    char *pChannelAddr = (char *)shmat(shmid, (void *)0, 0);
    if (pChannelAddr == (char *)(-1)) {
        printf("error: shmat");
        return -1;
    }

    ChannelMeta *pMeta = (ChannelMeta *)pChannelAddr;
    pMeta->iSendHead = pMeta->iSendTail = 0;
    pMeta->iRecvTail = pMeta->iRecvTail = 0;

    pChannel->iShmId = shmid;
    pCur += sizeof(Channel);

    GCIMMeta *pGCIMMeta = (GCIMMeta *)pGCIMAddr;
    pGCIMMeta->iChannelSize++;

    printf("create id %d succ\n", shmid);

    iRet = shmdt(pChannelAddr);
    if (iRet < 0) {
        printf("error: shmdt channel\n");
        return -1;
    }

}

int gcim_detach() {
    int iRet = shmdt(pGCIMAddr);
    if (iRet < 0) {
        printf("error: shmdt gcim\n");        
        return -1;
    }

    return 0;
}

int gcim_delete(key_t key) {
    int iRet = gcim_get_and_at(key);
    if (iRet < 0) {
        printf("error: gcim_get_and_at\n");
        return -1;
    }
    
    Channel *pCh = (Channel *)(pGCIMAddr + sizeof(GCIMMeta));
    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    printf("in gcim_delete, ver=%d, key=%d, size=%d\n", pMeta->iShmVer, pMeta->iShmKey, pMeta->iChannelSize);
    
    if (gcim_is_valid()) {
        int i=0;
        for (; i<pMeta->iChannelSize; i++) {
            iRet = shmctl(pCh->iShmId, IPC_RMID, NULL);
            if (iRet < 0) {
                printf("error: shmctl channel\n");
            }

            pCh = (char *)pCh + sizeof(Channel);
        }
    }
    

    iRet = gcim_detach();
    if (iRet < 0) {
        printf("error: gcim_detach\n");
        return -1;
    }
    
    iRet = shmctl(iGCIMShmId, IPC_RMID, NULL);
    if (iRet < 0) {
        printf("error: shmctl GCIM\n");
        return -1;
    }

    return 0;
}

int gcim_recv(uint32_t addr, void *pData, size_t *pDataLen) {

}
