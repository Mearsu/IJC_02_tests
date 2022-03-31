#!/usr/bin/python3
import time;
import os
import threading;
import queue;
import multiprocessing;

# Script for profiling wordcount, wordcount should expect file with words as 
#first argument, AVG_LEN_MIN as second and AVG_LEN_MAX as third for this script
# to work
#
#all this script does is runs wordcount x times (defined by [repeat_time]) and 
#averages running time
#
#ANG_LEN_MAX and MIN range from [1 * mult] to [range_end * mult]
#
#by default this script uses all cpu cores this can be changed using [num_cores],
#should not be more than actual number of cpu cores

#Please do not run this on merlin

repeat_time = 5;
mult = 10;
range_end = 200;
num_cores = multiprocessing.cpu_count();

shortest_time = pow(10, 100);
shortest_min = 1;
shortest_max = 1;

tt_exit = False;

timeLock = threading.Lock()
timeQueue = queue.Queue(10)
workLock = threading.Lock()
workQueue = queue.Queue(20)


def run_words():
    while not tt_exit:
        workLock.acquire();
        if not workQueue.empty():
            data = workQueue.get();
            workLock.release();
            avg = 0;
            for x in range(0, repeat_time):
                start = time.time();
#                os.putenv("AVG_LEN_MIN", str(data[0]));
#                os.putenv("AVG_LEN_MAX", str(data[1]));
                os.system("./wordcount ./words " + str(data[0]) + " " + str(data[1]) + " > /dev/null");
                end = time.time();
                total = end - start;
                avg += total/repeat_time;
            timeLock.acquire();
            timeQueue.put((avg, data))
            timeLock.release();
        else:
            workLock.release();
        time.sleep(1);

ts = [];
for i in range(0,num_cores):
    ts.append(threading.Thread(target=run_words));
    ts[i].start();


  


for i in range(1,range_end + 1):
    for j in range(1 + i,range_end + i + 1):
        workLock.acquire();
        if workQueue.full():
            workLock.release();
            time.sleep(1);
            j -= 1
            continue
        else:
            workQueue.put((i * mult,j * mult))
            workLock.release();
        timeLock.acquire();
        while not timeQueue.empty():
            data = timeQueue.get();
            if data[0] < shortest_time:
                print("new shortest: ", data[0], " MIN: ", data[1][0], " MAX: ", data[1][1]);
                shortest_time = data[0]
                shortest_min = data[1][0]
                shortest_max = data[1][1]

        timeLock.release();

while not tt_exit:
    workLock.acquire();
    if workQueue.empty():
        tt_exit = True;
    workLock.release();
    time.sleep(1);
    timeLock.acquire();
    while not timeQueue.empty():
        data = timeQueue.get();
        if data[0] < shortest_time:
            print("new shortest: ", data[0], " MIN: ", data[1][0], " MAX: ", data[1][1]);
            shortest_time = data[0]
            shortest_min = data[1][0]
            shortest_max = data[1][1]
    timeLock.release();

print("finished");
for i in range(0,num_cores):
    ts[i].join();

print("MIN: ", shortest_min)
print("MAX: ", shortest_max)
print("Time: ", shortest_time)
