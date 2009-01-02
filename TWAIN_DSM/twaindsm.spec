Summary: TWAIN Data Source Manager
Name: twaindsm
Version: 2.0.3
Release: 1
Vendor: TWAIN Working Group
License: LGPL
Group: Development/Libraries
URL: http://www.twain.org
Source0: %{name}-%{version}.tar.gz
Provides: libtwaindsm.so
Prefix: /usr/local

%description
The TWAIN Working Group's Data Source Manager (DSM) for managing communication
between applications and TWAIN drivers (data sources).

%prep
%setup -q
cmake src

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install
mkdir -p %{prefix}/twain

%clean
rm -rf $RPM_BUILD_ROOT

%post
# Configure Newly Added Libraries
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
/usr/local/include/twain.h
/usr/local/lib/libtwaindsm*
/usr/local/lib/twain
%doc doc/*

%changelog
* Wed Nov 26 2008 TWAIN Working Group <twaindsm@twain.org>
- Bug fixes
* Thu Jun 05 2008 TWAIN Working Group <twaindsm@twain.org>
- Bug fix for DAT_STATUS
* Fri Mar 14 2008 TWAIN Working Group <twaindsm@twain.org>
- The DSM now guarantees a datasource is fully closed when MSG_OPENDS fails
* Mon Feb 18 2008 TWAIN Working Group <twaindsm@twain.org>
- Initial Build
