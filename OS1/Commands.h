#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <ctime>


using namespace std;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
    pid_t pid;
    time_t startTime;
    time_t endTime;    

public:

    string cmd_line;

    Command(const string cmd_line);
    Command(const string cmd_line, pid_t pid);
    Command(const string cmd_line, pid_t pid, time_t startTime, time_t endTime);
    virtual ~Command() = default;
    virtual void execute() = 0;
    
    friend class JobEntry;
    friend class TimeoutEntry;
    friend class TimeoutList;
    
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const string cmd_line);
    BuiltInCommand(const string cmd_line, pid_t pid);
    BuiltInCommand(const string cmd_line, pid_t pid, time_t startTime, time_t endTime);
    virtual ~BuiltInCommand() = default;
};

class SmallShell;
class ExternalCommand : public Command {
	SmallShell* smash;
public:
    ExternalCommand(const string cmd_line, SmallShell* smash);
    virtual ~ExternalCommand();
    void execute() override;
};

class PipeCommand : public Command {
    int my_pipe[2];	
    string command1;
    string command2;
	bool isWithBackground;
    bool background;
    int fatherPid;
    int child1Pid;
    int child2Pid;
public:
    PipeCommand(string cmd_line);
	void executeExternalPipeWithSign();   
	void executeExternalPipeWithoutSign();       
    virtual ~PipeCommand();
    void execute() override;
};

class RedirectionCommand : public Command {
    string command;
    string path;
    bool isAppend; 
    bool background;
    SmallShell* smash;
	bool isError = false;
    
public:
    explicit RedirectionCommand(const string cmd_line, SmallShell* smash);
    virtual ~RedirectionCommand();
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
	
public:
    ChangeDirCommand(const string cmd_line);
    ChangeDirCommand(const string cmd_line, string** plastPwd);
    virtual ~ChangeDirCommand() = default;
    void execute() override;
};

class lsCommand : public BuiltInCommand {
public:
    lsCommand() ;
    virtual ~lsCommand() = default;
    void execute() override ;
};

class GetCurrDirCommand : public BuiltInCommand {
    char* path;
    bool isError = false;
    
public:
    GetCurrDirCommand();
    string getPath();
    virtual ~GetCurrDirCommand();
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
    string cmd_line;
public:
    ShowPidCommand(string cmd_line);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};

class JobsList;
class TimeoutList;
class QuitCommand : public BuiltInCommand {
	JobsList* jobs;
	TimeoutList* timeouts;
	
  public:
    QuitCommand(const string cmd_line, JobsList* jobs, TimeoutList* timeouts);
    virtual ~QuitCommand();
    void execute() override;
};

class JobsCommand : public BuiltInCommand {
	JobsList* jobs;
    
public:
    JobsCommand(JobsList* jobs, const string cmd_line);
    virtual ~JobsCommand();
    void execute() override;   
};

class SmallShell;
class JobEntry : public Command {
	bool isStopped;
	JobsList* jobs;
	int id;	
	time_t extraTime = 0;
		
public:
    JobEntry(JobsList* jobs, const string cmd_line, bool isStopped, int id = (-1));
	JobEntry(JobsList* jobs, const string cmd_line, pid_t pid, bool isStopped, int id = (-1));
	JobEntry(JobsList* jobs, const string cmd_line, pid_t pid, time_t startTime, time_t endTime, bool isStopped, int id = (-1));
    void execute() override {};
	virtual ~JobEntry();		
		    
	pid_t getPid();
	int getId();
    void setId(int id);
	JobsList* getJobList();
	time_t getStartTime();
	time_t getEndTime();
	string getCmdLine();
	void setEndTime();
	bool getIsStopped();
	void setStartTime(time_t startTime);
	time_t getExtraTime();
	void setExtraTime(time_t extraTime);
	void setIsStopped(bool isStopped);
};

class JobsList {
    list<JobEntry*> jCommands;
    SmallShell* smash;

public:
    JobsList(SmallShell* smash);
    ~JobsList();
    void addJob(JobEntry* cmd);
    void printJobsList();
    void printShorterJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);   
    void removeJobById(int jobId);
    bool removeJobByPid(pid_t pid);   
    JobEntry * getLastJob(int* lastJobId);   
    JobEntry *getLastStoppedJob(int *jobId); 
	void removeJob(JobEntry* cmd);
    int getSize();
    list<JobEntry*> getJobList();
	void clearJobList();
	void killStoppedJobs();    
};

class KillCommand : public BuiltInCommand {
    int jobId;
    int signal;
    JobsList* jobs;
    bool isError = false;
  
public:
    KillCommand(string cmdLine, JobsList *jobs);
    virtual ~KillCommand();
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    int id;
    JobsList* jobs;
    bool isError = false;
public:
    ForegroundCommand(const string cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand();
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    int jobId = -1;
    JobsList* jobs;
    bool isAskingForLast = false;
    bool isError = false;
public:
    BackgroundCommand(string cmdLine, JobsList *jobs);
    virtual ~BackgroundCommand();
    void execute() override;
};

class CopyCommand : public Command {            
	SmallShell* smash;	
	string srcFile;
	string dstFile;
	bool isBackground = false;
	bool filesAreSame = false;
    bool isError = false;	
public:
    CopyCommand(const string cmd_line, SmallShell* smash);
    virtual ~CopyCommand();
    void execute() override;
};

class TimeoutList;
class TimeoutEntry : public Command {           
	time_t duration;
	time_t timestamp;
	bool isStopped = false;	
public:
    TimeoutEntry(const string cmd_line, time_t duration, pid_t pid, bool isStopped);
    void execute() override {};
	virtual ~TimeoutEntry();		
		    
	pid_t getPid();
	string getCmdLine();
	time_t getDuration();
	time_t getTimestamp();
    bool getIsStopped();	
	void setTimestamp(time_t timestamp);		
};

class TimeoutList {                
    list<TimeoutEntry*> timeouts;
    SmallShell* smash;
    time_t currentAlarm = 0;
    time_t startAlarm = 0;
    time_t pastedAlarm = 0;
    JobsList* jobs;
    
public:
    TimeoutList(SmallShell* smash, JobsList* jobs);
    ~TimeoutList();
    void addTimeout(TimeoutEntry* cmd);
    void removeTimoutByTimestamp(time_t duration);
    list<TimeoutEntry*> getTimeouts();
    void setCurrentAlarm(time_t newAlarm);
    time_t getCurrentAlarm();    
	void updateDuration();
	void setAndUpdateAlarm();
	void clearTimeoutList();	
};

class TimeoutCommand : public Command {            
	SmallShell* smash;
	TimeoutList* timeouts;
	JobsList* jobs;
public:
    TimeoutCommand(const string cmd_line, SmallShell* smash, TimeoutList* timeouts, JobsList* jobs);
	void executeExternalTimeout(string cmd, string full_cmd, time_t duration);   
    virtual ~TimeoutCommand();
    void execute() override;
};

class SmallShell {
private:
	pid_t smashPid; 
    string currentPath = string(" ");
    string previousPath = string(" ");
    string prompt = string("smash");
    JobEntry* currentJobCmd = nullptr;
    TimeoutEntry* currentTimeoutCmd = nullptr;    
    JobsList* jList;     
    TimeoutList* tList;     

    SmallShell();
public:
    Command *CreateCommand(const string cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator

    string getPrompt();
    void setPrompt(const string newPrompt);
    void addStoppedCommand();
    void removeStoppedCommand();
    void addUnfinishedCommand();
    void removeUnfinishedCommand();
    JobEntry* getCurrentJobCmd();
    void setCurrentJobCmd(JobEntry* command);
    JobsList* getJList();  
    TimeoutList* getTList();      
    void setTime();   

    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    
    void executeCommand(const string cmd_line);
     
    string getCurrentPath();
    string getPreviousPath();
    void updateCurrentPath(string path);
    void updatePreviousPath(string path);
	pid_t getSmashPid();
    void setCurrentTimeoutCmd(TimeoutEntry* command);
    TimeoutEntry* getCurrentTimeoutCmd();    
};

class ChPromptCommand : public BuiltInCommand {
    string prompt = string("smash");
    SmallShell* smallShell;
    
public:
    ChPromptCommand(SmallShell* smallShell);
    ChPromptCommand(const string prompt, SmallShell* smallShell);
    void execute() override;
    ~ChPromptCommand();

};

#endif //SMASH_COMMAND_H_

