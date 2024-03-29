#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// retrieve file attributes
void file_stats(const std::string &filename) {
	struct stat st;
	if (stat(filename.c_str(), &st) != 0) {
		std::cout << "File error" << std::endl;
	} else {
		std::cout << st.st_uid << "\t";
		std::cout << st.st_gid << "\t";
		std::cout << st.st_size << "B" << "\t";
		std::cout << st.st_atime << "\t";
		std::cout << st.st_mtime << "\t";
		std::cout << st.st_ctime << "\t";
		std::cout << std::endl;
	}
}

// save command
void write_history(const std::string &line) {
	std::string username = getlogin();
	// the path of history.txt file
	std::string name_file = "/home/" + username + "/history.txt";
	struct stat buffer;
	// check if file exists or not
	// save user command
	if ((stat(name_file.c_str(), &buffer)) == 0) {
		std::ofstream outfile;
		outfile.open(name_file, std::ios_base::app);
		outfile << line;
		outfile << "\n";
	}
	else {
		std::ofstream saveFile(name_file);
		saveFile << line;
		saveFile << "\n";
		saveFile.close();
	}
}

// list commands entered in the past
int history_cmd() {
	std::string username = getlogin();
	std::ifstream myfile("/home/" + username + "/history.txt");
	std::string line;
	if (myfile.is_open()) {
		int counter = 1;
		while (getline(myfile, line)) {
			std::cout << counter << ". " << line << std::endl;
			counter++;
		}
		myfile.close();
		return 1;
	}
	else {
		std::cout << "Unable to list commands" << std::endl;
		return 1;
	}
}

// list files and directory of a given directory
int list_cmd() {
	std::cout << "name\t\t" << "uid\t" << "gid\t" << "size\t" << "atime\t\t" << "mtime\t\t" << "ctime\t\t" << std::endl;
	std::cout << std::endl;
	DIR* dir;
	struct dirent* sd;
	dir = opendir(".");
	if (dir == NULL) {
		std::cout << "Error! \n";
		exit(1);
	}

	while ((sd = readdir(dir)) != NULL) {
		std::string output = (*sd).d_name;
		int name_size = output.size();
		std::string tab1 = name_size > 7 ? "\t" : "\t\t";
		std::cout << output << tab1;
		file_stats(output);
	}

	closedir(dir);
	return 1;
}

// print current working directory
int printwd_cmd() {
	char* ppath = get_current_dir_name();
	if (ppath == NULL) {
		std::cout << "Error";
	}
	std::cout << ppath << std::endl;
	return 1;
}


// change directory
int chdir_cmd(const std::vector<std::string> &args) {
	int rc = chdir(args[1].c_str());
	if (rc < 0) {
		std::cout << "Directory not found!!!" << std::endl;
	}
	else {
		std::cout << "Current directory: ";
		printwd_cmd();
	}
	return 1;
}

// tokenize input
std::vector<std::string> split_line(std::string line) {
	std::vector<std::string> args;
	std::string delimiter = " ";
	size_t pos = 0;
	std::string token;
	while ((pos = line.find(delimiter)) != std::string::npos) {
		token = line.substr(0, pos);
		args.push_back(token);
		line.erase(0, pos + delimiter.length());
	}
	args.push_back(line);
	return args;
}

// read user input
std::string read_line() {
	std::string line;
	std::getline(std::cin, line);
	write_history(line); // write input to history
	return line;
}

// run an external program
int exe(const std::vector<std::string> &args) {
	pid_t pid;
	int size = args.size();
	char* argv[size];
	int i;

	for (i = 0; i < size; i++) {
		char* arg = new char[args[i].size() + 1];
		strcpy(arg, args[i].c_str());
		argv[i] = arg;
	}
	argv[i] = NULL; // argv terminated by null pointer

	pid = fork();
	if (pid == 0) { // child process
		if (execvp(args[0].c_str(), argv) == -1) {
			perror("error");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		perror("fork error");
	}
	else wait(NULL); //  wait until a state change in the child process

	return 1;
}

int run(const std::vector<std::string> &args) {
	// run command
	if (args[0] == "printwd") {
		return printwd_cmd();;
	}
	else if (args[0] == "list") {
		return list_cmd();
	}
	else if (args[0] == "history") {
		return history_cmd();
	}
	else if (args[0] == "chdir") {
		return chdir_cmd(args);
	}
	else if (args[0] == "clear") {
		system("clear");
		return 1;
	}
	else if (args[0] == "exit") {
		return 0;
	}
	else {
		return exe(args); // run an external program
	}
}

void shell_loop(void) {
	std::string input;
	std::vector<std::string> args;
	int status;

	do {
		char* curr_dir = get_current_dir_name();
		std::cout << "\033[1;32mSHELL@\033[0m"<<curr_dir << ": ";
		input = read_line(); // get input
		args = split_line(input); // tokenize input
		status = run(args);

	} while (status); // status 0 terminates loop
}

int main() {
	system("clear"); // clear the terminal
	shell_loop();
}
