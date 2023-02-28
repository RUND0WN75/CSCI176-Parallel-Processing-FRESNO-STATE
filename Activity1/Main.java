/*
File: Main.java

Name: Craig Walkup

Class: CSCI 176 Parallel Processing

Assignment abstract:

The program can be executed with the following command: .
*/

import java.util.ArrayList;

//Main class, same as file name
public class Main {
    //Global vector (ArrayList) of size 500 million
    static Integer threadCount;
    static Integer global_size = 10;
    static ArrayList<Integer> arr1 = new ArrayList<Integer>();
    static ArrayList<Thread> arr2 = new ArrayList<>();

    public static void main(String[] args) throws InterruptedException {
        threadCount = Integer.parseInt(args[0]);

        //Initializing array to i+1
        for (int i = 0; i < global_size; i++) arr1.add(i+1);

        globalSum gs = new globalSum();

        long a = System.currentTimeMillis();

        //Creating 4 computational threads
        for (int k = 0; k < threadCount; k++) {
            Thread compThread = new compThread(k,gs);
            arr2.add(compThread);
            compThread.start();
        }

        for (Thread x : arr2) x.join();

        a = System.currentTimeMillis() - a;

        //TODO: Turn a into Seconds or milliseonds and print using System.out.println()
        //For general testing purposes
        System.out.println("Hello world!");
    }
}

//Global sum class
class globalSum {
    private Long gsum;
    private int threads;

    //TODO: Create a constructor that sets gsum to 0 and threads to 0
    globalSum() {
        this.gsum = 0L;
        this.threads = 0;
    }

    //Implementing shared-variable (data) communication using 'synchronized'
    public synchronized void adding(Long local_sum) {
        gsum += local_sum; //Updating shared variable
        threads++;

        //If at the last thread, display the first global_sum
        if (threads == (Main.threadCount)) System.out.println("global_sum: " + gsum);
    }
}

//Thread class using the extends Thread
class compThread extends Thread {
    static globalSum gs;
    private final int thread_id;
    //private globalSum shared_data;
    Long local_sum = 0L;
    int my_first_index = 0;
    int my_last_index = 0;
    int div = Main.arr1.size() / Main.threadCount;
    int mod = Main.arr1.size() % Main.threadCount;

    //Constructor
    compThread(int rank, globalSum gs) {
        compThread.gs = gs;
        this.thread_id = rank;
        System.out.println("Thread for compThread " + thread_id + " created");
    }

    //Using run() method
    public void run() {
        //First calculate my_first_index and my_last_index
        //Next compute local_sum with a serial operation
        //TODO: Refer to HW Key 1 calculate Start and End indexes. use Main.arr.size() for n and Main.threadCount for p
        if (thread_id < mod) {
            //Main.threadCount = div + 1;
            my_first_index = thread_id * Main.threadCount;
        }
        else {
            //Main.threadCount = div;
            my_first_index = thread_id * Main.threadCount * mod;
        }
        my_last_index = my_first_index + Main.threadCount - 1;

        //TODO Colculate local_sum and store it
        Long local_sum = localSum(my_first_index,my_last_index);
        System.out.println("Thread " + thread_id + ", local_sum: " + local_sum);

        gs.adding(local_sum);
    }

    public long localSum(int start, int end) {
        local_sum = 0L;
        //TODO: For loop from start to end, NOT Main.arr1.size()
        for (int i = start; i < end; i++) local_sum += Main.arr1.get(i);
        return local_sum;
    }
}