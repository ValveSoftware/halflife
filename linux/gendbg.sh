#!/bin/bash

if [[ -n "$VALVE_NO_AUTO_P4" ]]; then
	p4_edit () {
		chmod -R +w "$@" || true
	}
	p4_revert () {
		true
	}
else
	p4_edit () {
		p4 edit "$@"
	}
	p4_revert () {
		p4 revert "$@"
	}
fi

UNAME=$(uname)
if [ "$UNAME" == "Darwin" ]; then
	p4_edit "$1.dSYM/..."
	dsymutil "$1"
	p4 revert -a "$1.dSYM/..."
	exit 0;
fi

if [[ -z "$USE_STEAM_RUNTIME" ]]; then
	OBJCOPY=/valve/bin/objcopy
else
	OBJCOPY=objcopy
fi

function usage {
	echo "$0 /path/to/input/file [-o /path/to/output/file ]"
	echo ""
}

if [ $# == 0 ]; then
	usage
	exit 2
fi

if [ $(basename "$1") == "$1" ]; then
	INFILEDIR=$PWD
else
	INFILEDIR=$(cd "${1%/*}" && echo "$PWD")
fi
INFILE=$(basename "$1")

OUTFILEDIR=$INFILEDIR
OUTFILE=$INFILE.dbg

while getopts "o:" opt; do
	case $opt in
		o)
			OUTFILEDIR=$(cd "${OPTARG%/*}" && echo "$PWD")
			OUTFILE=$(basename "$OPTARG")
			;;
	esac
done

if [ "$OUTFILEDIR" != "$INFILEDIR" ]; then
	INFILE=${INFILEDIR}/${INFILE}
	OUTFILE=${OUTFILEDIR}/${OUTFILE}
fi

pushd "$INFILEDIR" || exit
p4_edit "$OUTFILE"
$OBJCOPY "$INFILE" "$OUTFILE"
$OBJCOPY --add-gnu-debuglink="$OUTFILE" "$INFILE"
p4_revert -a "$OUTFILE"
popd || exit
