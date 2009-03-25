Summary: A framework for the development and deployment of scalable graphics applications
Name: Equalizer
Version: 0.6
Release: 1
License: LGPL
Group: System Environment/Libraries
Source: http://www.equalizergraphics.com/downloads/Equalizer-0.6.tar.gz
Buildroot: /var/tmp/%{name}-%{version}-buildroot
URL: http://www.equalizergraphics.com
Packager: Stefan Eilemann <eilemann@gmail.com>
Requires: glibc-devel xorg-x11-devel
BuildRequires: gcc-c++ e2fsprogs-devel xorg-x11-devel rsync

%description 
Equalizer is the standard middleware to create parallel OpenGL-based
applications. It enables applications to benefit from multiple graphics
cards, processors and computers to scale the rendering performance,
visual quality and display size. An Equalizer-based application runs
unmodified on any visualization system, from a simple workstation to
large scale graphics clusters, multi-GPU workstations and Virtual
Reality installations.

%prep
%setup -q

%build
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
make rpm

%clean
rm -rf $RPM_BUILD_ROOT

%files -f install.files
%defattr(-,root,root)
%doc README README.Linux RELNOTES LICENSE AUTHORS FAQ LGPL PLATFORMS ACKNOWLEDGEMENTS

%changelog -f ../RELNOTES
