#!/usr/bin/env python3
"""Generator plików testowych dla programów entropy_*.

Użycie:
  python3 tools/gen_data.py [--out data] [--seed 1]             # zestaw testowy
  python3 tools/gen_data.py [--out data] [--seed 1] --size N    # losowy_N.bin (N bajtów)
"""
import argparse
import os
import random

SIZE = 64 * 1024          # rozmiar podstawowych plików (64 KiB)
CHUNK = 1024 * 1024       # zapis dużych plików po 1 MiB
TEXT_ALPHABET = "abcdefghijklmnopqrstuvwxyz     "  # litery i spacje

# segmenty.bin: (rodzaj, długość); granice 10000, 25000, 45000 nie są wielokrotnościami 4096
SEGMENTS = [("tekst", 10000), ("zera", 15000), ("losowe", 20000), ("tekst", 20536)]

DEFAULT_SEED = 1


def text(rng, n):
    return "".join(rng.choices(TEXT_ALPHABET, k=n)).encode("ascii")


def segment(rng, kind, n):
    if kind == "tekst":
        return text(rng, n)
    if kind == "zera":
        return bytes(n)
    return rng.randbytes(n)


def write(path, data):
    with open(path, "wb") as f:
        f.write(data)


def main():
    print("Generator plików testowych dla programów entropy_*...")

    p = argparse.ArgumentParser()
    p.add_argument("--out", default="data")
    p.add_argument("--seed", type=int, default=DEFAULT_SEED)
    p.add_argument("--size", type=int, help="wygeneruj tylko losowy_N.bin o N bajtach")
    a = p.parse_args()

    os.makedirs(a.out, exist_ok=True)
    rng = random.Random(a.seed)
    with open(os.path.join(a.out, "ziarno.txt"), "w") as f:
        f.write(f"{a.seed}\n")

    if a.size is not None:
        with open(os.path.join(a.out, f"losowy_{a.size}.bin"), "wb") as f:
            left = a.size
            while left > 0:
                n = min(CHUNK, left)
                f.write(rng.randbytes(n))
                left -= n
        return

    write(os.path.join(a.out, "stale.bin"), bytes(SIZE))
    write(os.path.join(a.out, "wzorzec16.bin"), bytes(range(16)) * (SIZE // 16))
    write(os.path.join(a.out, "losowy.bin"), rng.randbytes(SIZE))
    write(os.path.join(a.out, "pusty.bin"), b"")
    write(os.path.join(a.out, "nierowny.bin"), rng.randbytes(SIZE + 1000))

    data = b""
    rows = ["pozycja,segment_przed,segment_po"]
    for i, (kind, n) in enumerate(SEGMENTS):
        if i > 0:
            rows.append(f"{len(data)},{SEGMENTS[i - 1][0]},{kind}")
        data += segment(rng, kind, n)
    write(os.path.join(a.out, "segmenty.bin"), data)
    with open(os.path.join(a.out, "segmenty_granice.csv"), "w") as f:
        f.write("\n".join(rows) + "\n")


if __name__ == "__main__":
    main()
