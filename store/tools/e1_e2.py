from collections import defaultdict
from meerkat_benchmarks import (
    Parameters, ParametersAndResult, parse_args, run_suite, zookeeper_clients,
    zookeeper_servers)
from sheets_query import header
import benchmark


"""
    The scalability graph.
"""

def main(args):
    # Set benchmark parameters.
    base_parameters = Parameters(
        config_file_directory=args.config_file_directory,
        f=1,
        key_file=args.key_file,
        num_keys=None,
        num_server_threads=None,
        repl_scheme=None,
        server_binary=args.server_binary,
        client_binary=args.client_binary,
        benchmark_duration_seconds=30,
        benchmark_warmup_seconds=10,
        transaction_length=1,
        write_percentage=0,
        zipf_coefficient=None,
        num_client_machines=None,
        num_clients_per_machine=1,
        num_threads_per_client=None,
        num_fibers_per_client_thread=None,
        suite_directory=args.suite_directory,
    )

    parameters_list = [
      base_parameters._replace(
          num_server_threads = num_server_threads,
          num_keys = num_keys,
          num_client_machines = num_client_machines,
          num_threads_per_client = num_threads_per_client,
          num_fibers_per_client_thread = num_fibers_per_client_thread,
          zipf_coefficient = zipf_coefficient,
      )

      # Keep load 24 clients per core
      for (num_server_threads,
           num_keys,
           num_client_machines,
           num_threads_per_client,
           num_fibers_per_client_thread) in [
                                       #(4, 4 * 500000, 1, 6, 8),
                                       #(8, 8 * 500000, 2, 6, 8),
                                       #(16, 16 * 500000, 4, 6, 8),
                                       #(32, 32 * 500000, 8, 6, 8),
                                       #(64, 64 * 500000, 10, 8, 8),
                                       #(68, 68 * 500000, 10, 11, 8),
                                       #(72, 72 * 500000, 10, 11, 8),
                                       #(76, 76 * 500000, 10, 12, 8),
                                       #(78, 78 * 500000, 10, 12, 8),
                                       #(78, 78 * 500000, 10, 12, 8),
                                       #(80, 80 * 500000, 8, 12, 8),
                                       #(80, 80 * 500000, 10, 12, 8),

                                       #(4, 4 * 500000, 1, 1, 1), # debug for testing

                                       #(4, 4 * 500000, 2, 4, 8), # 124M
                                       #(8, 8 * 500000, 4, 4, 8), # 248M
                                       (12, 12 * 500000, 6, 4, 8), # 372M
                                       #(16, 16 * 500000, 8, 4, 8), # 496M
                                       #(32, 32 * 500000, 16, 4, 8),
                                       ]

      for zipf_coefficient in [0]
    ]

    # Run every experiment three times.
    #parameters_list = [q for p in parameters_list for q in [p] * 3]

    # Run the suite.
    suite_dir = benchmark.SuiteDirectory(args.suite_directory, 'e1_and_e2')
    suite_dir.write_dict('args.json', vars(args))
    run_suite(suite_dir, parameters_list, zookeeper_clients(), zookeeper_servers())

if __name__ == '__main__':
    main(parse_args())
