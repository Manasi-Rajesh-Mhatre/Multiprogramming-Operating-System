#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

char M[100][4];
char buffer[40];
char IR[4];
char R[4];
bool C = false;
int IC = 0;
int SI = 0;

void loadProgram(const string& filename);
void startExecution();
void executeUserProgram();
void masterMode(string operand);
void readData(string operand);
void writeData(string operand);
void customTerminate();
void printMemory();

int main() {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 4; j++) {
            M[i][j] = '\0';
        }
    }
    cout << "Memory Initialized." << endl;
    loadProgram("input.txt");
    cout << "Program Loaded." << endl;
    printMemory();
    startExecution();
    printMemory();
    cout << "Execution Complete." << endl;
    return 0;
}

void printMemory() {
    cout << "\nMain Memory:\n";
    cout << "-------------------------------------------------\n";
    for (int i = 0; i < 100; i++) {
        cout << "M[" << i << "] = ";
        for (int j = 0; j < 4; j++) {
            cout << (M[i][j] ? M[i][j] : '.');  
        }
        cout <<  "\n";
    }
    cout << "-------------------------------------------------\n";
}

void printSystemStatus() {
    cout << "\nSystem Status:\n";
    cout << "------------------------\n";
    cout << "IC: " << IC << endl;
    cout << "IR: " << IR[0] << IR[1] << IR[2] << IR[3] << endl;
    cout << "R : " << R[0] << R[1] << R[2] << R[3] << endl;
    cout << "C : " << (C ? "True" : "False") << endl;
    cout << "------------------------\n";
}


void loadProgram(const string& filename) {
    ifstream fin(filename);
    string line;
    int m = 0;

    if (!fin) {
        cerr << "Error opening file." << endl;
        exit(1);
    }

    while (getline(fin, line)) {
        if (line.substr(0, 4) == "$AMJ") continue;
        if (line.substr(0, 4) == "$DTA") break;
        if (line.substr(0, 4) == "$END") break;
        int len = line.length();

       strncpy(buffer, line.c_str(), len);

        if (m >= 100) {
            cout << "Memory overflow. Aborting..." << endl;
            exit(0);
        }

        for (int i = 0; i < len; i += 4, m++) { 
            if (m >= 100) break;
            memset(M[m], 0, 4); 
            strncpy(M[m], buffer + i, 4);
        }
        
    }
    fin.close();
}

void startExecution() {
    IC = 0;
    executeUserProgram();
}

void executeUserProgram() {
    while (true) {
        strncpy(IR, M[IC], 4);
        IC++;

        string opcode(IR, 2);
        string operand = (strlen(IR) >= 3) ? string(IR + 2, 2) : "";

        cout << "\nExecuting Instruction: " << opcode << " " << operand << endl;
        printSystemStatus();

        cout << "Opcode: " << opcode << ", Operand: " << operand << endl;
        
        int address = 0;
        if (!operand.empty() && opcode != "H") {
            for (char c : operand) {
                if (c >= '0' && c <= '9') {
                    address = address * 10 + (c - '0');
                } else {
                    cout << "Bad Input" << endl;
                    return; 
                }
            }
        }

        if (opcode == "LR") {
            strncpy(buffer, M[address], 4);
            strncpy(R, buffer, 4);
        } else if (opcode == "SR") {
            strncpy(buffer, R, 4);
            strncpy(M[address], buffer, 4);
        } else if (opcode == "CR") {
            C = (strncmp(R, M[address], 4)  == 0);
        } else if (opcode == "BT") {
            if (C) IC = address;
        } else if (opcode == "GD") {
            SI = 1;
            masterMode(operand);
        } else if (opcode == "PD") {
            SI = 2;
            masterMode(operand);
        } else if (opcode == "H") {
            SI = 3;
            masterMode("");
            return;
        } else {
            return;
        }
    }
}

void masterMode(string operand) {
    switch (SI) {
        case 1:
            readData(operand);
            break;
        case 2:
            writeData(operand);
            break;
        case 3:
            customTerminate();
            break;
    }
}

void readData(string operand) {
    ifstream fin("input.txt");
    string line;
    int start = stoi(operand);

    if (!fin) {
        cerr << "Error opening input file." << endl;
        exit(1);
    }

    while (getline(fin, line)) {
        if (line == "$DTA") {  
            int i = 0;
            while (getline(fin, line)) {
                if (line.find("$END") != string::npos) break; 

                int len = line.length();
                int chunks = (len + 3) / 4; 

                memset(buffer, 0, sizeof(buffer));
                strncpy(buffer, line.c_str(), sizeof(buffer) - 1); 

                for (int j = 0; j < chunks; j++) {
                    if (start + i >= 100) {
                        cerr << "Memory overflow while reading data." << endl;
                        exit(1);
                    }
                    memset(M[start + i], 0, 4);  
                    strncpy(M[start + i], buffer + j * 4, 4);
                    i++;
                }
            }
            break;
        }
    }
    fin.close();
    SI = 0;
}

void writeData(string operand) {
    ofstream fout("output.txt", ios::app);
    int start = stoi(operand);

    if (!fout) {
        cerr << "Error opening output file." << endl;
        exit(1);
    }

    string output;
    memset(buffer, 0, sizeof(buffer)); 
    for (int i = 0; i < 10 && (start + i) < 100; i++) {
        if (M[start + i][0] == '\0') break;  
        strncpy(buffer + (i * 4), M[start + i], 4);
    }

    output.append(buffer); 
    fout << output << endl; 
    fout.close();
    SI = 0;
}


void customTerminate() {
    ofstream fout("output.txt", ios::app);
    if (!fout) {
        cerr << "Error opening output file." << endl;
        exit(1);
    }
    fout << endl;
    fout.close();
    cout << "Program Terminated." << endl;
    SI = 0;
}
