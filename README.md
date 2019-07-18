# MPI Words Count
This is a simple parallel implementation of words count using Open MPI developed in the context of Parallel, Concurrent and Cloud programming course at the University of Salerno.

## Problem Statement

We will be doing a version of map-reduce using MPI to perform word counting over a large number of files. There are 3 steps to this process:

1. is to read in the master file list which will contain the names of all the files that are to be counted. Note that only 1 of your processes should read this file. Then each of the processes should receive their portion of the file from the master process. Once a process has received its list of files to process, it should then read in each of the files and perform a word counting, keeping track of the frequency each word found in the files occurs. We will call the histogram produced the local histogram. This is similar to the map stage or map-reduce.
2. is to then combine frequencies of words across processes. For example the word 'cat' might be counted in multiple processes and we need to add up all these occurrences. This is similar to the reduce stage of map-reduce.
3. is to have each of the processes send their local histograms to the master process. The master process just needs to gather up all this information. Note that there will be duplicate words between processes. The master should then print out the results to the screen.

## Solution Approach
Before describing the solution algorithm, some **preliminary assumptions** must be carried out.
The algorithm considers a **single line of text** (with a maximum length of **256 characters**) as **work unit**. So, each process recieves an equal amount of lines and counts the words present in them.
To achieve a balanced workload among all the processes, a **preliminary stage**, described below, **of parrallel line counting is performed**.
Another preliminary assumption must be made: **each process can access to all the input files**. If the program is run in distributed memory, each node of the cluster must share the same directory structure and the same set of files.
Last but not least, **each computation step** described below **is performed both by the master and the slaves**. Operation made exclusively by the master are explicitly mentioned.
A word is considered as a sequence of alphabetic symbols, so "isn't", for example, is considered as two separate words. Also, numbers are not considered words.

### High Level Algorithm Description
1. The master process recieves from CLI a index file in which are written all the file whoose words need to be counted.
2. The master process assigns a balanced number of files to each processor, sending them the name of the files.
3. Every processor counts the number of lines in each recieved file and sends back the result to the master.
4. The master computes the amount of lines that each process should recieve, then file fragmets are constructed (chunk structure described below), and sent to the slaves.
5. Each process counts the words occurrences in the lines present in the fragments. 
6. The computed occurrences are sent to the master which merges the partial results and plots the final result.

### Program Structure
The entire codebase of the project consists of 5 modules:
- main
- chunk
- woccurrence
- wordsmap
- wcutils

All but the **main** have 2 files: the header (.h) and the source file (.c). The **main** has only the source.
The **main** module contains the implementation of the aforementioned algorithm. The other ones, except the wcutils that contains some general purpose utility functions, are the implementation of useful data structures used by the alglorithm. 
The **chunk** module contains the structure and the operations needed to represent and manipulate a file fragment. 
The **woccurrence** module contains the structure and the operation needed to represent and manipulate a word occuurence (string-int pair).
The **wordsmap** module is the implementation of a hash table for word occurrences using the word as key.
The entire program is compiled using **make**.

### Implementation details
As mentioned before, the **wordsmap** module implements a hash table. This is used to keep track of the various occurrences found by the workers. The table is implemented with **open addressing** method, in order to have data as much contiguous as possible. When all the buckets in the table are used, the table size is doubled using a `realloc` operation. This "optimistic" approach of sizing is used in almost all dynamic structures.

The **chunk** and **woccurrence** structures are also defined as **MPI Datatypes** so that can be passed among processes.

This solution has been developed using only `Scatter(v)` and `Gather(v)` communication routines, since almost all the communications needed to be arrays divisions and compositions. The execution time is computed using the `Wtime` function.

As mentioned before, two *round* of parallel work is performed, since the unit of work is a text line. This means that the total number of lines must be known to equally partition the domain. So, rather than keeping all the line counting work to the master process (and having all the others in an idle state), this work is also done in parallel, having each worker to count the lines of a subset of the files. On the other hand, this causes to have much more communication overhead.

### Benchmarking

The solution has been tested over a cluster of 8 AWS EC2 t2.small (1 virtual processor, 2 GB RAM) Ubuntu instances using the files inside the `input/` folder as input. Here are reported the results in terms of **Weak** and **Strong Scalability**,  which means using an increasing number of processors over a fixed input and using an increasing number of processors with the load-per-processor fixed respectively. Reported data are a mean of several executions. In a further section will be explained how to reproduce results. 

#### Strong Scalability
Here are reported data for strong scalability test. Input size is 100182 lines. The **Strong Scaling Efficency** is computed using the following formula: 

t_1 \ (N * t_n)

where t_1 is the execution time with one processor, N is the number of processors and t_n the execution time with N processors.

|Execution Time| N of Processors| Strong Scaling Efficency|
|---|---|---|
13,734902	s| 1 |	100%
7,618194	s| 2	|90%
6,0055595	s| 3	|76%
5,060087	s| 4	|68%
5,38419		s| 5	|51%
4,90386		s| 6	|47%
4,607374	s| 7	|43%
4,6461395	s| 8	|37%

![6fd31f74.png](https://github.com/mdestefano/MPI-Word-Count/blob/master/attachments/6fd31f74.png)

The tests show that the more processes are used, the less time is needed to complete the task. However, as the number of processors goes above 4, the execution time reduction decreases, assesting on about 4,6 seconds. This means that above 4 processors, the gained speedup is low. This is also confirmed by the decreasing values of **Strong Scaling efficency**.

#### Weak Scalability

Here are reported data for weak scalability test. Input per process ratio is 100182:1. The **Weak Scaling Efficency** is computed using the following formula: 

t1/t_n

where t_1 is the execution time with one processor and t_n the execution time with N processors.

|Execution Times|Processors|Weak Scaling Efficiency|
|---|---|---|
13,6810715  | 1 |	100%
15,4172175  | 2 | 89%
16,3163535  | 3 | 84%
17,654489  | 4 | 77%
18,9771515  | 5 | 72%
20,164919  |	6 |	68%
21,805584	  |	7 |63%
22,8588125	 |	8 | 60%

![4f64b230.png](https://github.com/mdestefano/MPI-Word-Count/blob/master/attachments/4f64b230.png)

As data shows, effincency constantly decrease as the number of processors rises.

#### Results Interpretation
As the tests results show, the proposed solution does not scale well. In particoular, the efficency drop can be seen when the processors used are more than 4. So 4 could be considered as the "magic number" of processes which, if exceded, leads to an increasing communication overhead.

This bad scaling could be explained by several factors:

1. To fully adhere to the assignment, only master process can access to the index file, meaning that only it can determine wich files must be read and assigned to the various workers. So, it has been chosen to have an extra round of parallel computation, to determine the total number of lines, as previously explained, rather than leaving all this work to only the master process and having from 1 to 7 idle processes. So, inevitably, the extra round couses more communication and more overhead.
2. Every time a `Scatterv` (or a `Gathterv`) function was needed, an extra `Scatter` (or `Gather`) was called, since the sending process(es) had to communicate the size of the buffer that would have been sent.
3. When it comes to the master to send the name of the files to the workers, in order to count lines, is not possible to `Scatter` the array of strings, since it would recieve a memory address rather than a real srtring. So, to correctly send the values, a for loop of `Send` is performed in master node that is known to be less efficient than a `Scatter`.
4. Data structures that uses collections (such as the wordsmap), are often resized with a `realloc` when the used array becomes full, wich means that copies memory to memory are performed, which are known to cause performance loss.

Due to time reasons (and also becouse this is already a second and optimized version of the project), this issues cannot be solved in the short time. However, here are some possible solutions: 
1. To avoid the `Send` loop, the filenames could be chained in a sigle string (with special character as separator) and a `Scatterv` applied on the new string, although it seems a dirty solution.
2. To avoid the extra `Scatter`, data could be packed in better data structures that allow the workers not to need a buffer size before recieving it, but it's difficoult, at the moment, to imagine one.
3. Use completely different data structures that doesn't need `realloc`, such as linked lists.
4. If the line is kept as working unit, the extra parallel round seems unavoidable.

## How to Reproduce Test Results

Once the folder has been unpacked, place in the project root.
Run 
```
make clean
```
to clean possible previously compiled versions.

Run
```
make 
```
to compile the solution.

Run 
```
mpirun -np i --hostfile hostfile ./bin/main input/index.txt
```
to run **Strong Scalability Tests** progressively increasing the **i** up to 8.

Run 
```
mpirun -np i --hostfile hostfile ./bin/main input/ws_i.txt
```
to run **Weak Scalability Tests** progressively increasing the **i** up to 8.

**NOTE:** Used input and index files are located in the input folder.



























