#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/netdevice.h>
#include "../nwdev.h"
#include "nw_cli.h"
/*
 * NW
 * Stateless Automatic IPv4 over IPv6 Tunneling
 *
 * NW setting command.
 *
 * nw_cli.c
 *
 * 
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/netdevice.h>
#include "../include/nw.h"
#include "nw_cli.h"

static int nw_chk_cmdstr_max(struct nw_cli_cmd_tbl *);
static int nw_print_all_cand(struct nw_cli_cmd_tbl *);
static void nw_chk_tab(char *, int *, int *);
static void nw_print_buf(int, char **, char *, int *, int *);
static void nw_cli_clr_line(char *, int *, int *);
static void nw_set_term(struct termios *);
static void nw_restore_term(struct termios *);
static void nw_print_sup(int, char **, int, char *, struct nw_cli_cmd_tbl *, int *, int *);
static void nw_print_cand(int, char **, int, char *, struct nw_cli_cmd_tbl *, int, int *, int *);
static void init_hist(hist_t *);

int main (int argc, char **argv) {

	struct termios save_term;
	hist_t hist[nw_CLI_HISTORY_MAX];
	char buf[nw_CLI_BUFSIZE], save_buf[nw_CLI_BUFSIZE], ch;
	hist_t *cur, *save_cur;
	int hist_flg;
	int end_pos = 0;	//文字列末尾位置('\0'の位置)
	int cursor_pos = 0;	//カーソル位置

	/* 外部入力 */
	if (argc == 3) {
		if (strcmp(argv[1], "-f") != 0) {
			printf("command error.\n");
			return -1;
		}

		if (nw_load_conf(argc, argv) != 0) {
			//printf("command error.\n");
			return -1;
		}
		return 0;
	}

	/* 非カノニカルモードへ */
	nw_set_term(&save_term);

	memset(hist, 0, sizeof(hist));
	init_hist(hist);
	cur = hist;

	for (;;) {
		memset(buf, 0, sizeof(buf));
		memset(save_buf, 0, sizeof(save_buf));
		save_cur = cur;
		hist_flg = 0;
		end_pos = 0;
		cursor_pos = 0;

		PUT_PROMPT;

		for (;;) {
			ch = fgetc(stdin);

			if (isprint(ch)) {
				if (end_pos >= (NW_CLI_BUFSIZE - 1)) {
					 ;
				} else {
					memmove(&buf[cursor_pos + 1], &buf[cursor_pos], (end_pos - cursor_pos + 1));
					buf[cursor_pos] = ch;
					buf[end_pos + 1] = '\0';
					printf("\x1b[0J");	//カーソル位置から末尾までクリア
					printf("%s", &buf[cursor_pos]);
					if (cursor_pos < end_pos) {
						printf("\x1b[%dD", (end_pos - cursor_pos));
					}
					end_pos++;
					cursor_pos++;
				}
			} else if ((ch == '\n') || (ch == '\r')) {
				printf("%c", ch);
				break;
			} else if (ch == '\t') {
				/* TAB */
				nw_chk_tab(buf, &end_pos, &cursor_pos);
			} else if (ch == '\b' || ch == 0x7f) {
				/* BackSpace */
				if (cursor_pos <= 0) {
					;
				} else {
					memmove(&buf[cursor_pos - 1], &buf[cursor_pos], (end_pos - cursor_pos + 1));
					buf[end_pos - 1] = '\0';
					printf("\b");
					printf("\x1b[0J");
					printf("%s", &buf[cursor_pos - 1]);
					if (cursor_pos < end_pos) {
						printf("\x1b[%dD", (end_pos - cursor_pos));
					}
					end_pos--;
					cursor_pos--;
				}
			} else if (ch == 0x1b) {
				/* エスケープシーケンス */
				ch = fgetc(stdin);
				if (ch == '[') {
					ch = fgetc(stdin);
					if (ch == 'A') {
						/* 上キー */
						if (strlen(cur->prev->str) != 0) {
							if (hist_flg == 0) {
								hist_flg = 1;
								strcpy(save_buf, buf);  /* 現在の入力を保存 */
							}
							if (cur->prev != save_cur) {
								nw_cli_clr_line(buf, &end_pos, &cursor_pos);
								strcpy(buf, cur->prev->str);
								printf("%s", buf);
								end_pos = strlen(buf);
								cursor_pos = strlen(buf);
								cur = cur->prev;
							}
						}
					} else if (ch == 'B') {
						/* 下キー */
						if (hist_flg == 1) {
							nw_cli_clr_line(buf, &end_pos, &cursor_pos);
							if (cur->next == save_cur) {
								strcpy(buf, save_buf);
								hist_flg = 0;
							} else {
								strcpy(buf, cur->next->str);
							}
							printf("%s", buf);
							end_pos = strlen(buf);
							cursor_pos = strlen(buf);
							cur = cur->next;
						}
					} else if(ch == 'C') {
						/* 右キー */
						if (cursor_pos < end_pos) {
							printf("\x1b[C");	/* カーソルを右移動 */
							cursor_pos++;
						}

					} else if (ch == 'D') {
						/* 左キー */
						if (cursor_pos > 0) {
							printf("\x1b[D");	/* カーソルを左移動 */
							cursor_pos--;
						}

					} else {
						/* その他矢印キー */
						;
					}
				}
			} else if (ch == 0x3) {
				/* CTRL+C */
				raise(SIGINT);  /* デバッグ用 */
			} else {
				/* その他 */
				;
			}
		}

		if (strlen(buf)) {

			nw_blank_del(buf);
			if (strlen(buf) == 0) {
				continue;
			}

			/* history保存 */
			memcpy(save_cur->str, buf, NW_CLI_BUFSIZE-1);
			cur = save_cur->next;

			if (strcmp(buf, "exit") == 0) {
				nw_restore_term(&save_term);
				exit(0);
			} else {
				nw_call_cmd(buf);
			}

		}

	}

	return 0;
}

void nw_call_cmd(char *buf)
{

	struct nw_cli_cmd_tbl *cmdp;
	int argc, i, ret, chk_ret = 0;
	int command_found = 0;
	char *argv[NW_TOKEN_MAX], *p, *save_p;
	char token[NW_TOKEN_MAX][NW_TOKEN_LEN_MAX];
	char *cmd_str_p = NULL;
	char *cmd_exp_p = NULL;

	memset(argv, 0, sizeof(argv));
	memset(token, 0, sizeof(token));
	ret = 0;
	save_p = NULL;

	for (argc = 0, p = buf; (p = strtok_r(p, " ", &save_p)) != NULL; p = NULL, argc++) {
		if (argc > (NW_TOKEN_MAX - 1)) {
			printf("command error. too much tokens.\n");
			return;
		} else {
			strcpy(token[argc], p);
			argv[argc] = token[argc];
		}
	}

	if (!argc)
		return;

	cmdp = cmd_root;

	for (i = 0; cmdp->cmd_str != NULL; ) {
		if (cmdp->chk_func != NULL) {
			chk_ret = cmdp->chk_func(argv[i], cmdp->cmd_str);
			cmd_str_p = cmdp->cmd_str;
			cmd_exp_p = cmdp->cmd_exp;
			if (chk_ret == 0) {
				goto CALL_NEXT;
			} else {
				cmdp++;
				if (cmdp == NULL) {
					break;
				} else {
					continue;
				}
			}
		} else {
			if (strcmp(argv[i], cmdp->cmd_str) == 0) {
CALL_NEXT:
				command_found = 1;
				if (cmdp->next == NULL) {
					if (i == (argc -1)) {
						break;
					} else {
						cmdp++;
						i++;
					}
				} else {
					/* 次のツリー */
					cmdp = cmdp->next;
					i++;
					if (i == argc) {
						break;
					}
				}
			} else {
				cmdp++;
			}
		}

	}

	if (cmdp->call_func != NULL) {
		ret = cmdp->call_func(argc, argv);
		if (ret != 0) {
			printf("command execution error.\n");
		}

	} else {
		if (command_found != 1) {	//該当するコマンド自体が見つからなかった場合
			for (i = 0; i < argc; i++) {
				printf("%s ", argv[i]);
			}
			printf(": command not found.\n");
		} else if (chk_ret == NW_CHKERR_IPV4ADDR) {
			printf("%s : invalid ipv4 address.\n", cmd_str_p);
		} else if (chk_ret == NW_CHKERR_IPV4MASK_VALUE) {
			printf("%s : invalid mask value.\n", cmd_str_p);
		} else if (chk_ret == NW_CHKERR_IPV6ADDR) {
			printf("%s : invalid ipv6 address.\n", cmd_str_p);
		} else if (chk_ret == NW_CHKERR_INVALID_VALUE) {
			printf("%s : invalid value. expected %s.\n", cmd_exp_p, cmd_str_p);
		} else if (chk_ret == NW_CHKERR_FILE_NOT_FOUND) {
			printf("file not found.\n");
		} else if (chk_ret == NW_CHKERR_NSNAME_LEN) {
			printf("NameSpace name length must be 65 characters or less.\n");
		} else if (chk_ret == NW_CHKERR_NSNAME) {
			printf("invalid NameSpace name.\n");
		} else if (chk_ret == NW_CHKERR_IFNAME_LEN) {
			printf("device name length must be 16 characters or less.\n");
		} else if (chk_ret == NW_CHKERR_IF_EXSIST) {
			printf("specified device name already exists.\n");
		} else if (chk_ret == NW_CHKERR_IP_CMD_ERROR) {
			printf("ip command error.\n");
		} else if (chk_ret == NW_CHKERR_IF_NOT_EXSIST) {
			printf("specified device does not exist.\n");
		} else if (chk_ret == NW_CHKERR_SWITCH) {
			printf("invalid flag : expected <on-off>\n");
		} else {			//default  SYNTAX ERROR
			for (i = 0; i < argc; i++) {
				printf("%s ", argv[i]);
			}
			printf(": command syntax error.\n");
		}
	}

	return;

}


static void nw_chk_tab(char *buf, int *end_pos_p, int *cursor_pos_p)
{
	struct nw_cli_cmd_tbl *cmdp, *cur_cmdp;
	int argc, cand, cnt, i, j;
	char tab_str[NW_CLI_BUFSIZE], token[NW_TOKEN_MAX][NW_TOKEN_LEN_MAX];
	char t_buf[NW_CLI_BUFSIZE];
	char *argv[NW_TOKEN_MAX], *save_p, *tab_str_p, *p;

	memset(tab_str, 0, sizeof(tab_str));
	memset(t_buf, 0, sizeof(t_buf));
	memset(token, 0, sizeof(token));
	memset(argv, 0, sizeof(argv));
	cmdp = cur_cmdp = cmd_root;

	/*提取token（分解用空格分隔的字符串）*/
	memcpy(t_buf, buf, sizeof(t_buf));
	for (argc = 0, tab_str_p = t_buf; (p = strtok_r(tab_str_p, " ", &save_p)) != NULL; tab_str_p = NULL, argc++) {
		if (argc > 64) {
			return;
		} else {
			strcpy(token[argc], p);
			argv[argc] = token[argc];
		}
	}

	/* コマンド入力無しの場合 */
	if (argc == 0) {
		/* root全表示 */
		if (nw_print_all_cand(cmdp) == 1) {
			/* 候補が１つだったら補完 */
			nw_cli_clr_line(buf, end_pos_p, cursor_pos_p);
			strcat(buf, cmdp->cmd_str);
			printf("%s", buf);
			*end_pos_p = strlen(buf);
			*cursor_pos_p = strlen(buf);
		} else {
			printf("\n");
			PUT_PROMPT;
		}
		return;
	}

	/* コマンド入力とコマンド（ツリー構造）の比較 */
	for (cnt = i = 0; cmdp->cmd_str != NULL;) {

		if (cmdp->chk_func != NULL) {

			/* チェック関数がある（=次のトークンが任意文字列）の場合 */
			if (cmdp->chk_func(argv[i], cmdp->cmd_str) == 0) {
				goto NEXT;	/* チェック結果OKなら、完全一致とみなす */
			} else {
				cmdp++;
				if (cmdp == NULL) {
					cnt = 0;
					break;
				} else {
					continue;
				}
			}

		} else {

			/* チェック関数が無い（=次のトークンがコマンド、オプション）の場合 */
			if (strcmp(argv[i], cmdp->cmd_str) == 0) {
NEXT:
				/* 完全一致 */
				if (cmdp->next == NULL) {
					/* 次のツリーがなかったら終了 */
					cnt = 0;
					if (i != (argc - 1)) {
						break;
					}
					nw_print_buf(argc, argv, buf, end_pos_p, cursor_pos_p);
					break;
				} else {
					/* 次のツリーへ */
					cmdp = cur_cmdp = cmdp->next;
					cnt = 0;
					i++;
					if (i == argc) {
						if (buf[strlen(buf)-1] == ' ') {
							cand = nw_print_all_cand(cmdp);
							if (cand == 1) {
								/* 候補が１つだったら補完 */
								/* チェック関数がある場合は補完しない */
								if (cmdp->chk_func) {
									/* 候補は表示する */
									printf("\n%s\n", cmdp->cmd_str);
									PUT_PROMPT;
									printf("%s", buf);
									*end_pos_p = strlen(buf);
									*cursor_pos_p = strlen(buf);
									break;
								}
								/* 補完 */
								strcpy(token[argc], cmdp->cmd_str);
								argv[argc] = token[argc];
								argc++;
								nw_print_buf(argc, argv, buf, end_pos_p, cursor_pos_p);
							} else if (cand > 1) {
								/* 補完できるところまで補完する */
								char buf_char;
								int k;
								for (k = 0; k < strlen(cmdp->cmd_str); k++, cmdp = cur_cmdp) {
									buf_char = cmdp->cmd_str[k];
									for (j = 0; cmdp->cmd_str != NULL; cmdp++, j++) {
										if (buf_char != cmdp->cmd_str[0]) {
											break;
										}
									}
									if (j == cand) {
										/* 全候補一致 */
										strncat(buf, &buf_char, 1);
										//strncpy(token[argc], &buf_char, 1);
										//argv[argc] = token[argc];
										//argc++;
										//cnt = cand;
									} else {
										PUT_PROMPT;
										printf("%s", buf);
										*end_pos_p = strlen(buf);
										*cursor_pos_p = strlen(buf);
										break;
									}
								}
							}
						} else {
							strcat(buf, " ");
							printf("%s", " ");
							(*end_pos_p)++;
							(*cursor_pos_p)++;
						}
						break;
					}
				}
			} else if (strncmp(argv[i], cmdp->cmd_str, strlen(argv[i])) == 0) {
				/* 一部一致 */
				cnt++;
				cmdp++;
			} else {
				cmdp++;
			}
		}
	}

	if (cnt == 1) {
		/* 補完処理 */
		nw_print_sup(argc, argv, i, buf, cur_cmdp, end_pos_p, cursor_pos_p);
	} else if (cnt > 1) {
		/* 候補表示 */
		nw_print_cand(argc, argv, i, buf, cur_cmdp, cnt, end_pos_p, cursor_pos_p);
	} else {
		/* ヒットなし */
		;
	}

	return;

}

static int nw_chk_cmdstr_max(struct nw_cli_cmd_tbl *cmdp)
{
	int max_len;

	for (max_len = 0; cmdp->cmd_str != NULL; cmdp++) {
		if (max_len < strlen(cmdp->cmd_str)) {
			max_len = strlen(cmdp->cmd_str);
		}
	}

	return max_len;
}

static int nw_print_all_cand(struct nw_cli_cmd_tbl *cmdp)
{
	int max_len, cnt;
	char tab_str[NW_CLI_BUFSIZE];
	char tab_fmt[NW_TAB_SIZE];
	char *p;

	memset(tab_str, 0, sizeof(tab_str));
	memset(tab_fmt, 0, sizeof(tab_fmt));

	/* 出力フォーマット作成(最大文字+3) */
	max_len = (NW_chk_cmdstr_max(cmdp) + 3);

	sprintf(tab_fmt, "%%-%ds", max_len);

	cnt = 0;
	for (p = tab_str; cmdp->cmd_str != NULL; cmdp++, cnt++) {
		sprintf(p, tab_fmt, cmdp->cmd_str);
		if (strlen(tab_str) > NW_TAB_WIDTH) {
			printf("\n%s", tab_str);
			memset(tab_str, 0, sizeof(tab_str));
			p = tab_str;
		} else {
			p += max_len;
		}
	}

	if (cnt != 1) {
		if (strlen(tab_str) != 0) {
			printf("\n%s\n", tab_str);
		}
	}
	return cnt;
}

/*
 * 入力バッファの再表示
 */
static void nw_print_buf(int argc, char **argv, char *buf, int *end_pos_p, int *cursor_pos_p)
{
	int i;

	/ *显示，清除输入缓冲区* /
	NW_cli_clr_line(buf, end_pos_p, cursor_pos_p);

	/ *将token（argv [i]）的内容复制到以空格分隔的缓冲区中* /
	for (i = 0; i < argc; i++) {
		strcat(buf, argv[i]);
		strcat(buf, " ");
	}
	/ * 再表示 * /
	printf("%s", buf);
	*end_pos_p = strlen(buf);
	*cursor_pos_p = strlen(buf);
}

/ *
  *清除当前输入的命令行
  * /
static void nw_cli_clr_line(char *buf, int *end_pos_p, int *cursor_pos_p)
{
	
/* Hyōji, nyūryoku baffa o kuria*/
19 / 5000
翻譯結果
/ *显示，清除输入缓冲区* /
	if (*cursor_pos_p < *end_pos_p) {
		printf("\x1b[%dC", (*end_pos_p - *cursor_pos_p));
	}
	for (; strlen(buf);) {
		buf[strlen(buf)-1] = '\0';
		printf("\b");
		printf("\x1b[0J");
	}
	*end_pos_p = 0;
	*cursor_pos_p = 0;
}

/*
 * 删除开头和结尾的空格
 */
void nw_blank_del(char *buf)
{
	int i, j, len, b_flag;
	char t_buf[NW_CLI_BUFSIZE];

	memset(t_buf, 0, sizeof(t_buf));

	for (b_flag = i = j = 0, len = strlen(buf); i < len; i++) {
		if (isspace(buf[i])) {
			if (b_flag == 0) {
				continue;
			} else {
				b_flag = 1;
				if (buf[i+1] == '\0') {
					break;
				}
				if (isspace(buf[i+1])) {
					continue;
				} else {
					t_buf[j] = buf[i];
					j++;
				}
			}
		} else {
			b_flag = 1;
			t_buf[j] = buf[i];
			j++;
		}
	}
	memcpy(buf, t_buf, sizeof(t_buf));
}

static void nw_set_term(struct termios *save_term)
{
    struct termios new_term;

    /* 現在の設定を保存 */
    tcgetattr(0, save_term);

    new_term = *save_term;
    new_term.c_lflag &= (~ICANON);
    new_term.c_lflag &= ECHOE;
    new_term.c_cc[VTIME] = 0;
    new_term.c_cc[VMIN]  = 1;

    tcsetattr(0, TCSANOW, &new_term);
}

static void nw_restore_term(struct termios *save_term)
{

    tcsetattr(0, TCSANOW, save_term);
}

/*
 * 補完処理
 */
static void nw_print_sup(int argc, char **argv, int pos, char *buf, struct nw_cli_cmd_tbl *cmdp, int *end_pos_p, int *cursor_pos_p)
{
	/* 部分一致したのが最後のトークンでない場合、何もしない */
	if (pos != argc -1) {
		return;
	}

	/* 再検索 */
	for (; cmdp->cmd_str != NULL; cmdp++) {
		if (strncmp(argv[pos], cmdp->cmd_str, strlen(argv[pos])) == 0) {
			break;
		}
	}

	/* 最後のトークンの後に空白があると、補完した文字の前に空白が入ってしまう */
	/* そのための対応として、表示とバッファをクリアする */
	/* ただし、前に別のトークンがある場合は、そこまでをバッファに入れて再表示する */
	if (argc > 1) {
		nw_print_buf((argc - 1), argv, buf, end_pos_p, cursor_pos_p);
	} else {
		nw_cli_clr_line(buf, end_pos_p, cursor_pos_p);
	}

	/* 最後のトークンを補完 */
	strcat(buf, cmdp->cmd_str);
	strcat(buf, " ");
	printf("%s", &buf[*end_pos_p]);		//補完した部分だけ追加表示

	*end_pos_p = strlen(buf);
	*cursor_pos_p = strlen(buf);

	return;
}

/*
 * 一部の候補を表示
 */
static void nw_print_cand(int argc, char **argv, int pos, char *buf, struct nw_cli_cmd_tbl *cmdp, int cnt, int *end_pos_p, int *cursor_pos_p)
{
	int max_len, i;
	char tab_str[NW_CLI_BUFSIZE], tab_fmt[NWE_TAB_SIZE];
	char *p;
	char tmp_str[cnt][MW_CLI_BUFSIZE];

	memset(tab_str, 0, sizeof(tab_str));
	memset(tab_fmt, 0, sizeof(tab_fmt));
	memset(tmp_str, 0, sizeof(tmp_str));

	/* 部分一致したのが最後のトークンでない場合、何もしない */
	if (pos != argc -1) {
		return;
	}

	/* 出力フォーマット作成(最大文字+3) */
	max_len = (nw_chk_cmdstr_max(cmdp) + 3);
	sprintf(tab_fmt, "%%-%ds", max_len);

	printf("\n");

	/* 再検索 */
	for (i = 0, p = tab_str; cmdp->cmd_str != NULL; cmdp++) {
		if (strncmp(argv[pos], cmdp->cmd_str, strlen(argv[pos])) == 0) {
			sprintf(p, tab_fmt, cmdp->cmd_str);
			strcpy(tmp_str[i], cmdp->cmd_str);
			i++;
			if (strlen(tab_str) > NW_TAB_WIDTH) {
				printf("%s\n", tab_str);
				memset(tab_str, 0, sizeof(tab_str));
				p = tab_str;
			} else {
				p += max_len;
			}
		}
	}

	/* 候補表示 */
	printf("%s\n", tab_str);

	/* 途中まで補完 */
	for (i = strlen(argv[pos]); i < strlen(tmp_str[0]); i++) {
		/* 全候補の中で一致するか？ 1文字単位で比較 */
		int j;
		for (j = 1; j < cnt; j++) {
			if (tmp_str[0][i] != tmp_str[j][i]) {
				break;
			}
		}
		if (j == cnt) {
			/* 全候補が一致したので1文字だけ追加 */
			strncat(buf, &tmp_str[0][i], 1);
		} else {
			break;
		}
	}

	/* 入力バッファの再出力 */
	PUT_PROMPT;
	nw_blank_del(buf);
	printf("%s", buf);
	*end_pos_p = strlen(buf);
	*cursor_pos_p = strlen(buf);

	return;
}

/*
 * history領域の初期化(リスト作成)
 */
static void init_hist(hist_t *hist)
{
	int i;

	for (i = 0; i < Nw_CLI_HISTORY_MAX; i++) {
		hist[i].next = &hist[i+1];
		hist[i].prev = &hist[i-1];
	}

	hist[NW_CLI_HISTORY_MAX-1].next = &hist[0];
	hist[0].prev = &hist[NW_CLI_HISTORY_MAX-1];

	return;
}

