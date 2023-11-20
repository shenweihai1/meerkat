make clean
#cmake . -DTRANSPORT=dpdk -DAZURE=on -DPERF=ON -DLOG_LEVEL=none
cmake . -DTRANSPORT=infiniband -DROCE=on -DPERF=ON -DLOG_LEVEL=none
make -j10
