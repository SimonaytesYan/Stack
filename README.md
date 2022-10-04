# Stack 
Implementation of a stack in C with two types of protection for different data structures.

## Using other type of elements
To specify the data type you are using, use
```c++
typedef *data_type* Elem;
```

## Using protection
```c++
#define PROTECTION_LEVEL
```
This define responsible for enabling/disabling protection:
* First bit of PROTECTION_LEVEL is  1 - enable  canary protection
* First bit of PROTECTION_LEVEL is  0 - disable canary protection
* Second bit of PROTECTION_LEVEL is 1 - enable  hash protection
* Second bit of PROTECTION_LEVEL is 0 - disable hash protection

## Logging
Use
```c++
#define LOGS_TO_FILE
```
If you want to put logs to file with name
```c++
const char LOGS
```
If you want to put logs to console use
```c++
#define LOGS_TO_CONSOLE
```
