#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>

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

void write_history(std::string line) {
	std::string name_file = "history.txt";
	struct stat buffer;
	if((stat(name_file.c_str(), &buffer)) == 0) {
		std::ofstream outfile;
  	outfile.open(name_file, std::ios_base::app);
  	outfile << line;
    outfile << "\n";
	} else {
		std::ofstream saveFile (name_file);
    saveFile << line;
 		saveFile << "\n";
    saveFile.close();
	}
}

std::string read_line() {
	std::string line;
	std::getline (std::cin, line);
	write_history(line);
	return line;
}

int history_cmd(std::vector<std::string> args) {
	std::ifstream myfile ("history.txt");
	std::string line;
	if (myfile.is_open()) {
		int counter = 1;
		while (getline(myfile, line)) {
			std::cout << counter << ". " << line << std::endl;
			counter++;
		}
		myfile.close();
		return 1;
	} else {
		std::cout << "Unable to open file";
		return 1;
	}
}

void file_stats(const std::string filename) {
	struct stat st;
	if(stat(filename.c_str(), &st) != 0) {
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

int list_cmd(std::vector<std::string> args) {
	std::cout << "name\t\t" << "uid\t" << "gid\t" << "size\t" << "atime\t\t" << "mtime\t\t" << "ctime\t\t" << std::endl;
	std::cout << std::endl;
	DIR *dir;
  struct dirent *sd;
	dir = opendir(".");
  if (dir == NULL) {
		std::cout << "Error! \n";
  	exit(1);
	}

	while ((sd=readdir (dir)) != NULL) {
		std::string output = (*sd).d_name;
		int name_size = output.size();
		std::string tab1 = name_size > 7? "\t": "\t\t";
		std::cout << output << tab1;
		file_stats(output);
	}

	closedir(dir);
  return 0;
}

int printwd_cmd(std::vector<std::string> args) {
	char* ppath = get_current_dir_name();
 	if (ppath == NULL){
		std::cout<< "Error";
    return 0;
	}
	std::cout<< ppath << std::endl;
	return 1;
}

int chdir_cmd(std::vector<std::string> args) {
	int rc = chdir(args[1].c_str());
  if (rc < 0) {
		std::cout << "Directory not found!!!" << std::endl;
  } else {
		std::cout << "Current directory: ";
		printwd_cmd(args);
	}
	return 1;
}

int exe(std::vector<std::string> args) {
	pid_t pid;
	int status;
	int size = args.size();
	char * argv[size];
	int i;

	for(i = 0; i<size ; i++) {
		char *arg = new char[args[i].size()+1] ;
		strcpy(arg, args[i].c_str());
		argv[i] = arg;
	}

	argv[i] = NULL;
  pid = fork();
  if (pid == 0) {
    if (execvp(args[0].c_str(), argv) == -1) {
      perror("error");
    }
    exit(EXIT_FAILURE);
	} else if (pid < 0) {
    		perror("fork error");
  } else wait(NULL);

	return 1;
}

int run(std::vector<std::string> args) {
	if(args[0] == "printwd") {
		printwd_cmd(args);
		return 1;
	} else if (args[0] == "list") {
		list_cmd(args);
		return 1;
	} else if (args[0] == "history") {
		history_cmd(args);
		return 1;
	} else if (args[0] == "chdir") {
		chdir_cmd(args);
		return 1;
	} else if (args[0] == "exit") {
		return 0;
	} else {
		exe(args);
		return 1;
	}
}

void shell_loop(void) {
  std::string input;
  std::vector<std::string> args;
  int status;

  do {
    std::cout<< "@SHELL8: ";
    input = read_line();
    args = split_line(input);
    status = run(args);

  } while (status);
}

int main() {
  shell_loop();
  return EXIT_SUCCESS;
}
