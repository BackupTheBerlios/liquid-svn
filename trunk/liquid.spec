Name: liquid
Summary: A Modern and Colorful theme
Version: 0.9.7
Release: 2
License: BSD
Group: System/GUI/KDE
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/build-root-%{name}
Packager: Sarath Menon <s_menon@users.sourceforge.net
Distribution: SuSE Linux 9.0 (i586)
Prefix: /opt/kde3
Url: http://www.kde-look.org/content/show.php?content=7043
Provides: liquid
Vendor: The S Factor <http://thesfactor.net>


%description
This package contains a very modern style with translucent elements. It contains a window decoration, widget style code, and a configuration element.

%prep
rm -rf $RPM_BUILD_ROOT 
mkdir $RPM_BUILD_ROOT

%setup -q

%build
CFLAGS="-O2 -g -march=pentium4 -mcpu=pentium4 -fmessage-length=0" 
CXXFLAGS="-O2 -g -march=pentium4 -mcpu=pentium4 -fmessage-length=0"
./configure --prefix=%{prefix}
make -j 2

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip

cd $RPM_BUILD_ROOT

find . -type d -fprint $RPM_BUILD_DIR/file.list.%{name}.dirs
find . -type f -fprint $RPM_BUILD_DIR/file.list.%{name}.files.tmp
sed '/\/man\//s/$/.gz/g' $RPM_BUILD_DIR/file.list.%{name}.files.tmp > $RPM_BUILD_DIR/file.list.%{name}.files
find . -type l -fprint $RPM_BUILD_DIR/file.list.%{name}.libs
sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' $RPM_BUILD_DIR/file.list.%{name}.dirs > $RPM_BUILD_DIR/file.list.%{name}
sed 's,^\.,\%attr(-\,root\,root) ,' $RPM_BUILD_DIR/file.list.%{name}.files >> $RPM_BUILD_DIR/file.list.%{name}
sed 's,^\.,\%attr(-\,root\,root) ,' $RPM_BUILD_DIR/file.list.%{name}.libs >> $RPM_BUILD_DIR/file.list.%{name}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/file.list.%{name}
rm -rf $RPM_BUILD_DIR/file.list.%{name}.libs
rm -rf $RPM_BUILD_DIR/file.list.%{name}.files
rm -rf $RPM_BUILD_DIR/file.list.%{name}.files.tmp
rm -rf $RPM_BUILD_DIR/file.list.%{name}.dirs

%files -f ../file.list.%{name}

%defattr(-,root,root,0755)

%changelog

* Wed Jul 07 2004 - Sarath Menon<s_menon@users.sourceforge.net>
- Initial Version and SuSE rpm.
