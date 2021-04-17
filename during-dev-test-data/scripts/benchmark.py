#! /usr/bin/env python
# Do benchmark test and generate resulting graphs.
# TODO: Use multithreading for faster parallel execution.

import time
import subprocess
import matplotlib.pyplot as plt


######## Set the variable and execute the script. #########
script = "/home/sachin/gnuastro_dev/gnuastro/tests/during-dev.sh"
maximum_executions = 10
maximun_jump_factor = 10
graph_name = "KD-tree"
###########################################################


# Wrapper function for calculating runtime.
def timed_execution(function):
    def wrapper(args):
        start = time.time()
        function(args)
        end = time.time()
        return (end-start)
    return wrapper


@timed_execution
def execute_script(script_):
    subprocess.call([script_], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)


# Make the list of number of times the script is to be executed.
def make_num_execution_list(num, jump_factor):
    num_execution_list = []
    i = 1
    while(i <= num):
        num_execution_list.append(i)
        i *= jump_factor

    return num_execution_list


# Make the list for the time taken by the script to run for a particular number of times.
def make_time_list(num_execution_list):
    time_list = []
    for num in num_execution_list:
        time_taken = 0
        for _ in range(int(num)):
            t = execute_script(script)
            time_taken += t
        time_list.append(time_taken)

    return time_list


# Make a graph between the number of times the script is executed and the time taken for it.
def make_graph(name, time_list, num_execution_list):
    plt.plot(num_execution_list, time_list, marker='o', markerfacecolor='red', markersize=10)
    plt.xlabel("number of executions")
    plt.ylabel("time required for executions")
    plt.title(name)
    plt.show()


# Interface
if __name__ == "__main__":
    num_execution_list = make_num_execution_list(maximum_execution, maximum_jump_factor)
    make_graph(graph_name, make_time_list(num_execution_list), num_execution_list)
