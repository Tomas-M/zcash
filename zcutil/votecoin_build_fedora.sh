#!/usr/bin/env bash

# actually untested, sorry

cd "$(dirname "$(readlink -f "$0")")"    #'"%#@!

sudo dnf install \
      git pkgconfig automake autoconf ncurses-devel python \
      python-zmq wget gtest-devel gcc gcc-c++ libtool patch

./fetch-params.sh || exit 1

./build.sh --disable-tests -j$(nproc) || exit 1

if [ ! -r ~/.votecoin/votecoin.conf ]; then
   mkdir -p ~/.votecoin
   echo "addnode=mainnet.votecoin.site" >~/.votecoin/votecoin.conf
   echo "rpcuser=username" >>~/.votecoin/votecoin.conf
   echo "rpcpassword=`head -c 32 /dev/urandom | base64`" >>~/.votecoin/votecoin.conf
fi

cd ../src/
cp -f zcashd votecoind
cp -f zcash-cli votecoin-cli
cp -f zcash-tx votecoin-tx

strip --strip-unneeded votecoind
strip --strip-unneeded votecoin-cli
strip --strip-unneeded votecoin-tx

echo ""
echo "--------------------------------------------------------------------------"
echo "Compilation complete. Now you can run ./src/votecoind to start the daemon."
echo "It will use configuration file from ~/.votecoin/votecoin.conf"
echo ""
