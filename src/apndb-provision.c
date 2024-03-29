/*
 *
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2008-2011  Intel Corporation. All rights reserved.
 *                2013 Simon Busch <morphis@gravedo.de>
 *  Copyright (C) 2014 Canonical Ltd.
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
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/types.h>
#include <ofono/log.h>
#include <ofono/plugin.h>
#include <ofono/modem.h>
#include <ofono/gprs-provision.h>

#include "ubuntu-apndb.h"

/*
 * Code copied from oFono's mbpi.c, licensed under GPL-2.0 only.
 * Copied from the same commit as the rest of the code.
 */

#define _(x) case x: return (#x)

static const char *mbpi_ap_type(enum ofono_gprs_context_type type)
{
	switch (type) {
		_(OFONO_GPRS_CONTEXT_TYPE_ANY);
		_(OFONO_GPRS_CONTEXT_TYPE_INTERNET);
		_(OFONO_GPRS_CONTEXT_TYPE_MMS);
		_(OFONO_GPRS_CONTEXT_TYPE_WAP);
		_(OFONO_GPRS_CONTEXT_TYPE_IMS);
#ifdef UBUNTU_API
		_(OFONO_GPRS_CONTEXT_TYPE_IA);
#endif
	}

	return "OFONO_GPRS_CONTEXT_TYPE_<UNKNOWN>";
}

#undef _


static int provision_get_settings(const char *mcc, const char *mnc,
				const char *spn,
#ifdef UBUNTU_API
				const char *imsi, const char *gid1,
#endif
				struct ofono_gprs_provision_data **settings,
				int *count)
{
	GSList *apns = NULL;
	GSList *l = NULL;
	GError *error = NULL;
	unsigned int i;
	char *tmp;
	int retval = 0;

#ifndef UBUNTU_API
	const char *imsi = NULL;
	const char *gid1 = NULL;
#endif

	if ((tmp = getenv("OFONO_CUSTOM_MCC")) != NULL)
		mcc = tmp;

	if ((tmp = getenv("OFONO_CUSTOM_MNC")) != NULL)
		mnc = tmp;

	if ((tmp = getenv("OFONO_CUSTOM_SPN")) != NULL)
		spn = tmp;

	if ((tmp = getenv("OFONO_CUSTOM_IMSI")) != NULL)
		imsi = tmp;

	if ((tmp = getenv("OFONO_CUSTOM_GID1")) != NULL)
		gid1 = tmp;

	ofono_info("Provisioning for MCC %s, MNC %s, SPN '%s', IMSI '%s', "
			"GID1 '%s'", mcc, mnc, spn, imsi, gid1);

	apns = ubuntu_apndb_lookup_apn(mcc, mnc, spn, imsi, gid1, &error);
	if (apns == NULL) {
		if (error != NULL) {
			ofono_error("%s: apndb_lookup error -%s for mcc %s"
					" mnc %s spn %s imsi %s", __func__,
					error->message, mcc, mnc, spn, imsi);
			g_error_free(error);
			error = NULL;
		}
	}

	*count = g_slist_length(apns);

	DBG("ap_count: '%d'", *count);

	if (*count == 0) {
		ofono_error("%s: provisioning failed - no APNs found.",
				__func__);

		retval = -1;
		goto done;
	}

	*settings = g_try_new0(struct ofono_gprs_provision_data, *count);
	if (*settings == NULL) {
		ofono_error("%s: provisioning failed: out-of-memory", __func__);

		g_slist_free_full(apns, ubuntu_apndb_ap_free);

		*count = 0;
		return -ENOMEM;
	}

	for (l = apns, i = 0; l; l = l->next, i++) {
		struct apndb_provision_data *ap = l->data;

		DBG("Name: '%s'", ap->gprs_data.name);
		DBG("APN: '%s'", ap->gprs_data.apn);
		DBG("Type: %s", mbpi_ap_type(ap->gprs_data.type));
		DBG("Username: '%s'", ap->gprs_data.username);
		DBG("Password: '%s'", ap->gprs_data.password);
		DBG("Message Proxy: '%s'", ap->gprs_data.message_proxy);
		DBG("Message Center: '%s'", ap->gprs_data.message_center);
		DBG("MVNO: %u", ap->mvno);

		memcpy(*settings + i, &ap->gprs_data, sizeof(ap->gprs_data));

		g_free(ap);
	}

done:
	if (apns != NULL)
		g_slist_free(apns);

	return retval;
}

static struct ofono_gprs_provision_driver apndb_provision_driver = {
	.name		= "Android-formated APN database Provisioning Plugin",
	.get_settings	= provision_get_settings
};

static int apndb_provision_init(void)
{
	return ofono_gprs_provision_driver_register(&apndb_provision_driver);
}

static void apndb_provision_exit(void)
{
	ofono_gprs_provision_driver_unregister(&apndb_provision_driver);
}

OFONO_PLUGIN_DEFINE(apndb_provision,
			"Android-formated APN database Provisioning Plugin", VERSION,
			OFONO_PLUGIN_PRIORITY_DEFAULT,
			apndb_provision_init, apndb_provision_exit)
