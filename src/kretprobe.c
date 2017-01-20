/*
 * kretprobe_example.c
 *
 * Here's a sample kernel module showing the use of return probes to
 * report the return value and total time taken for probed function
 * to run.
 *
 * usage: insmod kretprobe_example.ko func=<func_name>
 *
 * If no func_name is specified, do_fork is instrumented
 *
 * For more information on theory of operation of kretprobes, see
 * Documentation/kprobes.txt
 *
 * Build and insert the kernel module as done in the kprobe example.
 * You will see the trace data in /var/log/messages and on the console
 * whenever the probed function returns. (Some messages may be suppressed
 * if syslogd is configured to eliminate duplicate messages.)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <net/inet_sock.h>

/* per-instance private data */
struct my_data {
	int fd;
};

void long2ip(long l, char *ip) {
    sprintf(ip, "%ld.%ld.%ld.%ld",
            ((0xff << 24) & l) >> 24,
            ((0xff << 16) & l) >> 16,
            ((0xff << 8) & l) >> 8,
            (0xff & l));
}

/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct my_data *data;
    data = (struct my_data *)ri->data;
    data->fd = regs->di;
    return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    int retval = regs_return_value(regs);
    int err;
    struct socket *sock;
    struct sock *sk;
    struct my_data *data;
    int fd;
    struct inet_sock *inet;

    char ip[16];
    char local[16];

    data = (struct my_data *)ri->data;
    fd = data->fd;
    sock = sockfd_lookup(fd, &err);
    if (!sock) {
        goto out;
    }

    sk = sock->sk;
    inet = (struct inet_sock*)sk;

    if (inet->inet_dport) {
        long2ip(htonl(inet->inet_daddr), ip);
        long2ip(htonl(inet->inet_saddr), local);
        printk(KERN_INFO "sys_connect[%d]: task[%s] pid[%d] fd[%d] from localport[%s:%d] -> dest[%s:%d]",
                retval,
                current->comm, 
                current->pid, 
                fd, 
                local,
                htons(inet->inet_sport), 
                ip, 
                htons(inet->inet_dport));
    }

out:
    return 0;
}

static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	.data_size		= sizeof(struct my_data),
	/* Probe up to 5 instances concurrently. */
	.maxactive		= 5,
};

static int __init kretprobe_init(void)
{
	int ret;

	my_kretprobe.kp.symbol_name = "sys_connect";
	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_kretprobe failed, returned %d\n",
				ret);
		return -1;
	}
	printk(KERN_INFO "Planted return probe at %s: %p\n",
			my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);
	printk(KERN_INFO "kretprobe at %p unregistered\n",
			my_kretprobe.kp.addr);

	/* nmissed > 0 suggests that maxactive was set too low. */
	printk(KERN_INFO "Missed probing %d instances of %s\n",
		my_kretprobe.nmissed, my_kretprobe.kp.symbol_name);
}

module_init(kretprobe_init)
module_exit(kretprobe_exit)
MODULE_LICENSE("GPL");
