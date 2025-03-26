#ifndef UTILITY_H        
#define UTILITY_H   
#include <string>
#include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef TRACE_ON
    #define TRACE(format, ...) printf("TRACE: %s:%d: " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
    #define TRACE(format, ...) /* do nothing */
#endif

void printTaskStackSizes();
void printHeapInfo();
void printSystemMemoryInfo();
std::string toLower(std::string str);
void runTimeStats();
void printActivePCBs();
void logSystemStats();
void printTCPState();
void printMemsize();
int is_in_interrupt(void);

#endif // UTILITY_H
