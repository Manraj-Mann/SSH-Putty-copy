#include <iostream>
#include <fstream>
#include <thread>
#include <conio.h>
// Include different headers based on the operating system
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h> // Windows-specific headers for system calls
#else
#include <unistd.h> // POSIX-specific headers for Unix/Linux
#endif
#include "synQueue.h"

bool started = false;

void hit()
{

    while (!started)
    {

        _kbhit();
    }
}
int startListening(syncQueue * bufferContainer)
{

    FILE *pipe;
    std::thread hitter(hit);
    
    try
    {
    START:
#if defined(_WIN32) || defined(_WIN64)
        const char *command = "plink -batch rt2.olsendata.com -P 22121 -l damadaro -pw clearsky2715";
        pipe = _popen(command, "r");
#else
        const char *command = "sshpass -p 'clearsky2715' ssh -o HostKeyAlgorithms=+ssh-rsa -p 22121 damadaro@rt2.olsendata.com";
        pipe = popen(command, "r");
#endif

        if (!pipe)
        {
            std::cerr << "Error executing command" << std::endl;
            goto START;
        }

#if defined(_WIN32) || defined(_WIN64)
        if (pipe)
        {

            fprintf(pipe, "\n");
        }
#endif

        char buffer[128];
        while (pipe && !feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != nullptr)
            {
                if (strlen(buffer) != 0)
                {
                    // std::cout << "Output: " << buffer;
                    bufferContainer->push(buffer);
                    
                    started = 1;
                }
            }
        }

        std::cout << "Ended ...\n";
        started = 1;
        
    }
    catch (const std::exception &e)
    {
    }

    hitter.join();

    #if defined(_WIN32) || defined(_WIN64)
    _pclose(pipe);
    #else
    pclose(pipe);
    #endif
    std::cout << "closed threads\n";

    return 0;
}
