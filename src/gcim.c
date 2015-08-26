#include "gcim.h"

static char *pGCIMAddr = NULL;
static char *pCur = NULL;
static int iGCIMShmId = 0;

static void **pChannelAddrs = NULL;

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

int gcim_is_valid() {
    if (!pGCIMAddr) {
        return 0;
    }

    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    return pMeta->iMagic == MAGIC;
}

int gcim_attach_channel() {
    if (!pGCIMAddr || pChannelAddrs) {
        return -1;
    }

    GCIMMeta *pGCIMMeta = (GCIMMeta *)pGCIMAddr;
    int iChannelSize = pGCIMMeta->iChannelSize;

    //TODO: free
    pChannelAddrs = (void **)malloc(sizeof(void *) * iChannelSize);
    
    int i = 0;
    for (; i<iChannelSize; i++) {
        Channel *pCh = (Channel *)(pGCIMAddr + sizeof(GCIMMeta) + i * sizeof(Channel));
        
        pChannelAddrs[i] = shmat(pCh->iShmId, (void *)0, 0);
        if (pChannelAddrs[i] < 0) {
            printf("error: shmat in gcim_attach_channel\n");
            return -1;
        }

        printf("in gcim_attach_channel, index %d, addr %p\n", i, pChannelAddrs[i]);
    }

    return 0;
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
    printf("in gcim_set_addr1, addr1=%d\n", addr);

   return 0;

}

int gcim_set_addr2(uint32_t addr) {
    Channel *pChannel = (Channel *)pCur;

    if (!pCur) {
        printf("error: pCur is NULL");    
        return -1;
    }

    pChannel->iPair2 = addr;
    printf("in gcim_set_addr2, addr2=%d\n", addr);

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

    return 0;
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
    
    if (!gcim_is_valid()) {
        printf("error: gcim is not valid in gcim_delete\n");
    } else {
        int i=0;
        for (; i<pMeta->iChannelSize; i++) {
            iRet = shmctl(pCh->iShmId, IPC_RMID, NULL);
            if (iRet < 0) {
                printf("error: shmctl channel\n");
            }

            pCh = (Channel *)((char *)pCh + sizeof(Channel));
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

static int gcim_read(void *pAddr, int iLen, int *pHead, int *pTail, void *pData, size_t *pDataLen) {
    printf("in gcim_read, pAddr=%p, iLen=%d, Head=%d, tail=%d\n", pAddr, iLen, *pHead, *pTail);

    int iH = *pHead;
    int iT = *pTail;

    if (iH == iT) {
        printf("tbus is empty!\n");
        return -1;
    }
    
    size_t iDataLen;
    char buffer[PACKAGELEN];
    DataHeader *pstHeader = NULL;

    if (iH < iT) {
        DataHeader *pstHeader = (DataHeader *)(pAddr + iH);
        if (pstHeader->iMagic != MAGIC) {
            printf("fatal error: pstHeader->iMagic != MAGIC\n");
            return -1;
        }

        iDataLen = pstHeader->iDataLen;
        memcpy(pData, pAddr + iH + sizeof(DataHeader), iDataLen);
        *pDataLen = iDataLen;

        *pHead = iH + sizeof(DataHeader) + iDataLen;
        assert(*pHead <= iT);
    } else {
        // 分几种情况处理，header可能跨界，或者data可能跨界
        if (iLen - iH < sizeof(DataHeader)) {
            // header 跨界
            memcpy(buffer, pAddr + iH, iLen - iH);
            memcpy(buffer + iLen - iH, pAddr, sizeof(DataHeader) - iLen + iH);

        }
        
    }

    return 0;
}

int gcim_recv(uint32_t *pSrc, uint32_t iDst, void *pData, size_t *pDataLen) {
    int iRet = 0;

    if (!gcim_is_valid()) {
        printf("error: gcim is not valid in gcim_send\n");
        return -1;
    }

    Channel *pCh = (Channel *)(pGCIMAddr + sizeof(GCIMMeta));
    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    
    int i=0;
    for (; i<pMeta->iChannelSize; i++) {
        ChannelMeta *pCMeta = (ChannelMeta *)pChannelAddrs[i];

        if (iDst == pCh->iPair2) {
            printf("in gcim_recv: find iDst pair match!!! first if\n");
            *pSrc = pCh->iPair1;

            iRet = gcim_read((char *)pChannelAddrs[i] + sizeof(ChannelMeta),
                            pCh->iSendQueueSize,
                            &(pCMeta->iSendHead),
                            &(pCMeta->iSendTail),
                            pData,
                            pDataLen);

            if (iRet < 0) {
                printf("error: gcim_read in gcim_read\n"); 
                return -1;
            }

            break;
        }

        if (iDst == pCh->iPair1) {
            printf("in gcim_send: find iDst pair match!!! second if\n");
            *pSrc = pCh->iPair2;

            iRet = gcim_read((char *)pChannelAddrs[i] + sizeof(ChannelMeta),
                            pCh->iRecvQueueSize,
                            &(pCMeta->iRecvHead),
                            &(pCMeta->iRecvTail),
                            pData,
                            pDataLen);

            if (iRet < 0) {
                printf("error: gcim_write in gcim_send\n"); 
                return -1;
            }

            break;
        }

        pCh = (Channel *)((char *)pCh + sizeof(Channel));
    }

    return 0;
}

static int gcim_write(void *pAddr, int iLen, int *pHead, int *pTail, void *pData, size_t iDataLen) {
    int iRet = 0;
    int iPackageLen = iDataLen + sizeof(DataHeader);

    DataHeader stHeader;
    stHeader.iMagic = MAGIC;
    stHeader.iDataLen = iDataLen;

    //TODO: not use this package
    char package[PACKAGELEN];
    memcpy(package, &stHeader, sizeof(stHeader));
    memcpy(package + sizeof(stHeader), pData, iDataLen);

    printf("in gcim_write, pAddr=%p, iLen=%d, ihead=%d, iTail=%d\n", pAddr, iLen, *pHead, *pTail);
    

    int iH = *pHead;
    int iT = *pTail;

    int iEmpty = 0;
    if (iT >= iH) {
        iEmpty = iLen - (iT - iH);
    } else {
        iEmpty = iH - iT;
    }

    if (iPackageLen >= iEmpty) {
        printf("tbus is out of space\n");
        return -1;
    }

    if (iT + iPackageLen > iLen) {
        memcpy(pAddr + iT, package, iLen - iT);
        memcpy(pAddr, package + iLen - iT, iPackageLen - iLen + iT);
    } else {
        memcpy(pAddr + iT, package, iPackageLen);
    }

    *pTail = (iT + iPackageLen) % iLen;
    return 0;
}

int gcim_send(uint32_t iSrc, uint32_t iDst, void *pData, size_t iDataLen) {
    int iRet = 0;

    if (!gcim_is_valid()) {
        printf("error: gcim is not valid in gcim_send\n");
        return -1;
    }

    printf("in gcim_send\n");
    Channel *pCh = (Channel *)(pGCIMAddr + sizeof(GCIMMeta));
    GCIMMeta *pMeta = (GCIMMeta *)pGCIMAddr;
    
    int i=0;
    for (; i<pMeta->iChannelSize; i++) {
        ChannelMeta *pCMeta = (ChannelMeta *)pChannelAddrs[i];

        printf("channel %d, iSrc=%d, iDst=%d, pair1=%d, pair2=%d\n", i, iSrc, iDst, pCh->iPair1, pCh->iPair2);
        if (iSrc == pCh->iPair1 && iDst == pCh->iPair2) {
            printf("in gcim_send: find iSrc iDst pair match!!!\n");

            iRet = gcim_write((char *)pChannelAddrs[i] + sizeof(ChannelMeta),
                            pCh->iSendQueueSize,
                            &(pCMeta->iSendHead),
                            &(pCMeta->iSendTail),
                            pData,
                            iDataLen);

            if (iRet < 0) {
                printf("error: gcim_write in gcim_send\n"); 
                return -1;
            }

            break;
        }

        if (iSrc == pCh->iPair2 && iDst == pCh->iPair1) {
            printf("in gcim_send: find iDst iSrc pair match!!!\n");

            iRet = gcim_write((char *)pChannelAddrs[i] + sizeof(ChannelMeta),
                            pCh->iRecvQueueSize,
                            &(pCMeta->iRecvHead),
                            &(pCMeta->iRecvTail),
                            pData,
                            iDataLen);

            if (iRet < 0) {
                printf("error: gcim_write in gcim_send\n"); 
                return -1;
            }

            break;
        }

        pCh = (Channel *)((char *)pCh + sizeof(Channel));
    }

    return 0;
}

