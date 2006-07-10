##*****************************************************************************
## $Id: Makefile.am,v 1.15 2006-07-10 23:51:22 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

man1_MANS = whatsup.1

man3_MANS = \
	libnodeupdown.3 \
	nodeupdown.3 \
	nodeupdown_handle_create.3 \
	nodeupdown_handle_destroy.3 \
	nodeupdown_load_data.3 \
	nodeupdown_errnum.3 \
	nodeupdown_strerror.3 \
	nodeupdown_errormsg.3 \
	nodeupdown_perror.3 \
	nodeupdown_get_up_nodes_string.3 \
	nodeupdown_get_down_nodes_string.3 \
	nodeupdown_get_up_nodes_list.3 \
	nodeupdown_get_down_nodes_list.3 \
	nodeupdown_is_node_up.3 \
	nodeupdown_is_node_down.3 \
	nodeupdown_up_count.3 \
	nodeupdown_down_count.3 \
	nodeupdown_nodelist_create.3 \
	nodeupdown_nodelist_clear.3 \
	nodeupdown_nodelist_destroy.3 \
	nodeupdown_module.3

if WITH_PINGD
man5_MANS = nodeupdown.conf.5 pingd.conf.5
man8_MANS = pingd.8
else
man5_MANS = nodeupdown.conf.5
endif

EXTRA_DIST = \
	libnodeupdown.3 \
	nodeupdown.3 \
	nodeupdown_handle_create.3 \
	nodeupdown_handle_destroy.3 \
	nodeupdown_load_data.3 \
	nodeupdown_errnum.3 \
	nodeupdown_strerror.3 \
	nodeupdown_errormsg.3 \
	nodeupdown_perror.3 \
	nodeupdown_get_up_nodes_string.3 \
	nodeupdown_get_down_nodes_string.3 \
	nodeupdown_get_up_nodes_list.3 \
	nodeupdown_get_down_nodes_list.3 \
	nodeupdown_is_node_up.3 \
	nodeupdown_is_node_down.3 \
	nodeupdown_up_count.3 \
	nodeupdown_down_count.3 \
	nodeupdown_nodelist_create.3 \
	nodeupdown_nodelist_clear.3 \
	nodeupdown_nodelist_destroy.3 \
	nodeupdown_module.3 \
	nodeupdown.conf.5 \
	pingd.conf.5 \
	pingd.8
