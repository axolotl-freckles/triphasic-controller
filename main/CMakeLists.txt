idf_component_register(SRCS
"main.cpp" "unitTesting.cpp"
"kernel/kernel.cpp" "kernel/phases.cpp" "kernel/sensors.cpp"
"controller/open_loop.cpp"
"time_series/filters.cpp" "time_series/num_calculus.cpp"
"testing/test_phases.cpp" "testing/test_sensors.cpp"
                    INCLUDE_DIRS ".")
