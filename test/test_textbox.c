/***[Includes]****************************************************************/
#include "widget.h"
#include "unity.h"

/***[Static variables]********************************************************/

/***[Static functions prototypes]*********************************************/

/***[Static functions]********************************************************/

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}
/***[Public functions]********************************************************/

void test_function_should_doBlahAndBlah(void) {
    //test stuff
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