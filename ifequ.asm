TAM: EQU 0
VAR: EQU 2
SECTION TEXT
TEST: INPUT OLD_DATA
LOAD OLD_DATA
L1: DIV DOIS
STORE  NEW_DATA
MULT DOIS
JMPP L1
IF TAM
JMP TEST
STOP
SECTION DATA ;COMENTARIO
DOIS: CONST VAR
OLD_DATA: SPACE VAR
NEW_DATA: SPACE
TMP_DATA: SPACE