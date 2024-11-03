from pyexpat.errors import messages

p = 40
m = 200
receiver = 1

crashed = []
# read the file kek.txt line by line
with open('kek', 'r') as f:
    for line in f:
        # if line is of form "Sending SIGTERM to process {number}" extract the number
        if line.startswith('Sending SIGTERM to process'):
            # extract the number from the line
            number = int(line.split(' ')[-1])
            crashed.append(number)

if receiver in crashed:
    print(f'Process {receiver} crashed')
    exit(1)
print(crashed)

for i in range(1, p + 1):
    if i == receiver:
        messages = []
        # read file aux/proc{i}.output, where i has leading zeros up to 2 digits, line by line
        with open(f'aux/proc{i:02d}.output', 'r') as f:
            for line in f:
                _, from_, mes = line.split(' ')
                from_ = int(from_)
                mes = int(mes)
                if mes < 1 or mes > m:
                    print(f'Process {i} received invalid message {mes} from process {from_}')
                    exit(1)
                if (from_, mes) in messages:
                    print(f'Process {i} received message {mes} from process {from_} more than once')
                    exit(1)
                messages.append((from_, mes))
        # print(messages)

        for j in range(1, p + 1):
            if j != i and j not in crashed and i not in crashed:
                for cur_m in range(1, m+1):
                    if (j, cur_m) not in messages:
                        print(f'Process {i} did not receive message {cur_m} from process {j}')
                        exit(1)
    elif i not in crashed:
        sent = []
        with open(f'aux/proc{i:02d}.output', 'r') as f:
            for line in f:
                _, mes = line.split(' ')
                mes = int(mes)
                if mes < 1 or mes > m:
                    print(f'Process {i} sent invalid message {mes}')
                    exit(1)
                if mes in sent:
                    print(f'Process {i} sent message {mes} more than once')
                    exit(1)
                sent.append(mes)

        for cur_m in range(1, m + 1):
            if cur_m not in sent:
                print(f'Process {i} did not send message {cur_m}')
                exit(1)

print('All messages received')
