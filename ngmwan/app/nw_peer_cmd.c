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
#define PEER_PORT_LENGTH_MAX 5  /* port size max*/
#define PEER_ID_LENGTH_MAX 10 /*peer id size max*/
#define CMDID_LENGTH_MAX 10 /*cmd ID size max*/
/*error value */
#define FORMATERROR -1  /*format error */
#define CMDERROR -2     /* command error */
#define SOCKERROR -3     /*socket error */

static void nw_peer_cmd_malloc (char **);
static void nw_peer_cmd_free(int , char **);
static int nw_peer_ioctl(void *,int);


int nw_peer_usage(int argc, char **argv)
{
    /*nw_peer_usage*/
    printrf("\nUsage:\n");
    printf("peer set <mode> <ipv4addr> <port> <peerID>\n");
    printf("peer del <ipv4addr> <peerID>\n");
    printf("peer -d default\n");
    printf("peer -f <filepath> default <ipaddr:port> mode");
    printf("Peer Example\n");
    printf("peer add ser 192.168.1.1 80  1\n");
    printf("peer del ser 192.168.1.1");
    printf("peer -f /home/nw1/pr.txt")
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
    struct nw_peer_entry npe;
    struct nw_peer_info npi;
    int ret ;
    char *p,*q ,**err = NULL;
    if(strcmp(argv[2],"default",7) == 0)
    {
        if(argc != 4)
        {
            /*command error */
            nw_peer_usage(argc,argv);
            return CMDERROR;
        }
        /*default set */
        memset(&npe,0,sizeof(npe));
        inet_pton(AF_INET,argv[3],&npe.src_addr);
        npe.type = NW_SET_DEFPEER;

        ret = nw_peer_ioctl(&npe,NW_PEER);
        if(ret)
        {
            nw_debug_print(NW_PEER_CMD_ERR,NW_PEER_PERR_ADD);
        }
    }
    else
    {
        if(argc != 7)
        {
            /*command error */
            nw_peer_usage(argc,argv);
            return CMDERROR;
        }
        /*peer entry set*/
        memset(&npe,0,sizeof(npe));
      
        inet_pton(AF_INET,argv[3],&npe.bindip);
        
        npe.bindport = atoi(argv[4]);
        npe.peer_id = atoi(argv[5]);
        npe.cli_server = find_mode(argv[6]);
        
        npe.type = NW_SET_PEER;
        
        ret = nw_peer_ioctl(&npe,NW_PEER);
        if(ret)
        {
            nw_debug_print(NW_PEER_CMD_ERR,NW_PEER_PERR_ADD);
        }
    }
    return ret;
}
 int nw_peer_entry_del(int argc, char **argv)
{
    struct nw_peer_entry npe;
    struct nw_peer_info npi;
    int ret ;
    char *p,*q,**err = NULL;

    if(strncmp(argv[2],"default",7) == 0)
    {
        if(argc != 3)
            nw_peer_usage(argc,argv);
            return 0;
    }
    else
    {
        if(argc != 3)
        {
            nw_peer_usage(argc,argv);
            return 0 ;
        }
        memset(&npe,0,sizeof(spe));

        if(inet_pton(AF_INET,argv[3],npe.bindip) <= 0)
            return CMDERROR;
        
    }

    return 0;
}
