# 1. update system
sudo apt-get update

# 2. install rdma-core
cd ~
git clone https://github.com/linux-rdma/rdma-core
apt-get --assume-yes install build-essential cmake gcc libudev-dev libnl-3-dev libnl-route-3-dev ninja-build pkg-config valgrind python3-dev cython3 python3-docutils pandoc
cd rdma-core
bash build.sh
cmake .
make install

# 3. install pre-requisites
sudo apt --assume-yes install make cmake g++ gcc libnuma-dev libibverbs-dev libgflags-dev numactl
sudo modprobe ib_uverbs
sudo modprobe mlx4_ib


# 4. install dpdk
cd ~
sudo add-apt-repository ppa:canonical-server/server-backports -y
sudo apt-get install -y dpdk
wget http://static.dpdk.org/rel/dpdk-19.11.5.tar.gz
tar -xvf dpdk-19.11.5.tar.gz
cd dpdk-stable-19.11.5
make config T=x86_64-native-linuxapp-gcc
sed -ri 's,(MLX._PMD=)n,\1y,' build/.config
sed -i 's/CONFIG_RTE_LIBRTE_MLX5_PMD=n/CONFIG_RTE_LIBRTE_MLX5_PMD=y/g' config/common_base
sed -i 's/CONFIG_RTE_LIBRTE_MLX4_PMD=n/CONFIG_RTE_LIBRTE_MLX4_PMD=y/g' config/common_base
sudo make install T=x86_64-native-linuxapp-gcc DESTDIR=/usr


# 5. build eRPC (not necessary for testing, we have built-in eRPC in Meerkat)
cd ~
sudo apt-get --assume-yes install libpmem-dev

git clone https://github.com/erpc-io/eRPC.git
cd eRPC
cmake . -DTRANSPORT=dpdk -DAZURE=on -DPERF=ON -DLOG_LEVEL=cc
make -j4
make latency


# 6. init scripts (must be initizalied every time)
sudo modprobe ib_uverbs
sudo modprobe mlx4_ib
sudo modprobe vfio-pci
sudo bash -c "echo 2048 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages"
sudo mkdir -p /mnt/huge
sudo mkdir -p /mnt/log
sudo mount -t hugetlbfs nodev /mnt/huge
sudo mkdir -p /mnt/log && sudo chown azureuser:azureuser /mnt/log


# 7. install meerkat
sudo apt-get --assume-yes install git cmake make gcc g++ clang silversearcher-ag
sudo apt-get --assume-yes install libboost-dev python3-pip python-pip
sudo apt-get --assume-yes install libprotobuf-dev protobuf-compiler libssl-dev libnuma-dev
sudo apt-get --assume-yes install libibverbs-dev
sudo apt-get --assume-yes install libmnl-dev
sudo apt-get --assume-yes install libgflags-dev
sudo apt-get --assume-yes install libevent-dev
sudo apt-get --assume-yes install protobuf-compiler
sudo apt-get --assume-yes install libtbb-dev
sudo apt-get --assume-yes install libboost-all-dev
sudo apt-get --assume-yes install -y pkg-config
pip install PyREM

# install gtest
sudo apt-get --assume-yes install libgtest-dev 
cd /usr/src/gtest 
sudo cmake CMakeLists.txt 
sudo make
sudo cp *.a /usr/lib

# copied meerkat from 10.1.0.7 VM
