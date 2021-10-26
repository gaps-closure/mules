#! /usr/bin/env python3
import argparse
from dataclasses import dataclass
from shutil import copyfile
from pathlib import Path
import build

def install_xdcc_gen(out: Path) -> None:
    path = Path('xdcc_gen')
    out_bin = out / 'bin' 
    out_bin.mkdir(parents=True, exist_ok=True)
    copyfile(path / 'xdcc_gen', out_bin / 'xdcc_gen')

def install_schema(out: Path) -> None:
    path = Path('cle-spec/schema')
    out_schemas = out / 'schemas'
    out_schemas.mkdir(parents=True, exist_ok=True)
    copyfile(path / 'cle-schema.json', out_schemas / 'cle-schema.json')

def install_schema_gen(out: Path) -> None:
    path = Path('.')
    out_bin = out / 'bin' 
    out_bin.mkdir(parents=True, exist_ok=True)
    copyfile(path / 'schema_gen.sh', out_bin / 'schema_gen.sh')


@dataclass
class Args:
    output: Path

def main() -> None: 
    parser = argparse.ArgumentParser('install.py') 
    parser.add_argument('--output', '-o', default=False, help="Output directory", type=Path, required=True)
    args = parser.parse_args(namespace=Args)
    args.output.mkdir(parents=True, exist_ok=True)
    build.build()
    install_xdcc_gen(args.output)
    install_schema(args.output)
    install_schema_gen(args.output)
    
if __name__ == '__main__':
    main()