#include <iostream>
#include <string.h> 
using namespace std;

void primeiraPassagem();
void segundaPassagem();

int main(int argc, char **argv) { // argv[0] é sempre o nome do programa
    if(argc != 3)
    {
        cout << "Número errado de argumentos" << endl;
    }
    else
    {
        if(strncmp(argv[1], "-p", 2) == 0)
        {
            cout << "Processamento de EQU e IF. Extensão de saída PRE" << endl;
        }
        else if(strncmp(argv[1], "-m", 2) == 0)
        {
            cout << "Processamento de Macros. Saída MCR" << endl;
        }
        else if(strncmp(argv[1], "-o", 2) == 0)
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
