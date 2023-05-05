# Test creating an RDD9 MXF file containing MPEG-2 LG and PCM.

include("${TEST_SOURCE_DIR}/test_common.cmake")

set(tests
    mpeg2lg_422p_hl_1080i 14 "x" "12" "false"
    mpeg2lg_mp_h14_1080i 15 "x" "12" "false"
    mpeg2lg_mp_hl_1920_1080i 16 "x" "12" "false"
    mpeg2lg_422p_hl_1080i 14 "x" "6" "true"
)

run_tests("${tests}" 24)
