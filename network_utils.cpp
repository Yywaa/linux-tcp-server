/*
===========================================================
* The above function convert uint32_t ip_addr into A.B.C.D format
*and The above function is exactly opposite to Ist fn. It converts IP Address in A.B.C.D format in uint32_t format.





*/
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include "network_utils.h"

char *network_convert_ip_n_to_p(uint32_t ip_addr, char *output_buff)
{

    char *out = NULL;
    static char str_ip[16];
    out = !output_buff ? str_ip : output_buff;
    memset(out, 0, 16);
    ip_addr = htonl(ip_addr);
    // ip_addr = ip_addr;
    inet_ntop(AF_INET, &ip_addr, out, 16);
    out[15] = '\0';

    return out;
}

uint32_t network_convert_ip_p_to_n(const char *ip_addr)
{
    uint32_t binary_prefix = 0;
    inet_pton(AF_INET, ip_addr, &binary_prefix);
    binary_prefix = htonl(binary_prefix);
    return binary_prefix;
}