# $Id: whatsup.spec,v 1.25 2003-06-30 23:40:43 achu Exp $

Name:           whatsup
Version:        1.0
Release:        6

Summary:        whatsup
Group:          Applications/Communications
License:        GPL

BuildRoot:      %{_tmppath}/%{name}-%{version}

Source0:        %{name}-%{version}.tgz

Requires:       gendersllnl

Requires:       ganglia-monitor-core

Requires:       ganglia-monitor-core-lib 

%description
whatsup is a tool that lists which nodes are currently up or down in a
cluster.  It determines which nodes are up or down based on
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





