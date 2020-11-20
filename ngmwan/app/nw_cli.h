#ifndef NW_CLI_H_
#define NW_CLI_H_

#define PUT_BAR_T(str) printf("**********************************************\n");\
                       printf("* %s \n",str);\
                       printf("***********************************************\n");
#define NW_CLI_HISTORY_MAX 20
#define NW_CLI_BUFSIZE 512
#define NW_TOKEN_MAX 8
#define NW_TOKEN_LEN_MAX 64
#define NW_TAB_SIZE 8
#define NW_TAB_WIDTH 60

/*chk func error */
#define NW_CHKERR_SYNTAX -2
#define NW_CHKERR_IPV4ADDR -3 
#define NW_CHKERR_IPV4MASK_VALUE -4
#define NW_CHKERR_IPV6MASK_VALUE -5
#define NW_CHKERR_INVALID_VALUE -6
#define NW_CHKERR_FILE_NOT_FOUND -7
#define NW_CHKERR_NSNAME_LEN -8
#define NW_CHKERR_NSNAME -9
#define NW_CHKERR_IFNAME_LEN -10
#define NW_CHKERR_IF_EXIST -11
#define NW_CHKERR_IP_CMD_ERROR -12
#define NW_CHKERR_IF_NOT_EXIST -13
#define NW_CHKERR_SWITCH -14

#define max(a,b) ((a)>(b) ? (a):(b))

struct nw_cli_cmd_tbl
{
    char *cmd_str;
    char *cmd_exp;
    struct nw_cli_cmd_tbl *next;
    int (*chk_func)(char *, char *);
    int (*call_func)(int, char **);
    int max_len;
 };

/*system command*/
typedef struct  nw_sys_cmd_tbl
{
    char * cmd_str;
}nw_sys_cmd_tbl_t;



/* cli main*/
void nw_call_cmd(char *);
void nw_blank_del(char *);
/* common call */
void nw_debug_print( char *str1, char *str2)
{
    char buf[32];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s %s", str1, str2);
    perror(buf);
}
/*peer entry
int nw_peer_entry_show(int ,char **);
int nw_peer_get_ent_num(struct nw_peer_info*);
*/
int nw_peer_usage(int ,char **);
int nw_peer_entry_add(int ,char **);
int nw_peer_entry_del(int ,char **);
//int nw_peer_get_ent(struct nw_)


/*check funcs*/
int nw_chk_ipv4_mask(char *,char *);
int nw_chk_ipv4(char*,char*);
int nw_chk_num(char*,char*);
int nw_chk_filepath(char *, char *);
int nw_chk_ifname(char*, char*);
extern nw_sys_cmd_tbl_t cmd_sys[];
extern struct nw_cli_cmd_tbl cmd_root[];





#endif