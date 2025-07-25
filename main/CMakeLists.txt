idf_component_register(SRCS
"main.cpp" "unitTesting.cpp"
"kernel/firmware.cpp" "kernel/phases.cpp" "kernel/sensors.cpp"
"controller/controller.cpp"
"time_series/filters.cpp" "time_series/num_calculus.cpp"
"testing/test_phases.cpp" "testing/test_sensors.cpp"
"io/dials.cpp"
                    INCLUDE_DIRS ".")
