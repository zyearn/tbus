#include "gcim.h"

#define MAXLINE 1024
#define DELIM   "="

#define SHMKEY      "shmKey"
#define ADDRESS     "Address"
#define SENDSIZE    "SendSize"
#define RECVSIZE    "RecvSize"

static int iDeleteKey = -1;
static char *pConfFile = NULL;

int read_conf(char *pFilePath) {
    FILE *fp = fopen(pFilePath, "r");
    if (!fp) {
        printf("error: read_conf\n");
        return -1;
    }

    int iRet = 0;
    char *pDelimPos;
    char buf[MAXLINE];
    enum {
        ADDR1 = 0,
        ADDR2,
    } state;

    state = ADDR1;

    while (fgets(buf, MAXLINE, fp)) {
        if (buf[0] == '\n' || buf[0] == '\r') {
            printf("ready to create channel\n");
            iRet = gcim_create_channel();
            if (iRet < 0) {
                printf("error: gcim_create_channel\n");
                return -1;
            }

            continue;
        }

        pDelimPos = strstr(buf, DELIM);
        if (!pDelimPos) {
            continue;
        }

        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
    
        if (strncmp(SHMKEY, buf, strlen(SHMKEY)) == 0) {
            iRet = gcim_create(atoi(pDelimPos + 1));
            if (iRet < 0) {
                printf("error: gcim_create\n");
                return -1;
            }
            
            continue;
        }

        if (strncmp(ADDRESS, buf, strlen(ADDRESS)) == 0) {
            if (state = 0) {
                iRet = gcim_set_addr1(inet_addr(pDelimPos + 1));
            } else {
                iRet = gcim_set_addr2(inet_addr(pDelimPos + 1));
            }

            if (iRet < 0) {
                printf("error: gcim_set_addr\n");
                return -1;
            }

            state = 1 - state;
            continue;
        }

        if (strncmp(SENDSIZE, buf, strlen(SENDSIZE)) == 0) {
            iRet = gcim_set_send_size(atoi(pDelimPos + 1));
            if (iRet < 0) {
                printf("error: gcim_set_send_size\n"); 
                return -1;
            }

            continue;
        }

        if (strncmp(RECVSIZE, buf, strlen(RECVSIZE)) == 0) {
            iRet = gcim_set_recv_size(atoi(pDelimPos + 1));
            if (iRet < 0) {
                printf("error: gcim_set_recv_size\n"); 
                return -1;
            }

            continue;
        }

        printf("unrecognized line: %s\n", buf);
    }

    return 0;
}

int read_opts(int argc, char **argv) {
    int iRet = 0;

    int i = 1;
    for (; i<argc; i++) {
        if (argv[i][0] != '-') {
            printf("error: invalid argument %s\n", argv[i]);
            return -1;
        }

        switch(argv[i][1]) {
            case 'X':
                if (argv[i][2] == '\0') {
                    iDeleteKey = atoi(argv[++i]);
                } else {
                    iDeleteKey = atoi(&argv[i][2]);
                }

                break;
            case 'C':
                if (argv[i][2] == '\0') {
                    pConfFile = argv[++i];
                } else {
                    pConfFile = &argv[i][2];
                }

                break;
            default:
                printf("error: unknown argument %s\n", argv[i]);
                goto failed;
        }
    }

    return iRet;

failed:
    return -1;
}

void usage() {
    printf("usage: tbusmgr [-h] [-X key] [-C conffile]\n");
}

int main(int argc, char **argv) {
    // read from configure file
    if (argc <= 1) {
        usage();
        return -1;
    }

    int iRet = 0;

    iRet = read_opts(argc, argv);
    if (iRet < 0) {
        return -1;
    }

    if (iDeleteKey != -1) {
        iRet = gcim_delete(iDeleteKey);
        if (iRet < 0) {
            printf("error: gcim_delete\n");
        }

        return iRet;
    }

    if (pConfFile == NULL) {
        printf("error: no conf file\n");
        return -1;
    }

    iRet = read_conf(pConfFile);
    if (iRet < 0) {
        printf("error: read_conf\n");
        return -1;
    }


    iRet = gcim_detach();
    if (iRet < 0) {
        printf("error: gcim_detach()\n");
        return -1;
    }

    return 0;
}
