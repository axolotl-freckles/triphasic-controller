idf_component_register(SRCS
"main.cpp" "unitTesting.cpp"
"kernel/firmware.cpp" "kernel/phases.cpp" "kernel/sensors.cpp"
"controller/controller.cpp" "controller/windup.cpp"
"time_series/filters.cpp" "time_series/num_calculus.cpp"
"testing/test_phases.cpp" "testing/test_sensors.cpp"
"testing/mock_components/mck_sensors.cpp" "testing/mock_components/mck_phases.cpp"
"testing/mock_components/mck_AC_motor.cpp"
"io/dials.cpp"
                    INCLUDE_DIRS ".")
