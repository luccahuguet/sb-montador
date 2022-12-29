# sb-montador
Repositório para o trabalho 1 da disciplina de Software Básico 2022.2, UnB

## Integrantes
**Marcelo Aiache Postiglione - 180126652**

**Lucca Beserra Huguet - 160013259**

## Sistema operacional utilizado
Kali GNU/Linux Rolling          
Kernel: Linux 5.17.0-kali3-amd64

## Versão do C++
**C++17**

## Instruções de compilação
```
$ gcc montador.cpp -lstdc++ -o montador 
```

## Rodando o programa
Conforme solicitado na especificação o programa tem três modos de operação e leva como argumento o nome do arquivo sem a extensão.

### Pré-processamento de IF e EQU
O modo de operação para pré-processamento de IF e EQU é chamado pela flag "-p" e recebe como argumento o nome de um arquivo de extensão .asm. Por exemplo, para executar esse modo de operação para um arquivo chamado "exemplo.asm" que esteja presente no mesmo diretório de onde está sendo chamado o programa montador, digite o seguinte comando:

```
$ ./montador -p exemplo
```

Esse modo irá gerar um arquivo com mesmo nome só que com extensão ".pre". Para este exemplo o arquivo de saída gerado vai ser "exemplo.pre".

Obervação importante: Para o bom funcionamento das próximas funcionalidades, principalmente para a geração do código objeto, é necessária a passagem por esse modo, pois aqui são retirados os cometários e o tratamento para não ser case sensitive.

### Processamento de MACROS
O modo de operação para processamento de MACROS é chamado pela flag "-m" e recebe como argumento o nome de um arquivo de extensão .pre, gerado pelo modo de IF e EQU. Por exemplo, para executar esse modo de operação para um arquivo chamado "exemplo.pre" que esteja presente no mesmo diretório de onde está sendo chamado o programa montador, digite o seguinte comando:

```
$ ./montador -m exemplo
```

Esse modo irá gerar um arquivo com mesmo nome só que com a extensão ".mcr". Para este exemplo o arquivo de saída gerado vai ser "exemplo.mcr".

### Primeira e segunda passagens e geração do código objeto
O modo de operação para geração de código objeto é chamado pela flag "-o" e recebe como argumento o nome de um arquivo de extensão .mcr, gerado pelo modo de processamento de MACROS. Por exemplo, para executar esse modo de operação para um arquivo chamado "exemplo.mcr" que esteja presente no mesmo diretório de onde está sendo chamado o programa montador, digite o seguinte comando:

```
$ ./montador -o exemplo
```

Esse modo irá gerar um arquivo com mesmo nome só que com a extensão ".obj". Para este exemplo o arquivo de saída gerado vai ser "exemplo.obj".
