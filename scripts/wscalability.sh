#!/bin/bash

# Weak scalability test

# This script automates the execution of MPI-WordsCount with an increasing number of processors from 1 up to the value
# passed as --maxnp argument each called on the file passed as argument.

EXECUTABLE="./bin/main"
REPORTS="output/"
EXECTIMES="performances/weakscalability"

if [[ $# -lt 4 ]]
then
    echo "Usage:"
    echo -e "\t$0 --maxnp <value> --hostfile <hostfile> --runs <runs> $EXECUTABLE "
    exit
fi

if [[ $1 = "--maxnp" ]] && [[ $3 = "--hostfile" ]] && [[ $5 = "--runs" ]]
then
    mkdir -p "$EXECTIMES"

    MAXNP=$2
    HOSTFILE=$4
    RUNS=$6


    if [[ "$MAXNP" -le 0 ]] || [[ "$RUNS" -le 0 ]]
    then
        echo "Arguments --maxnp and --runs must be positive."
        exit
    fi

    DATE="$(date '+%Y-%m-%d %H:%M:%S')"
    FILENAME="$EXECTIMES/$DATE.csv"

    for (( i=1; i<=$MAXNP; i++ ))
        do
            
            for (( r=1; r<=$RUNS; r++))
            do

                echo "Testing with $i processes ... "                
                REPORT="$(mpirun -np "$i" "$EXECUTABLE" input/ws_"$i".txt | tail -1)"
                #REPORT="$(mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" -f "${ARGUMENTS[@]}" | tail -2)"

                echo "$REPORT"
                echo

                TIME="$(echo $REPORT | cut -d' ' -f 3 )"

                if [[ "$r" -ne "$RUNS" ]]
                then
                    echo -n "$TIME, " >> "$FILENAME"
                else
                    echo "$TIME" >> "$FILENAME"
                fi
            done
            
        done

    #echo "Tests done! Check reports in $REPORTS folder."
    echo "Execution times are reported in $FILENAME folder."
    exit
else
    echo "Unknown options \""$1"\" "$3"\" "$5"\"."
    exit
fi