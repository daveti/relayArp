/*
 * User-space main for RelayFS
 * Reference: http://www.ibm.com/developerworks/cn/linux/l-cn-relay/
 * daveti@cs.uoregon.edu
 * http://daveti.blog.com
 * June 27, 2013
 */

#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define MAX_BUFLEN	256
#define ETH_ALEN	6

const char filename_base[]="/sys/kernel/debug/cpu_arp";
/* NOTE: needs root permmission */
typedef struct _arpmsg {
__be16          ar_hrd;         /* format of hardware address   */
__be16          ar_pro;         /* format of protocol address   */
unsigned char   ar_hln;         /* length of hardware address   */
unsigned char   ar_pln;         /* length of protocol address   */
__be16          ar_op;          /* ARP opcode (command)         */
unsigned char           ar_sha[ETH_ALEN];       /* sender hardware address      */
unsigned char           ar_sip[4];              /* sender IP address            */
unsigned char           ar_tha[ETH_ALEN];       /* target hardware address      */
unsigned char           ar_tip[4];              /* target IP address            */
} arpmsg;

static void decode_display_arp(arpmsg *arp_ptr)
{
	unsigned char *ptr = (unsigned char *)arp_ptr;
	int i;
	for (i = 0; i < sizeof(arpmsg); i++)
	{
		printf("ptr[%d] = 0x%x\n", i, ptr[i]);
	}
}

int main()
{
	char relay_file_name[MAX_BUFLEN];
	char read_buffer[MAX_BUFLEN];
	char *tmp_ptr;
	long num_of_cpu;
	int num_of_read;
	int num_of_parse;
	int fd;
	int i;
	arpmsg *arp_msg_ptr;

	/*
	 * As debugfs should be mount'ed already, there is no need to do the mount
	 * again here. However, if the mount is needed, it looks like below:
	 * NOTE: in this machine (kernel 3.9.6), neither relay nor relayfs is a
	 * valid fs in the mount command - debugfs is used instead.
	 * if (mount("debugfs", "/mnt/relay", "debugfs", 0, NULL)
	 * {
	 * 	fprintf(stderr, "mount() failed: %s\n", strerror(errno));
	 * 	return 1;
	 * }
	 */

	/* Get the number of CPU in this system */
	num_of_cpu = sysconf(_SC_NPROCESSORS_CONF);
	if (num_of_cpu == -1)
	{
		fprintf(stderr, "sysconf failure: %s\n", strerror(errno));
		return 1;
	}

	/* Go thru all the CPUs */
	for (i = 0; i < num_of_cpu; i++)
	{
		/* Open per-cpu relay file */
		snprintf(relay_file_name, MAX_BUFLEN, "%s%d", filename_base, i);
		fd = open(relay_file_name, O_RDONLY);
		if (fd == -1)
		{
			fprintf(stderr, "open failure for file %s: %s\n", relay_file_name, strerror(errno));
			continue;
		}

		/* Read the relay file */
		num_of_read = read(fd, read_buffer, MAX_BUFLEN);
		if (num_of_read == -1)
		{
			fprintf(stderr, "read failure for file %s: %s\n", relay_file_name, strerror(errno));
			close(fd);
			continue;
		}

		/* Defensive checking */
		if (num_of_read % sizeof(arpmsg))
		{
			fprintf(stderr, "broken ARP msgs from file %s\n", relay_file_name);
			close(fd);
			continue;
		}
	
		/* Parse these ARP msgs */
		num_of_parse = 0;
		tmp_ptr = read_buffer;
		arp_msg_ptr = (arpmsg *)malloc(sizeof(arpmsg));
		while (num_of_parse != num_of_read)
		{
			/* Copy the msg into the mem */
			memcpy(arp_msg_ptr, tmp_ptr, sizeof(arpmsg));
			/* Decode this ARP msg */
			decode_display_arp(arp_msg_ptr);
			/* Update the ptr */
			num_of_parse += sizeof(arpmsg);
			tmp_ptr += sizeof(arpmsg);
		}
		/* Free the mem */
		free(arp_msg_ptr);
		/* Close the file */
		close(fd);
	}

	/*
	 * Possible umount here:
	 * if (umount("/mnt/relay"))
	 * {
	 * 	fprintf(stderr, "umount() failed: %s\n", strerror(errno));
	 * 	return 1;
	 * }
	 */

	return 0;
}
