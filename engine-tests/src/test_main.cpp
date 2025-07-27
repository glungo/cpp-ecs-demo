#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <stdexcept>

// Forward declarations of test functions
namespace EngineTests {
    void RunEngineTests();
}

namespace EntityTests {
    void TestEntityCreation();
    void TestComponentCreation();
    void TestPoolCreation();
    void TestArchetypeCreation();
    void TestArchetypeComponentAccess();
    void TestArchetypeEntityIteration();
    void TestBasicEntityCreation();
    void TestEntityPoolOverflow();
    void TestEntityReuse();
    void TestJobCreate();
    void TestJobParallelExecution();
    void TestJobCacheRefresh();
    void TestJobMultiple();
    void TestJobScheduler();
    void TestSystem();
}

// Structure to hold test information
struct Test {
    std::string name;
    std::function<void()> testFunction;
    bool enabled;
};

int main(int argc, char* argv[]) {
    // Define all tests
    std::vector<Test> tests = {
        {"Engine Basic Tests", EngineTests::RunEngineTests, true},
        {"Entity Creation", EntityTests::TestEntityCreation, true},
        {"Component Creation", EntityTests::TestComponentCreation, true},
        {"Pool Creation", EntityTests::TestPoolCreation, true},
        {"Archetype Creation", EntityTests::TestArchetypeCreation, true},
        {"Archetype Component Access", EntityTests::TestArchetypeComponentAccess, true},
        {"Archetype Entity Iteration", EntityTests::TestArchetypeEntityIteration, true},
        {"Basic Entity Creation", EntityTests::TestBasicEntityCreation, true},
        {"Entity Pool Overflow", EntityTests::TestEntityPoolOverflow, true},
        {"Entity Reuse", EntityTests::TestEntityReuse, true},
        {"Job Create", EntityTests::TestJobCreate, true},
        {"Job Parallel Execution", EntityTests::TestJobParallelExecution, true},
        {"Job Cache Refresh", EntityTests::TestJobCacheRefresh, true},
        {"Job Multiple", EntityTests::TestJobMultiple, true},
        {"Job Scheduler", EntityTests::TestJobScheduler, true},
        {"System Tests", EntityTests::TestSystem, false} // Disabled as noted in CMakeLists
    };

    // Run specific test if provided as command line argument
    if (argc > 1) {
        std::string testName = argv[1];
        bool found = false;
        for (const auto& test : tests) {
            if (test.name == testName) {
                if (test.enabled) {
                    std::cout << "Running test: " << test.name << std::endl;
                    try {
                        test.testFunction();
                        std::cout << "PASSED: " << test.name << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "FAILED: " << test.name << " - " << e.what() << std::endl;
                        return 1;
                    } catch (...) {
                        std::cerr << "FAILED: " << test.name << " - Unknown exception" << std::endl;
                        return 1;
                    }
                } else {
                    std::cout << "Test disabled: " << test.name << std::endl;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "Test not found: " << testName << std::endl;
            return 1;
        }
    }
    // Run all enabled tests
    else {
        int passed = 0;
        int failed = 0;
        int disabled = 0;

        for (const auto& test : tests) {
            if (!test.enabled) {
                std::cout << "DISABLED: " << test.name << std::endl;
                disabled++;
                continue;
            }

            std::cout << "Running test: " << test.name << std::endl;
            try {
                test.testFunction();
                std::cout << "PASSED: " << test.name << std::endl;
                passed++;
            } catch (const std::exception& e) {
                std::cerr << "FAILED: " << test.name << " - " << e.what() << std::endl;
                failed++;
            } catch (...) {
                std::cerr << "FAILED: " << test.name << " - Unknown exception" << std::endl;
                failed++;
            }
        }

        std::cout << "\n=== Test Summary ===\n";
        std::cout << "Total: " << tests.size() << "\n";
        std::cout << "Passed: " << passed << "\n";
        std::cout << "Failed: " << failed << "\n";
        std::cout << "Disabled: " << disabled << "\n";

        return failed > 0 ? 1 : 0;
    }

    return 0;
}
