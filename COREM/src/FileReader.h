#ifndef FILEREADER_H
#define FILEREADER_H


/* BeginDocumentation
 * Name: FileReader
 *
 * Description: parsing of retina script and initialization of retina modules
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 */

#include "DisplayManager.h"
#include "StaticNonLinearity.h"
#include "ShortTermPlasticity.h"

using namespace cimg_library;
using namespace std;

// fixed parameters
#define MAX_CHARS_PER_LINE 2048
#define MAX_TOKENS_PER_LINE 256

// Characters used as expecial tokens (and to separate tokens) in a action command, inside parameter block and to delimit a string
#define TOKENS_CMD "(),'{} \t.#"
#define TOKENS_BLK "(),'{} \t"
#define TOKENS_STR "'"


class FileReader{
private:
    // to check syntax errors
    bool CorrectFile;
    bool continueReading;

    // File reader
    const char* fileName;
    ifstream fin;

public:

    //Constructor, copy, destructor.
    FileReader(int X=25, int Y=25, double tstep=1.0);
    FileReader(const FileReader& copy);
    ~FileReader(void);

    //reset parameters
    void reset(int X, int Y, double tstep);
    //set directory
    void setDir(const char* s);
    // allocate values
    bool allocateValues();

    // This method copies a number of characters from the beginning of source string into destination buffer
    // plus the \0 termination character at the end of the copied chars (\0 is always copied if dest_size > 0).
    // This function limits the amount of chars copied according to the destination buffer size (dest_size).
    // src_len specifies the number of characters from the source string that must be copied (apart from the \0).
    // If src_len is -1, the whole source string is tried to be copied.
    // dest_buff is a pointer to a buffer where at least a space of dest_size chars is available.
    // src_str must point to a \0-terminated string if parameter src_len is -1, since \0 will mark the end of the copy process.
    // The functions retuns 1 if all the specified characters could be copied, and it returns 0 if there
    // was not enough space in the destination buffer (only dest_size chars were copied).
    bool nStringCopy(char *dest_buff, const char *src_str, size_t src_len, size_t dest_size);
    
    // This method slipts a string into tokens.
    // especial_tokens is a pointer to a \0-terminated string containing the chars which must be considered espcial tokens.
    // Apart from the especial tokens, any substring not containg any especial token is also considered a token.
    // Each call to this method returns a pointer to a new found token.
    // str is a pointer to the source string which must be parsed
    // The method must be called with src_str=NULL before a new line must be parsed to reset its internal state.
    // Once it is called with src_str=NULL it can be called repeatedly with the same src_str pointer until the
    // whole line is parsed.
    // The tokens are stored in a method internal array so the pointers returned by this method are valid until
    // the method is called with a NULL pointer as input string.
    // The method returns a pointer to the found token or NULL if no more tokern could be found.
    char *getNextToken(const char *str, const char *especial_tokens);
    
    // This method removes all the specified tokens from a token list (as the ones obtained from getNextToken). 
    // token is a pointer to a list of pointers to string which contain the tokens to process. The
    // last pointer of the pointer list must be NULL.
    // ign_tokens is a 0\-terminated string containing the tokens to discard, so only 1-char tokens can be removed.
    void discardTokens(char **tokens, const char *ign_tokens);
        
    // This method parses a script text file line into a list of tokens.
    // in_line must point to the input line string terminted with \0 and not including the new-line char.
    // out_tokens point to a pointer buffer where the pointers to token strings will be stored. The last
    // pointer stored will be NULL. At least a space of max_tokens pointers must be available in the buffer.
    // The method returns true if the whole line could be correctly parsed nd the tokens stored in out_tokens,
    // otherwise it returns false.
    bool parseLine(const char *in_line, char **out_tokens, int max_tokens);
    
    // This method parses the parameter block which can be found in the end of a Create, Input or Output action command.
    // block_tokens must points to a pointer array where the pointers to block tokens are stored. The last
    // pointer must be NULL. new_module is a pointer to a created module. If a parameter block if found in 
    // block_tokens, this module will receive the parsed parameters.
    void parseParameterBlock(char **block_tokens, module *new_module, int file_line);
    
    // parsing of retina script and initialization of retina modules
    void parseFile(Retina &retina,DisplayManager &displayMg);
    bool getContReading(){return continueReading;}
    void abort(int line, const char *error_msg);
};

#endif // FILEREADER_H
