#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <ctype.h>

using namespace std;

// Macro Class
class Macro {  
  public:            
    string name;
    int n_arguments;
    vector<pair<string, string>> arg_map;
    vector<string> lines;
};

void primeiraPassagem(string fname);
void segundaPassagem(string fname);
void ifequ(string fname);
void macro(string fname);
string macroProcessing(string line);
string writeMacro(vector<string> elements, Macro macroobj);
vector<string> splitString(string input);
string removeComments(string input);

int memory = 0;
int line_counter = 1;
string machine_code = "";
unordered_map<string, int> symbol_table;
unordered_map<string, int> equ_table;
unordered_map<string, vector<int>> opcode_table = {
    {"ADD",    {1, 2  }},
    {"SUB",    {2, 2  }},
    {"MULT",   {3, 2  }},
    {"MUL",    {3, 2  }},
    {"DIV",    {4, 2  }},
    {"JMP",    {5, 2  }},
    {"JMPN",   {6, 2  }},
    {"JMPP",   {7, 2  }},
    {"JMPZ",   {8, 2  }},
    {"COPY",   {9, 3  }},
    {"LOAD",   {10, 2 }},
    {"STORE",  {11, 2 }},
    {"INPUT",  {12, 2 }},
    {"OUTPUT", {13, 2 }},
    {"STOP",   {14, 1 }}
};

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
bool insideMacro = false;  // Variável para saber se está dentro de macro na hora que estiver salvando
bool defined = false; // Saber em qual macro salvar
bool symbol_break = false;

Macro macro1, macro2;


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
            macro(argv[2]);
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

void updateSymbolTable(string line, string label = "")
{
    // Split the line elements
    vector<string> tokens = splitString(line);
    bool only_label = label != "";
    // Check if the first token is a label

    if (tokens[0].back() == ':') // Se o primeiro elemento da linha é um rótulo
    {
        // cout << "updateSymbolTable init" << endl;
        if (!only_label)
        {
            label = tokens[0].substr(0, tokens[0].length() - 1);
            if(symbol_break == true) // Checar no caso de enter após o rótulo se a linha se inicia com outro rótulo
            {
                cout << "Erro sintático na linha " << line_counter << ": Duas definições de rótulo na mesma linha" << endl;
                exit(1);
            }
        }
        // Checar erros léxicos (Caracteres especiais ou iniciado por número)

        for (int c = 0; c < label.size(); c++)
        {
            // Checar se tem número como primeiro caractere do rótulo
            if (c == 0)
            {
                if (label[c] >= 48 && label[c] <= 57)
                {
                    cout << "Erro léxico na linha " << line_counter << ": Rótulo iniciado com número" << endl;
                    exit(1);
                }
            }
            // Checar se existe caractere especial no rótulo, exceto _
            if (
                !(
                    (label[c] >= 48 && label[c] <= 57) ||
                    (label[c] >= 65 && label[c] <= 90) ||
                    (label[c] >= 97 && label[c] <= 122)) &&
                label[c] != 95)
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
    else
    {
        if(symbol_break) symbol_break = false;
    }

    // Incrementar valores dos contadores
    line_counter++;
    if (!only_label)
    {
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
}

void primeiraPassagem(string fname)
{
    cout << "primeiraPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".mcr";

    // opens file
    ifstream file2(fname_asm);
    string line_raw;

    while (getline(file2, line_raw))
    {
        if (line_raw.find_first_not_of(" \t\n") != std::string::npos)
        {
            // checks if the last non space character is a colon and if there is a token before it
            if (line_raw.find_last_not_of(" \t\n") == line_raw.find(':') && line_raw.find(':') != 0)
            {
                symbol_break = true;
                // creates a symbol table
                updateSymbolTable(line_raw, line_raw.substr(0, line_raw.find(':')));
            }
            else
            {
                // creates a symbol table
                updateSymbolTable(line_raw);
            }
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
        if (tokens.size() > 1 && tokens[1].back() == ':')
        {
            cout << "Erro sintático na linha " << line_counter << ": Duas definições de rótulo na mesma linha" << endl;
            exit(1);
        }
        tokens.erase(tokens.begin());
    }
    // Esturura atual do vetor [Intrução, operando1, ...]
    if(tokens.size() > 0)  // Considerar caso de apenas rótulo na linha
    {    
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
                // Checar se é hexadecimal e converter para decimal
                if(tokens[1][0] == '0' && (tokens[1][1] == 'X' || tokens[1][1] == 'x'))
                {
                    tokens[1] = to_string(stoi(tokens[1].substr(2, tokens[1].length() - 1), nullptr, 16));
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
            else if (tokens[0] == "SECTION")
            {
                if (tokens.size() > 1)
                {
                    if (tokens[1] == "TEXT")
                        text_section = true;
                }
            }
        }
        else
        {
            cout << "Erro semântico na linha " << line_counter << ": Instrução inexistente" << endl;
            exit(1);
        }
    }
    line_counter++;
}

void segundaPassagem(string fname)
{
    cout << "segundaPassagem init" << endl;
    string fname_asm = static_cast<string>(fname) + ".mcr";
    line_counter = 1;
    // opens file
    ifstream file(fname_asm);
    string line_raw;

    while (getline(file, line_raw))
    {
        if (line_raw.find_first_not_of(" \t\n") != std::string::npos)
        {
            generateCode(line_raw);
        }
    }
    if (!text_section)
    {
        cout << "Erro sintático no arquivo assembly: Seção TEXT faltando" << endl;
        exit(1);
    }
    cout << "Print machine code" << endl;
    cout << "Machine code: " << machine_code << endl;
    ofstream outfile(static_cast<string>(fname) + ".obj");
    outfile << machine_code;
    cout << "Arquivo objeto gerado" << endl;
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
            result += toupper(input[i]);  // Já adiciona convertendo para maiúsculo. Feito para atender a especificação de não ser case sensitive
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

// Chama as funções relacionadas ao processamento de macros e escreve o novo arquivo
void macro(string fname)
{
    cout << "Processamento de macros" << endl;
    string fname_pre = static_cast<string>(fname) + ".pre";  // Abre o arquivo com extensão .pre

    ifstream file(fname_pre);  // Arquivo .asm de entrada
    string line_raw, file_line;

    ofstream outfile(static_cast<string>(fname) + ".mcr");  // Arquivo de saída mcr

    while (getline(file, line_raw))
    {
        // separa a linha em rótulo, operação, operandos, comentários
        string line = removeComments(line_raw);
        if (line.find_first_not_of(" \t\n") != std::string::npos)
        {
            // Realiza pré-processamento da linha
            file_line = macroProcessing(line);
            if(file_line != "")
            {
                // cout << "LINE: " << file_line << endl;
                // Write to file
                outfile << file_line;
            }
        }
    }
}

// Processa a linha a procura de definição ou chamada de macro
string macroProcessing(string line)
{
    // Split the line elements
    vector<string> tokens = splitString(line);
    int n_elements = tokens.size();
    string pre_line = "";
    pair<string, string> argument;
    if(insideMacro)  // Está no modo de salvar definição de macro
    {
        if(tokens[0] == "ENDMACRO")
        {
            insideMacro = false;
            defined = true;
        }
        else
        {
            if(!defined) macro1.lines.push_back(line);
            else macro2.lines.push_back(line);
        }
    }
    else  // Não está no modo de salvar macro
    {
        if(tokens[0].back() == ':' && n_elements > 1 && tokens[1] == "MACRO")  // Checa definição de macro
        {
            // Indicar entrada em uma Macro
            insideMacro = true;
            if(!defined)
            {
                macro1.name = tokens[0].substr(0, tokens[0].length() - 1);
                macro1.n_arguments = n_elements - 2;
                for(int i = 2; i < tokens.size(); i++)  // Salva o nome dos argumentos definidos
                {
                    argument.first = tokens[i];
                    macro1.arg_map.push_back(argument);
                }
            }
            else
            {
                macro2.name = tokens[0].substr(0, tokens[0].length() - 1);
                macro2.n_arguments = n_elements - 2;
                for(int i = 2; i < tokens.size(); i++)  // Salva o nome dos argumentos definidos
                {
                    argument.first = tokens[i];
                    macro2.arg_map.push_back(argument);
                }
            }
        }
        else if(tokens[0] == macro1.name)  // Checar se é nome de macro
        {
            pre_line = writeMacro(tokens, macro1);
        }
        else if (tokens[0] == macro2.name)
        {
            pre_line = writeMacro(tokens, macro2);
        }
        else  // Linha que não precisa de tratamento, apenas escrever no novo arquivo
        {
            pre_line = line + "\n";
        }
    }
    return pre_line;
}

// Quando uma macro for chamada, realiza a preparação das instruções substituindo os argumentos quando necessário
string writeMacro(vector<string> elements, Macro macroobj)
{
    bool foundarg = false;  // Indica se foi achado argumento e se precisa de substituição
    string instructions = "";  // String com o conteúdo da macro que será formatado
    vector<string> instruction_elements;
    int line_size = 0;
    // Monta o arg_map
    for(int i = 1; i < elements.size(); i++)
    {
        macroobj.arg_map[i-1].second = elements[i];  // Realiza o mapeamento para os argumentos definidos anteriormente
    }
    // Tratamento das instruções
    for(int i = 0; i < macroobj.lines.size(); i++)
    {
        instruction_elements = splitString(macroobj.lines[i]);
        line_size = instruction_elements.size();
        for(int j = 0; j < line_size; j++)
        {
            for(int a = 0; a < macroobj.arg_map.size(); a++)
            {
                if(instruction_elements[j] == macroobj.arg_map[a].first)
                {
                    if(!foundarg) foundarg = true;
                    // Substituir argumento
                    instruction_elements[j] = macroobj.arg_map[a].second;
                    break;
                }
            }
        }

        // Caso seja chamada de macro dentro da macro
        if(instruction_elements[0] == macro1.name)
        {
            instructions += writeMacro(instruction_elements, macro1);
        }  
        else if(instruction_elements[0] == macro2.name)
        {
            instructions += writeMacro(instruction_elements, macro2);
        }
        else{
            // Adicionar a linha a string final
            if(foundarg)  // Ocorreu alteração
            {
                foundarg = false;
                for(int e = 0; e < line_size; e++)
                {
                    if(instruction_elements[e] == "COPY")
                    {
                        if(e != 0) instructions += " ";
                        if(line_size >= 3)
                        {
                            instructions += instruction_elements[e] + " " + instruction_elements[e+1] + "," + instruction_elements[e+2] + "\n";
                        }
                        else
                        {
                            if(e != line_size - 1)
                            {
                                instructions += instruction_elements[e] + " " + instruction_elements[e+1] + "\n";
                            }
                            else
                                {
                                    instructions += instruction_elements[e] + "\n";
                                }
                        }
                        break;
                    }
                    else
                    {
                        if(e != 0) instructions += " ";
                        instructions += instruction_elements[e];
                        if(e == line_size - 1) instructions += "\n";
                    }
                }
            }
            else  // Não teve substituição
            {
                instructions += macroobj.lines[i] + "\n";
            }
        }
    }

    return instructions;
}
