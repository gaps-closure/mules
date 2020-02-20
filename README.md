# Multi-Level Security Source Tools (MULES)
This repository hosts the open source components of MULES. The `master` branch contains the most recent public release software while `develop` contains bleeding-edge updates and work-in-progress features for use by beta testers and early adopters.

This repository is maintained by Perspecta Labs.

## Contents
- [Build](https://github.com/gaps-closure/mules/blob/develop/README.md#building
- [Installing VS Code Extensions](https://github.com/gaps-closure/mules/blob/develop/README.md#install_extensions
- [Runing the Partitioner](https://github.com/gaps-closure/mules/blob/develop/README.md#running_the_partitioner
- [Troubleshooting](https://github.com/gaps-closure/mules/blob/develop/README.md#troubleshooting

## Installation
MULES has been developed, deployed, and tested using Ubuntu 19.10 x86_64 Linux. We recommend this distribution to simplify installation of external dependencies. Upon cloning the MULES repository, follow these steps to install required packages (assumes sudo permissions enabled for calling `apt`):

```
./build.sh 
```

This script downloads, installs or builds the following
* LLVM binaries
* Visual Studio Code and its Extension Manager
* CLE VS Code extensions
* pdg
* quala
* partitioner

## Installing VS Code Extensions
The MULES IDE is based on Visual Studio Code (vscode) with extensions. Two CLE extensions, CLE-Highlighter-0.0.1.vsix and CLE-themes-0.0.1.vsix, are available in the $MULES/build directory, if vscode is installed and the CLE extensions built successfullly in the Build step. In addition, many more extensions are available over the internet. We will install two extensions from the internet and two from the local build directory. Do the following to start VS Code then follow the instructions below to install extensions.

```
cd $MULES/partitioner
code .
```

Install the C/C++ extension
* Select the bottom icon (four squares) on the left task bar
* Type c/c++ into the search box.
* Find the extension 'C/C++ IntelliSense, debugging, and code browsing' and click on its green Install button on the lower right.

Install .dot viewer extension
* Select the bottom icon (four squares) on the left task bar.
* Type dot into the search box.
* Find the extension "Graphviz (dot) language support for Visual Studio Code" and click on its green Install button on the lower right. Note that there are two extensions with the same name. Choose the one by Joao Pinto.

Locally built extensions have to be sideloaded as follows.
* Select the bottom icon (four squares) on the left task bar.
* Click the ... button at the top-right corner of the Extensions pane and select "Install from VSIX..." on the menu that appears.
* Locate the CLE-Highlighter-0.0.1.vsix file in the $MULES/build directory and click "Install".
* Locate the CLE-themes-0.0.1.vsix file in the $MULES/build directory and click "Install".

** Syntax Highlighting for CLE
MULES' syntax highlighter are derived from Reloaded-cpp and Reloaded-themese. To see that it is functioning correctly, start VS Code as follows, if it is not already running.

```
cd $MULES/partitioner
code .
```

* Select the top icon (two pieces of paper) on the left task bar.
* Expand the example directory
* Double click on the ex1.c file to open it in the editor panel.
* Look for lines starting with #pragma cle, each of the two words should be highlighted with a different color.

** Runing the Partitioner
Start VS Code as follows, if it is not already running.

```
cd $MULES/partitioner
code .
```

The following tasks are defined in the partitioner project.
        Partitioner Compile,
        Partitioner Example Clang Build,
        Partitioner Preprocess,
        Partitioner LLVM IR and Bitcode Gen,
        Partitioner Create Dependency Graph,
        Partitioner Partition,
Each task depends on its immediate predecessor. All predecessors of a task will be run before the task itself is run.
To run all tasks, choose the Partiion task. The "Partitioner Clean" task cleans up all intermediate files.

To run a task,
* Select Run Task from the Terminal menu.
* A list of Partitioner tasks will be displayed.
* Select the desired task from the list.
* Select 'Continue with scanning the task output'

** Troubleshooting
Do the following if the backspace key is not working in vnc.
* File>Preferences>Settings
* Type Keyboard into the search box.
* Select keyboard
* Change the value to keyCode

