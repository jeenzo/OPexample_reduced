
# Combinations of demand classes - read from ../../tune_runs.sh (fist line)
with open(f"../../eval_runs_all.sh", "r") as f:
    line = next(f)
    line = next(f)
    tune_on = f.readline().split("Evaluating operaors on: ")[1].split("\n")[0].split(", ")

# remove unnecessary characters
tune_on[0] = tune_on[0].split("[")[1]
tune_on[-1] = tune_on[-1].split("]")[0]
# 0, 2, 4, 6, 8 are num, 1, 3, 5, 7, 9 are cap
numL = []
capL = []
numU = []
capU = []
for i in range(len(tune_on)-1):
    if i % 2 == 0:
        if int(tune_on[i].split("(")[1]) >= 200:
            numL.append(int(tune_on[i].split("(")[1]))
        else:
            numU.append(int(tune_on[i].split("(")[1]))
    else:
        if int(tune_on[i].split(")")[0]) > 15:
            capU.append(int(tune_on[i].split(")")[0]))
        else:
            capL.append(int(tune_on[i].split(")")[0]))

print(numL, capL, numU, capU)


import pandas as pd
import csv
import os

# solution_folder = '../results/retailer1Perf/'
solution_folder = '../results/benchmarksA2_0/'

replU = 39
replL = 9

for i, n in enumerate(numU):
    c = capU[i]
    name = "bm_cb"
    repl = replU
    if n >= 200:
        name = "bm_la"
        repl = replL
        c = capL[i]
    for r in range(1, repl + 1):
        batches = []
        solution_file = solution_folder + name +'_n' + str(n) + '_c' + str(c) + '_' + str(r) + '_final_batches.txt'
        # Read file if exists
        if not os.path.exists(solution_file):
            continue
        with open(solution_file, 'r') as csvfile:
            csvreader = csv.reader(csvfile, delimiter='\n')
            fist = True
            for row in csvreader:
                if not row: continue
                if row[0].startswith('W'):
                    w_id = int(row[0].split(':')[0].replace('W', ''))
                    if not fist:
                        batches.append(curr_batches)
                    curr_batches = []
                    curr_batch = []
                    for el in row[0].split(':')[2:]:
                        el = el.strip()
                        el = el.replace('{', '')
                        el = el.replace('}', '')
                        # el to list
                        el = el.split(';')
                        for e in el:
                            e = e.strip()
                            e = e.split(',')
                            if e[0] == '':
                                e = []
                            else:
                                e = [int(x) for x in e]
                            curr_batch.append(e)
                        curr_batches.append(curr_batch)
                        fist = False
                else:
                    curr_batch = []
                    for el in row[0].split(':')[1:]:
                        el = el.strip()
                        el = el.replace('{', '')
                        el = el.replace('}', '')
                        # el to list
                        el = el.split(';')
                        for e in el:
                            e = e.strip()
                            e = e.split(',')
                            if e[0] == '':
                                e = []
                            else:
                                e = [int(x) for x in e]
                            curr_batch.append(e)
                        curr_batches.append(curr_batch)
        batches.append(curr_batches)
        # Save to file
        with open('./resA2/n' + str(n) + '_c' + str(c) + '_' + str(r) + '_input_batches.csv', 'w') as csvfile:
            csvwriter = csv.writer(csvfile, delimiter=',')
            for w in batches:
                csvwriter.writerow("W")
                for b in w:
                    csvwriter.writerow("B")
                    for e in b:
                        csvwriter.writerow(e)

for i, n in enumerate(numL):
    c = capU[i]
    name = "bm_cb"
    repl = replU
    if n >= 200:
        name = "bm_la"
        repl = replL
        c = capL[i]
    for r in range(1, repl + 1):
        batches = []
        solution_file = solution_folder + name +'_n' + str(n) + '_c' + str(c) + '_' + str(r) + '_final_batches.txt'
        # Read file if exists
        if not os.path.exists(solution_file):
            continue
        with open(solution_file, 'r') as csvfile:
            csvreader = csv.reader(csvfile, delimiter='\n')
            fist = True
            for row in csvreader:
                if not row: continue
                if row[0].startswith('W'):
                    w_id = int(row[0].split(':')[0].replace('W', ''))
                    if not fist:
                        batches.append(curr_batches)
                    curr_batches = []
                    curr_batch = []
                    for el in row[0].split(':')[2:]:
                        el = el.strip()
                        el = el.replace('{', '')
                        el = el.replace('}', '')
                        # el to list
                        el = el.split(';')
                        for e in el:
                            e = e.strip()
                            e = e.split(',')
                            if e[0] == '':
                                e = []
                            else:
                                e = [int(x) for x in e]
                            curr_batch.append(e)
                        curr_batches.append(curr_batch)
                        fist = False
                else:
                    curr_batch = []
                    for el in row[0].split(':')[1:]:
                        el = el.strip()
                        el = el.replace('{', '')
                        el = el.replace('}', '')
                        # el to list
                        el = el.split(';')
                        for e in el:
                            e = e.strip()
                            e = e.split(',')
                            if e[0] == '':
                                e = []
                            else:
                                e = [int(x) for x in e]
                            curr_batch.append(e)
                        curr_batches.append(curr_batch)
        batches.append(curr_batches)
        # Save to file
        with open('./resA2/n' + str(n) + '_c' + str(c) + '_' + str(r) + '_input_batches.csv', 'w') as csvfile:
            csvwriter = csv.writer(csvfile, delimiter=',')
            for w in batches:
                csvwriter.writerow("W")
                for b in w:
                    csvwriter.writerow("B")
                    for e in b:
                        csvwriter.writerow(e)
