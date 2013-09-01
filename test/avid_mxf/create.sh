#!/bin/sh

MD5TOOL=../file_md5

SRC_DIR=$1
DURATION=$2
CREATE_TYPE=$3
INPUT_TYPE=$4
shift 4
EXTRA_OPTS=
MD5S_NAME=$INPUT_TYPE
if [ "$1" = "gf" ]; then
  EXTRA_OPTS="$EXTRA_OPTS --avid-gf "
  MD5S_NAME=$MD5S_NAME"_gf"
  shift
elif [ "$1" = "gfp" ]; then
  EXTRA_OPTS="$EXTRA_OPTS --regtest-end $2 "
  MD5S_NAME=$MD5S_NAME"_gfp"
  shift 2
elif [ "$1" != "" ]; then
  EXTRA_OPTS="$EXTRA_OPTS -f $1 "
  MD5S_NAME=$MD5S_NAME$1
  shift
fi
if [ "$INPUT_TYPE" = "unc" ]; then
  EXTRA_OPTS="$EXTRA_OPTS --height 576 "
fi


BASE_COMMAND="../../apps/raw2bmx/raw2bmx --regtest -t avid -o /tmp/avidmxftest -y 10:11:12:13 --clip test --tape testtape $EXTRA_OPTS"


# create essence data
../create_test_essence -t 1 -d $DURATION /tmp/pcm.raw
../create_test_essence -t $CREATE_TYPE -d $DURATION /tmp/test_in.raw

# write and calculate md5sum
if $BASE_COMMAND -a 16:9 --$INPUT_TYPE /tmp/test_in.raw -q 16 --locked true --pcm /tmp/pcm.raw -q 16 --locked true --pcm /tmp/pcm.raw >/dev/null
then
  $MD5TOOL < /tmp/avidmxftest_v1.mxf > $SRC_DIR/$MD5S_NAME.md5s
  $MD5TOOL < /tmp/avidmxftest_a1.mxf >> $SRC_DIR/$MD5S_NAME.md5s
  $MD5TOOL < /tmp/avidmxftest_a2.mxf >> $SRC_DIR/$MD5S_NAME.md5s
  RESULT=0
else
  RESULT=1
fi

# clean-up
rm -Rf /tmp/pcm.raw /tmp/test_in.raw /tmp/avidmxftest


exit $RESULT

