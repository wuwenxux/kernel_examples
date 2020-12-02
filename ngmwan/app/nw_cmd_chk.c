#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <ctype.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <nw_cli.h>
#include <linux/types.h>
int nw_chk_port(char *str, char *chk_str)
{
    int ret ; 
    u16 temp = 0;
    char tmp_s[NW_TOKEN_LEN_MAX] = {0};
    strncpy(tmp_s,str,5);
    ret = atoi(tmp_s);
    if(ret < 0 || ret > 65535)
    {
        return NW_CHKERR_PORT;
    }
    return 0;
}

int nw_chk_ipv4(char *str,char *chk_str)
{
    int digit,i,pos;
    char addr_tmp[NW_TOKEN_LEN_MAX];
    char *ip_addr_p,*save_p,*p;

    memset(addr_tmp,0,sizeof(addr_tmp));
    strcpy(addr_tmp,str);

    for( digit = i = pos = 0; addr_tmp[i]; i++)
    {
        if(isdigit(addr_tmp[i]) != 0 )
        {
            digit++;
        }
        else
        {
            if((addr_tmp[i] == '.') && (digit > 0) &&(digit < 4) &&(pos < 4))
            {
                digit = 0;
                pos++;
            }else
            {
                return NW_CHKERR_IPV4ADDR;
            }
        }
    }
    if( pos != 3)
        return NW_CHKERR_IPV4ADDR;
    for( i = 0, p = &addr_tmp[0]; i < 4; i++,p = NULL){
        if((ip_addr_p = (char *) strtok_r(p,".",&save_p)) == NULL)
            return NW_CHKERR_IPV4ADDR;

        if(( 0 > atoi(ip_addr_p)) || (atoi(ip_addr_p) > 255))
        {
            return NW_CHKERR_IPV4ADDR;
        }
    }
    return 0;
}
int nw_chk_num(char *str,char *chk_str )
{
    uint32_t min,max,num;
    int i;
    char buf[NW_CLI_BUFSIZE];
    char *tmp, **err = NULL;

    memset(buf,0,sizeof(buf));
    strcpy(buf,chk_str);
    min = 0;
    for( i = 0 ; buf[i]; i++)
    {
        if(isdigit(buf[i]) == 0)
        {
            continue;
        }
        tmp =   strchr(&buf[i],'-');
        if(tmp){
            *tmp = '\0';
        }
        min = strtoul(&buf[i],err,0);

        for(; buf[i] != '\0'; i++)
        {
                ;
        }
        i++;
        break;
    }
    max = strtoul(&buf[i],err,0);
    for(i = 0 ; str[i];i++)
    {
        if(isdigit(str[i] == 0))
        {
            return NW_CHKERR_INVALID_VALUE;
        }
    }
    errno = 0;
    num = strtoul(str,err,0);
    if(errno == ERANGE)
        return NW_CHKERR_INVALID_VALUE;
    if( num < min || num > max)
    {
        return NW_CHKERR_INVALID_VALUE;
    }
    return 0;
}
int nw_chk_filepath(char *str, char *chk_str)
{
        FILE *fp;
        if((fp = fopen( str,"r")) == NULL)
        {
            return NW_CHKERR_FILE_NOT_FOUND;
        }
        return 0;
}
int nw_chk_ifname(char *str, char *chK_str)
{
    FILE *fp;
    char buf[NW_CLI_BUFSIZE];
    const char *cmdline = "/sbin/ip link";
    char *i, *n;

    if(strlen(str) >= IF_NAMESIZE)
    {
        return NW_CHKERR_IF_NOT_EXIST;
    }
    if((fp = popen(cmdline,"r")) == NULL )
    {
        err(EXIT_FAILURE,"%s",cmdline);
        return NW_CHKERR_IP_CMD_ERROR;
    }
    memset(buf,0,sizeof(buf));
    /*index name search */
    while(fgets(buf,NW_CLI_BUFSIZE,fp) != NULL)
    {
        if(*buf != ' ')
        {
            i = strtok(buf,":");
            if(i == NULL)
            {
                pclose(fp);
                return NW_CHKERR_IP_CMD_ERROR;
            }
            n = strtok(NULL,":");
            if(n == NULL)
            {
                pclose(fp);
                return NW_CHKERR_IP_CMD_ERROR;
            }
            if(strcmp(&n[1],str) == 0)
            {
                pclose(fp);
                return 0;
            }
        }
    }
    pclose(fp);
    return NW_CHKERR_IF_NOT_EXIST;
}
