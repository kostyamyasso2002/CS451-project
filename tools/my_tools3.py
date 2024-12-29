# count lines in a file aux3/hosts

n = 0

with open('aux3/hosts', 'r') as f:
    for _ in f:
        n += 1

print(n)

crashed = []
# read the file kek.txt line by line
with open('kek', 'r') as f:
    for line in f:
        # if line is of form "Sending SIGTERM to process {number}" extract the number
        if line.startswith('Sending SIGTERM to process'):
            # extract the number from the line
            number = int(line.split(' ')[-1])
            crashed.append(number)

print(crashed)

configs = [[] for _ in range(n + 1)]
first_line = ""
for i in range(1, n + 1):
    with open(f'aux3/proc{i:02d}.config', 'r') as f:
        # skip the first line
        first_line = f.readline()
        for line in f:
            configs[i].append(list(map(int, line.strip().split(' '))))
p = int(first_line.split(' ')[0])
# print(configs)

decisions = [[] for _ in range(n + 1)]
for i in range(1, n + 1):
    with open(f'aux3/proc{i:02d}.output', 'r') as f:
        for line in f:
            # assert line[:2] == 'd '
            decisions[i].append(set(list(map(int, line.strip().split(' ')))))

# print(decisions)

unions = [set() for _ in range(p)]
for i in range(1, n + 1):
    for j in range(p):
        for s in configs[i][j]:
            unions[j].add(s)

# print(unions)

for i in range(1, n + 1):
    if i not in crashed:
        # assert len(decisions[i]) == p
        if len(decisions[i]) != p:
            print(f'Process {i} does not decide on all agreements')
            exit(1)
    for j in range(len(decisions[i])):
        config_sub_dec = True
        for s in configs[i][j]:
            if s not in decisions[i][j]:
                config_sub_dec = False
                break
        if not config_sub_dec:
            print(f'Process {i} does not decide on all values in agreement {j}')
            exit(1)

        dec_sub_union = True
        for s in decisions[i][j]:
            if s not in unions[j]:
                dec_sub_union = False
                break
        if not dec_sub_union:
            print(f'Process {i} decides on value not in agreement {j}')
            exit(1)

print('All checks passed')