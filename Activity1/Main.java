/*
Assignment abstract: an assignment meant to demonstrate parallel programming using Java threads. Using the local sum
and global sum concepts, threads are utilized to perform some amount of separate work to compute
the sums. The main emphasis of the implementation is through shared-variable (data) communication.
The program will output the creation of 8 threads, followed by a local sum value for each thread, and a global sum
being displayed at the end.

The program can be executed with the following command:
javac Main.java                     for compiling
java -Xmx8g Main.class              8 to run
*/

import java.util.ArrayList;

//Main class, same as file name
public class Main {
    //Global vector (ArrayList) of size 500 million
    static Integer threadCount;
    static Integer global_size = 350_000_000;
    static ArrayList<Integer> arr1 = new ArrayList<Integer>();
    static ArrayList<Thread> arr2 = new ArrayList<>();

    public static void main(String[] args) throws InterruptedException {
        threadCount = Integer.parseInt(args[0]);

        //Initializing array to i+1
        for (int i = 0; i < global_size; i++) arr1.add(i+1);

        globalSum gs = new globalSum();

        //Creating computational threads
        for (int k = 0; k < threadCount; k++) {
            Thread compThread = new compThread(k,gs);
            arr2.add(compThread);
        }

        long time1 = System.currentTimeMillis();
        for (Thread x : arr2) x.start();
        for (Thread x : arr2) x.join();

        time1 = System.currentTimeMillis() - time1;
    }
}

//Global sum class
class globalSum {
    private Long gsum;
    private int threads;
    private long start;

    globalSum() {
        this.gsum = 0L;
        this.threads = 0;
    }

    //Implementing shared-variable (data) communication using 'synchronized'
    public synchronized void adding(Long local_sum) {
        if (threads == 0) start = System.currentTimeMillis();
        gsum += local_sum; //Updating shared variable
        threads++;

        //If at the last thread, display the first global_sum
        if (threads == (Main.threadCount)) {
            start = System.currentTimeMillis() - start;
            System.out.println("Execution time: " + start/1000.0 + " seconds");
            System.out.println("global_sum: " + gsum);
        }
    }
}

//Thread class using the extends Thread
class compThread extends Thread {
    static globalSum gs;
    private final int thread_id;
    Long local_sum = 0L;
    int my_first_index = 0, my_last_index = 0;
    int div = Main.arr1.size() / Main.threadCount, mod = Main.arr1.size() % Main.threadCount;

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
        //Main.arr.size() for n and Main.threadCount for p
        if (thread_id < mod) {
            div++;
            my_first_index = thread_id * div;
        }
        else my_first_index = thread_id * div + mod;

        my_last_index = my_first_index + div - 1;

        Long local_sum = localSum(my_first_index,my_last_index);
        //System.out.println(my_first_index + " " + my_last_index + " " + thread_id);
        System.out.println("Thread " + thread_id + ", local_sum: " + local_sum);

        gs.adding(local_sum);
    }

    public long localSum(int start, int end) {
        local_sum = 0L;
        for (int i = start; i < end; i++) local_sum += Main.arr1.get(i);
        return local_sum;
    }
}
