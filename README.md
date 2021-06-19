# Assignment
Assignment does not clearly show what areas are being tested.
There are multiple areas such as memory efficient data structures, sharing a single data structure by all the modules.
However, I assume main focus is given to configuration management in the field in a full lifecycle of the products and variants

# Introduction
Configuration management is bit tricky in embedded devices as there are many modules associated with it. Deployed devices, various models that has slightly different configuration, field configuration update tool and its versions make it hard to manage configuration. Most of the times single source tree is maintained for a particular product series as majority of the features are common. 

# Design choices
Easiest approach is monolithic structure. It is easy to read and write. However, when more devise models arrives, many features are added, mixed of old/new firmware and configuration tool versions, it is very hard to manage the configuration as structure length varies from model to mode and FW version to version. Also cofiguring tool should aware of it.

Other approach is JSON or XML type configuration. There is no human readable requirement, those techniques will add unnecessary burden.

The best solution for this type of application is TLV type binary store.

# TLV approach
This is a serialized configuration store. Each variable has metadata and its value. This mechanism is called TLV (aka Type, Length, Value). However, another field is introduced for to address array type variables. Definition of the variable is given below
```
Type - Unique identifier of the variable
Index - index of the variable if it is and array type
Length - length of the variable
Value - Value of the variable (system endianness)
```

This can be easily serialized and any update can be appended to the tail.

## Implementation
There are couple of stores are used. The first one is the factory store which can be stored in read only area or separate sector in the flash.
Second one is the field configuration store. This is stored in separate flash area that can be written.
There is a special TLV config to terminate the store. It has CRC of the entire store to verify integrity.

At the boot up, firmware reads the default configuration. Then it reads the field configuration store. If a default is overridden, overridden value is used.

If firmware does not recognize the configuration it just ignores it. By this way backward compatibility is achieved. 

## Portability
This library should be portable (endianess should be handled properly) as same library complied to host environment can be used to write the configuration.

# Build and test Instruction
In Linux:
```
Navigate to the source folder
cmake .
make
ctest
```
![Linux test console ](linux.png)

In Windows:
```
Open the source folder in visual studio
build
load test explorer and run all
```
![Windows test](windows.png)





