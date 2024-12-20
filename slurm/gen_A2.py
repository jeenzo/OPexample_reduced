import os

tune_on = [(20, 75), (30, 75), (40, 75), (60, 75), (80, 75), (100, 75)]
print("Evaluating operators on: ", tune_on)

replU = 39
replL = 9

for n, c in tune_on:
    name = "bm_un"
    maxRep = replU
    if n >= 200:
        name = "bm_la"
        maxRep = replL
    filename = f"eval_n{n}_c{c}.sh"
    with open(filename, "w") as f:
            f.write("#!/bin/bash\n\n")
            f.write(f"#SBATCH --job-name={n}_{c}\n")
            f.write("#SBATCH --time=01:00:00\n")
            f.write("#SBATCH --ntasks=1\n")
            f.write("#SBATCH --cpus-per-task=1\n")
            f.write("#SBATCH --mem=8G\n")
            f.write("#SBATCH --mail-type=FAIL,END\n")
            f.write(f"#SBATCH --output=./out/{n}_{c}_%A_%a.out\n")
            f.write(f"#SBATCH --error=./err/{n}_{c}_%A_%a.err\n")
            f.write(f"#SBATCH --array=1-{maxRep}\n")

            # f.write("# Build\n")
            # f.write("mkdir -p build\n")
            f.write("cd build\n")
            # f.write("cmake ..\n")
            # f.write("cmake --build .\n")

            f.write("# Run\n")
            # f.write(f"./JOBASP-f --in {name} --N {n} --cap {c} --repl $SLURM_ARRAY_TASK_ID\n")
            f.write(f"./JOBASP-f --in {name} --N {n} --cap {c} --repl $SLURM_ARRAY_TASK_ID\n")

    filename = f"a_eval_n{n}_c{c}.sh"
    with open(filename, "w") as f:
            f.write("#!/bin/bash\n\n")
            f.write(f"#SBATCH --job-name=a_{n}_{c}\n")
            f.write("#SBATCH --time=01:00:00\n")
            f.write("#SBATCH --ntasks=1\n")
            f.write("#SBATCH --cpus-per-task=1\n")
            f.write("#SBATCH --mem=8G\n")
            f.write("#SBATCH --mail-type=FAIL,END\n")
            f.write(f"#SBATCH --output=./out/a_{n}_{c}_%A_%a.out\n")
            f.write(f"#SBATCH --error=./err/a_{n}_{c}_%A_%a.err\n")
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
        f.write(f"sbatch ./slurm/eval_n{n}_c{c}.sh\n") 
        # f.write(f"sbatch ./slurm/a_eval_n{n}_c{c}.sh\n")

# Make file executable
os.chmod(filename, 0o755)