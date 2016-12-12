#include <cpput/testing.h>
#include <cpput/lighttestrunner.h>


int main(void)
{
    CppUT::LightTestRunner runner;

    runner.addSuite(CppUT::Registry::getRootSuite());

    bool sucessful = runner.runTests();
    return sucessful ? 0 : 1;
}
