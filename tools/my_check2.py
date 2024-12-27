from pyexpat.errors import messages

p = 10
m = 100

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

messages_should_received = set()
messages_recieved = [[] for _ in range(p + 1)]
messages_sent = [[] for _ in range(p + 1)]

# print(messages_recieved, messages_sent)

for i in range(1, p + 1):
    # read file aux2/proc{i}.output, where i has leading zeros up to 2 digits, line by line
    with open(f'aux2/proc{i:02d}.output', 'r') as f:
        for line in f:
            splitted = line.split(' ')
            if len(splitted) != 3 and len(splitted) != 2:
                print(f'Process {i} outputed invalid message: ' + line)
                exit(1)

            if len(splitted) == 3:
                assert splitted[0] == 'd'
                messages_should_received.add((int(splitted[1]), int(splitted[2])))
                messages_recieved[i].append((int(splitted[1]), int(splitted[2])))
            if len(splitted) == 2:
                assert splitted[0] == 'b'
                messages_sent[i].append(int(splitted[1]))
                if i not in crashed:
                    messages_should_received.add((i, int(splitted[1])))

# print(messages_sent)

for i in range(1, p + 1):
    if i in crashed:
        continue
    for mess in range(1, m + 1):
        if mess not in messages_sent[i]:
            print(f'Process {i} did not send message {mess}')
            exit(1)
        # if mess is twice in messages_sent[i]
        if messages_sent[i].count(mess) > 1:
            print(f'Process {i} sent message {mess} more than once')
            exit(1)
    for (from_, mess) in messages_should_received:
        if (from_, mess) not in messages_recieved[i]:
            print(f'Process {i} did not receive message {mess} from process {from_}')
            exit(1)
        if messages_recieved[i].count((from_, mess)) > 1:
            print(f'Process {i} received message {mess} from process {from_} more than once')
            exit(1)
    # check the order of messages
    for j in range(1, p + 1):
        messages_from_j = [mess for (from_, mess) in messages_recieved[i] if from_ == j]
        for k in range(1, len(messages_from_j)):
            if messages_from_j[k] < messages_from_j[k - 1]:
                print(f'Process {i} received messages from process {j} in wrong order')
                exit(1)


    # with open(f'aux2/proc{i:02d}.output', 'r') as f:
    #     for line in f:
    #         _, from_, mes = line.split(' ')
    #         from_ = int(from_)
    #         mes = int(mes)
    #         if mes < 1 or mes > m:
    #             print(f'Process {i} received invalid message {mes} from process {from_}')
    #             exit(1)
    #         if (from_, mes) in messages:
    #             print(f'Process {i} received message {mes} from process {from_} more than once')
    #             exit(1)
    #         messages.append((from_, mes))
    # # print(messages)
    #
    # for j in range(1, p + 1):
    #     if j != i and j not in crashed and i not in crashed:
    #         for cur_m in range(1, m+1):
    #             if (j, cur_m) not in messages:
    #                 print(f'Process {i} did not receive message {cur_m} from process {j}')
    #                 exit(1)
    # elif i not in crashed:
    #     sent = []
    #     with open(f'aux2/proc{i:02d}.output', 'r') as f:
    #         for line in f:
    #             _, mes = line.split(' ')
    #             mes = int(mes)
    #             if mes < 1 or mes > m:
    #                 print(f'Process {i} sent invalid message {mes}')
    #                 exit(1)
    #             if mes in sent:
    #                 print(f'Process {i} sent message {mes} more than once')
    #                 exit(1)
    #             sent.append(mes)
    #
    #     for cur_m in range(1, m + 1):
    #         if cur_m not in sent:
    #             print(f'Process {i} did not send message {cur_m}')
    #             exit(1)

print('All messages received')
