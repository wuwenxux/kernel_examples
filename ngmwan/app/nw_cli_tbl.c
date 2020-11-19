#include <string.h>
#include <arpa/inet.h>
#include <linux/netdevice.h>
#include "nwdev.h"
/*--------------------------------------------------
    save & load conf
    -----------------------------------------------*/
/*load conf */
static struct nw_cli_cmd_tbl load_conf[] =
{
    {"<filepath>","config file filepath", NULL ,nw_chk_filepath,nw_load_conf},
    {NULL}
};
/*save conf*/
static struct nw_cli_cmd_tbl save_conf[] = 
{
    {"<filepath>","config file filepath", NULL, nw_dummy,nw_save_conf},
    {NULL}
};
/*conf root */
static struct nw_cli_cmd_tbl cmd_conf[] = 
{
    {"save","xxx",save_conf, NULL, NULL},
    {"load","xxx",load_conf,NULL, NULL},
    {NULL}
};
/*------------------------------------------------------
system command 
--------------------------------------------------------*/
nw_sys_cmd_tbl_t cmd_sys[]=
{
    {"ip rule"},
    {"ip route"},
    {"ifconfig"},
    {"route"},
    {"ip -6 route"},
    {NULL}
};
/*--------------------------------------------------
show
----------------------------------------------------*/
/*show root */
static struct nw_cli_cmd_tbl cmd_show[]=
{
    {"all","show all config",NULL , NULL ,nw_show_all},
    {"nw","show nw config", NULL, NULL, nw_show_nw},
    {"peer" ,"show nw-pr config",NULL ,NULL, nw_pr_entry_show},
    {"system","show system config", NULL, NULL, nw_show_sys},
    {NULL}
};
static struct nw_cli_cmd_tbl set_peer_v4addr[] =
{
    {"<ipv4addr>","ip v4 address",set_peer_prefix,nw_chk_ipv4,nw_peer_usage},
    {NULL}
};
/*---------------------------------------------------------------------
peer cmd 
-----------------------------------------------------------------------*/
/*
 *file
 * 
 */
/*entry add from file*/
static struct nw_cli_cmd_tbl pr_filepath[] =
{
    {"<filepath>","setting file filepath", NULL, nw_chk_filepath,nw_pr_entry_file},
    {NULL}
};
