/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2007-2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include <connman/ipaddress.h>

#include "connman.h"

struct connman_ipaddress *connman_ipaddress_alloc(int family)
{
	struct connman_ipaddress *ipaddress;

	ipaddress = g_try_new0(struct connman_ipaddress, 1);
	if (ipaddress == NULL)
		return NULL;

	ipaddress->family = family;
	ipaddress->prefixlen = 0;
	ipaddress->local = NULL;
	ipaddress->peer = NULL;
	ipaddress->broadcast = NULL;
	ipaddress->gateway = NULL;

	return ipaddress;
}

void connman_ipaddress_free(struct connman_ipaddress *ipaddress)
{
	if (ipaddress == NULL)
		return;

	g_free(ipaddress->broadcast);
	g_free(ipaddress->peer);
	g_free(ipaddress->local);
	g_free(ipaddress->gateway);
	g_free(ipaddress);
}

unsigned char __connman_ipaddress_netmask_prefix_len(const char *netmask)
{
	unsigned char bits;
	in_addr_t mask;
	in_addr_t host;

	if (netmask == NULL)
		return 32;

	mask = inet_network(netmask);
	host = ~mask;

	/* a valid netmask must be 2^n - 1 */
	if ((host & (host + 1)) != 0)
		return -1;

	bits = 0;
	for (; mask; mask <<= 1)
		++bits;

	return bits;
}

static gboolean check_ipv6_address(const char *address)
{
	unsigned char buf[sizeof(struct in6_addr)];
	int err;

	if (address == NULL)
		return FALSE;

	err = inet_pton(AF_INET6, address, buf);
	if (err > 0)
		return TRUE;

	return FALSE;
}

int connman_ipaddress_set_ipv6(struct connman_ipaddress *ipaddress,
				const char *address,
				unsigned char prefix_length,
				const char *gateway)
{
	if (ipaddress == NULL)
		return -EINVAL;

	if (check_ipv6_address(address) == FALSE)
		return -EINVAL;

	DBG("prefix_len %d address %s gateway %s",
			prefix_length, address, gateway);

	ipaddress->family = AF_INET6;

	ipaddress->prefixlen = prefix_length;

	g_free(ipaddress->local);
	ipaddress->local = g_strdup(address);

	g_free(ipaddress->gateway);
	ipaddress->gateway = g_strdup(gateway);

	return 0;
}

int connman_ipaddress_set_ipv4(struct connman_ipaddress *ipaddress,
		const char *address, const char *netmask, const char *gateway)
{
	if (ipaddress == NULL)
		return -EINVAL;

	ipaddress->family = AF_INET;

	ipaddress->prefixlen = __connman_ipaddress_netmask_prefix_len(netmask);

	g_free(ipaddress->local);
	ipaddress->local = g_strdup(address);

	g_free(ipaddress->gateway);
	ipaddress->gateway = g_strdup(gateway);

	return 0;
}

void connman_ipaddress_set_peer(struct connman_ipaddress *ipaddress,
				const char *peer)
{
	if (ipaddress == NULL)
		return;

	g_free(ipaddress->peer);
	ipaddress->peer = g_strdup(peer);
}

void connman_ipaddress_clear(struct connman_ipaddress *ipaddress)
{
	if (ipaddress == NULL)
		return;

	ipaddress->prefixlen = 0;

	g_free(ipaddress->local);
	ipaddress->local = NULL;

	g_free(ipaddress->peer);
	ipaddress->peer = NULL;

	g_free(ipaddress->broadcast);
	ipaddress->broadcast = NULL;

	g_free(ipaddress->gateway);
	ipaddress->gateway = NULL;
}

/*
 * Note that this copy function only copies the actual address and
 * prefixlen. If you need full copy of ipaddress struct, then you need
 * to create a new function that does that.
 */
void connman_ipaddress_copy_address(struct connman_ipaddress *ipaddress,
					struct connman_ipaddress *source)
{
	if (ipaddress == NULL || source == NULL)
		return;

	ipaddress->family = source->family;
	ipaddress->prefixlen = source->prefixlen;

	g_free(ipaddress->local);
	ipaddress->local = g_strdup(source->local);
}
