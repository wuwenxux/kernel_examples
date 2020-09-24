#ifndef _NGMWAN_NETLINK_H
#define _NGMWAN_NETLINK_H

#define NGMWAN_GENL_NAME "ngmwan"
#define NGMWAN_GENL_VERSION 1

enum nw_cmd {
	NW_CMD_GET_DEVICE,
	NW_CMD_SET_DEVICE,
	__NW_CMD_MAX
};
#define NW_CMD_MAX (__NW_CMD_MAX - 1)

enum nwdevice_attribute {
	NWDEVICE_A_UNSPEC,
	NWDEVICE_A_IFINDEX,
	NWDEVICE_A_IFNAME,
	__NWDEVICE_A_LAST
};
#define NWDEVICE_A_MAX (__NWDEVICE_A_LAST - 1)

int nw_genetlink_init(void);
void nw_genetlink_uninit(void);

#endif /* _NGMWAN_NETLINK_H */
