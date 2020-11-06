# CLE Schema

The json schema for the CLE json used by GAPS-Closure

## Requirements

- python jsonschema 3.2.0 or newer (install via pip) (MIT licensed)

## Json Schema Version

This schema utalizes json-schema draft 7

At the time of schema creation a newer draft exists, however most tooling is not yet upgraded to the newer draft, including the jsonschema python module, and the vscode plugin, while draft 7 is supported by a large portion of the other json schema tools, validators, ect.

## CLE Schema containts

- cle-schema.json - The Schema for the cle, consideres single CLE entries, as well as a full clemap.json (List of labled cle entires) valid
- json-schema-draft7.json - the metaschema to validate a json schema, used by the test scripts (used by tests scripts, not needed for the pre-processor)
- run-tests.sh - Run various unit tests on the schema (json files in tests-negative will fail, and json files in tests-valid should succeed, a tests.audit file contains the full output)
- validate-cle.py - validates a provided json (via commandline) file with the cle schema, also double validates the schema is valid. (used by run-tests.sh, and provides sample Python code for validating json information)
