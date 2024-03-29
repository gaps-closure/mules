import sys
import json

def main(oname, inlist):
    all = {}
    for inf in inlist:
        with open(inf) as f:
            in_one = json.load(f)
            for obj in in_one:
                all[obj['cle-label']] = obj
    with open(oname, "w") as of:
        json.dump(list(all.values()), of, indent=2)
            
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Output name and at least one input is necessary")
        exit()
    main(sys.argv[1], sys.argv[2:])