# Copyright (c) 2013 ahmet.bilgili@epfl.ch
#               2013 Stefan.Eilemann@epfl.ch

set(TESTS_CPP11 sharedptr tuple auto nullptr array final_override unordered_map)

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_sharedptr.cpp
"#include <memory>
int main()
{
   std::shared_ptr< int > a( new int );
   return 0;
}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_tuple.cpp
"#include <tuple>
int main()
{
   std::tuple< int, char> foo(10,'x');
   return 0;
}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_auto.cpp
"int main()
{
   int a = 2;
   auto foo = a;
   foo++;
   return 0;
}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_nullptr.cpp
"int main()
{
   int *ptr = nullptr;
   ptr++;
   return 0;
}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_array.cpp
"#include <array>
int main()
{
   std::array<int, 3> a2 = {{1, 2, 3}};
   a2[ 0 ] = 1;
   return 0;
}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_final_override.cpp
"class Foo
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

int main() {}")

file(WRITE ${CMAKE_BINARY_DIR}/cpp11_unordered_map.cpp
"#include <unordered_map>
int main()
{
   std::unordered_map< int, int > test;
   test[ 42 ] = 17;
   return 0;
}")

set(TEST_CPP11_PASSED)
set(TEST_CPP11_FAILED)
while(TESTS_CPP11)
  list(GET TESTS_CPP11 0 TEST_CPP11_name)
  list(REMOVE_AT TESTS_CPP11 0)
  string(TOUPPER ${TEST_CPP11_name} TEST_CPP11_NAME)

  try_compile(CXX_${TEST_CPP11_NAME}_SUPPORTED
    ${CMAKE_BINARY_DIR}/cpp11_${TEST_CPP11_name}
    ${CMAKE_BINARY_DIR}/cpp11_${TEST_CPP11_name}.cpp OUTPUT_VARIABLE output)

  if(CXX_${TEST_CPP11_NAME}_SUPPORTED)
    set(TEST_CPP11_PASSED "${TEST_CPP11_PASSED} ${TEST_CPP11_name}")
  else()
    set(TEST_CPP11_FAILED "${TEST_CPP11_FAILED} ${TEST_CPP11_name}")
    #message("${TEST_CPP11_name} failed: ${output}")
  endif()
endwhile()

message(STATUS
  "C++11 features PASSED:${TEST_CPP11_PASSED}, FAILED:${TEST_CPP11_FAILED}")
