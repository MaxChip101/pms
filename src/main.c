#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/limits.h>

const char* VERSION = "0.0.1-pre";

typedef enum
{
    Tok_Undefined,      // other stuff
    Tok_Plus,           // +
    Tok_Minus,          // -
    Tok_LeftParen,      // (
    Tok_RightParen,     // )
    Tok_LeftBrace,      // {
    Tok_RightBrace,     // }
    Tok_LeftBracket,    // [
    Tok_RightBracket,   // ]
    Tok_Colon,          // :
    Tok_Semi,           // ;
    Tok_At,             // @
    Tok_Tilde,          // ~
    Tok_Dollar,         // $
    Tok_Percent,        // %
    Tok_Caret,          // ^
    Tok_And,            // &
    Tok_Asterix,        // *
    Tok_Pipe,           // |
    Tok_ForwardSlash,   // /
    Tok_BackSlash,      /* \ */
    Tok_DoubleQuote,    // "
    Tok_SingleQuote,    // '
    Tok_LessThan,       // <
    Tok_GreaterThan,    // >
    Tok_Comma,          // ,
    Tok_Period,         // .
    Tok_Exclamation,    // !
    Tok_Hashtag,        // #
    Tok_Question,       // ?
    Tok_Equals,         // =
    Tok_Underscore,     // _
    Tok_BackTick,       // `
    Tok_Line,           // \n
    Tok_Identifier,     // name
    Tok_Num,            // 123
    Tok_EOF             // end
} TokenType;

typedef enum 
{
    Node_Copy,              // *()
    Node_Add,               // + / +()
    Node_Subtract,          // - / -()
    Node_Loop,              // {}
    Node_Bit_And,           // &()
    Node_Bit_Or,            // |()
    Node_Bit_Xor,           // ^()
    Node_Bit_Not,           // !()
    Node_Bit_Shift_Left,    // \ / \()
    Node_Bit_Shift_Right,   // / / /()
    Node_Allocate,          // $()
    Node_Free,              // ~()
    Node_If_Statement,      // ?():()
    Node_Goto,              // to()
    Node_Grab,              // grab()
    Node_Bracket            // []
} ASTNodeType;

typedef struct
{
    char* value;
    TokenType type;
    int line;
    int col;
} Token;

struct ASTNode;
typedef struct ASTNode ASTNode;

struct ASTNode
{
    ASTNodeType type;
    char *value;
    union
    {
        struct
        {
            int value;
            int destination;
        } value_destination_data;
    };

    struct ASTNode *children;

    int child_count;
};

typedef struct
{
    char fail; // bool
    Token *tokens;
    int length;
} TokenArray;

typedef struct
{
    char fail; // bool
    ASTNode *nodes;
    int size;
} AST;

TokenArray tokenize(char *content);
AST generate_AST(TokenArray);
Token create_token(TokenType type, int line, int col, int *token_count);
ASTNode create_copy_node(int value, int destination);
void free_tokens(TokenArray tokens);
void free_AST(AST tree);
void proper_free(void *ptr);

// uname -m # returns cpu archetecture

int main(int argc, char **argv)
{
    if(argc <= 1) // check for arguments
    {
        printf("pms: no input arguments\n");
        return 1;
    }

    if(strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
    {
        printf("pms (PMS) %s\n", VERSION);
        return 0;
    }

    // create and initialize the paths as NULL
    char* source_path = NULL;
    char* output_path = NULL;

    // refactor arguments so that pms --version or pms -v can exist

    // looping thorugh all the arguments and flags
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-o") == 0) // output flag
        {
            if(i + 1 >= argc) // check to see if output path exists after -o.
            {
                printf("pms: no output path mentioned\n");
                return 1;
            }

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
    if(output_path == NULL) // if output path does not exist, set it to source name
    {
        output_path = calloc(4, 1);
        strcpy(output_path, "bin");
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
    
    TokenArray tokens = tokenize(source_content);
    if(tokens.fail)
    {
        printf("pms: tokenization error\n");
        free_tokens(tokens);
        return 1;
    }

    AST tree = generate_AST(tokens);
    if (tree.fail)
    {

    }

    for (int i = 0; i < tokens.length; i++)
    {
        printf("%s:(%i,%i)\n", tokens.tokens[i].value, tokens.tokens[i].line, tokens.tokens[i].col);
    }
    free_tokens(tokens);
    
    // program ending, freeing pointers
    proper_free(output_path);
    proper_free(source_content);
    return 0;
}

// a more safe free function
void proper_free(void *ptr)
{
    free(ptr);
    ptr = NULL;
    return;
}

void free_tokens(TokenArray tokens)
{
    for(int i = 0; i < tokens.length; i++)
    {
        proper_free(tokens.tokens[i].value);
    }
    proper_free(tokens.tokens);
    return;
}

void free_AST_nodes(ASTNode *nodes)
{
    for(int i = 0; i < sizeof(nodes); i++)
    {
        proper_free(tokens.tokens[i].value);
    }
    proper_free(tokens.tokens);
    return;
}

AST AST_Error(ASTNode **nodes, char *error)
{
    proper_free(nodes)
}

Token create_token(char *value, TokenType type, int line, int col, int *token_count)
{
    (*token_count)++;
    Token token;
    token.value = value;
    token.type = type;
    token.line = line;
    token.col = col;
    return token;
}

ASTNode create_copy_node(int value, int destination)
{
    ASTNode node;
    node.type = Node_Copy;
    node.children = NULL;
    node.child_count = 0;
    return
}

TokenArray tokenize(char *content)
{
    int line = 1;
    int col = 1;
    int token_count = 0;
    TokenType type;
    Token *tokens = malloc(strlen(content) * sizeof(Token) + sizeof(Token));
    if(tokens == NULL)
    {
        TokenArray tokenArray;
        tokenArray.tokens = NULL;
        tokenArray.length = 0;
        tokenArray.fail = 1;
        return tokenArray;
    }

    for(int i = 0; i < strlen(content); i++)
    {
        char* str = calloc(2, 1);
        str[0] = content[i];
        
        switch (content[i])
        {
        case '+':
            type = Tok_Plus;
            break;
        case '-':
            type = Tok_Minus;
            break;
        case '(':
            type = Tok_LeftParen;
            break;
        case ')':
            type = Tok_RightParen;
            break;
        case '{':
            type = Tok_LeftBrace;
            break;
        case '}':
            type = Tok_RightBrace;
            break;
        case '[':
            type = Tok_LeftBracket;
            break;
        case ']':
            type = Tok_RightBracket;
            break;
        case '~':
            type = Tok_Tilde;
            break;
        case '!':
            type = Tok_Exclamation;
            break;
        case '@':
            type = Tok_At;
            break;
        case '#':
            type = Tok_Hashtag;
            break;
        case '$':
            type = Tok_Dollar;
            break;
        case '%':
            type = Tok_Percent;
            break;
        case '^':
            type = Tok_Caret;
            break;
        case '&':
            type = Tok_And;
            break;
        case '*':
            type = Tok_Asterix;
            break;
        case '_':
            type = Tok_Underscore;
            break;
        case '=':
            type = Tok_Equals;
            break;
        case '|':
            type = Tok_Pipe;
            break;
        case '\\':
            type = Tok_BackSlash;
            break;
        case ':':
            type = Tok_Colon;
            break;
        case ';':
            type = Tok_Semi;
            break;
        case '\"':
            type = Tok_DoubleQuote;
            break;
        case '\'':
            type = Tok_SingleQuote;
            break;
        case '<':
            type = Tok_LessThan;
            break;
        case '>':
            type = Tok_GreaterThan;
            break;
        case ',':
            type = Tok_Comma;
            break;
        case '.':
            type = Tok_Period;
            break;
        case '?':
            type = Tok_Question;
            break;
        case '/':
            type = Tok_ForwardSlash;
            break;
        case '`':
            type = Tok_BackTick;
            break;
        case ' ':
            col++;
            continue;
            break;
        case '\t':
            col++;
            continue;
            break;
        case '\n':
            type = Tok_Line;
            line++;
            break;
        default:
            if (isalpha(content[i]))
            {
                int start = i;
                int size = 0;
                while (isalpha(content[start + size]))
                    size++;
                i += size;
                col += size;
                str = realloc(str, size+1);
                strncpy(str, &content[start], size);
                str[size] = '\0';
                type = Tok_Identifier;
                break;
            } else if(isdigit(content[i]))
            {
                int start = i;
                int size = 0;
                while (isdigit(content[start + size]))
                    size++;
                i += size;
                col += size;
                str = realloc(str, size+1);
                strncpy(str, &content[start], size);
                str[size] = '\0';
                type = Tok_Num;
                break;
            }
            else
            {
                type = Tok_Undefined;
            }
            
            break;
        }
        tokens[token_count] = create_token(str, type, line, col, &token_count);
        if(type == Tok_Line)
            col = 0;
        col++;
    }
    char* str = calloc(2, 1);
    str[0] = content['\0'];
    tokens[token_count] = create_token(str, Tok_EOF, line, col, &token_count);

    tokens = realloc(tokens, token_count * sizeof(Token));

    TokenArray tokenArray;
    tokenArray.tokens = tokens;
    tokenArray.length = token_count - 1;
    tokenArray.fail = 0;
    return tokenArray;
}