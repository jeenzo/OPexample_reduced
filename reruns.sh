#!/bin/bash

#SBATCH --job-name=test
#SBATCH --time=08:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=24G
#SBATCH --mail-type=FAIL,END
#SBATCH --output=./out/test_%A_%a.out
#SBATCH --error=./err/test_%A_%a.err
#SBATCH --array=1-2

rm -r out/; rm -r err/
cd build; cmake --build .

if [[ ${SLURM_ARRAY_TASK_ID} == 1 ]]; then
        ./JOBASP-f --in bm_un --N 100 --cap 45 --repl 7 --seg 2000
elif [[ ${SLURM_ARRAY_TASK_ID}  == 2 ]]; then
        ./JOBASP-f --in bm_un --N 100 --cap 45 --repl 8 --seg 2000
else
        echo "Invalid array index" >&2
        exit 1
fi