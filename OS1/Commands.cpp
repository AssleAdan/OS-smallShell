#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <time.h>
#include <chrono>
#include "Commands.h"
#include <fcntl.h>
#include <algorithm>
#include <limits.h>
#include <dirent.h>

using namespace std;

extern SmallShell& smash;
pid_t parentPID = getpid();
bool isPipe = false;
bool isRedirect = false;
int redirectFile = -10;
int stdoutFile = -10;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const string& s){
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : s.substr(start);
}

string _rtrim(const string& s){
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const string& s){
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    istringstream iss(_trim(string(cmd_line)).c_str());
    for(string s; iss >> s; ) {
		
		try{
        args[i] = (char*)malloc(s.length()+1);
        } catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return (-10);
		}
		
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

bool isChPromptLegal(string cmd_line, char** prompt, bool* isStarting){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}    
    
    if(strcmp(args[0], "chprompt") == 0){
		*isStarting = true;
	}
    
    if(num > 1){
		try{
			*prompt = new char[(strlen(args[1]))+1];
		}catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(*prompt, args[1]);
	}
	
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}
	
    return true;
}

bool isPwdLegal(string cmd_line){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}
    
    if(strcmp(args[0], "pwd") == 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}
		return true;
	}	

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}		
    return false;
}

bool isShowpidLegal(string cmd_line){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}   
	
    if(strcmp(args[0], "showpid") == 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}
		return true;
	}	

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}		
    return false;
}

bool isQuitLegal(string cmd_line){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}   
	    
    if(strcmp(args[0], "quit") == 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}
		return true;
	}
	
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}		
    return false;
}

bool isJobsLegal(string cmd_line){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}   
	    
    if(strcmp(args[0], "jobs") == 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}
		return true;
	}	

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}		
    return false;
}

bool isCdLegal(string cmd_line, bool* isNotStarting){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}       
    
    if(strcmp(args[0], "cd") != 0){
		*isNotStarting = true;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
		return false;		
	}  

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}        
	  
    if(num > 2){
        std::cout << "smash error: cd: too many arguments" << endl;
        return false;
    }
    if(num == 1){
		return false;
	}
    return true;
}

bool is_positive_number(string s){
	return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
				return !std::isdigit(c); }) == s.end();
}

bool is_number(string s){   
	if(s.length() > 1){
		string sign = s.substr(0, 1); 
		string number = s.substr(1, (s.length())-1); 
		bool isMinus = (strcmp(sign.c_str(), "-") == 0);
		bool isNumber = is_positive_number(number);

		if(isMinus && isNumber){
			return true;
		}
	}
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                return !std::isdigit(c); }) == s.end();
}

bool isKillLegal(string cmd_line, bool* isNotStarting){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}   
	    
    if(strcmp("kill", args[0]) != 0){
		*isNotStarting = true;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        		
		return false;
	}
    
    char* argument1 = args[1];
    char* argument2 = args[2];
    if(num != 3 || argument1[0] != '-' ){
        std::cout << "smash error: kill: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }
    string arg1 = string(argument1+1);
    string arg2 = string(argument2);
    
    if(!is_number(arg1) || !is_number(arg2)){
        cout << "smash error: kill: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }
	
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}    
    return true;
}

bool isFgcommandLegal(string cmd_line, bool* isNotStarting){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}       
    
    char* argument1 = args[1];
   
    if(strcmp("fg", args[0]) != 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		*isNotStarting = true;  
        return false;
    }
    
    if(num == 1){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}    
		
		if(smash.getJList()->getSize() == 0){
			cout << "smash error: fg: jobs list is empty" << std::endl;
			return false;
		} else{
			return true;
		}
	}
	
    string arg1 = string(argument1);
    if((num == 2) && (!is_number(arg1))){
        cout << "smash error: fg: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }

    if(num > 2){
        cout << "smash error: fg: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}    
		return false;
	}
    return true;
}

bool isBgcommandLegal(string cmd_line, bool* isNotStarting){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return false;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(), args);
    if(num == -10){
		return false;
	}       
    
    if(strcmp("bg", args[0]) != 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		*isNotStarting = true;  
        return false;
    }
    
    if((num == 2) && (!is_number(string(args[1])))){
        cout << "smash error: bg: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }
    
    if(num > 2){
        cout << "smash error: bg: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}    
		return false;
	}    
    
    return true;
}

bool isCopyLegal(string cmd, bool* isNotStarting){
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd.c_str(),args);
    if(num == -10){
		return false;
	}   
	    
    if(strcmp("cp", args[0]) != 0){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		*isNotStarting = true;  
        return false;
    }
    
    if(num < 3){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}    
		return false;
	}    
    
    return true;
}

bool isRedirectionLegal(string cmd_line, bool* isStarting){
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		return false;
	}   
	    
    if((strcmp(">", args[0]) == 0) || (strcmp(">>", args[0]) == 0)){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		*isStarting = true;  
        return false;
    }    
           
    int count = 0;
    for(unsigned int i = 0; i < cmd_line.length(); i++){
		if(cmd_line[i] == '>')
			count++;
	}
    
    if(count > 2){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		return false;
    }
   
    return true;
}

bool isPipeLegal(string cmd_line, bool* isStarting){
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		return false;
	}       
    
    if((strcmp("|", args[0]) == 0) || (strcmp("|&", args[0]) == 0)){ 
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		*isStarting = true;  
        return false;
    }    
           
    int count = 0;
    for(unsigned int i = 0; i < cmd_line.length(); i++){
		if(cmd_line[i] == '|')
			count++;
	}
    
    if(count > 1){
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}      
		return false;
    }
   
    return true;
}

bool isTimeoutLegal(string cmd_line, bool* isNotStarting){

    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		return false;
	}   
	    
    if(strcmp("timeout", args[0]) != 0){
		*isNotStarting = true;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        		
		return false;
	}
    
    if(num < 3){
		cout << "smash error: timeout: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }
    
    string arg1 = string(args[1]);
    
    if(!is_positive_number(arg1)){
		cout << "smash error: timeout: invalid arguments" << std::endl;
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}        
        return false;
    }
	
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}    
    return true;
}

bool isCommandBuiltIn(string cmd_line){
    char* args[COMMAND_ARGS_MAX_LENGTH];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		return false;
	}   
	    
    string command = string(args[0]);
    char* command1;
	try{
		command1 = new char[(command.length())+1];
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}       
		return false;
	}
    strcpy(command1, command.c_str());
    _removeBackgroundSign(command1);
    command = string(command1);
    delete(command1);

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}           
   
    if (cmd_line.find("chprompt") != string::npos && command == string("chprompt"))
        return true;
 
    if (cmd_line.find("pwd") != string::npos && command == string("pwd"))
        return true;
///////////////////////////////////////////////////////////////////////////////
   /* if (cmd_line.find("ls") != string::npos && command == string("ls"))
        return true;*/

    if (cmd_line.find("jobs") != string::npos && command == string("jobs") )
        return true;
 
    if (cmd_line.find("quit") != string::npos && command == string("quit"))
        return true;

    if (cmd_line.find("showpid") != string::npos && command == string("showpid"))
        return true;
 
    if (cmd_line.find("cd") != string::npos && command == string("cd"))
        return true;
 
    if (cmd_line.find("kill") != string::npos && command == string("kill"))
        return true;
 
    if (cmd_line.find("bg") != string::npos && command == string("bg"))
        return true;

    if (cmd_line.find("fg") != string::npos && command == string("fg"))
        return true;
           
    return cmd_line.find("cp") != string::npos && command == string("cp");
}


// ------------------------------------------------SmallShell------------------------------------------------------

Command* SmallShell::CreateCommand(const string cmd_line) {   
  try{
/////////////////////////////////////////////////////////////////////////////////////
    if(cmd_line.find("ls") !=string::npos) {
        char *args[COMMAND_MAX_ARGS];
        int num = _parseCommandLine(cmd_line.c_str(), args);
        if(num !=0 && args[0] == "ls") {
            for (int i = 0; i < num; i++) {
                delete (args[i]);
            }
            return new lsCommand();

        }
        for (int i = 0; i < num; i++) {
            delete (args[i]);
        }
    }


	if (cmd_line.find("timeout") != string::npos) {  
		bool isNotStarting = false;
		if (isTimeoutLegal(cmd_line, &isNotStarting)) {
			if(!isNotStarting){
				return new TimeoutCommand(cmd_line, this, this->getTList(), this->getJList());
			}
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}
    }

	if ((cmd_line.find(">") != string::npos) || (cmd_line.find(">>") != string::npos)) {  
		bool isStarting = false;
		if (isRedirectionLegal(cmd_line, &isStarting)) {
			if(!isStarting){
				return new RedirectionCommand(cmd_line, this);
			}
        } else{
			if(!isStarting){
				return nullptr;
			}
		}
    }

	if (cmd_line.find("|") != string::npos) { 
		bool isStarting = false;
		if (isPipeLegal(cmd_line, &isStarting)) {
			if(!isStarting){
				return new PipeCommand(cmd_line);
			}
        } else{
			if(!isStarting){
				return nullptr;
			}
		}
    }

    if (cmd_line.find("chprompt") != string::npos) {
        char* prompt = nullptr;
        bool isStarting = false;
        if(isChPromptLegal(cmd_line, &prompt, &isStarting)){
			if(isStarting){
				if(prompt == nullptr){
					return new ChPromptCommand(this);
				} else {
					string newPrompt = string(prompt);
					delete(prompt);
					return new ChPromptCommand(newPrompt, this);
				}
			} 
		} else{
			if(isStarting){
				return nullptr;
			}
		}
    }

    if (cmd_line.find("pwd") != string::npos) {
		if(isPwdLegal(cmd_line)){
			return new GetCurrDirCommand();
		}
	}

	if (cmd_line.find("jobs") != string::npos) {
		if(isJobsLegal(cmd_line)){
			return new JobsCommand(this->jList, cmd_line);
		}
    }

	if (cmd_line.find("quit") != string::npos) {
		if(isQuitLegal(cmd_line)){
			return new QuitCommand(cmd_line, this->getJList(), this->getTList());
		} 
	}
	
	if (cmd_line.find("showpid") != string::npos) {
		if(isShowpidLegal(cmd_line)){
			return new ShowPidCommand(cmd_line);
		}
    } 
        
    if (cmd_line.find("cd") != string::npos) {  
		bool isNotStarting = false;
        if (isCdLegal(cmd_line, &isNotStarting)) {
			if(isNotStarting == false)
				return new ChangeDirCommand(cmd_line);	
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}
    }
    
    if (cmd_line.find("kill") != string::npos) {  
		bool isNotStarting = false;
        if (isKillLegal(cmd_line, &isNotStarting)) {
			if(!isNotStarting)
				return new KillCommand(cmd_line, this->getJList());
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}
    }
   
    if (cmd_line.find("bg") != string::npos) {  
		bool isNotStarting = false;
		if (isBgcommandLegal(cmd_line, &isNotStarting)) {
			if(!isNotStarting){
				return new BackgroundCommand(cmd_line, this->getJList());
			}
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}        
    }
	 
	if (cmd_line.find("fg") != string::npos) {  
		bool isNotStarting = false;
		if (isFgcommandLegal(cmd_line, &isNotStarting)) {
			if(!isNotStarting){
				return new ForegroundCommand(cmd_line, this->getJList());
			}
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}
    }

	if (cmd_line.find("cp") != string::npos) {  
		bool isNotStarting = false;
		if (isCopyLegal(cmd_line, &isNotStarting)) {
			if(!isNotStarting){
				return new CopyCommand(cmd_line, this);
			}
        } else{
			if(!isNotStarting){
				return nullptr;
			}
		}
    }

    return new ExternalCommand(cmd_line, this);
    
  } catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		return nullptr;
  }
}

void SmallShell::executeCommand(const std::string cmd_line) {
    Command* cmd = CreateCommand(cmd_line);

    if(cmd != nullptr) {
        cmd->execute();
    }
    
    this->currentJobCmd = nullptr;
    delete(cmd);
}

std::string SmallShell::getPrompt() {
    return prompt;
}

void SmallShell::setPrompt(const std::string newPrompt) {
    this->prompt = string(newPrompt);
}

void SmallShell::addStoppedCommand(){
	this->jList->addJob(this->currentJobCmd);
}

void::SmallShell::addUnfinishedCommand(){
	this->jList->addJob(this->currentJobCmd);
}

void SmallShell::setCurrentJobCmd(JobEntry* command){
	this->currentJobCmd = command;
}

JobsList* SmallShell::getJList(){
	return (this->jList);
}

TimeoutList* SmallShell::getTList(){
	return (this->tList);
}

SmallShell::SmallShell(){
	try{
		this->jList = new JobsList(this);   
		this->tList = new TimeoutList(this, this->getJList()); 
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		exit(7);
	}
    this->smashPid = getpid();      
}

SmallShell::~SmallShell(){
	delete(this->jList);
	delete(this->currentJobCmd);
}

void SmallShell::removeUnfinishedCommand(){
	(this->jList)->removeJob(this->currentJobCmd);
}

void SmallShell::removeStoppedCommand(){
	(this->jList)->removeJob(this->currentJobCmd);
}

void SmallShell::setTime(){
	if((this->currentJobCmd) == nullptr){
		return;
	}
	(this->currentJobCmd)->setEndTime();
}

JobEntry* SmallShell::getCurrentJobCmd(){
	return (this->currentJobCmd);
}

string SmallShell::getPreviousPath() {
    return previousPath;
}

string SmallShell::getCurrentPath(){
    return currentPath;
}

void SmallShell::updatePreviousPath(string path) {
    previousPath = string(path);
}

void SmallShell::updateCurrentPath(string path) {
    currentPath = string(path);
}

pid_t SmallShell::getSmashPid() {
    return smashPid;
}

void SmallShell::setCurrentTimeoutCmd(TimeoutEntry* command){
	this->currentTimeoutCmd = command;
}

TimeoutEntry* SmallShell::getCurrentTimeoutCmd(){
	return (this->currentTimeoutCmd);
}


// ------------------------------------------------External Command------------------------------------------------------

ExternalCommand::ExternalCommand(const string cmd_line, SmallShell* smash): Command(cmd_line){
	this->smash = smash;
}

void ExternalCommand::execute() {

    string cmd_line_with_no_sign;

    if (_isBackgroundComamnd(cmd_line.c_str())) {
        char *command;
        try {
            command = new char[(cmd_line.length()) + 1];
        } catch (std::bad_alloc &exc) {
            perror("smash error: allocation failed");
            return;
        }
        strcpy(command, cmd_line.c_str());
        _removeBackgroundSign(command);
        cmd_line_with_no_sign = string(command);
        delete (command);
    } else {
        cmd_line_with_no_sign = this->cmd_line;
    }

    char **argv;
    try {
        argv = new char *[4];
        argv[0] = new char[10];
        strcpy(argv[0], "/bin/bash");
        argv[1] = new char[3];
        strcpy(argv[1], "-c");
        argv[2] = new char[cmd_line_with_no_sign.length() + 1];
        strcpy(argv[2], cmd_line_with_no_sign.c_str());
        argv[3] = nullptr;
    } catch (std::bad_alloc &exc) {
        perror("smash error: allocation failed");
        return;
    }

    int child_pid = fork();
    if (child_pid == (-1)) {
        perror("smash error: fork failed");
        return;
    }

    if (child_pid == 0) { // child process

        setpgrp();
        execv(argv[0], argv);
        perror("smash error: execv failed");
        exit(7);

    } else { // parent process

        JobEntry *jCmd;

        try {
            jCmd = new JobEntry(((this->smash)->getJList()), this->cmd_line, child_pid, false);
        } catch (std::bad_alloc &exc) {
            perror("smash error: allocation failed");
            for (unsigned int j = 0; j < 4; j++) {
                if (argv[j] != nullptr)
                    delete (argv[j]);
            }
            delete[](argv);
            return;
        }

        if (_isBackgroundComamnd(cmd_line.c_str())) {
            (this->smash)->setCurrentJobCmd(jCmd);
            (this->smash)->addUnfinishedCommand();
            (this->smash)->setCurrentJobCmd(nullptr);
            delete (jCmd);
        } else {
            jCmd->setIsStopped(true);
            (this->smash)->setCurrentJobCmd(jCmd);

            int status;
            waitpid(child_pid, &status, WUNTRACED);

            (this->smash)->setCurrentJobCmd(nullptr);
            delete (jCmd);
        }
    }


    for (unsigned int j = 0; j < 4; j++) {
        if (argv[j] != nullptr) {
            delete [] argv[j];
        }

    }
    delete[](argv);
}

ExternalCommand::~ExternalCommand(){
}


// ------------------------------------------------ls Command------------------------------------------------------

void lsCommand::execute() {
    struct dirent **namelist;
    int n;
    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0) return;
    else {
        for(int i = 0; i <n ; i++){
            std::cout << namelist[i]->d_name << std::endl;
            delete namelist[i];
        }
        delete namelist;
    }
}

lsCommand::lsCommand() : BuiltInCommand("ls"){

}

// ------------------------------------------------ChPrompt Command------------------------------------------------------

ChPromptCommand::ChPromptCommand(const string prompt, SmallShell* smallShell): BuiltInCommand(string("chprompt")){
    this->smallShell = smallShell;
    this->prompt = string(prompt);
}

ChPromptCommand::ChPromptCommand(SmallShell* smallShell): BuiltInCommand(string("chprompt")){
    this->smallShell = smallShell;
}

void ChPromptCommand::execute() {
    this->smallShell->setPrompt(this->prompt);
}

ChPromptCommand::~ChPromptCommand(){
}


// ------------------------------------------------Get Curr Dir Command------------------------------------------------------

GetCurrDirCommand::GetCurrDirCommand(): BuiltInCommand(string("pwd")){
	int size = 100;

	for ((this->path) = nullptr; (this->path) == nullptr; size *= 2){
		
		this->path = (char*)realloc((this->path), size);
		if ((this->path) == nullptr)
		{
			perror("smash error: allocation failed");
			isError = true;
		}

		getcwd((this->path), size);

		if ((this->path) == nullptr && errno != ERANGE)
		{
			perror("smash error: getcwd failed");
			isError = true;
		}
	}
}

string GetCurrDirCommand::getPath(){
	return string(path);
}

GetCurrDirCommand::~GetCurrDirCommand(){
	delete(this->path);
}

void GetCurrDirCommand::execute() {
	if(isError){
		return;
	}
    for(unsigned int i = 0; i < strlen(this->path); i++){
        cout << this->path[i];
    }
    cout << endl;
}


// ------------------------------------------------Jobs List------------------------------------------------------

void JobsList::printJobsList(){	
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if((*it)->getIsStopped() == false){
			(*it)->setEndTime();
		}
		time_t time = ((*it)->getEndTime()) - ((*it)->getStartTime()) + ((*it)->getExtraTime());
		std::cout << "[" << (*it)->getId() << "] " << (*it)->getCmdLine() << " : " << (*it)->getPid() << " " << time << " secs ";
		if((*it)->getIsStopped()){
			std::cout << "(stopped)";
		}
		std::cout << endl;
	} 
}

void JobsList::printShorterJobsList(){
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		std::cout << (*it)->getPid() << ": " << (*it)->getCmdLine() << endl;
	} 
}

void JobsList::killAllJobs(){
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		int result = kill((*it)->getPid(), SIGKILL);
		if(result == (-1)){
			perror("smash error: kill failed");
		}
	}
}

void JobsList::killStoppedJobs(){
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if((*it)->getIsStopped() == true){
			int result = kill((*it)->getPid(), SIGKILL);
			if(result == (-1)){
				perror("smash error: kill failed");
			}
		}
	}
}

void JobsList::clearJobList(){
	for (std::list<JobEntry*>::reverse_iterator rit=this->jCommands.rbegin(); rit != this->jCommands.rend(); ++rit){
		this->removeJobByPid((*rit)->getPid());
	}
}

int JobsList::getSize(){
	return((this->jCommands).size());
}

void JobsList::addJob(JobEntry* cmd){	
	if(cmd == nullptr){
		return;
	}

	JobEntry* newJobEntry;

	try{
		newJobEntry = new JobEntry(cmd->getJobList(), cmd->getCmdLine(), cmd->getPid(), cmd->getStartTime(), cmd->getEndTime(), cmd->getIsStopped(), cmd->getId());
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		return;
	}
	
	if((getSize() == 0) || ((jCommands.back())->getId()) < (cmd->getId())){
		(this->jCommands).push_back(newJobEntry);
		return;
	}
	
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if(((*it)->getId())+1 == cmd->getId()){
			it++;
			(this->jCommands).insert(it, newJobEntry);
			break;
		}
		if(((*it)->getId()) == (cmd->getId())+1){
			(this->jCommands).insert(it, newJobEntry);
			break;
		}	
	} 		
}

JobsList::JobsList(SmallShell* smash){
	this->smash = smash;
}

JobsList::~JobsList(){
}

void JobsList::removeJob(JobEntry* cmd){
	if(cmd == nullptr){
		return;
	}
	
	int jobId = cmd->getId();
		
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if((*it)->getId() == jobId){
			JobEntry* jCmd = *it;
			(this->jCommands).remove(*it);
			delete(jCmd);
			break;
		}
	}
}

void JobsList::removeJobById(int jobId){		
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if((*it)->getId() == jobId){
			JobEntry* jCmd = *it;
			(this->jCommands).remove(*it);
			delete(jCmd);
			break;
		}
	}
}

bool JobsList::removeJobByPid(pid_t pid){
	int count = 1;
		
	for (std::list<JobEntry*>::iterator it=this->jCommands.begin(); it != this->jCommands.end(); ++it){
		if((*it)->getPid() == pid){
			JobEntry* jCmd = *it;
			(this->jCommands).remove(*it);
			delete(jCmd);
			count = -1;
			break;
		}
		count++;
	}
	
	if(count != -1){
		return false;
	}
	return true;
}

JobEntry* JobsList::getJobById(int jobId) {
    for (std::list<JobEntry*>::iterator it=jCommands.begin(); it != jCommands.end(); ++it){
        if((*it)->getId() == jobId){
            return *it;
        }
    }
    return nullptr;
}

JobEntry* JobsList::getLastStoppedJob(int *jobId){   	
	for (std::list<JobEntry*>::reverse_iterator rit = (this->jCommands).rbegin(); rit != (this->jCommands).rend(); ++rit){
		if((*rit)->getIsStopped() == true){
			*jobId = (*rit)->getId();
			return(*rit);
		}
	}
	return nullptr; 
}

JobEntry* JobsList::getLastJob(int* lastJobId){
	*lastJobId = (this->getSize());
	if(*lastJobId == 0){
		return nullptr;
	}
	return((this->jCommands).back());
}  

void JobsList::removeFinishedJobs(){	
	std::list<JobEntry*>::reverse_iterator rit=this->jCommands.rbegin();
	
	while(rit != this->jCommands.rend()){	
		int pid = (*rit)->getPid();
		int result = waitpid(pid, nullptr, WNOHANG);	
		if(result == pid){     
			this->removeJobById((*rit)->getId());
		} else{
			++rit;
		}
	}
}

list<JobEntry*> JobsList::getJobList(){
	return(this->jCommands);
}


// ------------------------------------------------Command------------------------------------------------------

Command::Command(const string cmd_line){
	this->cmd_line = cmd_line;
	this->pid = getpid();
	this->startTime = time(nullptr);
	this->endTime = startTime;
}

Command::Command(const string cmd_line, pid_t pid){
	this->cmd_line = cmd_line;
	this->pid = pid;
	this->startTime = time(nullptr);
	this->endTime = startTime;
}

Command::Command(const string cmd_line, pid_t pid, time_t startTime, time_t endTime){
	this->cmd_line = cmd_line;
	this->pid = pid;
	this->startTime = startTime;
	this->endTime = endTime;
}


// ------------------------------------------------Built In Command------------------------------------------------------

BuiltInCommand::BuiltInCommand(const string cmd_line): Command(cmd_line){
}

BuiltInCommand::BuiltInCommand(const string cmd_line, pid_t pid): Command(cmd_line, pid){
}

BuiltInCommand::BuiltInCommand(const string cmd_line, pid_t pid, time_t startTime, time_t endTime): Command(cmd_line, pid, startTime, endTime){
}


// ------------------------------------------------Job Entry------------------------------------------------------

JobEntry::JobEntry(JobsList* jobs, const string cmd_line, bool isStopped, int id) : Command(cmd_line){
	this->jobs = jobs;
	this->isStopped = isStopped;
	this->id = id;
	if((this->id) == (-1)){
		if(jobs->getSize() == 0){
			this->id = 1;
			return;
		}
		list<JobEntry*> list = (this->jobs)->getJobList();
		this->id = ((list.back())->getId()) + 1;
	} else{
		this->id = id;
	}
}

JobEntry::JobEntry(JobsList* jobs, const string cmd_line, pid_t pid, time_t startTime, time_t endTime, bool isStopped, int id) : Command(cmd_line, pid, startTime, endTime){
	this->jobs = jobs;
	this->isStopped = isStopped;
	this->id = id;
	if((this->id) == (-1)){
		if(jobs->getSize() == 0){
			this->id = 1;
			return;
		}
		list<JobEntry*> list = (this->jobs)->getJobList();
		this->id = ((list.back())->getId()) + 1;
	} else{
		this->id = id;
	}
}

JobEntry::JobEntry(JobsList* jobs, const string cmd_line, pid_t pid, bool isStopped, int id) : Command(cmd_line, pid){
	this->jobs = jobs;
	this->isStopped = isStopped;
	this->id = id;
	if((this->id) == (-1)){
		if(jobs->getSize() == 0){
			this->id = 1;
			return;
		}
		list<JobEntry*> list = (this->jobs)->getJobList();
		this->id = ((list.back())->getId()) + 1;
	} else{
		this->id = id;
	}
}

JobEntry::~JobEntry(){
}

pid_t JobEntry::getPid(){
	return (this->pid);
}

void JobEntry::setEndTime(){
	this->endTime = time(nullptr);
}

int JobEntry::getId(){
	return (this->id);
}

void JobEntry::setId(int id){
	this->id = id;
}
	
JobsList* JobEntry::getJobList(){
	return (this->jobs);
}

time_t JobEntry::getStartTime(){
	return (this->startTime);
}

time_t JobEntry::getEndTime(){
	return (this->endTime);
}

string JobEntry::getCmdLine(){
	return (this->cmd_line);
}

bool JobEntry::getIsStopped(){
	return (this->isStopped);
}

void JobEntry::setIsStopped(bool isStopped){
	this->isStopped = isStopped;
}

void JobEntry::setStartTime(time_t startTime){
	this->startTime = startTime;
}

void JobEntry::setExtraTime(time_t extraTime){
	this->extraTime += extraTime;
}

time_t JobEntry::getExtraTime(){
	return(this->extraTime);
}


// ------------------------------------------------Jobs Command------------------------------------------------------

JobsCommand::JobsCommand(JobsList* jobs, const string cmd_line) : BuiltInCommand(cmd_line){
	this->jobs = jobs;
}

JobsCommand::~JobsCommand(){
}

void JobsCommand::execute(){
	(this->jobs)->removeFinishedJobs();
	(this->jobs)->printJobsList();
}


// ------------------------------------------------Quit Command------------------------------------------------------

void QuitCommand::execute(){
	
	(this->jobs)->removeFinishedJobs();
	
	if(cmd_line.find("kill") != string::npos){
		
		char* args[COMMAND_MAX_ARGS];
		int num = _parseCommandLine(cmd_line.c_str(), args);
    
		for(int i = 0; i < num; i++){
			if(strcmp(args[i], "kill") == 0){		
				cout << "smash: sending SIGKILL signal to " << (this->jobs)->getSize() << " jobs:" << endl;
				(this->jobs)->printShorterJobsList();
				break;
			}
		}
	
		for(int i = 0; i < num; i++){
			delete(args[i]);
		}
		
		if(isPipe == false){	
			(this->jobs)->killAllJobs();		
		}	
	} else{	
		if(isPipe == false){
			(this->jobs)->killStoppedJobs();
		}
	}
	
	if(isPipe == false){
		(this->jobs)->clearJobList();
		(this->timeouts)->clearTimeoutList();
	}
	
	exit(7);
}

QuitCommand::QuitCommand(const string cmd_line, JobsList* jobs, TimeoutList* timeouts): BuiltInCommand(cmd_line){
	this->jobs = jobs;
	this->timeouts = timeouts;
}

QuitCommand::~QuitCommand(){
}


// ------------------------------------------------Show Pid Command------------------------------------------------------

ShowPidCommand::ShowPidCommand(string cmd_line) : BuiltInCommand(cmd_line) { 
	this->cmd_line = cmd_line;
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << smash.getSmashPid() << endl;
}


// ------------------------------------------------Change Dir Command------------------------------------------------------

void ChangeDirCommand::execute(){
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine((this->cmd_line).c_str(), args);
    if(num == -10){
		return;
	}
    string pre = get_current_dir_name();
        
    if(strcmp(args[1], "-") == 0){
        
        if((smash.getPreviousPath()) == " "){
            cout << "smash error: cd: OLDPWD not set" << endl ;
			for(int i = 0; i < num; i++){ 
				delete(args[i]);
			}                 
            return;
        }     
      
		char* previous;
      	try{
			previous = new char[(smash.getPreviousPath()).length() + 1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			for(int i = 0; i < num; i++){ 
				delete(args[i]);
			}   			
			return;
		}
      
			       
		strcpy(previous, (smash.getPreviousPath()).c_str());
      
        if(chdir(previous) != 0){
			perror("smash error: chdir failed");
			for(int i = 0; i < num; i++){ 
				delete(args[i]);
			}   		
			delete(previous);	
            return;
        }
        
        delete(previous);	
        string temp = get_current_dir_name();
        smash.updateCurrentPath(temp);
        smash.updatePreviousPath(pre);
    }
    else{
        if(chdir(args[1]) != 0){
            perror("smash error: chdir failed");
			for(int i = 0; i < num; i++){    
				delete(args[i]);
			}      				
            return;
        }
        string temp1 = get_current_dir_name();
        smash.updatePreviousPath(pre);
        smash.updateCurrentPath(temp1);     
    }
	for(int i = 0; i < num; i++){    
		delete(args[i]);
	}            
}

ChangeDirCommand::ChangeDirCommand(string cmd_line) : BuiltInCommand(cmd_line) {
}


// ------------------------------------------------Kill Command------------------------------------------------------

KillCommand::KillCommand(string cmdLine, JobsList *jobs) : BuiltInCommand(cmdLine) {
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmdLine.c_str(), args);
    if(num  == -1){
		isError = true;
	}
    char* signall = args[1]+1;
    char* jobid = args[2];
    this->signal = atoi(signall);
    this->jobId = atoi(jobid);
    this->jobs = jobs;
    
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}    
}

void KillCommand::execute() {
	
	if(isError){
		return;
	}
	
    JobEntry* job = jobs->getJobById(jobId);
    if(job == nullptr){
        cout << "smash error: kill: job-id " + to_string(jobId) + " does not exist" << std::endl;
        return;
    }   
	
	if((signal < 1) || (signal > 31)){
		cout << "smash error: kill: invalid arguments" << endl;
		return;
	}    
    
    pid_t currentJob = job->getPid();
    
	if(signal == 23){   // continue
		if(job->getIsStopped() == true){
			JobEntry* jCmd = job;			
			JobEntry* jobC;
			
			try{
				 jobC = new JobEntry((jCmd->getJobList()), jCmd->getCmdLine(), jCmd->getPid(), jCmd->getStartTime(), jCmd->getStartTime(), jCmd->getIsStopped(), jobId);
			} catch(std::bad_alloc& exc){
				perror("smash error: allocation failed");
				return;
			}
			
			(this->jobs)->removeJobById(jobId);						
			smash.setCurrentJobCmd(jobC);		
			int result = kill(jobC->getPid(), SIGCONT);
			if(result == -1){
				perror("smash error: kill failed");
				return;
			}
			waitpid((-1)*(jobC->getPid()), nullptr, WUNTRACED);
		    cout << "signal number " + to_string(signal) + " was sent to pid " << currentJob << endl;
			return;
		}
	}
	
	if(signal == 25){   // stop
		if(job->getIsStopped() == false){
			JobEntry* jCmd = job;			
			JobEntry* jobC;
			
			try{
				 jobC = new JobEntry((jCmd->getJobList()), jCmd->getCmdLine(), jCmd->getPid(), jCmd->getStartTime(), jCmd->getEndTime(), true, jobId);
			} catch(std::bad_alloc& exc){
				perror("smash error: allocation failed");
				return;
			}
			
			(this->jobs)->removeJobById(jobId);						
			smash.setCurrentJobCmd(jobC);	
			smash.setTime();
			smash.addStoppedCommand();	
			int result = kill((-1)*(jobC->getPid()), SIGSTOP);
			if(result == -1){
				perror("smash error: kill failed");
				return;
			}
			cout << "signal number " + to_string(signal) + " was sent to pid " << currentJob << endl;
			return;
		}
	}    
	    
    if(signal == 9){    // kill
		job->setIsStopped(false);
		(this->jobs)->removeJobById(jobId);	
		int result = kill((-1)*currentJob, SIGKILL);		
		if(result == -1){
			perror("smash error: kill failed");
			return;
		}	
		cout << "signal number " + to_string(signal) + " was sent to pid " << currentJob << endl;
		return;
	}
    
    if(kill((-1)*(job->getPid()), signal) == (-1)){
		perror("smash error: kill failed");
        return;
    }
	
    cout << "signal number " + to_string(signal) + " was sent to pid " << currentJob << endl;
}

KillCommand::~KillCommand(){
}


// ------------------------------------------------Background Command------------------------------------------------------

BackgroundCommand::BackgroundCommand(string cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		isError = true;
	}
    
    if(num == 2){
        jobId  = atoi(args[1]);
    }
	
	if(num == 1){
		isAskingForLast = true;
	}    
    
    this->jobs = jobs;

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}    
}

void BackgroundCommand::execute() {

	if(isError){
		return;
	}

	(this->jobs)->removeFinishedJobs(); 
  
    if(isAskingForLast) {
        JobEntry* job = jobs->getLastStoppedJob(&jobId);
        if(job != nullptr){
			job->setIsStopped(false);
			std::cout << job->getCmdLine() << " : " << job->getPid() << endl;
			int result = kill((-1)*(job->getPid()), SIGCONT);
			if(result == -1){
				perror("smash error: kill failed");
				return;
			}
		} else{
			cout << "smash error: bg: there is no stopped jobs to resume" << endl;
		}
    }
    else{
        JobEntry* job = jobs->getJobById(jobId);

        if(job == nullptr){
			cout << "smash error: bg: job-id " << jobId << " does not exist" << endl;
			return;
		}

		if(job->getIsStopped() == false){
			cout << "smash error: bg: job-id " << jobId << " is already running in the background" << endl;
			return;
		}

		job->setIsStopped(false);
        std::cout << job->getCmdLine() << " : " << job->getPid() << endl;
        int result = kill((-1)*(job->getPid()), SIGCONT);
    	if(result == -1){
			perror("smash error: kill failed");
			return;
		}
    }
}

BackgroundCommand::~BackgroundCommand(){
}


// ------------------------------------------------Foreground Command------------------------------------------------------

ForegroundCommand::ForegroundCommand(string cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		isError = true;
	}
	 
    if(num == 2){
        id  = atoi(args[1]);
    }
    
    if(num == 1){
		id = ((jobs->getJobList()).back())->getId();
	}
    
    this->jobs = jobs;

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}
}

void ForegroundCommand::execute() {   

	if(isError){
		return;
	}
		
	(this->jobs)->removeFinishedJobs();
		
	JobEntry* jCmd = (this->jobs)->getJobById(this->id);
	
	if(jCmd == nullptr){
		cout << "smash error: fg: job-id " << this->id << " does not exist" << endl;
		return;
	}

	JobEntry* job;
	
	try{
		job = new JobEntry((jCmd->getJobList()), jCmd->getCmdLine(), jCmd->getPid(), time(nullptr), time(nullptr), jCmd->getIsStopped(), this->id);
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		return;
	}
	
	(this->jobs)->removeJobById(this->id);						
	smash.setCurrentJobCmd(job);	
   
    std::cout << (job->getCmdLine()) << " : " << (job->getPid()) << endl;
    
    if(job->getIsStopped()){
		int result = kill((-1)*(job->getPid()), SIGCONT);
		if(result == -1){
			perror("smash error: kill failed");
			return;
		}
		waitpid(job->getPid(), nullptr, WUNTRACED);
	} else{
		job->setIsStopped(true);
		waitpid(job->getPid(), nullptr, WUNTRACED);
	}
	smash.setCurrentJobCmd(nullptr);
	delete(job);
}

ForegroundCommand::~ForegroundCommand(){
}


// ------------------------------------------------Copy Command------------------------------------------------------

CopyCommand::CopyCommand(const string cmd_line, SmallShell* smash) : Command(cmd_line) {

	this->smash = smash;

	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd_line.c_str())){
		this->isBackground = true;
		char* command;
		try{
			command = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			isError = true;
		}
		strcpy(command, cmd_line.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd_line;
	}	
	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line_with_no_sign.c_str(),args);
    if(num == -10){
		isError = true;
	}

	this->srcFile = string(args[1]);
	this->dstFile = string(args[2]);

    char* realPath1 = new char[PATH_MAX + 1];
    char* realPath2 = new char[PATH_MAX + 1];
    realPath1 = realpath(args[1], NULL);
    realPath2 = realpath(args[2], NULL);
    if(realPath2 != nullptr && realPath1!= nullptr && strcmp(realPath1, realPath2)==0){
        filesAreSame = true;
    }
	for(int i = 0; i < num; i++){
		delete(args[i]);
	}

    delete[] (realPath1);
    delete[] (realPath2);
}

void CopyCommand::execute() {
		
	if(filesAreSame){
        char* realPath1 = new char[PATH_MAX + 1];
        realPath1 = realpath(srcFile.c_str(), NULL);
        if(realPath1 == nullptr){
            delete [] realPath1;
			perror("smash error: open failed");
            return;
        }
        delete []  realPath1;
		cout << "smash: " << srcFile << " was copied to " << dstFile << endl;				 
		return;
	}
  
	int child_pid = fork();
	if(child_pid == (-1)){
		perror("smash error: fork failed");
		return;
	}
	if (child_pid == 0) { // child process		
		setpgrp();

		int src = open((this->srcFile).c_str(), O_RDONLY);
		if(src == (-1)){
			perror("smash error: open failed");
			exit(7);
		}
		int dst = open((this->dstFile).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if(dst == (-1)){
			perror("smash error: open failed");
			close(src);
			exit(7);
		}
		 
		int readSize, writeSize;
		size_t count = 10000;
		char* buf = new char[10000];
		do{
			readSize = read(src, buf, count);
			if(readSize == (-1)){
				perror("smash error: read failed");
				close(src);
				close(dst);
				exit(7);
			}
			writeSize = write(dst, buf, readSize);
			if(writeSize == (-1)){
				perror("smash error: write failed");
				close(src);
				close(dst);				
				exit(7);
			}		 
		} while(readSize > 0);

		cout << "smash: " << srcFile << " was copied to " << dstFile << endl;				 
				 
		int result = close(src);
		if(result == -1){
			perror("smash error: close failed");
			exit(7);
		}
		result = close(dst);	
		if(result == -1){
			perror("smash error: close failed");
			exit(7);
		}	 
		exit(1);
		
	} else { // parent process		
		JobEntry* jCmd;
		
		try{
			jCmd = new JobEntry(((this->smash)->getJList()), this->cmd_line, child_pid, false);
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return;
		}

		if (isBackground){			
			(this->smash)->setCurrentJobCmd(jCmd);	
			(this->smash)->addUnfinishedCommand();
			(this->smash)->setCurrentJobCmd(nullptr);
			delete(jCmd);
		} else{
			jCmd->setIsStopped(true);
			(this->smash)->setCurrentJobCmd(jCmd);

			int status;
			waitpid(child_pid, &status, WUNTRACED);			
			
			(this->smash)->setCurrentJobCmd(nullptr);
			delete(jCmd);	
		} 		
	}
}

CopyCommand::~CopyCommand(){
}


// -----------------------------------------------RedirectionCommand------------------------------------------------------

RedirectionCommand::RedirectionCommand(string cmd_line, SmallShell* smash) : Command(cmd_line) {
    this->smash = smash;
    this->background = _isBackgroundComamnd(cmd_line.c_str());
    string cmd;
    if(background){
        char* temp;
        
        try{
			temp = new char[(cmd_line.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			isError = true;
		}
        
        strcpy(temp, cmd_line.c_str());
        _removeBackgroundSign(temp);
        cmd = string(temp);
        delete(temp);
    }
    else {
        cmd = cmd_line;
    }
    
    for (unsigned int i = 0; i < cmd.length(); i++) {
            if(cmd[i] == '>'){
                if(cmd[i+1]=='>'){
					int j = i-1;
					while(cmd[j] == ' '){
						j--;
					}
                    command = cmd.substr(0,j+1);
                    j = i+2;
					while(cmd[j] == ' '){
						j++;
					}                    
                    path = cmd.substr(j, cmd.length() - j);
                    this->isAppend = true;
                    break;
                }
                else{
					int j = i-1;
					while(cmd[j] == ' '){
						j--;
					}					
                    command = cmd.substr(0,j+1);
                    j = i+1;
					while(cmd[j] == ' '){
						j++;
					}                               
                    path = cmd.substr(j, cmd.length() - j);
                    this->isAppend = false;
                    break;
                }
            }
        }
        
	for (unsigned int i = 0; i < path.length(); i++ ){
		if((path[i] == ' ') || (path[i] == '&')){
		    path = path.substr(0, i);	
		    return;			
		}
	}        
}

void executeExternalCommand(string cmd, string full_cmd){
	string cmd_line_with_no_sign;
	
	if (_isBackgroundComamnd(cmd.c_str())){
		char* command;
		
		try{
			command = new char[(cmd.length())+1];
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			isRedirect = false;
			return;
		}
		
		strcpy(command, cmd.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd;
	}

	char** argv;	
	try{
		argv = new char*[4];
		argv[0] = new char[10];
		strcpy(argv[0], "/bin/bash");
		argv[1] = new char[2];
		strcpy(argv[1], "-c");
		argv[2] = new char[cmd_line_with_no_sign.length()];
		strcpy(argv[2], cmd_line_with_no_sign.c_str());
		argv[3] = nullptr;
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		isRedirect = false;
		return;
	}
		
	int child_pid = fork();
	if(child_pid == (-1)){
		perror("smash error: fork failed");
		isRedirect = false;
		for(unsigned int j = 0; j<4; j++){
			if(argv[j] != nullptr)
				delete(argv[j]);	 
		}
		delete[](argv);		
		return;
	}
	if (child_pid == 0) { // child process							
		setpgrp();					
		execv(argv[0], argv);
		perror("smash error: execv failed");
		isRedirect = false;
		exit(7);
	} else { // parent process

		JobEntry* jCmd;		
	
		try{		
			jCmd = new JobEntry((smash.getJList()), full_cmd, child_pid, false);
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			isRedirect = false;
			for(unsigned int j = 0; j<4; j++){
				if(argv[j] != nullptr)
					delete(argv[j]);	 
				}
			delete[](argv);
			return;
		}	

		if (_isBackgroundComamnd(cmd.c_str())){			
			smash.setCurrentJobCmd(jCmd);	
			smash.addUnfinishedCommand();
			smash.setCurrentJobCmd(nullptr);
			delete(jCmd);
		} else{
			jCmd->setIsStopped(true);
			smash.setCurrentJobCmd(jCmd);

			int status;
			waitpid(child_pid, &status, WUNTRACED);			
			
			smash.setCurrentJobCmd(nullptr);
			delete(jCmd);	
		}
	}
		
	for(unsigned int j = 0; j<4; j++){
		if(argv[j] != nullptr)
			delete(argv[j]);	 
	}
		
	delete[](argv);
}

void RedirectionCommand::execute() {
	
	if(isError){
		return;
	}
	
	isRedirect = true;
    int fd = dup(1);
    stdoutFile = fd;
    if(fd == (-1)){
		perror("smash error: dup failed");
		isRedirect = false;
		return;
	}
    int file;
    if (isAppend == false) {
        file = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    } else {
        file = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    }
    
    if(file == -1){
		perror("smash error: open failed");
		isRedirect = false;
		return;
	}
    redirectFile = file;
    
    if (background) { 
        string s = " &";
        command = command + s;
    }
   
    int result = dup2(file, 1);
    if(result == (-1)){
		perror("smash error: dup2 failed");
		isRedirect = false;
		return;
	}
    if (isCommandBuiltIn(command)) {
        smash->executeCommand(command);
    } else {
        executeExternalCommand(command, cmd_line);
    }
 
    result = close(1);
    if(result == (-1)){
		perror("smash error: close failed");
		isRedirect = false;
		return;
	}
    result = dup2(fd, 1);
    if(result == (-1)){
		perror("smash error: dup2 failed");
		isRedirect = false;
		return;
	}
    result = close(file);
    if(fd == (-1)){
		perror("smash error: close failed");
		isRedirect = false;
		return;
	}
	isRedirect = false;
}

RedirectionCommand::~RedirectionCommand() {
}


// -----------------------------------------------PipeCommand------------------------------------------------------

PipeCommand::PipeCommand(string cmd_line) : Command(cmd_line) {
	int j = 0;
    for (unsigned int i = 0; i < cmd_line.length(); i++ ){
		if (cmd_line[i] == '|') {                  
			if (cmd_line[i + 1] == '&') {
				j = i-1;
				while(cmd_line[j] == ' '){
					j--;
				}
                command1 = cmd_line.substr(0,j+1);
                j = i+2;
				while(cmd_line[j] == ' '){
					j++;
				}                    
                command2 = cmd_line.substr(j, cmd_line.length() - j);	
                this->isWithBackground = true;
                break;
            } else {
				int j = i-1;
				while(cmd_line[j] == ' '){
					j--;
				}					
                command1 = cmd_line.substr(0,j+1);
                j = i+1;
				while(cmd_line[j] == ' '){
					j++;
				}                               
                command2 = cmd_line.substr(j, cmd_line.length() - j);
                this->isWithBackground = false;
                break;		
            }
		}
	}

	background = _isBackgroundComamnd(command2.c_str());
	if(background){	
		for (unsigned int i = 0; i < command2.length(); i++ ){
			if (command2[i] == '&') { 
				j = i - 1;
				while(command2[j] == ' '){
					j--;
				}	
			    command2 = command2.substr(0, j+1);	
			    return;
			}	
		}
	}
}

void PipeCommand::executeExternalPipeWithSign(){   

	char** argv1;
	char** argv2;
		
	try{
		argv1 = new char*[4];
		argv2 = new char*[4];
		
		argv1[0] = new char[10];
		strcpy(argv1[0], "/bin/bash");
		argv1[1] = new char[2];
		strcpy(argv1[1], "-c");
		argv1[2] = new char[command1.length()];
		strcpy(argv1[2], command1.c_str());
		argv1[3] = nullptr;	
		
		argv2[0] = new char[10];
		strcpy(argv2[0], "/bin/bash");
		argv2[1] = new char[2];
		strcpy(argv2[1], "-c");
		argv2[2] = new char[command2.length()];
		strcpy(argv2[2], command2.c_str());
		argv2[3] = nullptr;	
		
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		isPipe = false;	
		return;
	}
	
	child1Pid = fork();
	if(child1Pid == (-1)){
		perror("smash error: fork failed");
		for(unsigned int j = 0; j<4; j++){
			if(argv1[j] != nullptr){
				delete(argv1[j]);
			}
			if(argv2[j] != nullptr){
				delete(argv2[j]);	
			} 
		}
		delete[](argv1);
		delete[](argv2);
		isPipe = false;	
		return;
	}
	if (child1Pid == 0) { // child1 process		
		setpgrp();							
		int result = dup2(my_pipe[1],2);
		if(result == (-1)){
			perror("smash error: dup2 failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[0]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[1]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		
		if (isCommandBuiltIn(command1)) {
			smash.executeCommand(command1);
			exit(1);
		} else {		
			execv(argv1[0], argv1);
			perror("smash error: execv failed");
			isPipe = false;	
			exit(7);
		}
	}
	
	child2Pid = fork();
	if(child2Pid == (-1)){
		perror("smash error: fork failed");
		isPipe = false;	
		for(unsigned int j = 0; j<4; j++){
			if(argv1[j] != nullptr){
				delete(argv1[j]);
			}
			if(argv2[j] != nullptr){
				delete(argv2[j]);	
			} 
		}
		delete[](argv1);
		delete[](argv2);
		return;
	}
	if (child2Pid == 0) { // child2 process			
		setpgrp();					
		int result = dup2(my_pipe[0],0);
		if(result == (-1)){
			perror("smash error: dup2 failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[0]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[1]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		
		if (isCommandBuiltIn(command2)) {
			smash.executeCommand(command2);
			exit(1);	
		} else {		
			execv(argv2[0], argv2);
			perror("smash error: execv failed");
			isPipe = false;	
			exit(7);
		}	
	}
	
	for(unsigned int j = 0; j<4; j++){
		if(argv1[j] != nullptr){
			delete(argv1[j]);
		}
		if(argv2[j] != nullptr){
			delete(argv2[j]);	
		} 
	}
	delete[](argv1);
	delete[](argv2);	
	
	int result = close(my_pipe[0]);
	if(result == (-1)){
		perror("smash error: close failed");
		isPipe = false;	
		return;
	}
	result = close(my_pipe[1]);
	if(result == (-1)){
		perror("smash error: close failed");
		isPipe = false;	
		return;
	}		
	while(wait(nullptr) != (-1));				
}

void PipeCommand::executeExternalPipeWithoutSign(){

	char** argv1;
	char** argv2;
		
	try{
		argv1 = new char*[4];
		argv2 = new char*[4];
		
		argv1[0] = new char[10];
		strcpy(argv1[0], "/bin/bash");
		argv1[1] = new char[2];
		strcpy(argv1[1], "-c");
		argv1[2] = new char[command1.length()];
		strcpy(argv1[2], command1.c_str());
		argv1[3] = nullptr;	
		
		argv2[0] = new char[10];
		strcpy(argv2[0], "/bin/bash");
		argv2[1] = new char[2];
		strcpy(argv2[1], "-c");
		argv2[2] = new char[command2.length()];
		strcpy(argv2[2], command2.c_str());
		argv2[3] = nullptr;	
		
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		isPipe = false;	
		return;
	}

	child1Pid = fork();
	if(child1Pid == (-1)){
		perror("smash error: fork failed");
		for(unsigned int j = 0; j<4; j++){
			if(argv1[j] != nullptr){
				delete(argv1[j]);
			}
			if(argv2[j] != nullptr){
				delete(argv2[j]);	
			} 
		}
		delete[](argv1);
		delete[](argv2);		
		isPipe = false;	
		return;
	}
	if (child1Pid == 0) { // child1 process					
		int result = dup2(my_pipe[1],1);
		if(result == (-1)){
			perror("smash error: dup2 failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[0]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[1]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}

		if (isCommandBuiltIn(command1)) {
			smash.executeCommand(command1);
			exit(1);
		} else {		
			execv(argv1[0], argv1);
			perror("smash error: execv failed");
			isPipe = false;	
			exit(7);
		}
	}
		
	child2Pid = fork();
	if(child2Pid == (-1)){
		perror("smash error: fork failed");
		for(unsigned int j = 0; j<4; j++){
			if(argv1[j] != nullptr){
				delete(argv1[j]);
			}
			if(argv2[j] != nullptr){
				delete(argv2[j]);	
			} 
		}
		delete[](argv1);
		delete[](argv2);		
		isPipe = false;	
		return;
	}
	if (child2Pid == 0) { // child2 process						
		int result = dup2(my_pipe[0],0);
		if(result == (-1)){
			perror("smash error: dup2 failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[0]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		result = close(my_pipe[1]);
		if(result == (-1)){
			perror("smash error: close failed");
			isPipe = false;	
			exit(7);
		}
		
		if (isCommandBuiltIn(command2)) {
			smash.executeCommand(command2);
			exit(1);	
		} else {		
			execv(argv2[0], argv2);
			perror("smash error: execv failed");
			isPipe = false;	
			exit(7);
		}	
	}
	
	for(unsigned int j = 0; j<4; j++){
		if(argv1[j] != nullptr){
			delete(argv1[j]);
		}
		if(argv2[j] != nullptr){
			delete(argv2[j]);	
		} 
	}
	delete[](argv1);
	delete[](argv2);	
	
	int result = close(my_pipe[0]);
	if(result == (-1)){
		perror("smash error: close failed");
		isPipe = false;	
		return;
	}
	result = close(my_pipe[1]);	
	if(result == (-1)){
		perror("smash error: close failed");
		isPipe = false;	
		return;
	}	
	while(wait(nullptr) != (-1));	
}

void PipeCommand::execute(){
	isPipe = true;
	fatherPid = fork();
	if(fatherPid == (-1)){
		perror("smash error: fork failed");
		isPipe = false;		
		return;
	}
	if (fatherPid == 0) { // child process							
		setpgrp();					
		int result = pipe(my_pipe);
		if(result == (-1)){
			perror("smash error: pipe failed");
			isPipe = false;	
			exit(7);
		}
		if(isWithBackground){
			executeExternalPipeWithSign();
		} else{
			executeExternalPipeWithoutSign();	
		}	
		exit(1);
	} else { // parent process						
		JobEntry* jCmd;
	
		try{
			jCmd = new JobEntry((smash.getJList()), this->cmd_line, fatherPid, false);
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			isPipe = false;	
			return;
		}
		
		if (background){			
			smash.setCurrentJobCmd(jCmd);	
			smash.addUnfinishedCommand();
			smash.setCurrentJobCmd(nullptr);
			delete(jCmd);
		} else{
			jCmd->setIsStopped(true);
			smash.setCurrentJobCmd(jCmd);
			waitpid(fatherPid, nullptr, WUNTRACED);			
			smash.setCurrentJobCmd(nullptr);
			delete(jCmd);			
		}
	}
	isPipe = false;
}

PipeCommand::~PipeCommand() {
}


// ------------------------------------------------Timeout Command------------------------------------------------------

TimeoutCommand::TimeoutCommand(const string cmd_line, SmallShell* smash, TimeoutList* timeouts, JobsList* jobs) : Command(cmd_line) {
	this->smash = smash;
	this->timeouts = timeouts;
	this->jobs = jobs;
}

void TimeoutCommand::executeExternalTimeout(string cmd, string full_cmd, time_t duration){
	string cmd_line_with_no_sign;
	if (_isBackgroundComamnd(cmd.c_str())){
		char* command;
		try{
			command = new char[(cmd.length())+1];
		}
		catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			return;
		}
		strcpy(command, cmd.c_str());
		_removeBackgroundSign(command);
		cmd_line_with_no_sign = string(command);
		delete(command);
	} else{
		cmd_line_with_no_sign = cmd;
	}
	
	char** argv;
	
	try{
		argv = new char*[4];
		argv[0] = new char[10];
		strcpy(argv[0], "/bin/bash");
		argv[1] = new char[3];
		strcpy(argv[1], "-c");
		argv[2] = new char[cmd_line_with_no_sign.length()+1];
		strcpy(argv[2], cmd_line_with_no_sign.c_str());
		argv[3] = nullptr;
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		return;
	}
			
	int child_pid = fork();
	if(child_pid == (-1)){
		perror("smash error: fork failed");
		for(unsigned int j = 0; j<4; j++){
			if(argv[j] != nullptr)
				delete(argv[j]);	 
		}
		delete[](argv);
		return;
	}
	if (child_pid == 0) { // child process
							
		setpgrp();					
		execv(argv[0], argv);
		perror("smash error: execv failed");
		exit(7);
		
	} else { // parent process
				
		JobEntry* jCmd;
		TimeoutEntry* newTimeoutEntry; 

		try{
			jCmd = new JobEntry((smash->getJList()), full_cmd, child_pid, false);
			newTimeoutEntry = new TimeoutEntry(full_cmd, duration, child_pid, jCmd->getIsStopped()); 
		} catch(std::bad_alloc& exc){
			perror("smash error: allocation failed");
			for(unsigned int j = 0; j<4; j++){
			if(argv[j] != nullptr) {
                delete (argv[j]);
            }
			}
			delete[](argv);		
			return;
		}

		if (_isBackgroundComamnd(cmd.c_str())){			
			smash->setCurrentJobCmd(jCmd);	
			smash->setCurrentTimeoutCmd(newTimeoutEntry);				
			smash->addUnfinishedCommand();
			timeouts->addTimeout(newTimeoutEntry);    			
			smash->setCurrentJobCmd(nullptr);
			smash->setCurrentTimeoutCmd(nullptr);			
			delete(jCmd);
			delete(newTimeoutEntry);	
		} else{
			jCmd->setIsStopped(true);
			smash->setCurrentJobCmd(jCmd);
			smash->setCurrentTimeoutCmd(newTimeoutEntry);							
			timeouts->addTimeout(newTimeoutEntry);
			int status;
			waitpid(child_pid, &status, WUNTRACED);
			smash->setCurrentTimeoutCmd(nullptr);							
			smash->setCurrentJobCmd(nullptr);
			delete(jCmd);	
			delete(newTimeoutEntry);	
		}
	}
	
	for(unsigned int j = 0; j<4; j++){
		if(argv[j] != nullptr)
			delete(argv[j]);	 
	}
		
	delete[](argv);
}

void TimeoutCommand::execute(){	
    char* args[COMMAND_MAX_ARGS];
    int num = _parseCommandLine(cmd_line.c_str(),args);
    if(num == -10){
		return;
	}

	int duration = stoi(string(args[1]));
	string cmd_line = this->cmd_line;
	
	unsigned int i = 0;
	for(i = strlen(args[0]); i < cmd_line.length(); i++){
		if(cmd_line[i] != ' ')
			break;
    }
	for(i += strlen(args[1]); i < cmd_line.length(); i++){
		if(cmd_line[i] != ' ')
			break;
    }    
	string command = cmd_line.substr(i, (cmd_line.length())-i);	

    if (isCommandBuiltIn(command)) {
        smash->executeCommand(command);
    } else {		
        executeExternalTimeout(command, cmd_line, duration);
    } 			 		 		

	for(int i = 0; i < num; i++){
		delete(args[i]);
	}      	
}

TimeoutCommand::~TimeoutCommand(){
}


// ------------------------------------------------Timeout Entry------------------------------------------------------

TimeoutEntry::TimeoutEntry(const string cmd_line, time_t duration, pid_t pid, bool isStopped) : Command(cmd_line, pid){
	this->duration = duration;
	this->timestamp = duration;
	this->isStopped = isStopped;
}

TimeoutEntry::~TimeoutEntry(){
}

pid_t TimeoutEntry::getPid(){
	return (this->pid);
}

time_t TimeoutEntry::getDuration(){
	return (this->duration);
}

time_t TimeoutEntry::getTimestamp(){
	return (this->timestamp);
}

string TimeoutEntry::getCmdLine(){
	return (this->cmd_line);
}

void TimeoutEntry::setTimestamp(time_t timestamp){
	this->timestamp -= timestamp;
}

bool TimeoutEntry::getIsStopped(){
	return isStopped;
}


// ------------------------------------------------Timeout List------------------------------------------------------

TimeoutList::TimeoutList(SmallShell* smash, JobsList* jobs){
	this->smash = smash;
	this->jobs = jobs;
}

TimeoutList::~TimeoutList(){
}

void TimeoutList::addTimeout(TimeoutEntry* cmd){	
	if(cmd == nullptr){
		return;
	}
	
	if(timeouts.size() == 0){
		startAlarm = time(nullptr);
		pastedAlarm = time(nullptr) - startAlarm;	
		setCurrentAlarm(cmd->getDuration());
	}

	pastedAlarm = time(nullptr) - startAlarm;					
	if((timeouts.size() != 0) && (cmd->getDuration() < currentAlarm-pastedAlarm)){
		updateDuration();
		setCurrentAlarm(cmd->getDuration());
	}
	
	TimeoutEntry* newTimeoutEntry;
	
	try{
		newTimeoutEntry = new TimeoutEntry(cmd->getCmdLine(), cmd->getDuration(), cmd->getPid(), cmd->getIsStopped());
	} catch(std::bad_alloc& exc){
		perror("smash error: allocation failed");
		return;
	}
	(this->timeouts).push_back(newTimeoutEntry);
}

void TimeoutList::removeTimoutByTimestamp(time_t timestamp){
	
	(this->jobs)->removeFinishedJobs();
	
	std::list<TimeoutEntry*>::reverse_iterator rit=this->timeouts.rbegin();
	
	while(rit != this->timeouts.rend()){	
        if((*rit)->getTimestamp() == timestamp){
			TimeoutEntry* TCmd = *rit;
			bool isRemoved = smash->getJList()->removeJobByPid((*rit)->getPid());
			if(isRemoved) {
				cout << "smash: " << (*rit)->getCmdLine() << " timed out!" << endl;
			} 
			(this->timeouts).remove(*rit);
			delete(TCmd);			
        } else{
			++rit;
		}
	}
}

list<TimeoutEntry*> TimeoutList::getTimeouts(){
	return(this->timeouts);
}

void TimeoutList::setCurrentAlarm(time_t newAlarm){
	this->currentAlarm = newAlarm;
	startAlarm = time(nullptr);
	pastedAlarm = time(nullptr) - startAlarm;		
	alarm(newAlarm);
}

void TimeoutList::updateDuration(){
    for (std::list<TimeoutEntry*>::reverse_iterator rit=timeouts.rbegin(); rit != timeouts.rend(); ++rit){
			(*rit)->setTimestamp(pastedAlarm);
        }	
}

time_t TimeoutList::getCurrentAlarm(){
	return(this->currentAlarm);
}    

void TimeoutList::setAndUpdateAlarm(){
	pastedAlarm = currentAlarm;
	updateDuration();						
	
	if(timeouts.size() == 0){
	    startAlarm = time(nullptr);
		pastedAlarm = time(nullptr) - startAlarm;
		currentAlarm = 0;	
		return;
	}
	
	time_t min = (timeouts.front())->getTimestamp(); 
    for (std::list<TimeoutEntry*>::reverse_iterator rit=timeouts.rbegin(); rit != timeouts.rend(); ++rit){
        if((*rit)->getTimestamp() < min){
			min = (*rit)->getTimestamp();		
        }
    }
    	
    setCurrentAlarm(min);	
}

void TimeoutList::clearTimeoutList(){
	for (std::list<TimeoutEntry*>::reverse_iterator rit=this->timeouts.rbegin(); rit != this->timeouts.rend(); ++rit){
		TimeoutEntry* TCmd = *rit;
		(this->timeouts).remove(*rit);
		delete(TCmd);			
	}
}


