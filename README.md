# FujiNet-Tests

These are integration tests which exercise firmware functionality.

1. cd into remotecmd and build it, run the resulting disk image from r2r on your RetroBattlestation
2. cd into unitrix and run unitrix.py, passing arguments to the FujiNet serial console in /dev and the list of json files of tests you want to run
3. to create tests, make a json file with the commands to execute and their paramaters (see one of the current json files for an example)
