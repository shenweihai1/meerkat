# meerkat
forked from https://github.com/aaasz/meerkat

1. using latest eRPC(https://github.com/erpc-io/eRPC.git) up to commit e97d0b4ebece0a04c5f76f0f6c74835589db4151 as a library
2. using DPDK dpdk-stable-19.11.5 to ensure dpdk can work on the Azure platform
3. for each azure VM, we need to config two NIC, one is Accelerated networking disabled, the second one is Accelerated networking enabled for dpdk.
