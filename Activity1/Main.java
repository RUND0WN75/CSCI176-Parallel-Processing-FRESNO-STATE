import java.sql.SQLOutput;
import java.util.ArrayList;

//Main class, same as file name
public class Main {
    //Global vector (ArrayList) of size 500 million
    static Integer threadCount;
    static Integer global_size = 500_000_000;
    static ArrayList<Integer> arr1 = new ArrayList<Integer>();
    static ArrayList<Thread> arr2 = new ArrayList<>();

    static void main(String[] args) throws InterruptedException {
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

        //For general testing purposes
        System.out.println("Hello world!");
    }
}

//Global sum class
class globalSum {
    private Long gsum;
    private int threads;

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
    private int thread_id;
    private globalSum shared_data;
    Long local_sum;
    int my_first_index;
    int my_last_index;

    //Constructor
    compThread(int rank, globalSum gs) {
        this.gs = gs;
        this.thread_id = rank;
        System.out.println("Thread for compThread: " + thread_id + " created");
    }

    //Using run() method
    public void run() {
        //First calculate my_first_index and my_last_index
        //Next compute local_sum with a serial operation
        int myFirst = firstIndex();
        int myLast = lastIndex(firstIndex);

        System.out.println("Thread " + thread_id + ", local_sum: " + local_sum);

        gs.adding(local_sum);
    }

    public int firstIndex() {
        int size = Main.arr1.size() / Main.threadCount;
        int mod = Main.arr1.size() % Main.threadCount;
        int start = size * thread_id;

        if (thread_id > 0) {
            if (thread_id > 0 && thread_id < mod) start++;
            else start += mod;
        }
        return start;
    }

    public int lastIndex(int startIndex) {
        int end = startIndex + (Main.arr1.size() / Main.threadCount);
        int mod = Main.arr1.size() % Main.threadCount;
        if (thread_id < mod) end++;

        return end;
    }

    /*
    public int localSum(int start, int last) {
    }
    */
}