# MyLS Application Documentation

## Introduction
The MyLS application is a custom implementation of the traditional Unix/Linux `ls` command. It is designed to list the contents of directories, providing detailed information about files and directories, including their inode numbers, permissions, owners, and modification dates. This document serves as a comprehensive guide for users and contributors, detailing the application's features, usage instructions, and build process.

## Features

### Basic Listing
- Displays the names of files and directories in the specified directory.
- When no directory is specified, it lists the contents of the current directory.

### Options
The application supports several options that modify its output:

- **`-i` Option**: Displays the inode number of each file or directory alongside its name.
- **`-l` Option**: Provides a long-format listing that includes permissions, link count, owner, group, size, modification date, and filename.
- **`-R` Option**: Recursively lists subdirectories encountered. The application ensures that recursion does not lead to infinite loops by properly handling symbolic links and special directories like `.` and `..`.

### Sorting
Files and directories are sorted in lexicographical order by default. The `-R` option employs depth-first traversal for subdirectories, maintaining the sorted order.

### Date Format
In the long listing format (`-l`), dates are displayed in the `mmm dd yyyy hh:mm` format, ensuring clarity and consistency.

### Special Path Handling
MyLS handles various special paths effectively, such as:
- Root directory (`/`)
- Absolute paths (`/path/to/directory`)
- Relative paths (`./local/dir`, `../parent/dir`)
- User’s home directory (`~`)
- Current (`.`) and parent (`..`) directory shortcuts

## Usage
The application is invoked from the command line with the following syntax:
```
./myls [options] [file list]
```
- `[options]`: A combination of `-i`, `-l`, and `-R`, which can be grouped or separated.
- `[file list]`: A space-separated list of paths. If omitted, the application lists the contents of the current directory.

### Examples
- List the current directory in long format: `./myls -l`
- List files in the home directory and `/usr` with inode numbers: `./myls -i ~/ /usr`
- Recursively list all directories and files starting from the current directory: `./myls -R`

## Building the Application
The MyLS application comes with a `Makefile` for easy compilation:

- **To compile**: Simply run `make` or `make all`. This will compile the source code and generate the `myls` executable.
- **To clean**: Run `make clean` to remove the executable and object files, cleaning up the directory.

## Simplifications and Limitations
- The application does not support the full range of `ls` options, focusing instead on the most commonly used ones for educational purposes.
- It does not handle quoted strings as arguments, nor does it display total block size in the long listing format.
- Options must be specified before any file or directory arguments.

## Error Handling
MyLS robustly handles errors, providing clear messages for unsupported options and nonexistent paths, ensuring a user-friendly experience.

## Testing
Thorough testing is encouraged to ensure the application behaves as expected across various scenarios, including different option combinations and special path cases.

## Contributing
Contributors are welcome to enhance the application's functionality, improve its performance, or fix bugs. Please adhere to the coding standards and guidelines provided in the project repository.

## Conclusion
The MyLS application offers a simplified yet powerful alternative to the Unix/Linux `ls` command, suitable for educational purposes and everyday use. Its development and use serve as an excellent learning opportunity for understanding file system interactions in Unix-like environments.
