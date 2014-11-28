Copyright (c) Steve Maresca/Zentific LLC 

Instructions
  1) Build the module
    a) make sure kernel headers are installed
    b) make sure a full development toolchain is installed
    c) run 'make'
  2) Insert the kernel module. Parameters are required at insertion time.
    e.g. panicmon.ko src_ip=10.0.0.148 dst_ip=10.0.0.142 iface=eth0
         alternatively use modprobe if preference/requirement dictates.
    NOTE: parameters can be modified at runtime via /sys/module/panicmon/parameters/*
  3) run dmesg to see some status and initialization output

To test the module, run tcpdump on the receiving host and run 'echo c > /proc/sysrq-trigger'

Notes:
    IPv6 is unsupported at the moment.
    UDP is the protocol used for sending data.

Parameters supported:

Name            Criticality     Default value   Type
 iface          Required        unset           String (e.g., "eth0")
 src_ip         Required        unset           String (e.g., "1.2.3.44")
 dst_ip         Required        unset           String (e.g., "1.2.3.44")
 src_port       Optional        20000           Integer [1 <= n <= 65535]
 dst_port       Optional        20000           Integer [1 <= n <= 65535]

