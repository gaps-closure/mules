#!/bin/bash

# Dependencies: genson and jq

usage_exit() {
  [[ -n "$1" ]] && echo $1
  echo "Usage: $0 [ -ioh ] "
  echo "    -i dir  Directory containing all samples input JSON messages"
  echo "    -o dir  Directory where generated schemas are written"
  echo "    -h      Help"
  exit 1
}

if [ $# -eq 0 ]; then
   usage_exit
fi

dir=();

handle_opts() {
  local OPTIND
  while getopts "i:o:h" options; do
    case "${options}" in
      i) idir=( "${OPTARG}" )  ;;
      o) odir=( "${OPTARG}" )  ;;
      
      h) usage_exit               ;;
      :) usage_exit "Error: -${OPTARG} requires an argument." ;;
      *) usage_exit "" ;;
    esac
  done
  
  shift $((OPTIND -1))
}

handle_opts "$@"

if [ -z "$idir" -o -z "$odir" ]; then
    echo "Input (-i) and output (-o) directories should be specified."
    usage_exit
fi

SUFFIX=".json"
jsons=($idir*$SUFFIX)
suffix_len=${#SUFFIX}
prefix_len=${#idir}
let to_rm=$prefix_len+$suffix_len

for j in "${jsons[@]}"
do
    #name=${c:6:6}
    let len=${#j}-$to_rm
    file=${j:$prefix_len:$len}
    if [[ $file == *"_array"* ]]; then
        continue
    fi
    
    genson $j | jq . > $odir/${file}_schema.json
done

exit
