## MIMO-FUN project

MIMO-FUN stands for Multiple-Input and Multiple-Output Joint Fountain Coding and Network Coding.

The project consists of 3 parts: **terminal**, **edge** and **backbone**. 

It uses some open-source libraries: KODO, Eigen and PracticalSocket.

* ./src/ includes the source code for terminal, edge and backbone.

* ./build/ includes the KODO libraries.

* ./include/ includes the headers of KODO and Eigen.

* ./scenarios/ includes shell scripts, configure files and topology files for some examples. The '.imn' configure files are used in CORE network emulator.

* ./stat/ includes the running time results of each terminal node in each scenario.

