Name:           eap
Version:        1.0
Release:        0
Summary:        The "Hello World" program from GNU

License:        GPLv3+
URL:            http://ftp.gnu.org/gnu/%{name}
Source0:        %{name}-%{version}.tar.gz

#BuildRequires: gettext
      
#Requires(post): info
#Requires(preun): info

%define _unpackaged_files_terminate_build 0

%description 
The "Hello World" program, done with all bells and whistles of a proper FOSS 
project, including configuration, build, internationalization, help files, etc.

%prep
%autosetup

%build
%configure
make %{?_smp_mflags}

%install
%make_install
rm -f %{buildroot}/%{_infodir}/dir

%post

%preun

%files
/usr/lib64/libeap.so
#/usr/lib64/libeap.a
#/usr/lib64/libeap.la
#/usr/lib64/libext.so
/usr/lib64/libeap.so.0
/usr/lib64/libeap.so.0.0.0
/usr/bin/testeap

%doc AUTHORS ChangeLog NEWS README
%license COPYING

%changelog
* Tue Sep 06 2011 The Coon of Ty <Ty@coon.org> 2.10-1
- Initial version of the package

