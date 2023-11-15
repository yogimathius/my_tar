## Welcome to My Tar

my_tar is a custom implementation of the Unix tar command. It allows users to create, append, list, update, and extract from TAR archives, following the specifications of the traditional Unix TAR utility, but with custom handling and features.

## Task

The primary challenge of this project was to replicate the functionality of the classic tar command in Unix systems, but with specific constraints on which C library functions could be used. The project aims to demonstrate a deep understanding of file operations and TAR archive format, all within a constrained development environment.

## Description

my_tar is written in C and handles basic TAR operations like creating archives, appending files, listing contents, updating archives based on modification times, and extracting files. It includes custom implementations of standard library functions to fit the constraints of the project.

## Installation

To install and compile my_tar, follow these steps:

1. Clone the repository:

```
git clone [URL_of_Your_Repository]
```


2. Navigate to the cloned repository directory.


3. Compile the project:

```
make
```

## Usage

my_tar can be used with different modes, similar to the Unix tar command. Here's how to use each mode:

- Create an Archive:

```
./my_tar -cf archive_name.tar file1 file2 ...
```


- Append to an Archive:

```
./my_tar -rf archive_name.tar file1 file2 ...
```


- List Archive Contents:

```
./my_tar -tf archive_name.tar
```


- Update Archive:

```
./my_tar -uf archive_name.tar file1 file2 ...
```


- Extract from an Archive:

```
./my_tar -xf archive_name.tar
```

Replace archive_name.tar, file1, file2, etc., with your actual archive name and file names.


### The Core Team


<span><i>Made at <a href='https://qwasar.io'>Qwasar SV -- Software Engineering School</a></i></span>
<span><img alt='Qwasar SV -- Software Engineering School's Logo' src='https://storage.googleapis.com/qwasar-public/qwasar-logo_50x50.png' width='20px' /></span>
