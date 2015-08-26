#ifndef _TBUS_H
#define _TBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint32_t    TBUSADDR;

int tbus_init_ex(const char *pszShmkey, int iFlag);

int tbus_new(int *pHandle);
int tbus_delete(int iHandle);

int tbus_addr_aton(const char *pszAddr, TBUSADDR *pAddr);

int tbus_bind(int iHandle, TBUSADDR iAddr);

int tbus_recv(int   iHandle,
        TBUSADDR    *pSrc,
        TBUSADDR    *pDst,
        void        *pData,
        size_t      *pDataLen,
        int         iFlag);

int tbus_send(int   iHandle
        TBUSADDR    *pSrc,
        TBUSADDR    *pDst,
        void        *pData,
        size_t      iDataLen,
        int         iFlag);

char *tbus_error_string(int iErrorCode);

#ifdef __cplusplus
}
#endif

#endif
