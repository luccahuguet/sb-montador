#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

void primeiraPassagem(string fname);
void segundaPassagem(string fname);
void ifequ(string fname);
vector<string> splitString(string input);
string removeComments(string input);

int memory = 0;
int line_counter = 1;
string machine_code = "";
unordered_map<string, int> symbol_table;
unordered_map<string, int> equ_table;
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

bool first = true;
bool text_section = false;
bool jump_line = false;  // Operador booleano usado no pré-processamento de IF

void print_symbol_table()
{
    for (auto const &pair : symbol_table)
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

            ifequ(argv[2]);

            cout << endl;
            cout << endl;
        }
        else if (strncmp(argv[1], "-m", 2) == 0)
        {
            cout << "Processamento de Macros. Saída MCR" << endl;
        }
        else if (strncmp(argv[1], "-o", 2) == 0)
        {
            cout << "Processar tudo e criar arquivo objeto" << endl;
            primeiraPassagem(argv[2]);
            print_symbol_table();
            segundaPassagem(argv[2]);
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
        // Checar erros léxicos (Caracteres especiais ou iniciado por número)
        for(int c = 0; c < label.size(); c++)
        {
            // Checar se tem número como primeiro caractere do rótulo
            if (c == 0)
            {
                if(label[c] >= 48 && label[c] <= 57)
                {
                    cout << "Erro léxico na linha " << line_counter << ": Rótulo iniciado com número" << endl;
                    exit(1);
                }
            }
            // Checar se existe caractere especial no rótulo, exceto _
            if(
                !(
                    (label[c] >= 48 && label[c] <= 57)||
                    (label[c] >= 65 && label[c] <= 90) ||
                    (label[c] >= 97 && label[c] <= 122)
                ) && label[c] != 95
            )
            {
                cout << "Erro léxico na linha " << line_counter << ": Rótulo com caracteres especiais" << endl;
                exit(1);
            }
        }

        // Check if label already exists inside the symbol_table map
        if (symbol_table.find(label) != symbol_table.end())
        {
            cout << "Erro semântico na linha " << line_counter << ": Rótulo já existente" << endl;
            exit(1);
        }
        // Add the label to the symbol_table
        symbol_table[label] = memory;
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
        if (tokens[0] == "SPACE")
        {
            if (tokens.size() == 1)
            {
                memory += 1;
            }
            else
            {
                memory += stoi(tokens[1]);
            }
        }
        else
            memory += directive_table[tokens[0]];
    }
}

void primeiraPassagem(string fname)
{
    cout << "primeiraPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".asm";

    // opens file
    ifstream file2(fname_asm);
    string line_raw;

    cout << "while init" << endl;

    while (getline(file2, line_raw))
    {
        // separa a linha em rótulo, operação, operandos, comentários

        string line = removeComments(line_raw);
        if (line.find_first_not_of(" \t\n") != std::string::npos)
        { // cout << "after removeComments" << endl;
            cout << endl;

            // creates a symbol table
            updateSymbolTable(line);
        }
    }
    cout << "Primeira passagem FIM" << endl;
}

void generateCode(string line)
{
    // Separa elementos da linha
    vector<string> tokens = splitString(line);

    // Iterar pelos elementos da linha
    // Remover do vetor se o elemento for definição de rótulo
    if (tokens[0].back() == ':')
    {
        if(tokens.size() > 1)
        {
            if (tokens[1].back() == ':')
            {
                cout << "Erro sintático na linha " << line_counter << ": Duas definições de rótulo na mesma linha" << endl;
                exit(1);
            }
        }
        tokens.erase(tokens.begin());
    }
    // Esturura atual do vetor [Intrução, operando1, ...]

    // Consultar operação na tabela de opcodes(Erro de instrução inexistente)
    if (opcode_table.find(tokens[0]) != opcode_table.end())
    {
        // Checa número de argumentos passados
        if (tokens.size() != (opcode_table[tokens[0]][1]))
        {
            cout << "Erro sintático na linha " << line_counter << ": Número errado de argumentos" << endl;
            exit(1);
        }
        if (first)
        {
            machine_code += to_string(opcode_table[tokens[0]][0]);
            first = false;
        }
        else
            machine_code += " " + to_string(opcode_table[tokens[0]][0]);

        // Checar argumentos
        int address = 0;
        for (int i = 1; i <= opcode_table[tokens[0]][1] - 1; i++)
        {
            // Verificar se o operando está na tabela de símbolos
            if (symbol_table.find(tokens[i]) == symbol_table.end())
            {
                cout << "Erro semântico na linha " << line_counter << ": Rótulo " << tokens[i] << " não definido" << endl;
                exit(1);
            }
            else
            {
                // Verificar se tem operação. Exemplo: LOAD X+1, INPUT X+1 ... X: SPACE 2
                // if string tokens[i] contains a "+":
                if (tokens[i].find("+") != string::npos)
                {
                    // Adicionar endereço do rótulo com a operação
                    string str = "";
                    str += tokens[i][0];
                    address = symbol_table[str];
                    str = "";
                    str += tokens[i][2];
                    machine_code += " " + to_string(address + stoi(str));
                }
                else
                {
                    // Adicionar endereço do rótulo
                    machine_code += " " + to_string(symbol_table[tokens[i]]);
                }
            }
        }
    }
    else if (directive_table.find(tokens[0]) != directive_table.end())
    {
        // Se for diretiva, checar se é CONST ou SPACE
        if (tokens[0] == "CONST")
        {
            if (tokens.size() != 2)
            {
                cout << "Erro sintático na linha" << line_counter << ": Número errado de argumentos";
                exit(1);
            }
            // CONST, adicionar valor ao código
            machine_code += " " + tokens[1];
        }
        else if (tokens[0] == "SPACE")
        {
            // verificar numero de argumentos do space
            if (tokens.size() == 1)
            {
                machine_code += " 0";
            }
            else if (tokens.size() == 2)
            {
                for (int i = 0; i < stoi(tokens[1]); i++)
                {
                    machine_code += " 0";
                }
            }
            else
            {
                cout << "Erro sintático na linha" << line_counter << ": Número errado de argumentos";
                exit(1);
            }
        }
        else if (tokens[0] == "SECTION"){
            if (tokens.size() > 1)
            {
                if (tokens[1] == "TEXT") text_section = true;
            }
        }
    }
    else
    {
        cout << "Erro semântico na linha " << line_counter << ": Instrução inexistente" << endl;
        exit(1);
    }
    line_counter++;
}

void segundaPassagem(string fname)
{
    cout << "segundaPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".asm";
    line_counter = 1;
    // opens file
    ifstream file(fname_asm);
    string line_raw, line;

    while (getline(file, line_raw))
    {
        // Remover comentários
        line = removeComments(line_raw);
        generateCode(line);
    }
    if (!text_section)
    {
        cout << "Erro sintático no arquivo assembly: Seção TEXT faltando" << endl;
        exit(1);
    }
    cout << "Print machine code" << endl;
    cout << "Machine code: " << machine_code << endl;
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

/*
    Pré-processamento. A partir daqui estão implementadas as funções para pré-processamento
*/

// IF e EQU
string ifequprocessing(string line)  // Recebe a linha sem comentários, realiza o pré processamento e escreve no arquivo de saída
{
    // Split the line elements
    vector<string> tokens = splitString(line);
    int n_elements = tokens.size();
    bool foundequ = false;  // Indica se a linha vai ser escrita no novo arquivo .pre
    string pre_line = "";

    // Checa jump_line em caso de IF 0
    if(jump_line)
    {
        if(tokens[0] != "IF")
        {
            jump_line = false;
        }
        return "";
    }
    else
    {
        // Verifica se o elemento é IF
        if (tokens[0] == "IF")
        {
            if(n_elements > 1)
            {
                // Verifica valor do rótulo definido anteriormente
                if(equ_table[tokens[1]] == 0)
                {
                    jump_line = true;  // Indica o pulo de linha para a próxima
                }
            }
            return "";
        }

        // Checa se tem definição de rótulo
        if (tokens[0].back() == ':' && n_elements > 1 && tokens[1] == "EQU")
        {
            if(n_elements == 3)
            {
                // Salva valor do rótulo definido pelo EQU
                equ_table[tokens[0].substr(0, tokens[0].length() - 1)] = stoi(tokens[2]);
                return "";
            }
            else
            {
                cout << "EQU incorreto" << endl;
                exit(1);
            }
        }

        for(int i = 0; i < n_elements; i++)
        {
            if(equ_table.find(tokens[i]) != equ_table.end())
            {
                foundequ = true;
                tokens[i] = to_string(equ_table[tokens[i]]);
            }
            // Adicionar à string
            if(i == 0) pre_line += tokens[0];
            else pre_line += " " + tokens[i];
        }

        if(foundequ)  return pre_line;
        return line;
    }

}

void ifequ(string fname)
{
    cout << "Pré-processamento para IF e EQU" << endl;
    string fname_asm = static_cast<string>(fname) + ".asm";

    ifstream file(fname_asm);  // Arquivo .asm de entrada
    string line_raw, file_line;

    ofstream outfile(static_cast<string>(fname) + ".pre");  // Arquivo .pre de saída com os comentários removidos e pré-processamento de IF e EQU

    while (getline(file, line_raw))
    {
        // separa a linha em rótulo, operação, operandos, comentários
        string line = removeComments(line_raw);
        if (line.find_first_not_of(" \t\n") != std::string::npos)
        {
            // Realiza pré-processamento da linha
            file_line = ifequprocessing(line);
            if(file_line != "")
            {
                //write to file
                outfile << file_line << endl;
            }
        }
    }
}

// MACRO