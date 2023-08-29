# FileManagementLibrary
***
This project implements simple file management library including unit and integration tests. It allows user to call system function from library level. It allows:
1. Creating new file
2. Changing file perms
3. Renaming files
4. Removing files
5. Opening files with certain flags (rw)
6. Reading files content
7. Writing to files
8. Changing the cursor position in files
9. Closing files

Library returns the error code analogically to system functions additionaly it and server communicate via shared memory with help of semaphores and it will notify the process which have open fd of a given file when that file will be changed or removed.

Doxygen generated documentation [here](https://jjkedra.github.io/FileManagementLibrary/files.html)
