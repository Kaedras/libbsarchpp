#!/bin/sh

# usage: ./xedit_repack.sh <game name>

# this script is intended to be used with the following directory structure:
# archiveDirectory/<game name>/<archive files>

if [ $# = 0 ]; then
    echo "not enough parameters"
    echo "usage: ./xedit_repack.sh <game>"
    echo "games: tes3/tes4/tes5/sse/skyrimvr/fo3/fo4/fo4vr/sf"
    exit 1
fi

# convert argument to lower case
GAME=$(echo "$1" | tr "[:upper:]" "[:lower:]")

DATADIR="/path/to/archives"
TMPDIR="/var/tmp/xedit"
BSARCH="/path/to/BSArch.exe"
COMMAND="wine $BSARCH"

# append sha256sum to "$DATADIR/${GAME}_checksums"
SAVE_SHA=0
# remove files after saving sha256sum.
# ENSURE THAT TMPDIR IS EMPTY WHEN ENABLING THIS AND DO NOT RUN THIS SCRIPT AS ROOT!
# THIS WILL EXECUTE rm -rf "$TMPDIR"
CLEAN=0

if [ "$GAME" = "skyrimvr" ]; then
  FORMAT="sse"
elif [ "$GAME" = "fo4vr" ]; then
  FORMAT="fo4"
elif [ "$GAME" = "sf" ]; then
  FORMAT="sf1" # for whatever reason the format name for starfield is sf1 instead of sf
else
  FORMAT="$GAME"
fi

if [ "$FORMAT" = "fo4" ] || [ "$FORMAT" = "sf1" ]; then
  EXT="ba2"
else
  EXT="bsa"
fi


set -e
cd "$DATADIR/$GAME"
for file in ./*."$EXT"; do
  # save file name
  NAME=$(echo "$file" | sed "s/\.$EXT//" | sed 's/\.\///')
  # convert file name to lower case
  NAME_LOWER=$(echo "$NAME" | tr "[:upper:]" "[:lower:]")
  # create output directory
  mkdir -p "$TMPDIR/$GAME/$NAME"
  # unpack
  $COMMAND unpack "$file" "Z:/$TMPDIR/$GAME/$NAME" -q -mt
  # handle texture archives
  if [ "$GAME" = "fo4" ] || [ "$GAME" = "fo4vr" ] || [ "$GAME" = "sf" ]; then
    case "$NAME_LOWER" in
      *textures*)
         REAL_FORMAT="${FORMAT}dds"
       ;;
      *)
        REAL_FORMAT="$FORMAT"
      ;;
    esac
  else
    REAL_FORMAT="$FORMAT"
  fi
  # pack
  $COMMAND pack "Z:/$TMPDIR/$GAME/$NAME" "Z:/$TMPDIR/$GAME/$file" -"$REAL_FORMAT"
  # get sha256sum
  if [ "$SAVE_SHA" = 1 ]; then
    sha256sum "$TMPDIR/$GAME/$file" >> "$DATADIR/${GAME}_checksums"
    # delete files
    if [ "$CLEAN" = 1 ]; then
      rm -rf "$TMPDIR"
    fi
  fi
done
