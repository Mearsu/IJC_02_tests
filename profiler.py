#!/usr/bin/python3
import time;
import os
import threading;
import queue;
import multiprocessing;

# WIP!
# Script for profiling wordcount, wordcount should expect file with words as 
#first argument, AVG_LEN_MIN as second and AVG_LEN_MAX as third for this script
# to work
#
#all this script does is runs wordcount x times (defined by [repeat_time]) and 
#averages running time
#
#ANG_LEN_MAX and MIN range from 1 to [init_range]
#
#by default this script uses all cpu cores this can be changed using [num_cores],
#should not be more than actual number of cpu cores

#Please do not run this on merlin

repeat_time = 2;
init_range = 5000
init_step = 100;
num_cores = multiprocessing.cpu_count();

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
#            print(data)
            if data[0] <= 0 or data[1] <= 0:
                continue;
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


def run_range(max_start, max_stop, min_start, min_stop, step):
    times = [];
    for i in range(max_start, max_stop, step):
        for j in range(max(min_start, i), min_stop, step):
            workLock.acquire();
            if workQueue.full():
                workLock.release();
                time.sleep(1);
                j -= 1
            else:
                # i - AVG_LEN_MAX, j - AVG_LEN_MIN
                workQueue.put((i,j))
                workLock.release();
                continue
            timeLock.acquire();
            if timeQueue.empty():
                time.sleep(1);
                timeLock.release();
                continue;
            while not timeQueue.empty():
                data = timeQueue.get();
                timeLock.release();
                if len(times) < 5:
                    times.append(data);
                else:
                    for k in range(0,5):
                        if times[k][0] > data[0]:
                            times[k] = data;
                            break;
            workLock.release();
            timeLock.release();
    while True:
        workLock.acquire();
        timeLock.acquire();
        if workQueue.empty() and timeQueue.empty():
            workLock.release();
            timeLock.release();
            return times;
        workLock.release();
        while not timeQueue.empty():
            data = timeQueue.get();
            timeLock.release();
            if len(times) < 5:
                times.append(data);
            else:
                for k in range(0,5):
                    if times[k][0] > data[0]:
                        times[k] = data;
                        break
        timeLock.release();
        time.sleep(1);

def run_subranges(data, step):
    return run_range(
        int(data[1][0] - step * 2), int(data[1][0] + step * 2),
        int(data[1][1] - step * 2), int(data[1][1] + step * 2), int(step / 2));

print("Times: [(time (AVG_LEN_MIN, AVG_LEN_MAX)),...]")
times = run_range(init_step, init_range,init_step, init_range, init_step);

print("shortest(not final):", end="");
#print(times, end="\r");
print(times, end="\n");
step = init_step
while step > 2:
    new_times = []
    for ti in times:
        new_times += run_subranges(ti, int(step));
    new_times.sort(key=lambda y: y[0]);
    times = new_times[:5];
    print("shortest:", end="");
#    print(times, end="\r");
    print(times, end="\n");
    step /= 2;


    

print("Final results:", end = "")
print(times, end="                   \n");

tt_exit = True;

for i in range(0,num_cores):
    ts[i].join();

