import subprocess
import sys
import time
import random

TOTAL_BOTS = 5

def main():
    exe = "exec/KVEngine.exe"
    if sys.platform.startswith("linux"):
        exe = "./KVEngine"
    args = [exe, 'ip=grief.ly', '-nodraw', '-auto', '-auto_connect', 'login=Guest']
    processes = []

    total_bots = TOTAL_BOTS

    if len(sys.argv) == 2:
        total_bots = int(sys.argv[1])
    
    if total_bots > 20:
        raise ArgsError("Too much bots")
        
    print "Amount of bots: " + str(total_bots)

    for x in range(0, total_bots):
        p = subprocess.Popen(args, cwd='./Exec')
        processes.append(p)

    processes = processes[1:]

    while True:
        time.sleep(5)
        new_processes = []
        for p in processes:
            if random.randint(0, 10) == 0:
                p.kill()
                time.sleep(1)
                p.communicate()
                new_processes.append(subprocess.Popen(args, cwd='./Exec'))
            else:
                new_processes.append(p)
        processes = new_processes

if __name__ == "__main__":
    main()
