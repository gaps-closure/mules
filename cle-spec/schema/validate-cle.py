#!/usr/bin/env python3
"""
Test the cle schema, and a test cle json is valid using
json schema Draft 7

This requires MIT licensed jsonschema library 3.2.0 or newer (via pip)
For more information on jsonschema see:

github.com/Julian/jsonschema/

Author: Terrence Ezrol tezrol@perspectalabs.com
"""

import jsonschema
import json
import sys
import os.path

#the schema to json-schema draft 7
metaschema_file = "json-schema-draft7.json"
#the cle schema
cleschema_file = "cle-schema.json"

def validated_schema():
    """validates the cle schema against the meta schema
       thus is to split the exception in editing the cle schema, as the schema is validated
       in standard validation (thus this step)
    """
    if(jsonschema.__version__ < "3.2.0"):
        print("Please upgrade jsonschema, version 3.2.0 or newer is required")
        raise(ModuleNotFoundError("Newer jsonschema version required"))

    try:
        meta = json.loads(
            open(metaschema_file,"r",encoding="UTF-8").read()
        )
    except Exception as e:
        print("Error reading meta schema %s"%(metaschema_file))
        raise(e)
    
    try:
        cle = json.loads(
            open(cleschema_file,"r",encoding="UTF-8").read()
        )
    except Exception as e:
        print("Error reading cle schema %s"%(cleschema_file))
        raise(e)

    #schema loaded, validate and return schema
    jsonschema.validate(cle,meta)
    return(cle)

def validate_cle(clefile,schema):
    """ given in the cle json, and the schema json validate the json with the json """
    try:
        test = json.loads(
            open(clefile,"r",encoding="UTF-8").read()
        )
    except Exception as e:
        print("Error provided cle json %s"%(clefile))
        raise(e)
    
    jsonschema.validate(test,schema)

def main():
    """Run a test on the provided file sys.argv[1]"""
    argc = len(sys.argv)

    if(argc != 2 or sys.argv[1] == "--help"):
        print("Usage: %s <cle.json>"%(sys.argv[0]))
        return(-1)

    testfile = sys.argv[1]
    
    if(not os.path.isfile(testfile)):
        print("===== Test json (%s) not found/is not a file ====="%(testfile))
        return(-4)

    try:
        schema = validated_schema()
    except Exception as e:
        print("===== Invalid CLE Schema =====")
        print(str(e))
        return(-2)

    try:
        validate_cle(testfile,schema)
    except Exception as e:
        print("==== Invalid CLE ====")
        print(str(e))
        return(-3)

    print("CLE successfully validated")
    return(0)


if(__name__ == "__main__"):
    sys.exit(main())
