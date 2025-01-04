#!/bin/sh

# this script creates a header containing a sha256 hash map of all .bsa files inside the current directory

set -e

if [ $# = 0 ]; then
    echo "you have to specify the game name"
    exit 1
fi

# convert argument to lower case
GAME=$(echo "$1" | tr "[:upper:]" "[:lower:]")

# set extension based on game
case $GAME in
  fo4|fo4vr|sf)
    EXT=".ba2"
    ;;
  *)
    EXT=".bsa"
    ;;
esac

{
  echo "#pragma once"
  echo ""
  echo "#include <string>"
  echo "#include <unordered_map>"
  echo ""
  echo "namespace checksums {"
  echo "static const std::unordered_map<std::string, std::string> $GAME = {"
  # get sha256 hashes -> change '<hash>  <file>' to '{"<file>", "<hash>"},' -> remove "./" from file name-> remove trailing ',' from last line
  sha256sum ./*${EXT} | sed -E 's/(\w*\d*)  (.*)/{\"\2\", "\1"},/' | sed 's/\.\///' | sed '$ s/,$//'
  echo "};"
  echo "}"
} > "./$GAME.h"