Graph Coloring - Weighted Vertex Coloring Problem - Monte Carlo Tree Search
===========================================================================

|MIT License|

This project proposes several local searches and an MCTS algorithm for the weighted vertex coloring problem (WVCP).

This problem is a variant of the Graph Coloring Problem. Given a weighted graph :math:`G=(V,E)`, the set of vertices :math:`V`, the set of edges :math:`E` and let :math:`W` be the set of weights :math:`w(v)` associated to each vertex :math:`v` in :math:`V`, the WVCP consists in finding a partition of the vertices :math:`V` in into :math:`k` color groups :math:`S=(V_1,...,Vk)` :math:`(1 \leq k \leq |V|)` such that no adjacent vertices belong to the same color group and such that the objective function :math:`f(S) = \sum_{i=1}^{k}\max_{v\in V_i}{w(v)}` is minimized.

This project is coded in C++ for the calculation part and in Python for the data analysis. This work is related to the article :

Cyril Grelier, Olivier Goudet, Jin-Kao Hao. On monte carlo tree search for weighted vertex coloring. Proceedings of the 22nd European Conference on Evolutionary Computation in Combinatorial Optimization (EvoCOP 2022), 20-22 April, 2022, Lecture Notes in Computer Science 13222, pages 1-16.
https://link.springer.com/chapter/10.1007/978-3-031-04148-8_1

pre-print version : https://arxiv.org/abs/2202.01665


Requirements
------------

To compile this project you need :

-  `cmake 3.14+ <https://cmake.org/>`__
-  gcc/g++ 9+
-  `Doxygen <https://www.doxygen.nl/index.html>`__ (optional, to build the documentation)
-  `Python 3.9+ <https://www.python.org/>`__ (for the slurm jobs, data analysis and documentation)


Build the project
-----------------

Clone the project

.. code:: bash

    git clone https://github.com/Cyril-Grelier/gc_wvcp_mcts

Go to the project directory

.. code:: bash

    cd gc_wvcp_mcts

Load the instances

.. code:: bash

    git submodule init
    git submodule update

Create python environment (you can change the python version in ``scripts/build_python.sh``) :

.. code:: bash

    ./scripts/build_python.sh
    source venv/bin/activate

Build and compile the project :

.. code:: bash

    ./scripts/build.sh

Run the project :

.. code:: bash

    cd build_release
    ./gc_wvcp --help

Note : The project must be run from the build directory as it will look for the instances in the parent directory.

Prepare jobs for slurm
----------------------

*After cloning the project on your cluster and following the instruction from* :ref:`Build the project` *. Don't forget to import the instances and create the python environment.*

*Note : If you use slurm you may want to compile with (adapt to your cluster) :*

.. code:: bash

    srun --partition=SMP-short --exclude=cribbar[041-056] --time=00:10:00 ./scripts/build.sh


Create a folder for slurm output files :

.. code:: bash

    mkdir slurm_output


``scripts/generator_to_eval_ls.py`` (for local search) and ``scripts/generator_to_eval_mcts.py`` (for mcts) will generate a file with one job per line. See the scripts for the parameters of the jobs. You can run the scripts with the command :

.. code:: bash

    python scripts/generator_to_eval_ls.py
    python scripts/generator_to_eval_mcts.py

This will generate a file ``to_eval`` with all the jobs.

If the file is too long for slurm (often more than 1000 lines), split the file :

.. code:: bash

    split -l 1000 -d to_eval to_eval

Edit the slurm array size in ``one_job_slurm.sh`` with the line `#SBATCH --array=1-1000` and eventually the time or job name or other parameters.

Then you can submit your job to slurm :

.. code:: bash
    
    sbatch scripts/one_job_slurm.sh to_eval

When a job starts, it creates a file ``output-file-name.csv.running``. At the end of the job, the file is renamed by deleting the ``.running`` at the end of the name. If all your jobs are done but your file still has the ``.running`` then the job crashed.

When the jobs are done you can check for problems with :

.. code:: bash
    
    # delete the jobs with no problem (once all your jobs are done)
    find output_slurm/name-of-your-job -size 0 -delete
    # show the problem
    find output_slurm/name-of-your-job -ls -exec cat {} \;
    # To list eventual crash
    find output_test_slurm -name "*.csv.running" -ls

At the end of the slurm jobs, the last solution is checked with a python script to ensure there is no trouble with the solution.


Data analysis
-------------

``scripts/generate_table.py`` takes raw data and converts it to xlsx files (in xlsx_files repertory) with colored best scores and p-value calculation.

Make sure to set all required methods, instances, and output names directly in the script before running it.

Results
-------

You can find the raw results in ``outputs`` from runs of the code on different instances on the cluster of Nantes: https://ccipl.univ-nantes.fr/ (nazare nodes). These files are in CSV format with the header on the first line, followed by each improving solution found during the search (with the complete solution), the last line corresponds to the best solution found during the whole search with the number of iterations, the time,… at the end of the run. The processed data can be found in ``xlsx_files`` (files generated by ``scripts/generate_table.py`` script).

``greedy_vs_ls_vs_mcts_all.xlsx`` contains all results with each method on each instance. A blue score means the score is proven optimal, a red score is equal to the best-known score, and a green score is better than the best-known score. The last columns compare the methods between each other, gray means no significant gap, red and green significant gap, if the red or green is lighter, the gap is not significant enough but between 0.001 and 0.1 (it doesn't count in the total). The file has been created from the output files : ``outputs/greedy_only_all``, ``outputs/mcts_3_greedy``, ``outputs/ls_all_1h``, ``outputs/mcts_ls_all_1h``.

Results from ``outputs/mcts_constrained_coeff_4`` and ``outputs/coeff_C2000`` where used to generate the plots of the analysis of the coefficient exploration vs exploitation with the notebook ``plot_score_over_time_exploi_explo.ipynb``.

Documentation
-------------

You can generate the documentation with :

.. code:: bash

    cd docs
    make html

The doc main page will be located in :
`docs/_build/html/index.html <docs/_build/html/index.html>`__. It’s a basic and not really useful documentation generated from comments in the code.


Acknowledgements
----------------

We would like to thank Dr. Wen Sun for sharing the binary code of their
AFISA algorithm [1] (the AFISA algorithm has been reimplemented from
the article, `afisa_original`), Dr. Yiyuan Wang for sharing the code
of their RedLS algorithm [2] (the RedLS algorithm has been
reimplemented from the article, `redls`) and Pr. Bruno Nogueira for
sharing the code of their ILS-TS algorithm [3] (some parts of the code
has been used and adapted to the implementation of the project,
`ilsts`).

-  [1] Sun, W., Hao, J.-K., Lai, X., Wu, Q., 2018. Adaptive feasible and
   infeasible tabu search for weighted vertex coloring. Information
   Sciences 466, 203–219. https://doi.org/10.1016/j.ins.2018.07.037
-  [2] Wang, Y., Cai, S., Pan, S., Li, X., Yin, M., 2020. Reduction and
   Local Search for Weighted Graph Coloring Problem. AAAI 34, 2433–2441.
   https://doi.org/10.1609/aaai.v34i03.5624
-  [3] Nogueira, B., Tavares, E., Maciel, P., 2021. Iterated local
   search with tabu search for the weighted vertex coloring problem.
   Computers & Operations Research 125, 105087.
   https://doi.org/10.1016/j.cor.2020.105087

.. |MIT License| image:: https://img.shields.io/apm/l/atomic-design-ui.svg?
   :target: https://github.com/Cyril-Grelier/gc_wvcp_mcts/blob/main/LICENSE


Organisation
------------

.. code:: none

    gc_wvcp_mcts
    ├── build / build_release
    │   ├── gc_wvcp         <- project executable
    │   └── build directory
    ├── .clang-format       <- format project
    ├── CMakeLists.txt
    ├── docs
    │   └── documentation related folder (cd docs; make html to build)
    ├── instances
    │   └── see https://github.com/Cyril-Grelier/gc_instances for details
    ├── LICENSE
    ├── outputs
    │   ├── coeff_C2000.tgz                          <- tests on coeff exploi explo C2000.x
    │   ├── greedy_only_all.tgz                      <- results greedy
    │   ├── ls_all_1h.tgz                            <- results ls
    │   ├── mcts_3_greedy.tgz                        <- results mcts+greedy
    │   ├── mcts_constrained_coeff_4.tgz             <- tests on coeff exploi explo
    │   ├── mcts_ls_all_1h.tgz                       <- results mcts+ls
    │   ├── mcts_redls_freeze_or_not.tgz             <- tests freeze or not the vertices in ls
    │   ├── output_greedy.tgz                        <- old results conference article
    │   ├── output_local_search.tgz                  <- old results conference article
    │   ├── output_mcts_coeff_greedy_random.tgz      <- old results conference article
    │   ├── output_mcts_greedy.tgz                   <- old results conference article
    │   └── output_mcts_local_search_constrained.tgz <- old results conference article
    ├── plot_score_over_time_exploi_explo.ipynb
    ├── README.rst
    ├── requirements.txt
    ├── scripts
    │   ├── build_python.sh           <- to create python environment
    │   ├── build.sh                  <- to compile the project
    │   ├── generate_table.py         <- to create table of results
    │   ├── generator_to_eval_ls.py   <- to lists jobs to execute
    │   ├── generator_to_eval_mcts.py <- to lists jobs to execute
    │   ├── one_job_parallel.sh       <- to run a job (maybe doesn't work anymore)
    │   ├── one_job_slurm.sh          <- to run jobs
    │   ├── run_with_parallel.sh      <- to run jobs (maybe doesn't work anymore)
    │   └── solution_checker.sh       <- to check a solution
    ├── src
    │   ├── main.cpp
    │   ├── methods
    │   │   ├── afisa.cpp
    │   │   ├── afisa.h
    │   │   ├── afisa_original.cpp
    │   │   ├── afisa_original.h
    │   │   ├── greedy.cpp
    │   │   ├── greedy.h
    │   │   ├── hill_climbing.cpp
    │   │   ├── hill_climbing.h
    │   │   ├── ilsts.cpp
    │   │   ├── ilsts.h
    │   │   ├── LocalSearch.cpp
    │   │   ├── LocalSearch.h
    │   │   ├── MCTS.cpp
    │   │   ├── MCTS.h
    │   │   ├── redls.cpp
    │   │   ├── redls_freeze.cpp
    │   │   ├── redls_freeze.h
    │   │   ├── redls.h
    │   │   ├── tabu_col.cpp
    │   │   ├── tabu_col.h
    │   │   ├── tabu_weight.cpp
    │   │   └── tabu_weight.h
    │   ├── representation
    │   │   ├── enum_types.cpp
    │   │   ├── enum_types.h
    │   │   ├── Graph.cpp
    │   │   ├── Graph.h
    │   │   ├── Method.h
    │   │   ├── Node.cpp
    │   │   ├── Node.h
    │   │   ├── Parameters.cpp
    │   │   ├── Parameters.h
    │   │   ├── ProxiSolutionILSTS.cpp
    │   │   ├── ProxiSolutionILSTS.h
    │   │   ├── ProxiSolutionRedLS.cpp
    │   │   ├── ProxiSolutionRedLS.h
    │   │   ├── Solution.cpp
    │   │   └── Solution.h
    │   └── utils
    │       ├── random_generator.cpp
    │       ├── random_generator.h
    │       ├── utils.cpp
    │       └── utils.h
    ├── venv
    │   └── python environment
    └── xlsx_files
        ├── greedy_vs_ls_vs_mcts_all.xlsx <- table with every methods
        ├── greedy_vs_mcts_all.xlsx       <- table with greedy and mcts+greedy
        ├── local_search.xlsx             <- old results conference article
        ├── ls_vs_mcts_all.xlsx           <- table with ls and mcts+greedy
        ├── mcts_greedy.xlsx              <- old results conference article
        ├── mcts_local_search.xlsx        <- old results conference article
        └── mcts_redls_freeze_or_not.xlsx <- table freeze vertices in ls
