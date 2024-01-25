#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <libgen.h>

int opt;
int i_option = 0;
int l_option = 0;
int R_option = 0;

char *given_paths[1000]; //Adjust size as needed
int num_of_given_paths = 0;

typedef struct directory_info {
    char* name;
    struct directory_info *parent;
    char* path;
    struct directory_info **children;
    int num_children;
    unsigned long long inode;
} directory_info;

directory_info *root_list[1000]; //this is an array representing the given paths

char *get_symlink_path(const char *symlink_path) {
    char *buffer = (char *)malloc(4096);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }
    ssize_t len = readlink(symlink_path, buffer, 4096 - 1);
    if (len == -1) {
        perror("readlink");
        free(buffer);
        return NULL;
    }
    buffer[len] = '\0';
    return buffer;
}


bool is_hidden_file(char *filename)
{
    if(strcmp(filename, "..") == 0 ){
        return false;
    }
    if(strcmp(filename, ".") == 0){
        return false;
    }
    if(filename[0] == '.' && filename[1] == '/'){
        return false;
    }
    if (filename[0] == '.' && filename[1] != '\0')
    {
        return true;
    }

    return false;
} 


void parse_command_line_arguments(int argc, char *argv[]) {
    opterr = 0; // Suppress default error messages
    // The colon after the character in the optstring means that
    // the option requires an argument.
    // https://piazza.com/class/lhaqnxludbb4e3/post/476
    while ((opt = getopt(argc, argv, "ilR")) != -1) {
        switch (opt) {
        case 'i':
            i_option = 1;
            break;
        case 'l':
            l_option = 1;
            break;
        case 'R':
            R_option = 1;
            break;
        case '?':
            // Handle unsupported options here
            fprintf(stderr, "Error: Unsupported Option\n");
            exit(EXIT_FAILURE);
            break;
        case ':':
            // Handle unsupported options here
            fprintf(stderr, "Error: Unsupported Option\n");
            exit(EXIT_FAILURE);
            break;
        default: /* wrong format */
            fprintf(stderr, "Error: Unsupported Option\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        // printf("non-option arguments: ");
        while (optind < argc){
            given_paths[num_of_given_paths++] = argv[optind];
            optind++;
        }
    }

    if(num_of_given_paths == 0){
        given_paths[0] = ".";
        num_of_given_paths = 1;
    }

    // Create a directory_info struct for each given path and store it in the root_list
    for (int i = 0; i < num_of_given_paths; i++) {
        struct stat s;

        // Get the inode number for the root directory
        if (stat(given_paths[i], &s) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        directory_info *dir_info = malloc(sizeof(directory_info)); // Allocate memory for a directory_info struct
        dir_info->name = malloc(strlen(given_paths[i]) + 1); // Allocate memory for the directory name string
        strcpy(dir_info->name, given_paths[i]); // Copy the directory name from the given_paths

        dir_info->path = malloc(strlen(given_paths[i]) + 1);
        strcpy(dir_info->path, given_paths[i]); // Copy the directory name from the given_paths

        dir_info->num_children = 0; // Initialize the number of children to 0
    
        dir_info->children = NULL; // Initialize the children array to NULL
        
        dir_info->inode = (unsigned long long)s.st_ino;

        root_list[i] = dir_info; // Store the pointer to the struct in the root_list
    }
}

int alphasort_no_case(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

void print_file_info(struct stat file_stat, char *name, char* path) {

    if(is_hidden_file(name)){
        return;
    }

    if (i_option == 1){
        if(!l_option){
            printf("%llu ", (unsigned long long)file_stat.st_ino);
            printf("%s\n", basename(name));
        }
    }
    if (l_option == 1){
        // Content Permissions
        char permission_str[11];
        const char *permission_chars = "rwxrwxrwx";
        int i;
        if(i_option){
            printf("%llu ", (unsigned long long)file_stat.st_ino);
        }

        permission_str[0] = (S_ISDIR(file_stat.st_mode)) ? 'd' : '-';
        for (i = 0; i < 9; i++) {
            permission_str[i + 1] = (file_stat.st_mode & (1 << (8 - i))) ? permission_chars[i] : '-';
        }
        permission_str[i + 1] = '\0';
        printf("%s ", permission_str);

        // Number of Links
        printf("%ld ", file_stat.st_nlink);

        // Content Owner
        struct passwd *pw = getpwuid(file_stat.st_uid);
        if (pw) {
            printf("%s ", pw->pw_name);
        } else {
            perror("UNKNOWN");
        }

        // Content Owner Group
        struct group *gr = getgrgid(file_stat.st_gid);
        if (gr) {
            printf("%s ", gr->gr_name);
        } else {
            perror("UNKNOWN");
        }

        // Content Size
        printf("%ld ", file_stat.st_size);

        // Modification Date
        char* date = (char*)malloc(20 * sizeof(char));
        strftime(date, 20, "%b %d %Y %H:%M", localtime(&file_stat.st_mtime));
        printf("%s ", date);
        free(date);

        struct stat symlink_stat;

        if (lstat(path, &symlink_stat) < 0) {
            perror("lstat");
            return;
        }

        if (S_ISLNK(symlink_stat.st_mode)) {
            char* buffer =  get_symlink_path(path);
            printf("%s -> %s\n", basename(name), buffer);
            free(buffer);
        } else {
            printf("%s\n", basename(name));
        }
    }
    if(!l_option && !i_option){
        printf("%s\n", basename(name));
    }
}

void handle_directory(directory_info *parent) {
    if(is_hidden_file(parent->name)){
        return;
    }
    struct dirent **namelist;
    int number_of_children = scandir(parent->path, &namelist, NULL, alphasort_no_case);

    if (number_of_children == -1) { //path given is not a directory
        struct stat file_stat1;
        
        if (lstat(parent->path, &file_stat1) == 0) { // is a file
            print_file_info(file_stat1, parent->name, parent->path);
        }
        else{ //is not a directory or file, exit
            printf("Error : Nonexistent files or directories\n");
            exit(EXIT_FAILURE);
        }
        //return;
    } 

    if(number_of_children < 0){
        return;
    }

    parent->children = malloc(number_of_children * sizeof(directory_info *));

    if(number_of_children != 0){
        printf("%s:\n", parent->path);
    }

    int numberOfSkips = 0;
    for (int index = 0; index < number_of_children; index++) {
        struct stat file_stat; //hold a file's stats

        // Skip the current directory and parent directory entries.
        if (strcmp(namelist[index]->d_name, ".") == 0 || strcmp(namelist[index]->d_name, "..") == 0 || is_hidden_file(namelist[index]->d_name)) {
            free(namelist[index]);
            numberOfSkips++;
            continue;
        }

        // Allocate memory for a new directory_info struct for the child
        directory_info *child = malloc(sizeof(directory_info));
        if (!child) {
            fprintf(stderr, "Failed to allocate memory for directory info.\n");
            exit(EXIT_FAILURE);
        }

        // Initialize the fields of the child directory_info struct
        child->name = strdup(namelist[index]->d_name);
        child->parent = parent;
        child->children = NULL;
        child->num_children = 0;

        // Create a full path for the child directory
        child->path = malloc(strlen(parent->path) + strlen(child->name) + 2); // 1 for the '/' and 1 for the '\0'
        if (!child->path) {
            fprintf(stderr, "Failed to allocate memory for child path.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(child->path, parent->path);
        strcat(child->path, "/");
        strcat(child->path, child->name);


        
        if (stat(child->path, &file_stat) == -1){
            printf("%s\n", child->path);
            perror("Error with stat of file\n");
            return;
        }

        child->inode =  (unsigned long long)file_stat.st_ino;

        parent->num_children++;
        parent->children[index] = child;

        print_file_info(file_stat, child->name, child->path);
    }

    
    for(int index = numberOfSkips; index < number_of_children; index++){


        const char* file_path = parent->children[index]->path;
        struct stat symlink_stat2;

        if (lstat(file_path, &symlink_stat2) < 0) {
            perror("lstat");
            return;
        }

        if (S_ISLNK(symlink_stat2.st_mode)) {
            free(parent->children[index]->name);
            free(parent->children[index]->path);
            free(parent->children[index]);
            free(namelist[index]);
            continue;
        } else {
            if(!is_hidden_file(parent->children[index]->name)){
                free(parent->children[index]->name);
                free(parent->children[index]->path);
                free(parent->children[index]);
            }
        }
        
        free(namelist[index]);
    }
    free(namelist);
    free(parent->children);
}


void printRecursively(directory_info *parent) {
    if(is_hidden_file(parent->name)){
        return;
    }
    struct dirent **namelist;
    int number_of_children = scandir(parent->path, &namelist, NULL, alphasort_no_case);
    // printf("%s", parent->name);
    // it is a file no need to check
    if(number_of_children < 0){
        return;
    }

    parent->children = malloc(number_of_children * sizeof(directory_info *));

    printf("%s:\n", parent->path);
    
    int numberOfSkips = 0;
    for (int index = 0; index < number_of_children; index++) {
        struct stat file_stat; //hold a file's stats

        // Skip the current directory and parent directory entries.
        if (strcmp(namelist[index]->d_name, ".") == 0 || strcmp(namelist[index]->d_name, "..") == 0 || is_hidden_file(namelist[index]->d_name)) {
            free(namelist[index]);
            numberOfSkips++;
            continue;
        }

        // Allocate memory for a new directory_info struct for the child
        directory_info *child = malloc(sizeof(directory_info));
        if (!child) {
            fprintf(stderr, "Failed to allocate memory for directory info.\n");
            exit(EXIT_FAILURE);
        }

        // Initialize the fields of the child directory_info struct
        child->name = strdup(namelist[index]->d_name);
        child->parent = parent;
        child->children = NULL;
        child->num_children = 0;

        // Create a full path for the child directory
        child->path = malloc(strlen(parent->path) + strlen(child->name) + 2); // 1 for the '/' and 1 for the '\0'
        if (!child->path) {
            fprintf(stderr, "Failed to allocate memory for child path.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(child->path, parent->path);
        strcat(child->path, "/");
        strcat(child->path, child->name);


        
        if (stat(child->path, &file_stat) == -1){
            printf("%s\n", child->path);
            perror("Error with stat of file\n");
            return;
        }

        child->inode =  (unsigned long long)file_stat.st_ino;

        parent->num_children++;
        parent->children[index] = child;

        if(l_option){
            if(i_option){
                printf("%llu ", child->inode);
            }
            char permission_str[11];
            const char *permission_chars = "rwxrwxrwx";
            int i;
            permission_str[0] = (S_ISDIR(file_stat.st_mode)) ? 'd' : '-';
            for (i = 0; i < 9; i++) {
                permission_str[i + 1] = (file_stat.st_mode & (1 << (8 - i))) ? permission_chars[i] : '-';
            }
            permission_str[i + 1] = '\0';
            printf("%s ", permission_str);

            // Number of Links
            printf("%ld ", file_stat.st_nlink);

            // Content Owner
            struct passwd *pw = getpwuid(file_stat.st_uid);
            if (pw) {
                printf("%s ", pw->pw_name);
            } else {
                perror("UNKNOWN");
            }

            // Content Owner Group
            struct group *gr = getgrgid(file_stat.st_gid);
            if (gr) {
                printf("%s ", gr->gr_name);
            } else {
                perror("UNKNOWN");
            }

            // Content Size
            printf("%ld ", file_stat.st_size);

            // Modification Date
            char* date = (char*)malloc(20 * sizeof(char));
            strftime(date, 20, "%b %d %Y %H:%M", localtime(&file_stat.st_mtime));
            printf("%s ", date);
            free(date);

            const char* file_path = child->path;
            struct stat symlink_stat;
            
            if (lstat(file_path, &symlink_stat) < 0) {
                perror("lstat");
                return;
            }

            if (S_ISLNK(symlink_stat.st_mode)) {
                char* buffer =  get_symlink_path(file_path);
                printf("%s -> %s\n", basename(child->name), buffer);
                free(buffer);
            } else {
                printf("%s\n", child->name);
            }
        }

        if(!l_option){
            if(i_option){
                printf("%llu ", child->inode);
            }
            printf("%s  ", child->name);
        }
    }
    if(l_option){
        printf("\n");
    }
    if(!l_option){
        printf("\n\n");
    }
    
    for(int index = numberOfSkips; index < number_of_children; index++){
        // printf("%s\n", parent->children[index]->path);
        // checking if file has a symbolic link


        // if its a first child even if -R is off we need to go through first level

        const char* file_path = parent->children[index]->path;
        struct stat symlink_stat2;

        if (lstat(file_path, &symlink_stat2) < 0) {
            perror("lstat");
            return;
        }

        if (S_ISLNK(symlink_stat2.st_mode)) {
            free(parent->children[index]->name);
            free(parent->children[index]->path);
            free(parent->children[index]);
            free(namelist[index]);
            continue;
        } else {
            if(!is_hidden_file(parent->children[index]->name)){
                printRecursively(parent->children[index]);
                free(parent->children[index]->name);
                free(parent->children[index]->path);
                free(parent->children[index]);
            }
        }
        
        free(namelist[index]);
    }
    free(namelist);
    free(parent->children);
}

bool is_option(const char *arg) {
    return arg[0] == '-' && arg[1] != '\0';
}

void check_valid(int argc, char *argv[]) {
    bool filenames_started = false;

    for (int i = 1; i < argc; i++) {
        if (is_option(argv[i])) {
            if (filenames_started) {
                // Display an error message if there is another valid option after filenames.
                printf("Error: All options must be specified before any files/directories.\n");
                exit(EXIT_FAILURE);
            }
        } 
        else {
            //file is encountered
            filenames_started = true;
        }
    }
}

void free_directory_info(directory_info *info) {
    free(info->name);
    free(info->path);
    // if (info->children != NULL) {
    //     for (int i = 0; i < info->num_children; i++) {
    //         free_directory_info(info->children[i]);
    //     }
    //     //free(info->children);
    // }
    free(info);
}

int main(int argc, char *argv[]) {
    check_valid(argc, argv);
    parse_command_line_arguments(argc, argv);
    if(!R_option){
        for(int i = 0; i < num_of_given_paths; i++){
            handle_directory(root_list[i]);
            if(i != num_of_given_paths-1){
                printf("\n");
            }
        }
    }

    if(R_option){
        for(int i = 0; i < num_of_given_paths; i++){
            printRecursively(root_list[i]);
            if(i != num_of_given_paths-1){
                printf("\n");
            }        
        }
    }

    for (int i = 0; i < num_of_given_paths; i++) {
        free_directory_info(root_list[i]);
    }

    return 0;
}