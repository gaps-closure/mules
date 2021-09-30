import sys
import pickle

def main(oname, inlist):
    all = {}#obj by 'cle-label'
    for inf in inlist:
        with open(inf + ".offset.pkl","rb") as f:
            in_one = pickle.load(f)
            for dstline in in_one:
                all[dstline] = in_one[dstline]
    #print(json.dumps(list(all.values()), indent=2))
    with open(oname, "wb") as of:
        pickle.dump(all)
            
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Combining pickle dictionaries")
        exit()
    main(sys.argv[1], sys.argv[2:])