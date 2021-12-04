#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"

extern SmallShell& smash;
extern bool isPipe;
using namespace std;
extern bool isRedirect;
extern int redirectFile;
extern int stdoutFile;

void ctrlZHandler(int sig_num) {
	int result;
	std::cout << "smash: got ctrl-Z" << std::endl;
	if(smash.getCurrentJobCmd() != nullptr){
		smash.setTime();
		smash.addStoppedCommand();
		int processId = (smash.getCurrentJobCmd())->getPid();
		result = kill((-1)*processId, SIGSTOP);
		if(result == (-1)){
			perror("smash error: kill failed");	
		}	
		std::cout << "smash: process " << processId << " was stopped" << std::endl;
	}

	if(isRedirect){
	    result = close(1);
		if(result == (-1)){
			perror("smash error: close failed");
		}
		result = dup2(stdoutFile, 1);
		if(result == (-1)){
			perror("smash error: dup2 failed");
		}
		result = close(redirectFile);
		if(result == (-1)){
			perror("smash error: close failed");
		}
		isRedirect = false;
	}
}

void ctrlCHandler(int sig_num) {
	int result;
    std::cout << "smash: got ctrl-C" << std::endl;
    JobEntry* current = smash.getCurrentJobCmd();
    if(current == nullptr) return;
    int processId = current->getPid();
	result = kill((-1)*processId, SIGKILL);
	if(result == (-1)){
		perror("smash error: kill failed");	
	}
    std::cout << "smash: process " << processId << " was killed" << std::endl;
    	if(isRedirect){
	    result = close(1);
		if(result == (-1)){
			perror("smash error: close failed");
		}
		result = dup2(stdoutFile, 1);
		if(result == (-1)){
			perror("smash error: dup2 failed");
		}
		result = close(redirectFile);
		if(result == (-1)){
			perror("smash error: close failed");
		}
		isRedirect = false;
	}	
}

void alarmHandler(int sig_num) {
	int result;
    std::cout << "smash: got an alarm" << std::endl;
	if(smash.getCurrentTimeoutCmd() != nullptr){ 
		smash.addStoppedCommand();
	}    
    time_t timestamp = (smash.getTList())->getCurrentAlarm();	
    list<TimeoutEntry*> list =(smash.getTList())->getTimeouts();
    for (std::list<TimeoutEntry*>::reverse_iterator rit=list.rbegin(); rit != list.rend(); ++rit){
        if((*rit)->getTimestamp() == timestamp){
			int processId = (*rit)->getPid();
			kill((-1)*processId, SIGKILL);		
        }
    }    
	(smash.getTList())->removeTimoutByTimestamp(timestamp);
	(smash.getTList())->setAndUpdateAlarm();
	if(isRedirect){
	result = close(1);
	if(result == (-1)){
		perror("smash error: close failed");
	}
	result = dup2(stdoutFile, 1);
	if(result == (-1)){
		perror("smash error: dup2 failed");
	}
	result = close(redirectFile);
	if(result == (-1)){
		perror("smash error: close failed");
	}
	isRedirect = false;
	}	
}
