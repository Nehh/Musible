#set -ex

if [ ! -d "./Build" ]; then
    mkdir Build
fi
if [ ! -d "./Musible" ]; then
    mkdir Musible
fi
cd Musible
rm -rf ./Musible
cd ../Build
rm -rf ./*
cmake ..
make
