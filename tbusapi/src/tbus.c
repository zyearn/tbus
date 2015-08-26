#include "tbus.h"
#include "gcim.h"
#include <arpa/inet.h>
#include <string.h>

#define TBUS_SUCCESS    0

#define MAXLINE 100

struct tagHandleNode {
    int iHandle;
    TBUSADDR iTbusAddr;
    struct tagHandleNode *next;
};

typedef struct tagHandleNode  HandleNode;

static HandleNode *pFirstNode = NULL;
static int iHandleNum = 0;

int tbus_init_ex(const char *pszShmkey, int iFlag) {
    int iShmKey = atoi(pszShmkey); 

    int iRet = gcim_get_and_at(iShmKey);
    if (iRet < 0) {
        printf("error: gcim_get_and_at\n");
        return -1;
    }

    if (!gcim_is_valid()) {
        printf("error: gcim is not valid in tbus_init_ex\n");
        gcim_delete(iShmKey);
        return -1;
    }

    iRet = gcim_attach_channel();
    if (iRet < 0) {
        printf("error: gcim_attach_channel\n");
        return -1;
    }

    return 0;
}

int tbus_new(int *pHandle) {
    HandleNode *pNode = (HandleNode *)malloc(sizeof(HandleNode));
    if (pNode == NULL) {
        printf("error: malloc\n");
        return -1;
    }

    pNode->iHandle = iHandleNum;
    pNode->next = pFirstNode;
    pFirstNode = pNode;

    *pHandle = iHandleNum;
    iHandleNum++;

    return 0;
}

int tbus_delete(int iHandle) {

    return 0;
}

int tbus_addr_aton(const char *pszAddr, TBUSADDR *pAddr) {
    int iRet = 0;

    iRet = inet_addr(pszAddr);
    if (iRet < 0) {
        printf("error: inet_addr, input string is %s\n", pszAddr);
        return -1;
    }

    *pAddr = iRet;
    return iRet;
}

int tbus_bind(int iHandle, TBUSADDR iAddr) {
    if (iHandle < 0) {
        printf("invalid argument iHandle %d", iHandle);
        return -1;
    }

    HandleNode *pNodeCur = pFirstNode;
    while (pNodeCur != NULL) {
        if (pNodeCur->iHandle == iHandle) {
            pNodeCur->iTbusAddr = iAddr;
            break;
        }

        pNodeCur = pNodeCur->next;
    }

    return (pNodeCur == NULL)? -1: 0;
}


int tbus_recv(int   iHandle,
        TBUSADDR    *pSrc,
        TBUSADDR    *pDst,
        void        *pData,
        size_t      *pDataLen,
        int         iFlag) 
{
    int iRet = 0;

    //TODO: support more options
    if (*pSrc != 0 || *pDst != 0) {
        printf("error: at present *pSrc and *pDst must be zero\n");
        return -1;
    }
        
    TBUSADDR iAddr = 0;
    HandleNode *pNodeCur = pFirstNode;
    while (pNodeCur != NULL) {
        if (pNodeCur->iHandle == iHandle) {
            iAddr = pNodeCur->iTbusAddr;
            break;
        }

        pNodeCur = pNodeCur->next;
    }

    if (pNodeCur == NULL) {
        printf("error: can't find iHandle %d in linkedlist\n", iHandle);
        return -1;
    }
    
    iRet = gcim_recv(pSrc, iAddr, pData, pDataLen);
    if (iRet < 0) {
        printf("error: gcim_recv\n");
        return -1;
    }

    return iRet;
}

int tbus_send(int   iHandle,
        TBUSADDR    *pSrc,
        TBUSADDR    *pDst,
        void        *pData,
        size_t      iDataLen,
        int         iFlag)
{
    int iRet = 0;
    int iValid = 0;

    HandleNode *pNodeCur = pFirstNode;
    for (; pNodeCur != NULL; pNodeCur = pNodeCur->next) {
        if (pNodeCur->iHandle != iHandle) {
            continue;
        }

        if (*pSrc == pNodeCur->iTbusAddr) {
            iValid = 1;
            break;
        }
    }
    
    if (!iValid) {
        printf("invalid src addr\n");
        return -1;
    }

    iRet = gcim_send(*pSrc, *pDst, pData, iDataLen);
    if (iRet < 0) {
        printf("error: gcim_send\n");
        return -1;
    }
    
    return 0;
}

char *tbus_error_string(int iErrorCode) {
    static char errMsg[MAXLINE];

    snprintf(errMsg, MAXLINE, "NOT IMPLEMENTED");
    return errMsg;
}
