
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "lexer.h"
#include "inputbuf.h"
#include "program.h"
#include <algorithm>

using namespace std;

void Program::PrintStringVector(vector<string> vec){
    for(int i = 0; i < vec.size(); i++){
        if(i == vec.size() - 1){
            cout << vec[i];
        } else {
            cout << vec[i] << " ";
        }
    }
}

void Program::PrintBooleanVector(vector<bool> vec){
    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }
}

void Program::PrintRules(){
    for(int i = 0; i < rules.size(); i ++){
        Rule *rule = rules[i];
        cout << rule->LHS << " -> ";
        for(int j = 0; j < rule->RHS.size(); j++){
            cout << rule->RHS[j] << " "; 
        }

        cout << endl;
    }
}

Token Program::peek(){
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}


bool Program::isMember(vector<string> currentSet, string possibleMember){
    if(find(currentSet.begin (), currentSet.end (), possibleMember) == currentSet.end()){

        return false;
    }

    return true;
}


void Program::ParseTokens(){

    Rule *currentRule = new Rule();
    while(true){
        Token nextToken = lexer.GetToken();
        if(nextToken.token_type == DOUBLEHASH){
            /* Stopping condition */
            break;
        }
        if(nextToken.token_type == HASH){
            /**
             * Push back currentRule into rules
             * reset currentRule
            */
            rules.push_back(currentRule);
            currentRule = new Rule();
        }
        if(nextToken.token_type == ID){
            
            Token peekToken = peek();
            if(peekToken.token_type == ARROW){
                if(!isMember(unorderedNonTerminals, nextToken.lexeme)){
                    unorderedNonTerminals.push_back(nextToken.lexeme);
                }

                currentRule->LHS = nextToken.lexeme;;
            } else {
                currentRule->RHS.push_back(nextToken.lexeme);
            }
        }
    }


    for(int i = 0; i < rules.size(); i ++){
        Rule *rule = rules[i];
        if(!isMember(nonTerminals, rule->LHS)){
            nonTerminals.push_back(rule->LHS);
        }

        for(int j = 0; j < rule->RHS.size(); j++){
            if(isMember(unorderedNonTerminals, rule->RHS[j])){

                if(!isMember(nonTerminals, rule->RHS[j])){
                    nonTerminals.push_back(rule->RHS[j]);
                }
            } else {

                if(!isMember(terminals, rule->RHS[j])){
                    terminals.push_back(rule->RHS[j]);
                }
            }
        }
    }

    combinedSymbols.push_back("#"); // This represents the empty string
    combinedSymbols.push_back("$"); // This represents EOF
    combinedSymbols.reserve( terminals.size() + nonTerminals.size() ); // preallocate memory
    combinedSymbols.insert( combinedSymbols.end(), terminals.begin(), terminals.end() );
    combinedSymbols.insert( combinedSymbols.end(), nonTerminals.begin(), nonTerminals.end() );
}


void Program::GenerateBooleanRules(){
    BooleanRule *currentRule = new BooleanRule();

    for(int i = 0; i < rules.size(); i ++){
        Rule *rule = rules[i];

        int LHSPos = find(combinedSymbols.begin(), combinedSymbols.end(), rule->LHS) - combinedSymbols.begin();
        currentRule->LHS = LHSPos;
        for(int j = 0; j < rule->RHS.size(); j++){
            int nextRHSPos = find(combinedSymbols.begin(), combinedSymbols.end(), rule->RHS[j]) - combinedSymbols.begin();
            currentRule->RHS.push_back(nextRHSPos);
        }

        booleanPositionRules.push_back(currentRule);
        currentRule = new BooleanRule();
    }
}

void Program::CreateGeneratingVector(){
    GenerateBooleanRules();

    /* Generating Vector */
    for(int i = 0; i < combinedSymbols.size(); i ++){
        if(isMember(terminals, combinedSymbols[i]) || combinedSymbols[i] == "#"){
            generatingVector.push_back(true);
        } else {
            generatingVector.push_back(false);
        }
    }

    while(true){
        bool changed = false;
        for(int i = 0; i < booleanPositionRules.size(); i++){
            BooleanRule *rule = booleanPositionRules[i];
            /* Check for empty string */
            if(rule->RHS.size() == 0){
                if(!generatingVector[rule->LHS]){
                    generatingVector[rule->LHS] = true;
                    changed = true;
                }
            }

            for(int j = 0; j < rule->RHS.size(); j++){
                if(!generatingVector[rule->RHS[j]]){
                    break;
                }
                if(j == rule->RHS.size()-1){
                    if(!generatingVector[rule->LHS]){
                        generatingVector[rule->LHS] = true;
                        changed = true;
                    }
                }
            }
        }

        if(changed == false){
            break;
        }
    }
}

void Program::GenerateUsefulRules(){
    CreateGeneratingVector();

    usefulRules = booleanPositionRules;


    /* Remove Indicies */
    vector<int> removeableIndices;
    for(int i = 0; i < usefulRules.size(); i++){
        if(!generatingVector[usefulRules[i]->LHS]){
            removeableIndices.push_back(i);
        } else {
            for(int j = 0; j < usefulRules[i]->RHS.size(); j++){
                if(!generatingVector[usefulRules[i]->RHS[j]]){
                    removeableIndices.push_back(i);
                }
            }
        }
    }
    for(int i = 0; i < removeableIndices.size(); i++){
        usefulRules.erase(usefulRules.begin() + removeableIndices[i] - i);
    }

    /* Generate ReachableVector */
    for(int i = 0; i < combinedSymbols.size(); i ++){
        if(isMember(terminals, combinedSymbols[i]) || combinedSymbols[i] == "#"){
            reachableVector.push_back(true);
        } else {
            reachableVector.push_back(false);
        }
    }

    /* Add to ReachableVectors */
    for(int i = 0; i < usefulRules.size(); i++){
        if(combinedSymbols[usefulRules[i]->LHS] == rules[0]->LHS || reachableVector[usefulRules[i]->LHS]){
            reachableVector[usefulRules[i]->LHS] = true;
            for(int j = 0; j < usefulRules[i]->RHS.size(); j++){
                reachableVector[usefulRules[i]->RHS[j]] = true;
            }
        }
    }    

    /* Remove Indicies */
    removeableIndices.clear();
    for(int i = 0; i < usefulRules.size(); i++){
        if(!reachableVector[usefulRules[i]->LHS]){
            removeableIndices.push_back(i);
        }
    }
    for(int i = 0; i < removeableIndices.size(); i++){
        usefulRules.erase(usefulRules.begin() + removeableIndices[i] - i);
    }


    /* Print Reachable Symbols */
    for(int i = 0; i < usefulRules.size(); i ++){
        BooleanRule *rule = usefulRules[i];
        cout << combinedSymbols[rule->LHS] << " -> ";
        if(rule->RHS.size() == 0){
            cout << "#";
        } else {
            for(int j = 0; j < rule->RHS.size(); j++){
                cout << combinedSymbols[rule->RHS[j]] << " "; 
            }
        }

        cout << endl;
    }

}

void Program::GetFirstSetForRule(BooleanRule *rule){
    if(rule->RHS.size() == 0){
        firstSetVector[rule->LHS][0] = true;
    } else {

        if(isMember(terminals, combinedSymbols[rule->RHS[0]])){

            int pos = find(universeVector.begin(), universeVector.end(), combinedSymbols[rule->RHS[0]]) - universeVector.begin();
            firstSetVector[rule->LHS][pos] = true;
        } else {

        }
    }
}

void Program::CalculateFirstSets(bool print){
    GenerateBooleanRules();

    /* EX: # $ a b c */
    universeVector.push_back("#");
    universeVector.push_back("$");
    universeVector.reserve( terminals.size() ); // preallocate memory
    universeVector.insert( universeVector.end(), terminals.begin(), terminals.end() );

    for(int i = 0; i < combinedSymbols.size(); i++){
        vector<bool> currentVector;
        for(int j = 0; j < universeVector.size(); j++){
            if(combinedSymbols[i] == universeVector[j] && universeVector[j] != "$"){
                currentVector.push_back(true);
            } else {
                currentVector.push_back(false);
            }
        }

        firstSetVector.push_back(currentVector);
    }
    while(true){
        bool changed = false;
        for(int i = 0; i < booleanPositionRules.size(); i++){
            BooleanRule *rule = booleanPositionRules[i];
            if(rule->RHS.size() == 0){
                if(!firstSetVector[rule->LHS][0]){
                    firstSetVector[rule->LHS][0] = true;
                    changed = true;
                }
            } else {
                /* 
                    Check for epsilons 
                    If everything in the set can be epsilon, than the sets first set can include epsilon
                */

                int epsilons = 0;
                for(int j = 0; j < rule->RHS.size(); j++){
                    int currentRHSBooleanPosition = rule->RHS[j];

                    if(isMember(universeVector, combinedSymbols[currentRHSBooleanPosition])){
                        int pos = find(universeVector.begin(), universeVector.end(), combinedSymbols[rule->RHS[j]]) - universeVector.begin();
                        if(!firstSetVector[rule->LHS][pos]){
                            firstSetVector[rule->LHS][pos] = true;
                            changed = true;
                        }
                    } else {
                        /* Add first set of current RHS */
                        vector<bool> tmp = firstSetVector[rule->RHS[j]];
                        for(int k = 1; k < tmp.size(); k++){
                            if(tmp[k]){
                                if(!firstSetVector[rule->LHS][k]){
                                    firstSetVector[rule->LHS][k] = true;
                                    changed = true;
                                }
                            }
                        }
                    }
                    /* Check if the empty set is part of the firstSetVector */
                    if(firstSetVector[rule->RHS[j]][0] == false){
                        break;
                    } else {
                        epsilons++;
                    }
                }
                /* 
                    So if the # of epslions is equal to the amount of RHS we know that
                    every RHS value can be epsilon which means the set includes epsilon
                */
                if(epsilons == rule->RHS.size()){
                    firstSetVector[rule->LHS][0] = true;
                }

            }
        }
        if(changed == false){
            break;
        }
    }

    if(print){
        for(int i = universeVector.size(); i < firstSetVector.size(); i++){
            vector<string> firstOf;
            for(int j = 0; j < firstSetVector[i].size(); j++){
                if(firstSetVector[i][j]){
                    firstOf.push_back(universeVector[j]);
                }
            }
            cout << "FIRST(" << combinedSymbols[i] << ") = { ";
            for(int j = 0; j < firstOf.size(); j++){
                if(j == firstOf.size() - 1){
                    cout << firstOf[j];
                } else {
                    cout << firstOf[j] << ", ";
                }
            }
            cout << " }" << endl;
        }
    }
}

void Program::isPredictive(){
    CalculateFirstSets(false);
    CalculateFollowSets(false);

    bool isPredictive = true;

    for(int i = 0; i < combinedSymbols.size(); i++){
        if(firstSetVector[i][0]){
            for(int k = 0; k < universeVector.size(); k++){
                if(firstSetVector[i][k] == followSetVector[i][k]){
                    isPredictive = false;
                    break;
                }
            }
            if(!isPredictive){
                break;
            }
        }
    }
    if(isPredictive){
        for(int i = 0; i < booleanPositionRules.size(); i ++){
            for(int j = i; j < booleanPositionRules.size(); j ++){
                if(booleanPositionRules[i]->LHS == booleanPositionRules[j]->LHS){
                    int pos1 = booleanPositionRules[i]->RHS[0];
                    int pos2 = booleanPositionRules[j]->RHS[0];
                    for(int k = 0; k < universeVector.size(); k++){
                        if(firstSetVector[pos1][k] == firstSetVector[pos2][k]){
                            isPredictive = false;
                            break;
                        }
                    }
                }
                if(!isPredictive){
                    break;
                }
            }

            if(!isPredictive){
                break;
            }
        }
    }

    // for(int i = 0; i < combinedSymbols.size(); i++){

    //     for(int k = 0; k < universeVector.size(); k++){
    //         cout << firstSetVector[i][k] << " ";
    //     }
    //     cout << endl;
    // }

    // cout << endl;

    // for(int i = 0; i < combinedSymbols.size(); i++){

    //     for(int k = 0; k < universeVector.size(); k++){
    //         cout << followSetVector[i][k] << " ";
    //     }
    //     cout << endl;
    // }

    // cout << endl;

    if(isPredictive){
        cout << "YES" << endl;
    } else {
        cout << "NO" << endl;
    }

}

void Program::CalculateFollowSets(bool print){
    CalculateFirstSets(false);

    for(int i = 0; i < combinedSymbols.size(); i++){
        vector<bool> currentVector;
        for(int j = 0; j < universeVector.size(); j++){
            if(combinedSymbols[i] == universeVector[j] && universeVector[j] != "$"){
                currentVector.push_back(true);
            } else {
                currentVector.push_back(false);
            }
        }

        followSetVector.push_back(currentVector);
    }

    /* Rule #1 */
    followSetVector[booleanPositionRules[0]->LHS][1] = true;

    bool changed = true;
    while(changed){
        changed = false;
        for(int i = 0; i < booleanPositionRules.size(); i++){
            BooleanRule *rule = booleanPositionRules[i];
            bool nonEpsilonSeen = false;
            for(int j = rule->RHS.size() - 1; j >= 0; j--){
                if(isMember(nonTerminals, combinedSymbols[rule->RHS[j]] )) {
                    
                    /* rule #2 and #3 */
                    /* k == 0 would be the empty set, we don't want to add that */
                    /* And if a terminals been seen than we don't want to add EOF */
                    if(!nonEpsilonSeen){
                        for(int k = 0; k < universeVector.size(); k++){
                            if(followSetVector[rule->LHS][k]){
                                if(!followSetVector[rule->RHS[j]][k]){
                                    followSetVector[rule->RHS[j]][k] = true;
                                    changed = true;
                                }
                            }
                        }
                    }

                    /* Rule #4 */
                    if(j < rule->RHS.size() - 1){
                        for(int k = 2; k < universeVector.size(); k ++){
                            if(firstSetVector[rule->RHS[j+1]][k]){
                                if(!followSetVector[rule->RHS[j]][k]){
                                    followSetVector[rule->RHS[j]][k] = true;
                                    changed = true;
                                }
                            }
                        }
                    }
                    /* Rule #5 */
                    if(j < rule->RHS.size() - 1){
                        for(int q = j+1; q < rule->RHS.size(); q++){
                            for(int k = 2; k < universeVector.size(); k++){
                                if(firstSetVector[rule->RHS[q]][k]){
                                    if(!followSetVector[rule->RHS[j]][k]){
                                        followSetVector[rule->RHS[j]][k] = true;
                                        changed = true;
                                    }
                                }
                            }

                            if(!firstSetVector[rule->RHS[q]][0]){
                                break;
                            }
                        }
                    }

                    /* Part of rule all rules
                        Check if current rules first set includes Epsilon. 
                    */
                    if(!firstSetVector[rule->RHS[j]][0]){
                        nonEpsilonSeen = true;
                    }
                } else {
                    nonEpsilonSeen = true;
                }
            }
        }
    }

    if(print){
        for(int i = universeVector.size(); i < followSetVector.size(); i++){
            vector<string> followOf;
            for(int j = 0; j < followSetVector[i].size(); j++){
                if(followSetVector[i][j]){
                    followOf.push_back(universeVector[j]);
                }
            }
            cout << "FOLLOW(" << combinedSymbols[i] << ") = { ";
            for(int j = 0; j < followOf.size(); j++){
                if(j == followOf.size() - 1){
                    cout << followOf[j];
                } else {
                    cout << followOf[j] << ", ";
                }
            }
            cout << " }" << endl;
        }
    }
}

int Program::ParseProgram(int argc, char* argv[]){
    int task;

    if (argc < 2){
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
    */

    task = atoi(argv[1]);


    /* Add the empty and EOF */
    ParseTokens();

    switch (task) {
        case 1:
            PrintStringVector(terminals);
            cout << " ";
            PrintStringVector(nonTerminals);
            cout << endl;
            break;

        case 2:
            
            GenerateUsefulRules();
            break;

        case 3:
            CalculateFirstSets(true);
            break;

        case 4:
            CalculateFollowSets(true);
            break;

        case 5:
            isPredictive();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
};

int main (int argc, char* argv[]){
    Program *program = new Program();

    return program->ParseProgram(argc, argv);
}
