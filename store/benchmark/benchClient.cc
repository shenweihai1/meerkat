// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/benchmark/benchClient.cc:
 *   Benchmarking client for a distributed transactional store.
 *
 **********************************************************************/

#include "store/common/truetime.h"
#include "store/common/frontend/client.h"
#include "store/meerkatstore/meerkatir/client.h"
#include "store/meerkatstore/leadermeerkatir/client.h"
#include "store/common/flags.h"
#include <unistd.h>

#include <boost/fiber/all.hpp>

#include <signal.h>
#include <random>

using namespace std;

// Function to pick a random key according to some distribution.
int rand_key();

bool ready = false;
double *zipf;
vector<string> keys;

thread_local std::random_device rd;
thread_local std::mt19937 key_gen;
thread_local std::uniform_int_distribution<uint32_t> key_dis;

// TODO(mwhittaker): Make command line flags.
bool twopc = false;
bool replicated = true;

void client_fiber_func(int thread_id,
                transport::Configuration config,
                FastTransport *transport) {
    Client* client;
    vector<string> results;

    uint32_t localReplica = -1;

    // Open file to dump results
    uint32_t global_client_id = FLAGS_nhost * 1000 + FLAGS_ncpu * FLAGS_numClientThreads + thread_id;
    std::cout << "intput: " << (FLAGS_logPath + "/client." + std::to_string(global_client_id) + ".log") << std::endl;
    FILE* fp = fopen((FLAGS_logPath + "/client." + std::to_string(global_client_id) + ".log").c_str(), "w");
    uint32_t global_thread_id = FLAGS_nhost * FLAGS_numClientThreads * FLAGS_numClientFibers + thread_id;

    // Trying to distribute as equally as possible the clients on the
    // replica cores.

    // pick the prepare and commit thread on the replicas in a round-robin fashion
    uint8_t preferred_thread_id = global_thread_id % FLAGS_numServerThreads;

    // pick the replica and thread id for read in a round-robin fashion
    int global_preferred_read_thread_id  = global_thread_id % (FLAGS_numServerThreads * config.n);
    int local_preferred_read_thread_id = global_preferred_read_thread_id / config.n;

    if (FLAGS_closestReplica == -1) {
        //localReplica =  (global_thread_id / nsthreads) % nReplicas;
        // localReplica = replica_dis(replica_gen);
        localReplica = global_preferred_read_thread_id % config.n;
    } else {
        localReplica = FLAGS_closestReplica;
    }

    fprintf(stderr, "global_thread_id = %d; thread_id = %d, localReplica = %d, mode = %s\n", global_thread_id, thread_id, localReplica, FLAGS_mode.c_str());
    if (FLAGS_mode == "meerkatstore") {
        client = new meerkatstore::meerkatir::Client(config,
                                            transport,
                                            FLAGS_numServerThreads,
                                            FLAGS_numShards,
                                            localReplica,
                                            preferred_thread_id,
                                            local_preferred_read_thread_id,
                                            twopc, replicated,
                                            TrueTime(FLAGS_skew, FLAGS_error));
	std::cout << "complete a meerkatstore client..." << std::endl;
    } else if (FLAGS_mode == "meerkatstore-leader") {
        client = new meerkatstore::leadermeerkatir::Client(config,
                                            transport,
                                            FLAGS_numServerThreads,
                                            FLAGS_numShards,
                                            localReplica,
                                            preferred_thread_id,
                                            local_preferred_read_thread_id,
                                            twopc, replicated,
                                            TrueTime(FLAGS_skew, FLAGS_error));
    } else {
        fprintf(fp, "option --mode is required\n");
        exit(0);
    }

    struct timeval t0, t1, t2;

    int nTransactions = 0;
    int tCount = 0;
    double tLatency = 0.0;
    int getCount = 0;
    double getLatency = 0.0;
    int commitCount = 0;
    double commitLatency = 0.0;
    string key, value;
    char buffer[100];
    bool status;
    string v (56, 'x'); //56 bytes

    gettimeofday(&t0, NULL);
    srand(t0.tv_sec + t0.tv_usec);

    // Eliminate randomness from the number of reads and writes we perform
    // but keep randomness in when the operations are performed
    int nr_writes = (FLAGS_wPer * FLAGS_tLen / 100);
    int nr_reads = FLAGS_tLen - nr_writes;
    bool dd = false;
    if (dd) std::cout << "XXXXXXXXXXXXXXXXX: " << thread_id << std::endl;
    while (1) {
        if (dd) std::cout << "commit a transction..." << ", " << thread_id << std::endl;
        status = true;

        gettimeofday(&t1, NULL);
        if (dd) std::cout << "1...."<< ", " << thread_id  << std::endl;
        client->Begin();
        if (dd) std::cout << "2...."<< ", " << thread_id  << std::endl;

        int r = 0;
        int w = 0;

        for (int j = 0; j < FLAGS_tLen; j++) {
            key = keys[rand_key()];

            int coin = rand() % 2;
            if (coin == 0) {
                // write priority
                if (w < nr_writes) {
                    if (dd) std::cout << "4...." << ", " << thread_id << std::endl;
                    client->Put(key, v);
                    if (dd) std::cout << "5...."<< ", " << thread_id  << std::endl;
                    w++;
                } else {
                    //gettimeofday(&t3, NULL);
                    if (dd) std::cout << "6...."<< ", " << thread_id  << std::endl;
                    status = client->Get(key, value) == REPLY_OK;
                    if (dd) std::cout << "7...."<< ", " << thread_id  << std::endl;
                    //gettimeofday(&t4, NULL);

                    // the INC workload
                    client->Put(key, v);

                    //getCount++;
                    //getLatency += ((t4.tv_sec - t3.tv_sec)*1000000 + (t4.tv_usec - t3.tv_usec));
                    r++;
                }
            } else {
                // read priority
                if (r < nr_reads) {
                    //gettimeofday(&t3, NULL);
                    if(dd) std::cout << "8...."<< ", " << thread_id  << std::endl;
                    status = client->Get(key, value) == REPLY_OK;
                    if (dd) std::cout << "9...."<< ", " << thread_id  << std::endl;
                    //gettimeofday(&t4, NULL);

                    // the INC workload
                    if(dd) std::cout << "10...."<< ", " << thread_id  << std::endl;
                    client->Put(key, v);
                    if(dd) std::cout << "11...."<< ", " << thread_id  << std::endl;

                    //getCount++;
                    //getLatency += ((t4.tv_sec - t3.tv_sec)*1000000 + (t4.tv_usec - t3.tv_usec));
                    r++;
                } else {
                    if(dd) std::cout << "12...."<< ", " << thread_id  << std::endl;
                    client->Put(key, v);
                    if (dd) std::cout << "13...."<< ", " << thread_id  << std::endl;
                    w++;
                }
            }
        }

        //gettimeofday(&t3, NULL);
        if (dd) std::cout << "3...."<< ", " << thread_id  << std::endl;
        if (status) {
		//std::cout << "xxxxx: " << status << std::endl;
            status = client->Commit();
	    //if (!status) {
	    //        std::cout << "fail to commit\n";
	    //}
		//std::cout << "xxxxx after: " << status << std::endl;
            if (dd) std::cout << "3.5....(mid commit)"<< ", " << thread_id  << std::endl;
        }
        if (dd) std::cout << "4....(after commit)"<< ", " << thread_id  << std::endl;
        gettimeofday(&t2, NULL);

        commitCount++;
        //commitLatency += ((t2.tv_sec - t3.tv_sec)*1000000 + (t2.tv_usec - t3.tv_usec));

        long latency = (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec);

        // log only the transactions that finished in the interval we actually measure
        if ((t2.tv_sec > FLAGS_secondsFromEpoch + FLAGS_warmup) &&
            (t2.tv_sec < FLAGS_secondsFromEpoch + FLAGS_duration - FLAGS_warmup)) {
            //sprintf(buffer, "%d %ld.%06ld %ld.%06ld %ld %d\n", ++nTransactions, t1.tv_sec,
            //        t1.tv_usec, t2.tv_sec, t2.tv_usec, latency, status?1:0);
            //results.push_back(string(buffer));

            if (status) {
                tCount++;
                tLatency += latency;
            }
        }
        gettimeofday(&t1, NULL);
        if ( ((t1.tv_sec-t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec)) > FLAGS_duration*1000000)
            break;

	if (commitCount % 1000 == 0) {
		std::cout <<"thread_id: " << thread_id << ", global_id: " << global_thread_id << ", commits: " << commitCount << ", tCount: " << tCount << ", elasped: " <<  ((t1.tv_sec-t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec)) / 1000000 << std::endl;
	}
    }

    for (auto line : results) {
        //fprintf(fp, "%s", line.c_str());
    }

    std::cout <<"thread_id: " << thread_id << ", global_id: " << global_thread_id << ", commits: " << commitCount << ", elasped: " <<  ((t1.tv_sec-t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec)) / 1000000 << std::endl;

    std::cout << "output results..." << std::endl;
    fprintf(fp, "# Commit_Ratio: %lf\n", (double)tCount/nTransactions);
    fprintf(fp, "# Overall_Latency: %lf\n", tLatency/tCount);
    fprintf(fp, "# Get: %d, %lf\n", getCount, getLatency/getCount);
    fprintf(fp, "# Commit: %d, %lf\n", commitCount, commitLatency/commitCount);
    fclose(fp);
    std::cout << "output results - end" << std::endl;
}

void* client_thread_func(int thread_id, transport::Configuration config) {

    // Initialize the uniform distribution
    key_gen = std::mt19937(rd());
    key_dis = std::uniform_int_distribution<uint32_t>(0, FLAGS_numKeys - 1);

    std::cout << "starting a transport..., thread_id: " << thread_id << std::endl;

    // create the transport
    FastTransport *transport = new FastTransport(config,
                                                FLAGS_ip,
                                                FLAGS_numServerThreads,
                                                0,
                                                FLAGS_physPort,
                                                0,
                                                thread_id);
    std::cout << "complete a transport..., " << thread_id << std::endl;

    // create the client fibers
    boost::fibers::fiber client_fibers[FLAGS_numClientFibers];

    for (int i = 0; i < FLAGS_numClientFibers; i++) {
        std::cout << "starting a client_fiber_func..., " << thread_id * FLAGS_numClientFibers + i << ", " << thread_id << ", " << FLAGS_numClientFibers << std::endl;
        boost::fibers::fiber f(client_fiber_func, thread_id * FLAGS_numClientFibers + i, config, transport);
        client_fibers[i] = std::move(f);
    }

    for (int i = 0; i < FLAGS_numClientFibers; i++) {
        std::cout << "start join a client_fiber_func..." << i << std::endl;
        client_fibers[i].join(); // join has some problems
        std::cout << "end join a client_fiber_func..." << i << std::endl;
    }
    return NULL;
};


void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    fprintf(stderr, "Caught segfault at address %p, code = %d\n", si->si_addr, si->si_code);
    exit(0);
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);

    // Read in the keys from a file.
    string key, value;
    ifstream in;
    in.open(FLAGS_keysFile);
    if (!in) {
        fprintf(stderr, "Could not read keys from: %s\n",
                FLAGS_keysFile.c_str());
        exit(0);
    }
    for (int i = 0; i < FLAGS_numKeys; i++) {
        getline(in, key);
        keys.push_back(key);
    }
    std::cout << "path: " << FLAGS_keysFile << " : " << ", num: " << FLAGS_numKeys << ", key: " << key << std::endl;
    in.close();

    // Load configuration
    std::ifstream configStream(FLAGS_configFile);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n",
                FLAGS_configFile.c_str());
    }
    transport::Configuration config(configStream);

    // Create the transport threads; each transport thread will run
    // FLAGS_numClientThreads client fibers
    std::vector<std::thread> client_thread_arr(FLAGS_numClientThreads);
    for (size_t i = 0; i < FLAGS_numClientThreads; i++) {
        client_thread_arr[i] = std::thread(client_thread_func, i, config);
        // uint8_t idx = i/2 + (i % 2) * 12;
        erpc::bind_to_core(client_thread_arr[i], 0, i);
    }
    for (auto &thread : client_thread_arr) {
            std::cout << "start join a client_thread_func..." << std::endl;
	    thread.join();
            std::cout << "end join a client_thread_func..."   << std::endl;
    }

    return 0;
}

int rand_key()
{
    if (FLAGS_zipf <= 0) {
        // Uniform selection of keys.
        return key_dis(key_gen);
    } else {
        // Zipf-like selection of keys.
        if (!ready) {
            zipf = new double[FLAGS_numKeys];

            double c = 0.0;
            for (int i = 1; i <= FLAGS_numKeys; i++) {
                c = c + (1.0 / pow((double) i, FLAGS_zipf));
            }
            c = 1.0 / c;

            double sum = 0.0;
            for (int i = 1; i <= FLAGS_numKeys; i++) {
                sum += (c / pow((double) i, FLAGS_zipf));
                zipf[i-1] = sum;
            }
            ready = true;
        }

        double random = 0.0;
        while (random == 0.0 || random == 1.0) {
            random = (1.0 + rand())/RAND_MAX;
        }

        // binary search to find key;
        int l = 0, r = FLAGS_numKeys, mid;
        while (l < r) {
            mid = (l + r) / 2;
            if (random > zipf[mid]) {
                l = mid + 1;
            } else if (random < zipf[mid]) {
                r = mid - 1;
            } else {
                break;
            }
        }
        return mid;
    }
}
