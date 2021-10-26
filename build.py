#! /usr/bin/env python3
import argparse
import subprocess
from dataclasses import dataclass
from pathlib import Path


def build_xdcc_gen() -> None:
    cwd = Path('xdcc_gen')
    subprocess.check_call(['make', '-j', '8'], cwd=cwd)



def clean_xdcc_gen() -> None:
    cwd = Path('xdcc_gen')
    subprocess.check_call(['make', 'clean'], cwd=cwd)


@dataclass
class Args:
    clean: bool

def build() -> None:
    build_xdcc_gen()

def clean() -> None:
    clean_xdcc_gen()

def main() -> None: 
    parser = argparse.ArgumentParser('build.py') 
    parser.add_argument('--clean', '-c', action='store_true', default=False)
    args = parser.parse_args(namespace=Args)
    if args.clean:
        clean()
    else:
        build()

if __name__ == '__main__':
    main()