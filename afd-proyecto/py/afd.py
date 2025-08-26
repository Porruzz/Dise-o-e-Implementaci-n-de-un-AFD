# -*- coding: utf-8 -*-
"""
AFD en Python.
Lee Conf.txt (definición del autómata) y Cadenas.txt (pruebas).
Uso:
    python py/afd.py tests/Conf.txt tests/Cadenas.txt
Si no das argumentos, usa esos por defecto.
Autor: estudiante 🇨🇴
"""

import sys
import re
from pathlib import Path

# -----------------------------
# Utilidades de parsing
# -----------------------------

def _clean_line(s: str) -> str:
    """Quita comentario (#...) y espacios alrededor."""
    s = s.split('#', 1)[0]
    return s.strip()

def _split_csv(s: str):
    s = s.strip()
    if not s:
        return []
    return [x.strip() for x in s.split(',') if x.strip()]

# -----------------------------
# Cargar configuración de AFD
# -----------------------------

def cargar_afd(ruta_conf: Path):
    """
    Formato:
      states: q0,q1,q2
      alphabet: 0,1
      start: q0
      accepts: q1
      transitions:
      q0,0->q1
      q1,1->q0
      ...
    """
    states = []
    alphabet = []
    start = None
    accepts = set()
    transitions = {}  # (src, sym) -> dst
    en_trans = False

    with open(ruta_conf, encoding="utf-8") as fh:
        for raw in fh:
            line = _clean_line(raw)
            if not line:
                continue

            if not en_trans:
                low = line.lower()
                if low.startswith("states:"):
                    states = _split_csv(line.split(':',1)[1])
                elif low.startswith("alphabet:"):
                    alphabet = _split_csv(line.split(':',1)[1])
                    # validación simple: símbolos 1 carácter
                    for a in alphabet:
                        if len(a) != 1:
                            raise ValueError(f"Símbolo no permitido (solo 1 char): {a}")
                elif low.startswith("start:"):
                    start = line.split(':',1)[1].strip()
                elif low.startswith("accepts:"):
                    accepts = set(_split_csv(line.split(':',1)[1]))
                elif low.startswith("transitions:"):
                    en_trans = True
                else:
                    raise ValueError(f"Línea no reconocida en config: {line}")
            else:
                # línea de transición: qX,s -> qY  (espacios opcionales)
                # quitamos espacios para facilitar
                l = re.sub(r"\s+", "", line)
                m = re.match(r"([^,]+),(.+)->(.+)$", l)
                if not m:
                    raise ValueError(f"Transición inválida: {line}")
                src, sym, dst = m.group(1), m.group(2), m.group(3)
                if sym not in alphabet:
                    raise ValueError(f"Símbolo {sym} no está en el alfabeto")
                if src not in states or dst not in states:
                    raise ValueError(f"Estado desconocido en transición: {src} o {dst}")
                transitions[(src, sym)] = dst

    if not states or not alphabet or start is None:
        raise ValueError("Config incompleta: faltan states/alphabet/start.")
    if start not in states:
        raise ValueError("El estado start no está en 'states'.")
    if not accepts.issubset(set(states)):
        raise ValueError("Algún estado en 'accepts' no existe.")

    afd = {
        "states": states,
        "alphabet": alphabet,
        "start": start,
        "accepts": accepts,
        "transitions": transitions
    }
    return afd

# -----------------------------
# Simular una cadena
# -----------------------------

def simular(afd, cadena: str) -> bool:
    curr = afd["start"]
    for ch in cadena:
        if ch not in afd["alphabet"]:
            return False
        key = (curr, ch)
        if key not in afd["transitions"]:
            return False
        curr = afd["transitions"][key]
    return curr in afd["accepts"]

# -----------------------------
# Main
# -----------------------------

def main():
    root = Path(__file__).resolve().parents[1]
    conf = Path(sys.argv[1]) if len(sys.argv) > 1 else (root / "tests" / "Conf.txt")
    cad  = Path(sys.argv[2]) if len(sys.argv) > 2 else (root / "tests" / "Cadenas.txt")

    afd = cargar_afd(conf)

    # Leemos todas las líneas, incluyendo vacías (cadena ε)
    with open(cad, encoding="utf-8") as fh:
        lineas = fh.read().splitlines()

    for s in lineas:
        ok = simular(afd, s)
        visible = s if s != "" else "ε"
        print(f"{visible} -> {'acepta' if ok else 'NO acepta'}")

if __name__ == "__main__":
    main()
