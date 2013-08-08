# Copyright (c) 2013 ahmet.bilgili@epfl.ch

# Test std::shared_ptr feature
set(TEST_SRC ${CMAKE_BINARY_DIR}/cpp11_stdsharedptr_test.cpp)
file(WRITE ${TEST_SRC} "
#include <memory>
int main()
{
   std::shared_ptr< int > a( new int );
   return 0;
}
")
try_compile( CPP11_STDSHAREDPTR_SUPPORTED 
             ${CMAKE_BINARY_DIR}/cpp11_stdsharedptr_test        
             ${TEST_SRC}
             COMPILE_DEFINITIONS "-std=c++0x"
             OUTPUT_VARIABLE output )

# Test tuple class feature
set(TEST_SRC ${CMAKE_BINARY_DIR}/cpp11_tuple_test.cpp)
file(WRITE ${TEST_SRC} "
#include <tuple>
int main()
{
   std::tuple< int, char> foo(10,'x');
   return 0;
}
")
try_compile( CPP11_TUPLE_SUPPORTED 
             ${CMAKE_BINARY_DIR}/cpp11_tuple_test       
             ${TEST_SRC}
             COMPILE_DEFINITIONS "-std=c++0x"
             OUTPUT_VARIABLE output )

# Test auto keyword feature
set(TEST_SRC ${CMAKE_BINARY_DIR}/cpp11_auto_test.cpp)
file(WRITE ${TEST_SRC} "int main()
{  
   int a = 2;
   auto foo = a;
   foo++;
   return 0;
}
")
try_compile( CPP11_AUTO_SUPPORTED 
             ${CMAKE_BINARY_DIR}/cpp11_auto_test        
             ${TEST_SRC}
             COMPILE_DEFINITIONS "-std=c++0x"
             OUTPUT_VARIABLE output )

