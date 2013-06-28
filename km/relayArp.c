/*
 * Kernel Module for RelayFS and ARP msg
 * Reference: https://www.ibm.com/developerworks/cn/linux/l-cn-relay/
 * Reference: https://www.kernel.org/doc/Documentation/filesystems/relay.txt
 * June 23, 2013
 * daveti@cs.uoregon.edu
 * http://daveti.blog.com
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/relay.h>
#include <linux/debugfs.h>
#include <linux/if_arp.h>

/* Global defs */
static struct rchan *global_rchan;

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

/*
 * create_buf_file() callback.  Creates relay file in debugfs.
 */
static struct dentry *create_buf_file_handler(const char *filename,
                                              struct dentry *parent,
                                              int mode,
                                              struct rchan_buf *buf,
                                              int *is_global)
{
        return debugfs_create_file(filename, mode, parent, buf,
	                           &relay_file_operations);
}

/*
 * remove_buf_file() callback.  Removes relay file from debugfs.
 */
static int remove_buf_file_handler(struct dentry *dentry)
{
        debugfs_remove(dentry);

        return 0;
}

/*
 * relay interface callbacks
 */
static struct rchan_callbacks relay_callbacks =
{
        .create_buf_file = create_buf_file_handler,
        .remove_buf_file = remove_buf_file_handler,
};

/* This function is called when the module is loaded. */
int relayArp_init(void)
{
       	printk(KERN_INFO "Loading Module\n");

	/* Construct an ARP msg */
	arpmsg arp_msg;
	memset(&arp_msg, 0, sizeof(arp_msg));
	unsigned char *ptr = (unsigned char *)&arp_msg;
	/* Set the ethernet type */
	ptr[0] = 0x00;
	ptr[1] = 0x01;
	/* Set the Internet protocol */
	ptr[2] = 0x80;
	ptr[3] = 0x00;
	/* Set the length of hardware address */
	ptr[4] = 0x6;
	/* Set the length of protocal address */
	ptr[5] = 0x4;
	/* Set the ARP opcode */
	ptr[6] = 0x00;
	ptr[7] = 0x01;
	/* Set the sender hardware address */
	ptr[8] = 0x49;
	ptr[9] = 0x72;
	ptr[10] = 0x16;
	ptr[11] = 0x08;
	ptr[12] = 0x64;
	ptr[13] = 0x14;
	/* Set the sender IP address */
	ptr[14] = 129;
	ptr[15] = 25;
	ptr[16] = 10;
	ptr[17] = 72;
	/* Set the target hardware address */
	ptr[18] = 0x00;
	ptr[19] = 0x00;
	ptr[20] = 0x00;
	ptr[21] = 0x00;
	ptr[22] = 0x00;
	ptr[23] = 0x00;
	/* Set the target IP address */
	ptr[24] = 129;
	ptr[25] = 25;
	ptr[26] = 10;
	ptr[27] = 11;

	/* Open the channel */
	global_rchan = relay_open("cpu_arp", NULL, 8192, 2, &relay_callbacks, NULL);
	if (global_rchan == NULL)
	{
		printk(KERN_ERR "relay_open failure\n");
		return -ENOMEM;
	}

	/* Write to the channel - no return value for relay_write */
	relay_write(global_rchan, &arp_msg, sizeof(arp_msg));

	/* Construct another ARP msg */
	memset(&arp_msg, 0, sizeof(arp_msg));
        /* Set the ethernet type */
        ptr[0] = 0x00;
        ptr[1] = 0x01;
        /* Set the Internet protocol */
        ptr[2] = 0x80;
        ptr[3] = 0x00;
        /* Set the length of hardware address */
        ptr[4] = 0x6;
        /* Set the length of protocal address */
        ptr[5] = 0x4;
        /* Set the ARP opcode */
        ptr[6] = 0x00;
        ptr[7] = 0x02;
        /* Set the sender hardware address */
        ptr[8] = 0x49;
        ptr[9] = 0x72;
        ptr[10] = 0x16;
        ptr[11] = 0x08;
        ptr[12] = 0x64;
        ptr[13] = 0x14;
        /* Set the sender IP address */
        ptr[14] = 129;
        ptr[15] = 25;
        ptr[16] = 10;
        ptr[17] = 72;
        /* Set the target hardware address */
        ptr[18] = 0x49;
        ptr[19] = 0x78;
        ptr[20] = 0x21;
        ptr[21] = 0x21;
        ptr[22] = 0x23;
        ptr[23] = 0x90;
        /* Set the target IP address */
        ptr[24] = 129;
        ptr[25] = 25;
        ptr[26] = 10;
        ptr[27] = 11;


	/* Write to the channel again */
	relay_write(global_rchan, &arp_msg, sizeof(arp_msg));
	
	printk(KERN_INFO "Loading done\n");
		
       	return 0;
}

/* This function is called when the module is removed. */
void relayArp_exit(void)
{
	printk(KERN_INFO "Removing Module\n");

	/* Free the rchan */
	if (global_rchan != NULL)
	{
		relay_close(global_rchan);
		global_rchan = NULL;
	}

	printk(KERN_INFO "Removing done\n");
}

/* Macros for registering module entry and exit points. */
module_init( relayArp_init );
module_exit( relayArp_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RelayArp Module");
MODULE_AUTHOR("daveti");

