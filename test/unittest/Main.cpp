#include "AssertAbortProbe.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

int main(int argc, char** argv)
{
    const char* probeName = Spark::Test::FindAssertAbortProbeName(argc, argv);
    if (probeName != nullptr)
    {
        Spark::Test::RunAssertAbortProbe(probeName);
        return 1;
    }
    doctest::Context context(argc, argv);
    return context.run();
}
