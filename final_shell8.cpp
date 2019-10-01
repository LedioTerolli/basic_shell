#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

// retrieve file attributes
void file_stats(const std::string filename) {
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
void write_history(std::string line) {
	std::string username = getlogin();
	std::string name_file = "/home/" + username + "/history.txt";
	struct stat buffer;
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
// args not used, but present for consistency
int history_cmd(std::vector<std::string> args) {
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
// args not used, but present for consistency
int list_cmd(std::vector<std::string> args) {
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
	return 0;
}

// print current working directory
// args not used, but present for consistency
int printwd_cmd(std::vector<std::string> args) {
	char* ppath = get_current_dir_name();
	if (ppath == NULL) {
		std::cout << "Error";
		return 0;
	}
	std::cout << ppath << std::endl;
	return 1;
}


// change directory
int chdir_cmd(std::vector<std::string> args) {
	int rc = chdir(args[1].c_str());
	if (rc < 0) {
		std::cout << "Directory not found!!!" << std::endl;
	}
	else {
		std::cout << "Current directory: ";
		printwd_cmd(args);
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
int exe(std::vector<std::string> args) {
	pid_t pid;
	int status;
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
	else wait(NULL); // parent process waits for child

	return 1;
}

int run(std::vector<std::string> args) {
	// run command
	if (args[0] == "printwd") {
		printwd_cmd(args);
		return 1;
	}
	else if (args[0] == "list") {
		list_cmd(args);
		return 1;
	}
	else if (args[0] == "history") {
		history_cmd(args);
		return 1;
	}
	else if (args[0] == "chdir") {
		chdir_cmd(args);
		return 1;
	}
	else if (args[0] == "clear") {
		system("clear");
		return 1;
	}
	else if (args[0] == "exit") {
		return 0;
	}
	else {
		exe(args); // run an external program
		return 1;
	}
}

void shell_loop(void) {
	std::string input;
	std::vector<std::string> args;
	int status;

	do {
		std::cout << "@SHELL8: ";
		input = read_line(); // get input
		args = split_line(input); // tokenize input
		status = run(args);

	} while (status); // status 0 terminates loop
}

int main() {
	system("clear"); // clear the terminal
	shell_loop();
	return EXIT_SUCCESS;
}
