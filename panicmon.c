/* Copyright (c) Steve Maresca/Zentific LLC */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <linux/notifier.h> /* For notifier support */

#include <linux/proc_fs.h>
#include <linux/inet.h>
#include <linux/stat.h>

MODULE_AUTHOR("steve@zentific.com");
MODULE_DESCRIPTION("A last-gasp panic notifier");
MODULE_LICENSE("GPL");


#include <linux/netpoll.h>
#define MESSAGE_SIZE 1024
#define MODULE_NAME "panicmon"

/* -------------------------------------------------------------------------*/

static struct netpoll* np = NULL;
static struct netpoll np_t;

static char *iface = NULL;
static char *src_ip = NULL;
static char *dst_ip = NULL;
char message[MESSAGE_SIZE];
/* wish we could use u16 here, but module_param macros
 *	only handle u32. do range check
 */
static unsigned int src_port = 20000;
static unsigned int dst_port = 20000;

MODULE_PARM_DESC(iface, "Panicmon src iface");
MODULE_PARM_DESC(src_ip, "Panicmon src_ip");
MODULE_PARM_DESC(src_port, "Panicmon dst_ip");
MODULE_PARM_DESC(src_port, "Panicmon src_port");
MODULE_PARM_DESC(dst_port, "Panicmon dst_port");
module_param(iface, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(src_ip, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(dst_ip, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(src_port, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
module_param(dst_port, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

static void update_panicmon_net(void) {
    np_t.name = "LRNG";

    // to auto-configure ... which is probably not desireable
    // http://stackoverflow.com/questions/9941030/get-ip-address-of-local-nic-from-kernel-module
    strlcpy(np_t.dev_name, iface, IFNAMSIZ);

#ifndef LEGACY
    /* for systems where the below is not a union */
    np_t.local_ip = in_aton(src_ip);
    np_t.remote_ip = in_aton(dst_ip);
#else
    /* for systems where this is a union */
    np_t.local_ip.ip = in_aton(src_ip);
    np_t.remote_ip.ip = in_aton(dst_ip);
#endif

    np_t.local_port = src_port;
    np_t.remote_port = dst_port;
    memset(np_t.remote_mac, 0xff, ETH_ALEN);

}



/*
 * Panicing.
 *
 * Don't. But if you did, this is what happens.
 */
static int panicmon(struct notifier_block *nb, unsigned long l, void *p) {
    int len = 0;
    int buflen = 0;
    char *buf = p;

    if(buf){
        buflen = strlen(buf);
    }

    //OH NO...\n == 25 c
    //1024 - \0 - 25 = 998
    sprintf(message,"OH NO WE ARE PANICING - %.998s\n", buf);
    len = strlen(message);

    // in case params have been updated    
    update_panicmon_net();

    netpoll_send_udp(np,message,len);

    return NOTIFY_DONE;
}

static struct notifier_block onpanic = {
    .notifier_call = panicmon

};

int init_module(void) {
    printk(KERN_INFO MODULE_NAME ": Loaded.\n");

    //validation and init

    if ( !src_ip ){
        printk(KERN_INFO MODULE_NAME ": Could not initialize (src_ip parameter not set).\n");
        return -1;
    }

    if ( !dst_ip ){
        printk(KERN_INFO MODULE_NAME ": Could not initialize (dst_ip parameter not set).\n");
        return -1;
    }

    if ( !iface ){
        printk(KERN_INFO MODULE_NAME ": Could not initialize (iface parameter not set).\n");
        return -1;
    }

    if ( src_port > 65535 || dst_port > 65535 || src_port == 0 || dst_port == 0 ){
        printk(KERN_INFO MODULE_NAME ": Invalid src/dst port value. Valid range is [1,65535].\n");
        return -1;
    }

    update_panicmon_net();
    netpoll_print_options(&np_t);
    netpoll_setup(&np_t);
    np = &np_t;

    atomic_notifier_chain_register(&panic_notifier_list, &onpanic);
    printk(KERN_INFO MODULE_NAME ": Registered panic notifier.\n");

    return 0;
}

void cleanup_module(void) {

    atomic_notifier_chain_unregister(&panic_notifier_list, &onpanic);
    printk(KERN_INFO MODULE_NAME ": Unregistered panic notifier.\n");
    printk(KERN_INFO MODULE_NAME ": Uninitialized.\n");
}
