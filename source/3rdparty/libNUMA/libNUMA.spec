Summary: Library to discover properties and handle NUMA systems
Name: libNUMA
Version: 1.0
Release: 1
License: GPLv2 with exception
Group: Development/Tools
URL: http://www.akkadia.org/drepper/libNUMA-%{version}.tar.bz2
Source0: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: numactl-devel

%define ABI 1

%description
libNUMA is a library that provides routines to discover and handle the
properties of the cache and memory hierarchy and how programs use them.
For good performance cache must be utilized wisely and accesses to memory
must be fast as possible.  Changing machine architectures make this hard
and hardcoding any property must be avoided.  The functions in the libNUMA
allow discovering the necessary property and calling the appropriate
system interfaces more easily.

%package devel
Summary: Development libraries to handle NUMA
Group: Development/Tools
Requires: libNUMA = %{version}-%{release}

%description devel
Development support for libNUMA.


%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p ${RPM_BUILD_ROOT}

%makeinstall

%check
make check

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc
%{_libdir}/libNUMA-%{version}.so.%{ABI}

%files devel
%{_includedir}/libNUMA.h
%{_libdir}/libNUMA.so
%{_mandir}/man3/*

%changelog
* Mon Apr  9 2012 Ulrich Drepper <drepper@gmail.com> -
- Initial build.
