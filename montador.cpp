#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

void primeiraPassagem(string fname);
void segundaPassagem(string fname);
vector<string> splitString(string input);
string removeComments(string input);

int memory = 0;
int line_counter = 1;
string machine_code= "";
unordered_map<string, int> table;
unordered_map<string, vector<int>> opcode_table = {
    {"ADD", {1, 2}},
    {"SUB", {2, 2}},
    {"MULT", {3, 2}},
    {"DIV", {4, 2}},
    {"JMP", {5, 2}},
    {"JMPN", {6, 2}},
    {"JMPP", {7, 2}},
    {"JMPZ", {8, 2}},
    {"COPY", {9, 3}},
    {"LOAD", {10, 2}},
    {"STORE", {11, 2}},
    {"INPUT", {12, 2}},
    {"OUTPUT", {13, 2}},
    {"STOP", {14, 1}}};

unordered_map<string, int> directive_table = {
    {"CONST", 1},
    {"SPACE", 1},
    {"SECTION", 0},
    {"EQU", 0},
    {"IF", 0},
    {"MACRO", 0},
    {"ENDMACRO", 0}};

void print_symbol_table()
{
    for (auto const &pair : table)
    {
        cout << "{" << pair.first << ": " << pair.second << "}" << endl;
    }
}

int main(int argc, char **argv)
{ // argv[0] é sempre o nome do programa
    if (argc != 3)
    {
        cout << "Número errado de argumentos" << endl;
    }
    else
    {
        if (strncmp(argv[1], "-p", 2) == 0)
        {
            cout << "Processamento de EQU e IF. Extensão de saída PRE" << endl;
            cout << endl;

            primeiraPassagem(argv[2]);

            cout << endl;
            cout << endl;
            print_symbol_table();
        }
        else if (strncmp(argv[1], "-m", 2) == 0)
        {
            cout << "Processamento de Macros. Saída MCR" << endl;
        }
        else if (strncmp(argv[1], "-o", 2) == 0)
        {
            cout << "Processar tudo e criar arquivo objeto" << endl;
        }
        else
        {
            cout << "Modo de operação não reconhecido" << endl;
        }
    }
    return 0;
}

void updateSymbolTable(string line)
{
    // Split the line elements
    vector<string> tokens = splitString(line);
    // Check if the first token is a label

    if (tokens[0].back() == ':')
    {
        // cout << "updateSymbolTable init" << endl;
        string label = tokens[0].substr(0, tokens[0].length() - 1);
        // cout << "label: " << label << endl;

        // Check if label already exists inside the table map
        if (table.find(label) != table.end())
        {
            cout << "Erro semântico na linha " << line_counter << ": Rótulo já existente" << endl;
            exit(1);
        }
        // Add the label to the symbol table
        table[label] = memory;
        tokens.erase(tokens.begin());
    }

    // print every element of the array
    for (int i = 0; i < tokens.size(); i++)
    {
        cout << tokens[i] << " ";
    }

    // Incrementar valores dos contadores
    line_counter++;

    if (opcode_table.find(tokens[0]) != opcode_table.end())
    {
        memory += opcode_table[tokens[0]][1];
    }
    else if (directive_table.find(tokens[0]) != directive_table.end())
    {
        memory += directive_table[tokens[0]];
    }
    else
    {
        cout << "Erro na linha " << line_counter << ": Instrução inexistente" << endl;
    }

    //  cout << "after loop3" << endl;
}

void primeiraPassagem(string fname)
{
    cout << "primeiraPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".asm";

    // opens file
    ifstream file(fname_asm);
    string line_raw;

    cout << "while init" << endl;

    while (getline(file, line_raw))
    {
        // separa a linha em rótulo, operação, operandos, comentários

        string line = removeComments(line_raw);

        // cout << "after removeComments" << endl;
        cout << endl;

        // creates a symbol table
        updateSymbolTable(line);
    }
}

void generateCode(string line)
{
    // Separa elementos da linha
    vector<string> tokens = splitString(line);

    // Iterar pelos elementos da linha
    for (int i = 0; i < tokens.size(); i++)
    {
        // Remover do vetor se o elemento for definição de rótulo
        if(tokens[0].back() == ':')
        {
            tokens.erase(tokens.begin());
        }
        // Esturura atual do vetor [Intrução, operando1, ...]
        if(i == 0)
        {

            // Consultar operação na tabela de opcodes(Erro de instrução inexistente)
            if (opcode_table.find(tokens[0]) != opcode_table.end())
            {
                machine_code + to_string(opcode_table[tokens[i]][0]);
            }
        }
        else{
            // Checar número de operandos
            // ...
            // Se o operando for um símbolo, consultar tabela de símbolos e substituir valor (Erro caso não encontrar)
            // ...
        }

    }
}

void segundaPassagem(string fname)
{
    cout << "segundaPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".asm";

    // opens file
    ifstream file(fname_asm);
    string line_raw, line;

    while (getline(file, line_raw))
    {
        // Remover comentários
        line = removeComments(line_raw);


    }
}

vector<string> splitString(string input)
{
    vector<string> tokens;
    // Split the string on spaces, commas, and semicolons
    string delimiters = " ,";
    size_t pos = input.find_first_of(delimiters);
    while (pos != string::npos)
    {
        // Add the token to the vector
        tokens.push_back(input.substr(0, pos));

        // Remove extra spaces, line breaks, tabs:
        while (input[pos + 1] == ' ' || input[pos + 1] == '\t' || input[pos + 1] == '\n')
        {
            input.erase(pos + 1, 1);
        }

        // Remove the token from the original string
        input.erase(0, pos + 1);
        // Find the next token
        pos = input.find_first_of(delimiters);
    }
    // Add the remaining string to the vector (if any)
    if (!input.empty())
    {
        tokens.push_back(input);
    }
    return tokens;
}

string removeComments(string input)
{
    string result;
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == ';')
        {
            break;
        }
        else
        {
            result += input[i];
        }
    }
    return result;
}