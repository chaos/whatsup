# $Id: whatsup.spec,v 1.20 2003-04-24 19:25:25 achu Exp $

Name:		whatsup
Version:	1.0
Release:	5

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
./configure --prefix=/usr
make

%install
rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/man/man3
cd man; gzip *.3; gzip *.1; cd ..
install man/whatsup.1.gz $RPM_BUILD_ROOT/usr/man/man1
install man/libnodeupdown.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_handle_create.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_handle_destroy.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_load_data.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_errnum.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_strerror.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_errormsg.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_perror.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_dump.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_string.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_string.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_list.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_list.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_string_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_string_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_up_nodes_list_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_down_nodes_list_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_is_node_up.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_is_node_down.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_convert_string_to_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_convert_list_to_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_create.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_clear.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist_destroy.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_errors.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_nodelist.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_nodes_string.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_get_nodes_list.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_is_node.3.gz $RPM_BUILD_ROOT/usr/man/man3
install man/nodeupdown_convert_altnames.3.gz $RPM_BUILD_ROOT/usr/man/man3
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
/usr/man/man1/
/usr/man/man3/





