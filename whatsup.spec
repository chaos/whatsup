# $Id: whatsup.spec,v 1.13 2003-02-28 16:33:39 achu Exp $

Name:		whatsup
Version:	1.0
Release:	2

Summary:	whatsup
Group:		Applications/Communications
License:	GPL

BuildRoot:	%{_tmppath}/%{name}-%{version}

Source0:	%{name}-%{version}.tgz

%description
whatsup is a tool that lists which nodes are currently up or down in a
cluster.  It determines which nodes are up or down bsaed on
information gathered from genders and ganglia.

The nodeupdown library is a C library that allows users to 
programmatically determine if a node is up or down.

%prep
%setup

%build
%configure --program-prefix=%{?_program_prefix:%{_program_prefix}}
make

%install
rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/man/man3
gzip man/whatsup.1 man/libnodeupdown.3 man/nodeupdown.3 man/nodeupdown_create.3 man/nodeupdown_load_data.3 man/nodeupdown_destroy.3 man/nodeupdown_errnum.3 man/nodeupdown_strerror.3 man/nodeupdown_perror.3 man/nodeupdown_dump.3 man/nodeupdown_get_up_nodes_hostlist.3 man/nodeupdown_get_down_nodes_hostlist.3 man/nodeupdown_get_up_nodes_list.3 man/nodeupdown_get_down_nodes_list.3 man/nodeupdown_is_node_up.3 man/nodeupdown_is_node_down.3 man/nodeupdown_get_hostlist_alternate_names.3 man/nodeupdown_nodelist_create.3 man/nodeupdown_nodelist_clear.3 man/nodeupdown_nodelist_destroy.3 man/nodeupdown_get_list_alternate_names.3 
install man/whatsup.1.gz $RPM_BUILD_ROOT/usr/man/man1 
install man/libnodeupdown.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_create.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_load_data.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_destroy.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_errnum.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_strerror.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_perror.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_dump.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_hostlist.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_hostlist.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_list.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_list.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_is_node_up.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_is_node_down.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_hostlist_alternate_names.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_create.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_clear.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_destroy.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_list_alternate_names.3.gz $RPM_BUILD_ROOT/usr/man/man3
DESTDIR="$RPM_BUILD_ROOT" make install

%clean
rm -rf "$RPM_BUILD_ROOT"

#%pre
#if [ -x /etc/rc.d/init.d/munge ]; then
#  if /etc/rc.d/init.d/munge status | grep running >/dev/null 2>&1; then
#    /etc/rc.d/init.d/munge stop
#  fi
#fi
#
#%post
#if [ -x /etc/rc.d/init.d/munge ]; then
#  [ -x /sbin/chkconfig ] && /sbin/chkconfig --del munge
#  [ -x /sbin/chkconfig ] && /sbin/chkconfig --add munge
#  if ! /etc/rc.d/init.d/munge status | grep running >/dev/null 2>&1; then
#    /etc/rc.d/init.d/munge start
#  fi
#fi
#
#%preun
#if [ "$1" = 0 ]; then
#  if [ -x /etc/rc.d/init.d/munge ]; then
#    [ -x /sbin/chkconfig ] && /sbin/chkconfig --del munge
#    if /etc/rc.d/init.d/munge status | grep running >/dev/null 2>&1; then
#      /etc/rc.d/init.d/munge stop
#    fi
#  fi
#fi
%post
if [ "$1" = 1 ]; then
  /sbin/ldconfig %{_libdir}
fi

%postun
if [ "$1" = 0 ]; then
  /sbin/ldconfig %{_libdir}
fi

%files
%defattr(-,root,root,0755)
%doc AUTHORS
%doc COPYING
%doc ChangeLog
%doc INSTALL
%doc NEWS
%doc README
%{_bindir}/*
%{_includedir}/*
%{_libdir}/*
/usr/man/man1/whatsup.1.gz
/usr/man/man3/libnodeupdown.3.gz
/usr/man/man3/nodeupdown.3.gz
/usr/man/man3/nodeupdown_create.3.gz
/usr/man/man3/nodeupdown_load_data.3.gz
/usr/man/man3/nodeupdown_destroy.3.gz
/usr/man/man3/nodeupdown_errnum.3.gz
/usr/man/man3/nodeupdown_strerror.3.gz
/usr/man/man3/nodeupdown_perror.3.gz
/usr/man/man3/nodeupdown_dump.3.gz
/usr/man/man3/nodeupdown_get_up_nodes_hostlist.3.gz
/usr/man/man3/nodeupdown_get_down_nodes_hostlist.3.gz
/usr/man/man3/nodeupdown_get_up_nodes_list.3.gz
/usr/man/man3/nodeupdown_get_down_nodes_list.3.gz
/usr/man/man3/nodeupdown_is_node_up.3.gz
/usr/man/man3/nodeupdown_is_node_down.3.gz
/usr/man/man3/nodeupdown_get_hostlist_alternate_names.3.gz
/usr/man/man3/nodeupdown_nodelist_create.3.gz
/usr/man/man3/nodeupdown_nodelist_clear.3.gz
/usr/man/man3/nodeupdown_nodelist_destroy.3.gz
/usr/man/man3/nodeupdown_get_list_alternate_names.3.gz

