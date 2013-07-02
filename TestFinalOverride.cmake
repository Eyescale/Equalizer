# Copyright (c) 2013 Stefan.Eilemann@epfl.ch

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
  list(APPEND FIND_PACKAGES_DEFINES CXX_FINAL_OVERRIDE_SUPPORTED)
endif()
