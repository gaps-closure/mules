import sys
import pickle

def main(oname, inlist):
    all = {}
    def to_dict(fn):
        with open(fn, 'rb') as f:
            return pickle.load(f)
    dicts = [ to_dict(f) for f in inlist ]
    for d in dicts:
        all.update(d)
    with open(oname, "wb") as of:
        pickle.dump(all, of)
            
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Combining pickle dictionaries")
        exit()
    main(sys.argv[1], sys.argv[2:])