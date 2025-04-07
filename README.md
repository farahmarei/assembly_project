# csce-2303-s25-project-1-group-9
csce-2303-s25-project-1-group-9 created by GitHub Classroom

Throughout the development and testing of the Z16 simulator, we faced a several problems with execution. One of the primary challenges involved the simulator not receiving the required .bin file as a command-line argument, which caused it to terminate immediately with a usage error. 
This was resolved by correctly configuring the run settings in the CLion IDE to pass the file path as a program argument. Another issue emerged when the simulator failed to open the binary file due to an incorrect file path or the file being located in a different directory from the executable. This was addressed by either placing the file in the correct directory (cmake-build-debug) or specifying the full file path. 
Once the simulator was able to load the file, it encountered repeated unknown instruction errors and fell into an infinite loop. This was caused by the test binary containing 0x0000 instructions, which were misinterpreted as valid but unimplemented R-type instructions. Since these instructions did not alter the program counter or halt execution, the simulator kept processing the same or meaningless instructions endlessly. To resolve this, we are adding logic to recognize 0x0000 as a special no-operation or halt instruction to safely terminate execution. 
Additionally, we created a new test binary file with meaningful system calls (ecall instructions) and manually set the required register values (such as a0) in the code to ensure the simulator behaves as expected. These steps have helped us move from initial file-handling and setup problems toward meaningful instruction processing and simulation.







