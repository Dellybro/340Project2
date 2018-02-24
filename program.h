
#ifndef __PROGRAM_H__
#define __PROGRAM_H__


#include <string>
#include "lexer.h"


using namespace std;


struct BooleanRule {
    int LHS;
    vector<int> RHS;
};

struct Rule {
    string LHS;
    vector<string> RHS;
};

class Program  {

    private:
        LexicalAnalyzer lexer;
        Token peek();

        bool isMember(vector<string> currentSet, string possibleMember);
        void ParseTokens();
        void PrintRules();
        void PrintStringVector(vector<string> vec);
        void PrintBooleanVector(vector<bool> vec);
        void CreateGeneratingVector();
        void GenerateBooleanRules();
        void GenerateUsefulRules();
        void CalculateFirstSets(bool);
        void GetFirstSetForRule(BooleanRule *rule);
        void CalculateFollowSets(bool);
        void isPredictive();
        /* Symbols - Terminals, nonTerminals */

        vector< vector<bool> > followSetVector;
        vector< vector<bool> > firstSetVector;
        vector<string> universeVector;

        vector<bool> reachableVector;
        vector<bool> generatingVector;
        vector<string> combinedSymbols;
        vector<string> terminals;
        vector<string> nonTerminals;
        vector<string> unorderedNonTerminals;

        vector< Rule* > rules;
        vector< BooleanRule* > usefulRules;
        vector< BooleanRule* > booleanPositionRules;

    public:
        int ParseProgram(int argc, char* argv[]);
    
};

#endif