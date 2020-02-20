#!/bin/bash

BUILD="$(pwd)/build"

usage_exit() {
  [[ -n "$1" ]] && echo $1
  echo "Usage: $0 [ -d ] \\"
  echo "-h        Help"
  echo "-d        Dry run"
  exit 1
}

handle_opts() {
  local OPTIND
  while getopts "dc" options; do
    case "${options}" in
      c) CLEAN=1                ;;
      d) DRY_RUN="--dry-run"    ;;
      h) usage_exit             ;;
      :) usage_exit "Error: -${OPTARG} requires an argument." ;;
      *) usage_exit             ;;
    esac
  done
}

install_llvm () {
  LLVM_URL="packages"
  LLVM_DEB="packages/LLVM-10.0.0svn-Linux.deb"
 
  echo "Installing Qualatype LLVM"
  # wget $LLVM_URL/$LLVM_DEB
  sudo dpkg $DRY_RUN -i $LLVM_DEB
  if ! [[ $DRY_RUN ]]; then
      rm -f $LLVM_DEB
  fi
}

install_vscode () {
  CODE=$(code -h)
  if [ $? -eq 0 ]; then
      echo "VS CODE is installed"
      if ! [[ $DRY_RUN ]]; then
          return
      fi
  fi

  CODE_URL="packages"
  CODE_DEB="packages/code_1.40.2-1574694120_amd64.deb"
 
  echo "Installing Visual Studio Code"
  # wget $CODE_URL/$CODE_DEB
  sudo dpkg $DRY_RUN -i $CODE_DEB
  if ! [[ $DRY_RUN ]]; then
      rm -f $CODE_DEB
  fi
}

install_vsce () {
  VSCE=$(vsce -h)
  if [ $? -eq 0 ]; then
      echo "VSCE is installed"
      if ! [[ $DRY_RUN ]]; then
          return
      fi
  fi
  
  NODE=$(nodejs -v)
  if [ $? -eq 0 ]; then
      echo "Node.js $NODE is installed"
  else
      echo "Installing Node.js"
      sudo apt install $DRY_RUN nodejs
  fi
  
  echo "Installing Visual Studio Code Extension Manager (vsce)"
  if ! [[ $DRY_RUN ]]; then
      sudo npm install -g vsce
  fi
}

build_vscode_extensions () {
  VSCE=$(vsce -h)
  if [ ! $? -eq 0 ]; then
      echo "VSCE is NOT installed or not on the PATH"
      exit
  fi

  TMP_DIR=$(pwd)
  echo $TMP_DIR
  echo "Bulding CLE-themes"
  cd cle-themes
  vsce package
  mv CLE-themes-0.0.1.vsix $BUILD
  
  echo "Bulding CLE-highlighter"
  cd ../cle-highlighter
  vsce package
  mv CLE-Highlighter-0.0.1.vsix $BUILD
  
  cd $TMP_DIR
}

build_pdg () {
  echo "Building PDG"

  TMP_DIR=$(pwd)
  cd pdg
  make
  mv build/libpdg.so $BUILD
  cd $TMP_DIR
}

clean_pdg () {
  echo "Cleaning PDG"

  TMP_DIR=$(pwd)
  cd pdg
  make clean
  mv build/libpdg.so $BUILD
  cd $TMP_DIR
}

build_quala () {
  echo "Bulding Quala"

  TMP_DIR=$(pwd)
  cd quala/examples/tainting
  make

  cd ../nullness
  make
  #mv build/libpdg.so $BUILD
  cd $TMP_DIR
}

clean_quala () {
  echo "Cleaning Quala"

  TMP_DIR=$(pwd)
  cd quala/examples/tainting
  make clean

  cd ../nullness
  make clean
  cd $TMP_DIR
}

build_partitioner () {
  echo "Bulding partitioner"

  TMP_DIR=$(pwd)
  cd partitioner/src
  make

  cd $TMP_DIR
}

clean_partitioner () {
  echo "Cleaning partitioner"

  TMP_DIR=$(pwd)
  cd partitioner/src
  make clean

  cd $TMP_DIR
}

handle_opts "$@"

echo "BUILD=${BUILD}"

if [[ $CLEAN ]]; then
    rm -rf $BUILD
    clean_pdg
    clean_quala
    clean_partitioner
else
    git submodule add https://github.com/gaps-closure/partitioner.git
    git submodule add https://github.com/gaps-closure/pdg.git
    git submodule add https://github.com/gaps-closure/cle-preprocessor.git
exit
    mkdir -p $BUILD
    install_llvm
    install_vscode
    install_vsce
    build_vscode_extensions
    build_pdg
    build_quala
    build_partitioner
fi
