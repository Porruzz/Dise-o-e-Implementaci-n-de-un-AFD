# AFD en C y Python (Windows/Linux)

## Formato Conf.txt
states: q0,q1,q2
alphabet: 0,1
start: q0
accepts: q1
transitions:
q0,0->q0
q0,1->q1
q1,0->q1
q1,1->q0

## Cadenas.txt
<una cadena por línea; línea vacía = ε>

## Windows (VS Code)
- Terminal → Run Task… → **C: run AFD (tests)**
- Terminal → Run Task… → **Python: run AFD (tests)**

## Linux
cd c && make run
o
python py/afd.py tests/Conf.txt tests/Cadenas.txt
