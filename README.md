Linux Kernel Relay Filesystem Example

This is an example of using Linux kernel relay on kernel 3.9.6. relayArp.c under km is the kernel module used to pop out 2 ARP msg to the user space via relay; relayArp_us.c is the user-space program used to read the ARP msg popped up by the kernel.

Reference: https://www.kernel.org/doc/Documentation/filesystems/relay.txt

June 28, 2013

daveti@cs.uoregon.edu

http://daveti.blog.com
