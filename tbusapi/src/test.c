#include "tbus.h"
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER      "1.1.1.0"
#define CLIENT      "1.1.1.1"

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("need a argument\n");
        return -1;
    }

    int iRet = 0;

    iRet = tbus_init_ex("5987", 0);
    if (iRet < 0) {
        printf("error: tbus_init_ex\n");
        return -1;
    }
    
    int iBusHandle;
    iRet = tbus_new(&iBusHandle);
    if (iRet < 0) {
        printf("error: tbus_new\n");
        return -1;
    }

    printf("after tbus_new, handle=%d\n", iBusHandle);
    
    TBUSADDR iServerAddr;
    TBUSADDR iClientAddr;
    iRet = tbus_addr_aton(SERVER, &iServerAddr);
    if (iRet < 0) {
        printf("error: tbus_addr_aton\n");
        return -1;
    }

    iRet = tbus_addr_aton(CLIENT, &iClientAddr);
    if (iRet < 0) {
        printf("error: tbus_addr_aton\n");
        return -1;
    }

    TBUSADDR iAddr;
    if (argv[1][0] == 'S') {
        iAddr = iServerAddr;
    } else if (argv[1][0] == 'C') {
        iAddr = iClientAddr;
    } else {
        printf("invalid argument %c\n", argv[1][0]);
        return -1;
    }

    iRet = tbus_bind(iBusHandle, iAddr);
    if (iRet < 0) {
        printf("error: tbus_bind\n");
        return -1;
    }

    char testData[100];

    if (argv[1][0] == 'S') {
        strncpy(testData, "I am server!", 12);
        iRet = tbus_send(iBusHandle, &iServerAddr, &iClientAddr, testData, 12, 0);
        if (iRet < 0) {
            printf("error: tbus_send\n");
            return -1;
        }

        TBUSADDR iSrc, iDst;

        while (1) {
            iSrc = iDst = 0;
            size_t iLen = sizeof(testData);

            iRet = tbus_recv(iBusHandle, &iSrc, &iDst, testData, &iLen, 0);
            if (iRet < 0) {
                //printf("error: tbus_recv\n");
                //return -1;
                sleep(1);
                continue;
            }

            printf("data in tbus is %.*s from %d\n", (int)iLen, testData, iSrc);

            strncpy(testData, "I am server!", 12);
            iRet = tbus_send(iBusHandle, &iServerAddr, &iClientAddr, testData, 12, 0);
            if (iRet < 0) {
                printf("error: tbus_send\n");
                return -1;
            }

            sleep(1);
        }

    }

    if (argv[1][0] == 'C') {
        
        TBUSADDR iSrc, iDst;

        while (1) {
            iSrc = iDst = 0;
            size_t iLen = sizeof(testData);

            iRet = tbus_recv(iBusHandle, &iSrc, &iDst, testData, &iLen, 0);
            if (iRet < 0) {
                //printf("error: tbus_recv\n");
                //return -1;
                sleep(1);
                continue;
            }

            printf("data in tbus is %.*s from %d\n", (int)iLen, testData, iSrc);

            strncpy(testData, "I am client!", 12);
            iRet = tbus_send(iBusHandle, &iClientAddr, &iServerAddr, testData, 12, 0);
            if (iRet < 0) {
                printf("error: tbus_send\n");
                return -1;
            }

            sleep(1);
        }
    }

    return iRet;
}
