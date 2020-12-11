#include "../nwdev.h"
#include "nw_cli.h"
#include "nw_err.h"

/* read line max length */
#define NW_PEER_FILESET_LENGTH_MAX 128

#define PEER_SET_FILENAME 2
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

int nw_peer_usage(int argc, char **argv);
static int nw_peer_ioctl(void *p, int kind);
int nw_peer_entry_add(int argc, char **argv);
int nw_peer_entry_del(int argc, char **argv);
