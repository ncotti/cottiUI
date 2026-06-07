/***[Includes]****************************************************************/
#include "widget.h"
#include "unity.h"
#include "fff.h"

/***[Static variables]********************************************************/
DEFINE_FFF_GLOBALS


/***[Static functions prototypes]*********************************************/
void some_function(void);

/***[Static functions]********************************************************/

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

FAKE_VOID_FUNC(some_function)


/***[Public functions]********************************************************/

void test_function_should_doBlahAndBlah(void) {
    //test stuff
    RESET_FAKE(some_function);
    some_function();
}

void test_function_should_doAlsoDoBlah(void) {
    //more test stuff
}

// not needed when using generate_test_runner.rb
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_should_doBlahAndBlah);
    RUN_TEST(test_function_should_doAlsoDoBlah);
    return UNITY_END();
}