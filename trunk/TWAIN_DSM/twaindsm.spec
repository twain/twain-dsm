Summary: TWAIN Data Source Manager
Name: twaindsm
Version: 2.1.4
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
* Fri Nov 19 2010 TWAIN Working Group <twaindsm@twain.org>
- fully restore original 2.1 behavior for reentrant and threaded apps
* Tue Oct 19 2010 TWAIN Working Group <twaindsm@twain.org>
- reenterent check enforced for 2.2 apps and later...
* Thu Mar 18 2010 TWAIN Working Group <twaindsm@twain.org>
- thread related bug fixes
* Wed Jan 20 2010 TWAIN Working Group <twaindsm@twain.org>
- bug fixes
* Tue Nov 04 2009 TWAIN Working Group <twaindsm@twain.org>
- TWAIN 2.1 features
* Tue Sep 09 2009 TWAIN Working Group <twaindsm@twain.org>
- TWAIN 2.1 features (not released)
* Tue Jun 02 2009 TWAIN Working Group <twaindsm@twain.org>
- Bug fixes
* Wed Nov 26 2008 TWAIN Working Group <twaindsm@twain.org>
- Bug fixes
* Thu Jun 05 2008 TWAIN Working Group <twaindsm@twain.org>
- Bug fix for DAT_STATUS
* Fri Mar 14 2008 TWAIN Working Group <twaindsm@twain.org>
- The DSM now guarantees a datasource is fully closed when MSG_OPENDS fails
* Mon Feb 18 2008 TWAIN Working Group <twaindsm@twain.org>
- Initial Build
