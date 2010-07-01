ECHO OFF
Echo Downloading Boost
Echo This may take some time, depending on your internet connection.
ECHO ON

bin\wget.exe http://downloads.sourceforge.net/project/boost/boost/1.43.0/boost_1_43_0.zip?use_mirror=ignum -O boost.zip

bin\7z.exe x boost.zip

rename boost_1_43_0 Boost

del boost.zip

cd Boost

..\bin\bjam toolset=msvc-8.0 variant=debug,release link=static address-model=64 --stagedir=.\EQ_stage64 --with-system --with-date_time --with-regex stage