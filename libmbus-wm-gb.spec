#
# spec file for package libmbus-wm-gb
#
# All rights reserved.
#
# huarun.lin@163.com
#

Summary:  GB Water Meter M-bus library
Name: 		libmbus-wm-gb
Version: 	0.0.1
Release: 	1
Source:	 	https://github.com/huarunlin/%{name}/archive/%{version}.tar.gz
URL:		https://github.com/huarunlin/%{name}
License:	BSD
Vendor:		
Packager:	
Group: 		Development/Languages/C and C++ 
BuildRoot: 	{_tmppath}/%{name}-%{version}-build
AutoReqProv:	on 

%description
libmbus-wm-gb: GB Water Meter M-bus library

%package devel
License:        BSD
Summary:        Development libraries and header files for using the M-bus library
Group:          Development/Libraries/C and C++
AutoReqProv:    on
Requires:       %{name} = %{version}

%description devel
This package contains all necessary include files and libraries needed
to compile and link applications which use the M-bus (Meter-Bus) library.

%prep -q
%setup -q
# workaround to get it's build
autoreconf

%build
./configure --prefix=/usr
make

%install
rm -Rf "%buildroot"
mkdir "%buildroot"
make install DESTDIR="%buildroot"

%clean
rm -rf "%buildroot"

%files
%defattr (-,root,root)
%doc COPYING README.md
%{_libdir}/libmbus.so*
%{_mandir}/man1/libmbus-wm-gb.1
%{_mandir}/man1/mbus-wm-gb-*

%files devel
%defattr (-,root,root)
%{_includedir}/mbus-wm-gb
%{_libdir}/libmbus-wm-gb.a
%{_libdir}/libmbus-wm-gb.la
%{_libdir}/pkgconfig/libmbus-wm-gb.pc

%changelog
* Sat Aug 31 2019 huaurnlin <huarun.lin@163.com> - 0.0.1
- Initial package based on the last official release
