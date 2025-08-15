#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include "wlp4data.h"
#include <memory>

using namespace std;
const std::string CFG    = ".CFG";
const std::string INPUT      = ".INPUT";
const std::string TRANSITION = ".TRANSITIONS";
const std::string REDUCTION = ".REDUCTIONS";
const std::string END       = ".END";
const std::string EMPTY = ".EMPTY";
const std::string SHIFT = "shift";
const std::string PRINT = "print";
vector<string> cfg_rules;


struct PNode{
    string name;
    vector<shared_ptr<PNode>> children;
};
vector<shared_ptr<PNode>> reduct_sequence;
deque<shared_ptr<PNode>> input_sequence;
std::stack<string> states;
std::unordered_map<string,std::unordered_map<string,string>> transition;
std::unordered_map<string,unordered_map<string,string>> reduction;
int input_size;
string BOF_name = "BOF BOF";
string EOF_name = "EOF EOF";

void print(){
  for (int i = 0; i < reduct_sequence.size();i++){
    cout << reduct_sequence[i]->name << endl;
  }
  cout << "." << endl;

  for (int i = 0; i < input_sequence.size();i++){
    cout << input_sequence[i]->name << endl;
  }
  cout << endl;
}

void printTree(shared_ptr<PNode> cur){
    cout << cur->name <<endl;
    for(int i = cur->children.size()-1;i >= 0;i--){
        printTree(cur->children.at(i));
    }
}

void reduce(int n){
  string cur_rule = cfg_rules[n];
  istringstream tokens(cur_rule);
  string lhs;
  string rhs;
  tokens >> lhs; // remove the first symbol
  std::shared_ptr<PNode> internal = std::make_shared<PNode>(cur_rule);
  while(tokens >> rhs){
    if(rhs == EMPTY){
      continue;
    } else {
      internal->children.push_back(reduct_sequence.back());
      reduct_sequence.pop_back();
      states.pop();
    }
  }
  reduct_sequence.push_back(std::move(internal));
  states.push(transition[states.top()][lhs]);
}
void shift(){
  shared_ptr<PNode> temp = input_sequence.front();
  input_sequence.pop_front();
  reduct_sequence.push_back(temp);
}


int main() {
    istream& input = cin;
    string leaf_name;
    while(getline(input,leaf_name)){
        std::shared_ptr<PNode> leaf = std::make_shared<PNode>(leaf_name);
        input_sequence.emplace_back(leaf);
    }
    std::shared_ptr<PNode> bof = std::make_shared<PNode>(BOF_name);
    std::shared_ptr<PNode> eof = std::make_shared<PNode>(EOF_name);
    input_sequence.emplace_front(bof);
    input_sequence.emplace_back(eof);

    istringstream in (WLP4_COMBINED);
    std::string s;  
    std::getline(in, s); // CFG section (skip header)
    //Read CFG by lines
    getline(in, s);
    cfg_rules.push_back(s);
    // istringstream iss(s);
    // string temp;
    // iss >> temp;
    // start_symbol = temp;
    while(getline(in, s)) {
    if (s == TRANSITION) { 
            break; 
        } else {
            cfg_rules.push_back(s);
        }
    }

    getline(in,s); // first line
    std::string fromState, symbol, toState;
    std::istringstream line(s);
    std::vector<std::string> lineVec;
    while(line >> s) {
        lineVec.push_back(s);
    }
    fromState = lineVec.front();
    toState = lineVec.back();
    symbol = lineVec[1];
    states.push(fromState); // start_state
    transition[fromState][symbol] = toState;
    while(getline(in, s)) {
        if (s == REDUCTION) { 
            break;  
        } else {
            std::string fromState, symbol, toState;
            std::istringstream line(s);
            std::vector<std::string> lineVec;
            while(line >> s) {
                lineVec.push_back(s);
            }
            fromState = lineVec.front();
            toState = lineVec.back();
            symbol = lineVec[1];
            transition[fromState][symbol] = toState;
        }
    }


    while(std::getline(in, s)) {
        if (s == END) {
            break; 
        } else {
            std::string accpetingState, rule_num, lookahead;
            std::istringstream line(s);
            std::vector<std::string> lineVec;
            while(line >> s) {
                lineVec.push_back(s);
            }
            accpetingState = lineVec[0];
            rule_num = lineVec[1];
            lookahead = lineVec[2];
            reduction[accpetingState][lookahead] = rule_num;
        }    
    } 

    input_size = input_sequence.size();
    // print();
    while(reduct_sequence.size() == 0 || input_sequence.size() != 0){
        string cur_state = states.top();
        istringstream next_node(input_sequence.front()->name);
        string type;
        next_node >> type;
        // cout << cur_state << " "<<input_sequence[0]<< endl;
        if (reduction.find(cur_state) != reduction.end() && 
            reduction.at(cur_state).find(type)!= reduction.at(cur_state).end()){
            // we can reduce
            int rule_num = stoi(reduction[cur_state][type]);
            reduce(rule_num);
            // cout << cur_state <<" reduce"<< endl;
            // print();
            
        } else if (transition.find(cur_state)!= transition.end() &&
                   transition.at(cur_state).find(type) != transition.at(cur_state).end()){
            // we can shift
            states.push(transition[cur_state][type]);
            
            shift();
            // cout << cur_state << " shift"<< endl;
            // print();
        } else {
            // error
            int cur_sym = input_size - input_sequence.size() + 1 - 1;// doesnt count BOF
            cerr << "ERROR at " << cur_sym << endl;
            break;
        }
    }
    // if we get to BOF EOF we succeed
    // print();
    if (reduct_sequence.front()->name == BOF_name && reduct_sequence.back()->name == EOF_name){
        // cout << "WE R HERE!" << endl;
        reduce(0);
        // print();
    }
    // cout << reduct_sequence.size() << endl;
    // print tree;
    shared_ptr<PNode> cur_node = reduct_sequence.front();
    // cout << cur_node->name << endl;
    printTree(cur_node);
}
