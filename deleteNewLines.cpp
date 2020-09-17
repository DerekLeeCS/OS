#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::istringstream;


// Checks if a string is entirely composed of whitespace
bool isEmpty( string );


int main() {

    string fileIn, fileOut, line;
    bool removeComments;

    cout << "Enter the name of the file to change: ";
    cin >> fileIn;
    cout << "\n";

    cout << "Do you want to remove all comments? ( 1/0 )" << "\n";
    cin >> removeComments;
    cout << "\n";

    size_t lastIndex = fileIn.find_last_of(".");
    string rawName = fileIn.substr( 0, lastIndex );
    string fileExtension = fileIn.substr( lastIndex, fileIn.length() );

    fileOut = rawName + "Truncated" + fileExtension;

    ifstream input( fileIn );
    ofstream output( fileOut );

    size_t commentIndex;
    string truncatedLine;

    while ( getline( input, line ) ) {

        if ( line.length() > 0 ) {

            // Remove comments from code
            if ( removeComments ) {

                // Search code for comments and truncate
                commentIndex = line.find( "//" );
                if ( commentIndex != string::npos ) {

                    // Skip if entire line was a comment
                    if ( commentIndex == 0 )
                        continue;

                    line = line.substr( 0, commentIndex );

                    // Skip if entire line is composed of whitespace
                    if ( isEmpty( line ) )
                        continue;
                }

            }

            output << line << "\n";

        }

    }

    cout << "Truncation completed.\n";
    cout << "File written to: " << fileOut << endl;

    return 0;

}


// https://stackoverflow.com/questions/3981510/getline-check-if-line-is-whitespace
bool isEmpty( string line ) {

    istringstream iss( line );
    char ch;

    while ( iss >> ch ) {

        if ( !isspace( ch ) )
            return 0;

    }

    return 1;

}

