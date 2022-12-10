#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

void primeiraPassagem(string fname);
void segundaPassagem();
vector<string> splitString(string input);
string removeComments(string input);

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

void primeiraPassagem(string fname)
{
    string fname_asm = static_cast<string>(fname) + ".asm";

    // opens file
    ifstream file(fname_asm);
    string line;
    while (getline(file, line))
    {
        // separa a linha em rótulo, operação, operandos, comentários

        string line2 = removeComments(line);
        vector token = splitString(line2);

        for (int i = 0; i < token.size(); i++)
        {
            cout << token[i] << " ";
        }
        cout << endl;
        // cout << line << endl;
    }
}

vector<string> splitString(string input)
{
    vector<string> tokens;
    // Split the string on spaces, commas, and semicolons
    string delimiters = " ,;";
    size_t pos = input.find_first_of(delimiters);
    while (pos != string::npos)
    {
        // Add the token to the vector
        tokens.push_back(input.substr(0, pos));

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