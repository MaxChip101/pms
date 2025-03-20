#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

// a more safe free function
void proper_free(void *ptr)
{
    free(ptr);
    ptr = NULL;
    return;
}

int main(int argc, char **argv)
{
    if(argc <= 2) // check for arguments
    {
        printf("pms: no input arguments\n");
        return 1;
    }

    // create and initialize the paths as NULL
    char* source_path = NULL;
    char* output_path = NULL;

    // looping thorugh all the arguments and flags
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-o") == 0) // output flag
        {
            if(i + 1 < argc) // check to see if output path exists after -o.
            {
                i++;
                output_path = malloc(strlen(argv[i]) + 1);
                if(output_path == NULL)
                {
                    printf("pms: could not allocate memory for output path");
                    if(source_path != NULL) {proper_free(source_path);}
                    return 1;
                }
                strcpy(output_path, argv[i]);
            }
            else // if there was not path after -o
            {
                printf("pms: no output path mentioned\n");
                return 1;
            }
        }
        else // the first argument that is not a flag would be the source file
        {
            source_path = malloc(strlen(argv[i]) + 1);
            if(source_path == NULL)
            {
                printf("pms: could not allocate memory for source path");
                if(output_path != NULL) {proper_free(output_path);}
                return 1;
            }
            strcpy(source_path, argv[i]);
        }
    }

    if(source_path == NULL) // check to make sure the source path exists
    {
        printf("pms: no source path mentioned\n");
        if(output_path != NULL) {proper_free(output_path);} // in case the user specified an output path, free the memory used for the output path
        return 1;
        
    }
    if(output_path == NULL) // check to make sure the output path exists
    {
        printf("pms: no output path mentioned\n");
        if(source_path != NULL) {proper_free(source_path);} // in case the user specified a source path, free the memory used for the source path
        return 1;
    }

    // create and initialize working directory to NUll. Also creates a buffer to read
    char* cwd = NULL;
    char cwd_buffer[PATH_MAX];

    cwd = getcwd(cwd_buffer, PATH_MAX);
    if (cwd == NULL)
    {
        printf("pms: failed to get working directory\n");
        return 1;
    }

    // creates and initializes the final path as NULL, the final path is the cwd + / + source path
    char *final_source_path = NULL;
    final_source_path = calloc(strlen(cwd) + strlen(source_path) + 2, 1); // allocates memory for working directory, source path, and a forward slash plus a null terminator
    if(final_source_path == NULL)
    {
        printf("pms: could not allocate memory for source path\n");
        proper_free(source_path);
        proper_free(output_path);
        return 1;
    }
    strcat(final_source_path, cwd);
    strcat(final_source_path, "/");
    strcat(final_source_path, source_path);

    // create the file variable to read the source file
    FILE *source_file;
    source_file = fopen(final_source_path, "r");
    if (source_file == NULL)
    {
        printf("pms: failed to open source file: %s\n", source_path);
        proper_free(final_source_path);
        proper_free(output_path);
        proper_free(source_path);
        return 1;
    }

    // getting the source file size
    fseek(source_file, 0L, SEEK_END);
    const long int SOURCE_FILE_SIZE = ftell(source_file);
    char *source_content = calloc(SOURCE_FILE_SIZE + 1, 1);
    if(source_content == NULL)
    {
        printf("pms: failed to allocate memory for source file content\n");
        proper_free(final_source_path);
        proper_free(output_path);
        proper_free(source_path);
        return 1;
    }
    char source_file_read_buffer[100];
    rewind(source_file);

    // reads the source file
    size_t source_file_read_size = fread(source_content, 1, SOURCE_FILE_SIZE, source_file);
    if(source_file_read_size != SOURCE_FILE_SIZE) // ensures the source file length and the source file length found match
    {
        printf("pms: failed to read source file\n");
        proper_free(final_source_path);
        proper_free(output_path);
        proper_free(source_path);
        return 1;
    }

    // free the memory to read the file
    proper_free(final_source_path);
    proper_free(source_path);
    fclose(source_file);
    
    // temporary: prints the contents
    printf("%s\n",source_content);
    
    // program ending, freeing pointers
    proper_free(output_path);
    proper_free(source_content);
    return 0;
}