
d = {}
for line in open("lock.txt"):
    if not line.startswith("wlock") and \
            not line.startswith("rlock") and \
            not line.startswith("unlock"):
                continue
    parts = line[:-1].split("@")
    if (len(parts) != 4):
        print parts
    assert(len(parts) == 4)
    if parts[3] != "Node" and parts[3] != "HEADER":
        continue

    d.setdefault(parts[2], 0)
    if parts[0] == "unlock":
       d[parts[2]] -= 1
    else:
       d[parts[2]] += 1

for key, value in d.items():
    if value != 0:
        print key, value




