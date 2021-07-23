[200~add-apt-repository ppa:canonical-server/server-backports -y
sudo apt-get update

sudo apt-get --assume-yes install git cmake make gcc g++ clang
sudo apt-get --assume-yes install libboost-dev python3-pip
sudo apt-get --assume-yes install libprotobuf-dev protobuf-compiler libevent-dev libssl-dev libnuma-dev
sudo apt-get --assume-yes install libibverbs-dev
sudo apt-get --assume-yes install libmnl-dev
sudo apt-get --assume-yes install libgflags-dev
sudo apt-get --assume-yes install libevent-dev
sudo apt-get --assume-yes install protobuf-compiler
sudo apt-get --assume-yes install libtbb-dev
sudo apt-get --assume-yes install libboost-all-dev
sudo apt-get --assume-yes install -y pkg-config

pip3 install PyREM

# install gtest
sudo apt-get --assume-yes install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib
