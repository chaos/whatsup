/*****************************************************************************\
 *  $Id: nodeupdown_backend_openib.c,v 1.9 2010-02-02 00:01:58 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *  
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see http://www.llnl.gov/linux/.
 *  
 *  Whatsup is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Whatsup is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/types.h>

#include <opensm/osm_log.h>
#include <vendor/osm_vendor_api.h>
#include <vendor/osm_vendor_sa_api.h>
#include <opensm/osm_mad_pool.h>
#include <complib/cl_debug.h>

#include "nodeupdown.h"
#include "nodeupdown_module.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

#define OPENIB_BACKEND_DEFAULT_PORT        10
#define OPENIB_BACKEND_DEFAULT_TIMEOUT_LEN 60
#define OPENIB_BACKEND_CONNECT_LEN         5 
#define OPENIB_BACKEND_BUFLEN              1024

#define OPENIB_MAX_PORTS                   (8)

static osmv_query_res_t   _openib_result;
static osm_log_t          _openib_log_osm;
static osm_mad_pool_t     _openib_mad_pool;
static osm_vendor_t      *_openib_vendor = NULL;

/*
 * openib_backend_default_hostname
 */
static char *
openib_backend_default_hostname(nodeupdown_t handle)
{
  return "localhost";
}

/*
 * openib_backend_default_port
 *
 * openib backend module default_port function
 */
static int 
openib_backend_default_port(nodeupdown_t handle)
{
  return OPENIB_BACKEND_DEFAULT_PORT;
}

/*
 * openib_backend_default_timeout_len
 *
 * openib backend module default_timeout_len function
 */
static int 
openib_backend_default_timeout_len(nodeupdown_t handle)
{
  return OPENIB_BACKEND_DEFAULT_TIMEOUT_LEN;
}

/* 
 * openib_backend_preferred_clusterlist_module
 *
 * openib backend preferred_clusterlist_module function
 */
static char *
openib_backend_preferred_clusterlist_module(nodeupdown_t handle)
{
  return NULL;
}

/*
 * openib_backend_setup
 *
 * openib backend module setup function
 */
static int 
openib_backend_setup(nodeupdown_t handle)
{
  if (getuid() != 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "Permission denied\n");
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return (-1);
    }
  return 0;
}

/*
 * openib_backend_cleanup
 *
 * openib backend module cleanup function
 */
static int
openib_backend_cleanup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/**
 * Call back for the various record requests.
 */
static void
_query_res_cb(osmv_query_res_t *res)
{
	_openib_result = *res;
}

/**
 * Get the OpenSM bind handle
 */
static int
_get_bind_handle(nodeupdown_t nodeupdown_handle, osm_bind_handle_t *handle)
{
	uint32_t           i = 0;
	uint64_t           port_guid = (uint64_t)-1;
	osm_bind_handle_t  bind_handle;
	ib_api_status_t    status;
	ib_port_attr_t     attr_array[OPENIB_MAX_PORTS];
	uint32_t           num_ports = OPENIB_MAX_PORTS;

	complib_init();

	osm_log_construct(&_openib_log_osm);
	if ((status = osm_log_init( &_openib_log_osm, TRUE,
				    0x0001, NULL, TRUE )) != IB_SUCCESS) {
#ifndef NDEBUG
		fprintf(stderr, "Failed to init osm_log: %s\n",
			ib_get_err_str(status));
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (-1);
	}

#if 0
	osm_log_set_level(&_openib_log_osm, OSM_LOG_DEFAULT_LEVEL);
#else
	osm_log_set_level(&_openib_log_osm, OSM_LOG_NONE);
#endif

        _openib_vendor = osm_vendor_new(&_openib_log_osm, 100);
	osm_mad_pool_construct(&_openib_mad_pool);
#ifdef HAVE_FUNC_OSM_MAD_POOL_INIT_2
	if ((status = osm_mad_pool_init(&_openib_mad_pool, &_openib_log_osm)) != IB_SUCCESS) {
#else
	if ((status = osm_mad_pool_init(&_openib_mad_pool)) != IB_SUCCESS) {
#endif
#ifndef NDEBUG
		fprintf(stderr, "Failed to init mad pool: %s\n",
			ib_get_err_str(status));
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (-1);
	}

	if ((status = osm_vendor_get_all_port_attr(_openib_vendor, attr_array, &num_ports)) != IB_SUCCESS) {
#ifndef NDEBUG
		fprintf(stderr, "Failed to get port attributes: %s\n",
			ib_get_err_str(status));
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (-1);
	}

	for (i = 0; i < num_ports; i++) {
		if (attr_array[i].link_state == IB_LINK_ACTIVE)
			port_guid = attr_array[i].port_guid;
	}

	if (port_guid == (uint64_t)-1) {
#ifndef NDEBUG
		fprintf(stderr, "Failed to find active port, check port status with \"ibstat\"\n");
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (-1);
	}

	bind_handle = osmv_bind_sa(_openib_vendor, &_openib_mad_pool, port_guid);

	if (bind_handle == OSM_BIND_INVALID_HANDLE) {
#ifndef NDEBUG
		fprintf(stderr, "Failed to bind to SA\n");
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return(-1);
	}
	*handle = bind_handle;
	return (0);
}

/**
 * Get all the records available for requested query type.
 */
static ib_api_status_t
_get_all_records(nodeupdown_t nodeupdown_handle,
		 osm_bind_handle_t bind_handle,
		 ib_net16_t query_id,
		 ib_net16_t attr_offset,
		 int trusted)
{
	ib_api_status_t   status;
	osmv_query_req_t  req;
	osmv_user_query_t user;

	memset( &req, 0, sizeof( req ) );
	memset( &user, 0, sizeof( user ) );

	user.attr_id = query_id;
	user.attr_offset = attr_offset;

	req.query_type = OSMV_QUERY_USER_DEFINED;
	req.timeout_ms = 100;
	req.retry_cnt = 1;
	req.flags = OSM_SA_FLAGS_SYNC;
	req.query_context = NULL;
	req.pfn_query_cb = _query_res_cb;
	req.p_query_input = &user;
	if (trusted)
		req.sm_key = OSM_DEFAULT_SM_KEY;
	else
		req.sm_key = 0;

	if ((status = osmv_query_sa(bind_handle, &req)) != IB_SUCCESS) {
#ifndef NDEBUG
		fprintf(stderr, "Query SA failed: %s\n",
			ib_get_err_str(status));
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (status);
	}

	if (_openib_result.status != IB_SUCCESS) {
#ifndef NDEBUG
		fprintf(stderr, "Query _openib_result returned: %s\n",
			ib_get_err_str(_openib_result.status));
#endif /* NDEBUG */
		nodeupdown_set_errnum(nodeupdown_handle, NODEUPDOWN_ERR_BACKEND_MODULE);
		return (_openib_result.status);
	}
	return (status);
}

/**
 * Return the mad query data.
 */
static void
_return_mad(void)
{
	/*
	 * Return the IB query MAD to the pool as necessary.
	 */
	if( _openib_result.p_result_madw != NULL ) {
		osm_mad_pool_put( &_openib_mad_pool, _openib_result.p_result_madw );
		_openib_result.p_result_madw = NULL;
	}
}

/*
 * openib_backend_get_updown_data
 *
 * openib backend module get_updown_data function
 */
static int 
openib_backend_get_updown_data(nodeupdown_t nodeupdown_handle, 
			       const char *hostname,
			       unsigned int port,
			       unsigned int timeout_len,
			       char *reserved) 
{
	osm_bind_handle_t bind_handle;
	ib_node_record_t *node_record = NULL;
	ib_net16_t        attr_offset = ib_get_attr_offset(sizeof(*node_record));
	ib_api_status_t   status = IB_SUCCESS;
	int               rc = 0;
	int               i = 0;

	rc = _get_bind_handle(nodeupdown_handle, &bind_handle);
	if (rc != 0) { return (rc); }

	status = _get_all_records(nodeupdown_handle, bind_handle, IB_MAD_ATTR_NODE_RECORD, attr_offset, 0);
	if (status != IB_SUCCESS) { return (-1); }

	for (i = 0; i < _openib_result.result_cnt; i++) {
		ib_node_info_t *p_ni = NULL;
		node_record = osmv_get_query_node_rec(_openib_result.p_result_madw, i);
		p_ni = &(node_record->node_info);
		if (p_ni->node_type == IB_NODE_TYPE_CA) {
                  nodeupdown_add_up_node(nodeupdown_handle, (char *)node_record->node_desc.description);
		}
	}
	_return_mad();
	osm_mad_pool_destroy(&_openib_mad_pool);
	osm_vendor_delete(&_openib_vendor);
	return (0);
}

struct nodeupdown_backend_module_info backend_module_info = 
{
    "openib",
    &openib_backend_default_hostname,
    &openib_backend_default_port,
    &openib_backend_default_timeout_len,
    &openib_backend_preferred_clusterlist_module,
    &openib_backend_setup,
    &openib_backend_cleanup,
    &openib_backend_get_updown_data
};
