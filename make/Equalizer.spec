Summary: A framework for the development and deployment of scalable graphics applications
Name: Equalizer
Version: 0.3.0
Release: 1
License: LGPL
Group: System Environment/Libraries
Source: http://www.equalizergraphics.com/downloads/Equalizer-0.3.0.tar.gz
Buildroot: /var/tmp/%{name}-%{version}-buildroot
Requires: uuid-dev
URL: http://www.equalizergraphics.com
Packager: Stefan Eilemann <eilemann@gmail.com>

%description
Equalizer is an open source programming interface and resource
management system for scalable OpenGL applications. An Equalizer
application can run unmodified on any visualization system, from a
singlepipe workstation to large scale graphics clusters and shared
memory visualization systems. Equalizer is build upon a parallel,
scalable programming interface solving the problems common to any
multipipe application.

%prep
%setup -q

%build
make 

%install
make PREFIX=$RPM_BUILD_ROOT install
make rpm

%clean
rm -rf $RPM_BUILD_ROOT

%files -f install.files
%defattr(-,root,root)
%doc README README.Linux RELNOTES LICENSE AUTHORS FAQ LGPL PLATFORMS

%changelog -f ../RELNOTES
