#include <stdio.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include "../nwdev.h"
#include "nw_cli.h"
#include "nw_err.h"

/* read line max length */
#define NW_PEER_FILESET_LENGTH_MAX 128

#define PR_SET_FILENAME 2
#define ENTRY_ADD 0 /*entry add*/
#define FORMAT_CHK 1 /*format check */

/*option length */
#define CMD_OPTIONS_MAX 6   /*command size max */
#define IPV4_LENGTH_MAX 18  /*IPv4 size max */
#define PEER_LENGTH_MAX 39  /* prefix size max*/
#define CMDID_LENGTH_MAX 10 /*cmd ID size max*/
/*error value */
#define FORMATERROR -1  /*format error */
#define CMDERROR -2     /* command error */
#define SOCKERROR -3     /*socket error */

static void nw_peer_cmd_malloc (char **);
static void nw_peer_cmd_free(int , char **);
static int nw_peer_ioctl(void *,int);
static int nw_peer_sort(const void *, const void *);

int nw_peer_usage(int argc, char **argv)
{
    /*nw_peer_usage*/
    printrf("\nUsage:\n");
    printf("peer add <ipv4addr> <port> <peerID>\n");
    printf("peer del <ipv4addr>\n")
    printf("peer del <peerID>\n")
    printf()
    printf(""); 
    return 0;
}
static int nw_peer_ioctl(void *p, int kind)
{
    struct ifreq req;
    int sock,ret;

    memset(&req,0,sizeof(req));
    req.ifr_data = p;
    strcpy(req.ifr_name,"nw1");

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
        nw_debug_print(NW_PEER_CMD_ERR,NW_PEER_PERR_SOCK);
        return -1;
    }
    
    ret = ioctl(sock,kind,&req);
    if(ret)
        nw_debug_print(NW_PEER_CMD_ERR,NW_PEER_PERR_IOCTL);

    close(sock);
    return ret;
}
int nw_peer_entry_add(int argc, char **argv)
{
    return 0;
}
int nw_peer_entry_del(int argc, char **argv)
{
    return 0;
}