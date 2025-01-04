#!/bin/sh

# this script creates a test suite
# usage: ./create_test_suite.sh <game name> <archive directory>

if [ $# -lt 3 ]; then
    echo "not enough parameters provided"
    echo "usage: ./create_test_suite.sh <game name> <archive directory>"
    echo "expected game names: tes3/tes4/tes5/sse/fo3/fo4/sf"
    exit 1
fi

set -e

GAME="$1"
DIR="$2"

cd "$DIR"

GAME_UPPER=$(echo "$GAME" | tr "[:lower:]" "[:upper:]")
GAME_LOWER=$(echo "$GAME" | tr "[:upper:]" "[:lower:]")

# set extension based on game
case "$GAME_LOWER" in
  fo4|fo4dds|sf|sfdds)
    EXT=".ba2"
    ;;
  *)
    EXT=".bsa"
    ;;
esac

{
  echo "// $GAME_LOWER"
  echo "#ifdef TESTS_${GAME_UPPER}_ENABLE"
  echo "INSTANTIATE_TEST_SUITE_P(${GAME_UPPER},"
  echo "BsaTest,"
  echo "testing::Values("
  # get file list -> remove leading "./" -> create tuple -> remove trailing ','
  find ./*${EXT} | sed 's/\.\///' | sed -E "s/^(.*)$/tuple<const char \*, const char \*>{\"${GAME_LOWER}\", \"\1\"},/" | sed '$ s/,$//'
  echo "));"
  echo "#endif"
} > "./$GAME_LOWER.cpp"
