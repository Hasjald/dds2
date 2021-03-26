#include <iostream>
#include "interface.hh"
#include "stdio.h"
#include <list>
using namespace std;

#define MAX_ENTRIES 127
#define D_ENTRY 6
#define MAX_D 511
#define MIN_D 0
#define INFLIGHT 32

list<Addr> Candidates_1, C_Prefetches, InFlight_Buf;

struct Entry
{
    Entry(Addr pc) : pc(0), lastAddress(0), lastPrefetch(0) {} // constuctor
    Addr pc;
    Addr lastAddress;
    Addr lastPrefetch;
    list<Addr> deltaArray;
};

struct Table
{
    Table()  {} // Constructor
    list<Entry *> Dcptlist;
    Entry *findEntry(AccessStat stat); // entry obj
};

// Class Function Definition
Entry *Table::findEntry(AccessStat stat)
{
    list<Entry *>::iterator i = Dcptlist.begin();
    for (; i != Dcptlist.end(); ++i)
    {
        Entry *search_entry = *i;

        if (stat.pc == search_entry->pc)
            return search_entry; //returning the existing entry with the right pc
    }

    Entry *newentry = new Entry(stat.pc);
    newentry->pc = stat.pc;
    newentry->lastAddress = stat.mem_addr;
    newentry->lastPrefetch = 0;
    newentry->deltaArray.push_front(1);
    Dcptlist.push_front(newentry);
    
    if (Dcptlist.size() > MAX_ENTRIES)
        Dcptlist.pop_back();
    return newentry;
}

// Instantiate DCPT Table Object
static Table *table;

// Functions
//----------------------------------------------
void deltaCorrelation(Entry *entry)
{
    Candidates_1.clear();

    list<Addr>::iterator it_deltaArray = entry->deltaArray.end();
    for(int x:entry->deltaArray){
            cout<<x<<endl;
    }
    advance(it_deltaArray,-1);
    Addr delta1 = *it_deltaArray; //last delta
    advance(it_deltaArray,-1);
    Addr delta2 = *it_deltaArray; //second last delta
    Addr address = entry->lastAddress;
    
    it_deltaArray = entry->deltaArray.begin();
    while (it_deltaArray != entry->deltaArray.end())
    {
        Addr u = *it_deltaArray;
        it_deltaArray++;
        Addr v = *it_deltaArray;
        //cout <<"hahah"<<u<<endl;
        //cout<<"hah"<<v<<endl;
        /*Search pair by pair for  matach between the two last deltas and the pairs from the beginning of the buffer */
        if (u == delta2 && v == delta1)
        { //found a pattern from the previous deltas
            for (it_deltaArray++; it_deltaArray != entry->deltaArray.end(); ++it_deltaArray)
            {
                address += *it_deltaArray * BLOCK_SIZE; //add the rest of deltas to the address
                Candidates_1.push_back(address);
                cout<<"haha"<<endl;
            }
        }
    }
}

void prefetch_filter(Entry *entry)
{
    C_Prefetches.clear();

    list<Addr>::iterator it_candidates;

    for (it_candidates = Candidates_1.begin(); it_candidates != Candidates_1.end(); ++it_candidates)
    {

        /*Check if the prefetch candidate is already in cache, mshr or is InFlight_Buf(a buffer that holds other prefetch requests that have not been completed */
        /*if (std::find(InFlight_Buf.begin(), InFlight_Buf.end(), *it_candidates) == InFlight_Buf.end() && !in_mshr_queue(*it_candidates) && !in_cache(*it_candidates))
        {
            C_Prefetches.push_back(*it_candidates);

            if (InFlight_Buf.size() == INFLIGHT)
            {
                InFlight_Buf.pop_front();
            }

            InFlight_Buf.push_back(*it_candidates);
            entry->lastPrefetch = *it_candidates;
        }*/
    }
}

// Simulator Functions
//----------------------------------------------
void prefetch_init(void)
{
    table = new Table;
    //DPRINTF(HWPrefetch, "DCPT Prefetcher Initiallized.\n");
}

void prefetch_access(AccessStat stat)
{
    int64_t Delta;
    Entry *current_entry = table->findEntry(stat);
    
    
   
        Delta = (int64_t)stat.mem_addr - (int64_t)current_entry->lastAddress;
        cout<<Delta<<"haha12"<<endl;
        Delta /= BLOCK_SIZE >> 1;
        cout<<Delta<<"haha12"<<endl;
        if (Delta != 0)
        {
            if (Delta < MIN_D || Delta > MAX_D)
                Delta = MIN_D;

            if (current_entry->deltaArray.size() == D_ENTRY)
                current_entry->deltaArray.pop_front();

            current_entry->deltaArray.push_back((Addr)Delta);
            current_entry->lastAddress = stat.mem_addr;
        }

        deltaCorrelation(current_entry);
        
        list<Addr>::iterator iterator;
        for (iterator = C_Prefetches.begin(); iterator != C_Prefetches.end(); ++iterator)
        {
            //issue_prefetch(*iterator); //issue a prefetch for the addresses in prefetches list
        }
    
    cout<<stat.mem_addr<<endl;   
}

void prefetch_complete(Addr addr)
{
}

int main( ) {
    AccessStat stat;
    prefetch_init();

    //int pc[12] = {1,2,3,4,5,6,7,1,1,1};
    //int miss_addresses[12] = {1147,1245,1149,1250,1255, 1260,1270,1154,1156,1158,1163,1165};
    int pc[12] = {1,2,3,4,1,1,1,1,2,2,3,4};
    int miss_addresses[12]={1147,1245,1147,1245,1147,1245,1147,1245,1267,1234,1256,1389};

    for (int i = 0; i < 12; i++ ){
        stat.pc = pc[i];
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }
    prefetch_complete(stat.mem_addr);
    return 0;
}



