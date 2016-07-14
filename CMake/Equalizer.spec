Name:		Equalizer
Version:	1.12.0
Release:	1%{?dist}
Summary:	Middleware to create and deploy parallel OpenGL-based applications

Group:		Development/Libraries
License:	LGPLv2, examples are BSD licensed
URL:		http://www.equalizergraphics.com/
Source0:	http://www.equalizergraphics.com/downloads/%{name}-%{version}.tar.gz
Patch0:		Equalizer-1.12.0-build-fix.patch
BuildRequires:	cmake bison flex
BuildRequires:	boost-devel glew-devel
BuildRequires:	libX11-devel mesa-libGL-devel
BuildRequires:	OpenSceneGraph-devel OpenThreads-devel


%description
Equalizer is the standard middleware to create and deploy parallel
OpenGL-based applications. It enables applications to benefit from
multiple graphics cards, processors and computers to scale the rendering
performance, visual quality and display size. An Equalizer application
runs unmodified on any visualization system, from a simple workstation
to large scale graphics clusters, multi-GPU workstations and Virtual
Reality installations.

%package devel
Summary:	Development files for Equalizer
Group:		Development/Libraries
Requires:	%{name} = %{version}-%{release}

%description devel
Development files for the Equalizer.

%prep
%setup -q
%patch0 -p1 -b .build-fix

%build
%cmake

make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}
mv %{buildroot}%{_datadir}/%{name}/doc _tmpdoc/


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%doc _tmpdoc/*
%{_bindir}/*
%{_libdir}/lib*.so.*
%{_datadir}/%{name}

%files devel
%{_includedir}/*
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc

%changelog
* Mon Sep 19 2011 Richard Shaw <hobbes1069@gmail.com> - 1.0.1-1
- Initial Release
