sudo modprobe ib_uverbs
sudo modprobe mlx4_ib
sudo modprobe vfio-pci
sudo bash -c "echo 2048 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages"
sudo mkdir -p /mnt/huge
sudo mkdir -p /mnt/log
sudo mount -t hugetlbfs nodev /mnt/huge
sudo mkdir -p /mnt/log && sudo chown azureuser:azureuser /mnt/log

