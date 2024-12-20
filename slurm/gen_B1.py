import os

tune_on = [(300, 60), (400, 60)]
print("Evaluating operators on: ", tune_on)

replU = 10

for n, c in tune_on:
    maxRep = replU
    name = "bm_un"
    filename = f"un_{n}_{c}.sh"
    with open(filename, "w") as f:
            f.write("#!/bin/bash\n\n")
            f.write(f"#SBATCH --job-name=un_{n}_{c}\n")
            f.write("#SBATCH --time=02:00:00\n")
            f.write("#SBATCH --ntasks=1\n")
            f.write("#SBATCH --cpus-per-task=1\n")
            f.write("#SBATCH --mem=16G\n")
            f.write("#SBATCH --mail-type=FAIL,END\n")
            f.write(f"#SBATCH --output=./out/un_{n}_{c}_%A_%a.out\n")
            f.write(f"#SBATCH --error=./err/un_{n}_{c}_%A_%a.err\n")
            f.write(f"#SBATCH --array=1-{maxRep}\n")

            # f.write("# Build\n")
            # f.write("mkdir -p build\n")
            f.write("cd build\n")
            # f.write("cmake ..\n")
            # f.write("cmake --build .\n")

            f.write("# Run\n")
            f.write(f"./JOBASP-f --in {name} --N {n} --cap {c} --repl $SLURM_ARRAY_TASK_ID --eval 0\n")
    name = "bm_cb"
    filename = f"cb_{n}_{c}.sh"
    with open(filename, "w") as f:
            f.write("#!/bin/bash\n\n")
            f.write(f"#SBATCH --job-name=cb_{n}_{c}\n")
            f.write("#SBATCH --time=02:00:00\n")
            f.write("#SBATCH --ntasks=1\n")
            f.write("#SBATCH --cpus-per-task=1\n")
            f.write("#SBATCH --mem=16G\n")
            f.write("#SBATCH --mail-type=FAIL,END\n")
            f.write(f"#SBATCH --output=./out/cb_{n}_{c}_%A_%a.out\n")
            f.write(f"#SBATCH --error=./err/cb_{n}_{c}_%A_%a.err\n")
            f.write(f"#SBATCH --array=1-{maxRep}\n")

            # f.write("# Build\n")
            # f.write("mkdir -p build\n")
            f.write("cd build\n")
            # f.write("cmake ..\n")
            # f.write("cmake --build .\n")

            f.write("# Run\n")
            f.write(f"./JOBASP-f --in {name} --N {n} --cap {c} --repl $SLURM_ARRAY_TASK_ID --eval 0\n")
    


# Generate script that runs all the above scripts
filename = f"../eval_runs_all.sh"
with open(filename, "w") as f:
    f.write("#!/bin/bash\n\n")
    # Write tuning on:
    f.write("# Evaluating operaors on: [")
    for pair in tune_on:
        f.write(f"({pair[0]}, {pair[1]}), ")
    f.write("]\n")
    f.write("cd build; cmake --build . ; cd ..\n")
    for n, c in tune_on:
        f.write(f"sbatch ./slurm/un_{n}_{c}.sh\n") 
        f.write(f"sbatch ./slurm/cb_{n}_{c}.sh\n") 

# Make file executable
os.chmod(filename, 0o755)