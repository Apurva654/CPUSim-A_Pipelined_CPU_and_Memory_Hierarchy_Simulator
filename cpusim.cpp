//                   CPUSim 🕸️ 💻
//                          - A Pipelined CPU & Memory Hierarchy Simulator

//    U must be finding a total of 7 MAJOR CODE SECTIONS in my program,
//     every comment at the start of the code block summarizes what the code does.

// RUN THIS COMMAND IN YOUR TERMINAL - "g++ .\cpusim.cpp -o .\cpusim.exe; .\cpusim.exe" 🏃🏃
//--------------------------------------------------------------------------------------

//                       Dependencies:  🔖🔖 

#include <iostream> // used for printing output
#include <fstream> //reading input file
#include <sstream> //splitting each  input line into tokens
#include <string> 
#include <vector> //storing memory blocks and tasks
#include <deque>//cache FIFO queue storage 
#include <queue>  //Round Robin ready queue
#include <algorithm>  //searching with find
#include <iomanip> //formatted percentages
#include <stdexcept> //throwing  runtime errors
using namespace std;

// --------------------------------------------------------------------------------
//  SECTION 1 — Task 💻
//  A Task represents one process waiting to run on the CPU.
//  It tracks how many cycles it still needs and which memory blocks it reads.

struct Task
 {
    string id;           
    // stores tasks name
    int burstTime;     
    // total CPU cycles needed for that task
    int remainingTime; 
    // CPU cycles still left
    vector<string> memBlocks;    
    // ordered list of memory blocks that the tasks accesses
    int memIndex;      
    //  tracks next block to request
    int startCycle;    
    //  Stores the cycle number when task first starts running( initially=-1)
    int finishCycle;   
    //Stores the cycle number when the task finishes (initially=0)
    bool completed;
    // Tells whether the task has finished execution.

    
    Task(const string& id, int burst, const vector<string>& mem)   //constructor
        : id(id), burstTime(burst), remainingTime(burst),  //It creates a new Task
        memBlocks(mem), memIndex(0), startCycle(-1), 
        finishCycle(0), completed(false)
         {}

    const string& nextBlock() const
     { return memBlocks[memIndex % memBlocks.size()];}
    //returns the memory block the task wants right now.
    //It selects the memory block to request.
    //we have used modulo so if the task runs longer than its memory list,
    //it loops back to the beginning

    void stepMemIndex() 
    {++memIndex;}  
     //move to the next memory block
};
// --------------------------------------------------------------------------------
//  SECTION 2 — Cache Level ©️
//  Each level is a fixed-size FIFO queue. When it's full, the block at the
//  front is evicted. Newest blocks are pushed to the back.

struct CacheLevel 
{
    string name;    //Stores cache name 
    int capacity;  // max number of memory blocks this cache can hold
    int latency;   // cycles paid on a hit in the cache
    int hits;     //number of successful accesses in this level
    int misses;   //number of failed accesses in this level
    deque<string> slots; //Stores memory blocks currently in the cache

     //constructor for creating a cache level.
    CacheLevel(const string& name, int cap, int lat)
        : name(name), capacity(cap), latency(lat), hits(0), misses(0) 
        {}

    // Checks whether a memory block exists in this cache. 
    bool has(const string& block) const 
    {return find(slots.begin(), slots.end(), block) != slots.end();}

    //Insert a block and returns the evicted block name if evicted
    // we follow FIFO i.e. we always evict from the front
    string insert(const string& block) 
    {
        string evicted; //it is used to stores the block removed from cache
        if ((int)slots.size()>=capacity)  //Checks if the deque is full
        //casting to int avoids signed/unsigned comparison warnings.
        {
            evicted=slots.front();   // Grabs the oldest block
            slots.pop_front();         //Removes it from the cache
        }
        slots.push_back(block);        // Adds the new block at the back
        return evicted; //If nothing was evicted,it returns empty string
    }

    //Remove a specific block from the cache(used when promoting it to a higher level) 
    //This prevents duplicates and maintains the proper hierarchy state.
    void remove(const string& block) {
        auto found=find(slots.begin(), slots.end(), block);
        //auto lets the compiler do the type-figuring work.
        if (found!= slots.end()) //Finds the block
            slots.erase(found); //If found,removes it.
    }

    //used for printing the current cache contents in a readable format
    string display() const 
    {
        string out=name+": [";
        for (size_t i=0; i<slots.size(); i++) 
        {   //Iterates over each cached block by index
            out += slots[i];
            if (i+1 < slots.size()) 
                out += ", ";
        }
        return out + "]";
    }
};

// --------------------------------------------------------------------------------
//  SECTION 3 — Cache Hierarchy 🧑‍💻
//  it Ties L1, L2, L3, and RAM together.
//  On every memory access: 
//    1. Search L1 → L2 → L3 in order.
//    2. On a HIT at any level: pay that level's latency and promote the block
//       to L1 for faster access purpose.
//    3. On a total MISS: fetch from RAM (which will always succeeds :) ), load into L1.
//       If L1 is full its evicted block cascades into L2, and so on ...

struct CacheHierarchy 
{
    CacheLevel l1, l2, l3; //Creates three cache levels
    int ramAccesses;   // Counts how many times RAM was accessed
    int totalLatency;  // counts total cycles spent across all memory accesses

    CacheHierarchy()         //constructor
        : l1("L1",32,4), l2("L2",128,12), l3("L3",512,40),
        // things inside bracket indicates the name,capacity,latency cycle
          ramAccesses(0), totalLatency(0) 
          {}

    //This is the Main memory access function 🙇‍♀️
    int access(const string& block) 
    {
        int latency =0;

        //              checking L1....🔁
        if (l1.has(block)) 
        {
            l1.hits++;
            latency = l1.latency;
            cout << "  " << l1.display() << "  >> HIT (" << latency << " cycles)\n";
            cout << "  " << l2.display() << "\n";
            cout << "  " << l3.display() << "\n";
            totalLatency += latency;
            return latency;
        }
        l1.misses++;
        cout<<"  "<< l1.display()<<"  >>  MISS\n";

        //            checking L2.....🔁
        if (l2.has(block)) 
        {
            l2.hits++;
            latency = l2.latency;
            cout <<"  "<< l2.display()<<"  >>  HIT  ("<<latency<<" cycles)\n";
            cout <<"  "<< l3.display()<< "\n";

            //         Promote: L2 → L1 🐾
            l2.remove(block);   
            //Removes block from L2 because it will be promoted to L1
            string ev = l1.insert(block);
            // if L1 is full, evict the oldest L1 block
            //That evicted block is then pushed down into L2
            if (!ev.empty())
             {
                // Evicted from L1 → cascade into L2
                string ev2 = l2.insert(ev);
                cout << "  Promoting " << block << " L2 -> L1  |  " << ev << " evicted L1 -> L2\n";
                if (!ev2.empty()) 
                l3.insert(ev2);// if L2 is also fully occupied then cascade it to L3
            } 
            else 
            {
             cout<<"  Promoting "<< block <<" L2 -> L1\n";
            }
            totalLatency += latency;
            return latency;
        }
        l2.misses++; //If not in L2, record L2 miss
        cout << "  " << l2.display() << "  >>  MISS\n";

        //              checking L3....🔁
        if (l3.has(block)) 
        {
            l3.hits++;
            latency = l3.latency;
            cout<<"  "<< l3.display()<<"  >>  HIT  (" <<latency<< " cycles)\n";

            // Promote: L3 → L1
            l3.remove(block);
            string ev = l1.insert(block);
            if (!ev.empty()) 
            {//if L1 evicts something, that block goes to L2, and further to L3 as required
                string ev2 = l2.insert(ev);
                cout << "  Promoting " << block << " L3 -> L1  |  " << ev << " evicted L1 -> L2\n";
                if (!ev2.empty()) 
                    l3.insert(ev2);
            }  
            else
            {
                cout << "  Promoting " << block << " L3 -> L1\n";
            }

            totalLatency += latency;
            return latency;
        }
        l3.misses++;
        cout << "  " << l3.display() << "  >>  MISS\n";

        //        checking RAM ....🔁 (which always wins o(*^＠^*)o  )
        ramAccesses++;
        latency = 200;
        cout << "  Fetching " << block << " from RAM  (200 cycles)\n";

        // Fetched block is inserted into L1
        // cascade evictions downward through L2 and if required L3 as well
        string evL1 = l1.insert(block);
        if (!evL1.empty()) 
        {
            string evL2 = l2.insert(evL1);
            cout << "  " << evL1 << " evicted L1 -> L2\n";
            if (!evL2.empty()) 
            {
                l3.insert(evL2);
                cout << "  " << evL2 << " evicted L2 -> L3\n";
            }
        }
        totalLatency += latency;
        return latency;
    }

    // Print current state of all three cache levels 🖨️
    void printState() const
    {
        cout << "  " << l1.display() << "\n";
        cout << "  " << l2.display() << "\n";
        cout << "  " << l3.display() << "\n";
    }
};

// --------------------------------------------------------------------------------
//  SECTION 4 — Round Robin Scheduler

//  Why Round Robin? 💭
//  cuz it's Fair,Responsive,Traceable, Realistic
//  A task runs for up to `quantum` cycles.When the quantum expires it is
//  moved to the back of the ready queue and the next task runs.If a task
//  finishes before its quantum is up it releases the CPU immediately.

class Scheduler {
public:
    int quantum;
    //The maximum number of cycles a task can run before switching

    //constructor
    explicit Scheduler (int q = 3) 
    //explicit is used to stop C++ from doing an automatic conversion 
    //from an int into a Scheduler
    : quantum(q), 
    running(nullptr), 
    //sets the scheduler’s running pointer to nothing
    // when the Scheduler object is first created.
    cyclesThisSlice(0)//slice counter is at 0
     {}

    void load(vector<Task>& tasks) //Loads all tasks into the scheduler
     {
        for (auto& t : tasks) 
         // & sign indicates that t is a reference to the actual task, not a copy
            readyQueue.push(&t); //Pushes addresses of tasks into the ready queue
        dequeue();  // prime the first task to run 🏃
    }

    Task* current() const //Returns the currently running task               
    {return running;}

    // Call once per cycle AFTER the memory access has been processed.
    // it Decrements remaining time, handles completion and preemption.
    void tick(int cycle) 
    {
        if (!running) //If no task is running, do nothing
        return;

        if (running->startCycle == -1) // if this task has not started yet
           {  // "->" is used to dereference the pointer and accesses the member.
            running->startCycle = cycle;
           }
           running->remainingTime--; //reduce the task’s remaining execution time by 1
           cyclesThisSlice++;

        bool done=(running->remainingTime == 0);
        //task is finished if it has no time left
        bool preempted=(cyclesThisSlice>=quantum);
        //Checks if the task used up its time quantum

        if(done) 
        {
            running->completed  = true; //mark the task as completed
            running->finishCycle = cycle; //record when it finished
            running = nullptr; //clear the current running task
            cyclesThisSlice = 0;//reset the slice counter
            dequeue(); //choose the next task from the ready queue
        } 
        else if(preempted) 
        {
            readyQueue.push(running);
            //put the current task back at the end of the queue
            running = nullptr;
            cyclesThisSlice = 0;
            dequeue();
        }
    }

    bool allDone() const //Checks whether the  simulation is finished
    { 
     return running == nullptr  &&  readyQueue.empty(); 
    }
// --------------------------------------------------------------------------------

private:
    queue<Task*> readyQueue; //Queue of waiting tasks
    Task* running; //Pointer to the currently running task
    int cyclesThisSlice;//Counts how many cycles current task has used in this time slice
    void dequeue() //checks whether the queue has any waiting tasks
    {
        if (!readyQueue.empty()) 
        {
            running=readyQueue.front();
            readyQueue.pop();
        }
    }
};

// --------------------------------------------------------------------------------
//  SECTION 5 — Simulator
// This is The main loop.
// Each iteration is equalto one CPU cycle, what it does is:
//    1.  Finds the running task.
//    2.  Asks it for its next memory block.
//    3.  Passes that block to the cache hierarchy.
//    4.  Prints the updated cache state.
//    5.  Advances the scheduler.

class Simulator {   //controls the whole simulation.
public:
    Simulator(vector<Task>& tasks, int quantum)   //constructor
        : tasks(tasks), sched(quantum), cycle(0), elapsedCycles(0)
         {}

    void run() 
    {
        sched.load(tasks); //Loads all tasks into the scheduler's ready queue
        printBanner();    //Displays the simulation header
        while (!sched.allDone()) //Keeps running cycles until all tasks finish.
        {
            runOneCycle();
        }
    }

    void printSummary() const {
        printTaskTable();
        printTotals();
        printCacheStats();
    }
// --------------------------------------------------------------------------------
private:
    vector<Task>& tasks; //Reference to task list
    Scheduler sched;
    CacheHierarchy cache;
    int cycle; //CPU time step count
    int elapsedCycles; //CPU steps plus memory latency

    void runOneCycle() //Runs one CPU cycle
    {
        ++cycle;
        Task* t = sched.current();
        if (!t)           //Get the currently running task. 
                return;   //If no task is running, stop this cycle.

        const string& block = t->nextBlock();
        // Get memory block requested by current task 

        printCycleHeader(*t, block); 
        int latency = cache.access(block); //Access memory through cache hierarchy
        elapsedCycles += (1 + latency); //Adds 1 CPU cycle plus memory latency.

        printCacheState();
        t->stepMemIndex();//Move the task to its next memory request,
        sched.tick(cycle);//tell the scheduler that one cycle has passed.
    }
// --------------------------------------------------------------------------------

    //Prints cycle number, running task, and requested memory block
    void printCycleHeader(const Task& t, const string& block) const 
    {
        cout << "------------------------------------------------------\n";
        cout << "Cycle " << cycle
             << "  |  Running: " << t.id
             << "  |  Requesting: " << block << "\n";
        cout << "------------------------------------------------------\n";
    }
    //Prints all cache levels after memory access.
    void printCacheState() const {
        cout << "\n  Cache state after access:\n";
        cache.printState();
        cout << "\n";
    }
    //Prints final table with task stats.
    void printTaskTable() const {
        cout << "======================================================\n";
        cout << "                   Final Results                     \n";
        cout << "======================================================\n\n";
        cout
            << "Task     "
            << "Burst    "
            << "Start     "
            << "Done      "
            << "Turnaround  "
            << "\n";
        cout << string(48, '-') << "\n";

        for (const auto& t : tasks) {
            int turnaround = t.completed ? t.finishCycle : -1;
            //If the task completed, use finishCycle; otherwise use -1 as a placeholder.
            cout
                << t.id << "        "
                << t.burstTime << "        "
                << (t.startCycle >= 0 ? to_string(t.startCycle) : "---") << "         "
                << (t.completed ? to_string(t.finishCycle) : "---") << "         "
                << (t.completed ? to_string(turnaround) : "---") << "          "
                << "\n";
        }
        cout << "\n";
    }
    //Prints total CPU cycles, memory latency, total elapsed cycles,
    //  completed tasks, scheduler type, and RAM accesses.
    void printTotals() const {
        int completedCount = 0;
        for (const auto& t : tasks) {
            if (t.completed) completedCount++;
        }
        cout << "  CPU Time Steps         : " << cycle << "\n";
        cout << "  Total Memory Latency   : " << cache.totalLatency << " cycles\n";
        cout << "  Total Cycles           : " << elapsedCycles << "\n";
        cout << "  Tasks Completed        : " << completedCount << " / " << tasks.size() << "\n";
        cout << "  Scheduler              : Round Robin  (quantum = " << sched.quantum << ")\n";
        cout << "  RAM Accesses           : " << cache.ramAccesses << "\n";
        cout << "\n";
    }
    //used to print hit/miss stats for L1, L2, and L3
    void printCacheStats() const {
        cout<<"  Cache Hit/Miss Breakdown:\n";
        printCacheRow(cache.l1);
        printCacheRow(cache.l2);
        printCacheRow(cache.l3);
        cout << "\n";
    }
    
    //Prints one cache level’s statistics.
    void printCacheRow(const CacheLevel& lvl) const {
        int total = lvl.hits + lvl.misses;
        double rate = total>0?(100.0 * lvl.hits / total) : 0.0;
        //Calculates hit rate percentage

        cout << "    " << lvl.name
             << "  hits=" << lvl.hits
             << "  misses=" << lvl.misses
             << "  hit rate=" << fixed << setprecision(1) << rate << "%\n";
    }

    void printBanner() const {
        cout << "\n";
        cout << "======================================================\n";
        cout << " CPUSim - A Pipelined CPU & Memory Hierarchy Simulator \n";
        cout << "======================================================\n";
        cout << "  Scheduler  : Round Robin  (quantum = " << sched.quantum << ")\n";
        cout << "  Cache      : L1=" << cache.l1.capacity
                  << " | L2=" << cache.l2.capacity
                  << " | L3=" << cache.l3.capacity << "  (FIFO eviction)\n";
        cout << "  Tasks      : " << tasks.size() << " loaded\n";
        cout << "======================================================\n\n";
    } 
};

// --------------------------------------------------------------------------------
//  SECTION 6 — Input Parser
//  Reads a plain text file.  
// Each non-blank, non-comment line must follow this format:-
//(i have assumed that the comments, if any, in the input file begins with a #)
// TASK <id> BURST <n> MEM <block1> <block2>,...

vector<Task> parseInput(const string& filename) {
    ifstream file(filename); //Reads the input file and returns a vector of tasks
    if (!file.is_open())
        throw runtime_error("Cannot open input file: " + filename);

    vector<Task> tasks; //Creates task list and a variable to store each line
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  //skip blanks and comments
        istringstream ss(line);  // Allows the line to be read word by word
        string token, taskId;  //Temporary variables for one task
        int burst = 0;
        vector<string> mem;

        while (ss >> token) {
            if (token == "TASK")  { ss >> taskId; }
            else if (token == "BURST") { ss >> burst;  }
            else if (token == "MEM")   {
                string b;
                while (ss >> b) mem.push_back(b);//Stores memory block names
            }
        }//If line is valid, create a Task and add it to the task list
        if (!taskId.empty() && burst > 0 && !mem.empty())
            tasks.emplace_back(taskId, burst, mem);
    }
        //If no valid tasks were found, throw error.
    if (tasks.empty())
        throw runtime_error("No valid tasks found in " + filename);

    return tasks;
}

// --------------------------------------------------------------------------------
//  SECTION 7 — main()

int main() {
    string filename = "input.txt";
    int quantum = 3;
    vector<Task> tasks;

    //Tries to read tasks from the input file
    try {tasks = parseInput(filename);} 

    //Catches errors from parsing
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;}

    //Prints how many tasks were loaded.
    cout << "Loaded " << tasks.size() << " task(s) from \"" << filename << "\"\n";

    Simulator sim(tasks, quantum); //Creates simulator object
    sim.run();  //Runs simulation cycle by cycle
    sim.printSummary(); //Prints final results

    return 0;
}

// ---------------------AND WE ARE DONEEE!!!!😭😭🥳--------------------------------//