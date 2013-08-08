# Copyright (c) 2013 ahmet.bilgili@epfl.ch
#               2013 Stefan.Eilemann@epfl.ch

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
try_compile(CXX_STDSHAREDPTR_SUPPORTED
  ${CMAKE_BINARY_DIR}/cpp11_stdsharedptr_test ${TEST_SRC}
  OUTPUT_VARIABLE output)

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
try_compile(CXX_TUPLE_SUPPORTED ${CMAKE_BINARY_DIR}/cpp11_tuple_test
  ${TEST_SRC} OUTPUT_VARIABLE output)

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
try_compile(CXX_AUTO_SUPPORTED ${CMAKE_BINARY_DIR}/cpp11_auto_test
  ${TEST_SRC} OUTPUT_VARIABLE output)

# Test nullptr keyword feature
set(TEST_SRC ${CMAKE_BINARY_DIR}/cpp11_nullptr_test.cpp)
file(WRITE ${TEST_SRC} "int main()
{
   int *ptr = nullptr;
   ptr++;
   return 0;
}
")
try_compile( CXX_NULLPTR_SUPPORTED ${CMAKE_BINARY_DIR}/cpp11_nullptr_test        
   ${TEST_SRC} OUTPUT_VARIABLE output )

# Test array keyword feature
set(TEST_SRC ${CMAKE_BINARY_DIR}/cpp11_array_test.cpp)
file(WRITE ${TEST_SRC} "#include <array>
int main()
{
   std::array<int, 3> a2 = {{1, 2, 3}};
   a2[ 0 ] = 1;
   return 0;
}
")
try_compile( CXX_ARRAY_SUPPORTED ${CMAKE_BINARY_DIR}/cpp11_array_test        
   ${TEST_SRC} OUTPUT_VARIABLE output )

# Test for final and override
set(TEST_SRC ${CMAKE_BINARY_DIR}/override_test.cpp)
file(WRITE ${TEST_SRC} "
class Foo
{
public:
    virtual void one();
    virtual void two() final;
    virtual ~Foo();
};

class Bar : public Foo
{
    virtual void one() override;
    virtual ~Bar();
};

int main() {}
")
try_compile(CXX_FINAL_OVERRIDE_SUPPORTED ${CMAKE_BINARY_DIR}/override_test
  ${TEST_SRC} OUTPUT_VARIABLE output)

if(CXX_FINAL_OVERRIDE_SUPPORTED)
  add_definitions(-DCXX_FINAL_OVERRIDE_SUPPORTED)
endif()
