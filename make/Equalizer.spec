Summary: A framework for the development and deployment of parallel OpenGL applications
Name: Equalizer
Version: 0.6
Release: 1
License: LGPL
Group: Development/Libraries/Parallel
Source: http://www.equalizergraphics.com/downloads/Equalizer-0.6.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-buildroot
URL: http://www.equalizergraphics.com
Requires: Mesa
BuildRequires: gcc-c++ e2fsprogs-devel xorg-x11-devel rsync Mesa bison flex glew-devel
%define eq_build_dir build/`uname`

%description 
Equalizer is the standard middleware to create parallel OpenGL-based
applications. It enables applications to benefit from multiple graphics
cards, processors and computers to scale the rendering performance,
visual quality and display size. An Equalizer-based application runs
unmodified on any visualization system, from a simple workstation to
large scale graphics clusters, multi-GPU workstations and Virtual
Reality installations.

%package devel
License: LGPL
Summary: Equalizer Parallel Rendering Library
Group: Development/Libraries/Parallel
Requires: Equalizer = %{version} glibc-devel xorg-x11-devel glew
 
%description devel
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
mkdir -p $RPM_BUILD_ROOT%{_bindir}
rsync -avx --exclude .svn %{eq_build_dir}/bin/ $RPM_BUILD_ROOT%{_bindir}

mkdir -p $RPM_BUILD_ROOT%{_includedir}
rsync -avx --exclude .svn --exclude GL %{eq_build_dir}/include/ $RPM_BUILD_ROOT%{_includedir}

mkdir -p $RPM_BUILD_ROOT%{_libdir}
rsync -avx --exclude .svn %{eq_build_dir}/lib/ $RPM_BUILD_ROOT%{_libdir}

mkdir -p $RPM_BUILD_ROOT/%{_datadir}
rsync -avx --exclude .svn %{eq_build_dir}/share/ $RPM_BUILD_ROOT/%{_datadir}


%clean
rm -rf $RPM_BUILD_ROOT
rm -f %{_bindir}/*.d

%files
%defattr(-,root,root)
%{_libdir}/*
%{_bindir}/*
%{_datadir}/Equalizer/*
%doc README README.Linux RELNOTES LICENSE AUTHORS FAQ LGPL PLATFORMS ACKNOWLEDGEMENTS

%files devel
%defattr(-,root,root)
%{_includedir}/eq/*
%{_includedir}/vmmlib/*

%changelog -f ../RELNOTES
