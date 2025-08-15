#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <memory>
using namespace std;

int ERROR = 0;

unordered_set<string> terminals = {"INT","LPAREN","RPAREN","COMMA","ID","LBRACE","RBRACE",
                                    "RETURN","SEMI","STAR","NUM","NULL", "BOF","EOF","WAIN","BECOMES",
                                    "PLUS","MINUS","SLASH","PCT","AMP","NEW","LBRACK","RBRACK",
                                    "GETCHAR",
                                    "IF","ELSE","WHILE","PRINTLN","PUTCHAR","DELETE",
                                    "EQ","NE","LT","LE","GE","GT" };
enum class BaseType {
    INT,
    INTSTAR,
    FUNC,
    NA
};
queue<string> error_code_q;
int if_counter = 0;
int while_counter = 0;
int delete_counter = 0;
// unordered_map<string,BaseType> symbol_table;
struct PNode{
    BaseType type;
    string name;
    vector<shared_ptr<PNode>> children;
    PNode(){
        type = BaseType::NA;
    }
};


struct Procedure{
    string name;
    vector<BaseType> paramList;
    unordered_map<string,pair<BaseType,int>> value_symbol_table;
    int offset;
    int param_num;
    int num_of_var;
    void addToSymbolTable(string key, BaseType value){
        // cout << "ADDING TO SYMBOL TABLE " <<  key << " " << offset << endl;
        this->value_symbol_table[key] = make_pair(value,offset);
        offset -= 4;
        num_of_var++;  
    }
    // void computeParamList(shared_ptr<PNode> paramlist){
    //     int num_of_children = paramlist->children.size();
    //     shared_ptr<PNode> dcl = paramlist->children[0];
    //     shared_ptr<PNode> type = dcl->children[0];
    //     int param_type = type->children.size();
    //     if(param_type == 1){
    //         paramList.emplace_back(BaseType::INT);
    //     } else {
    //         paramList.emplace_back(BaseType::INTSTAR);
    //     }

    //     if(num_of_children != 1){
    //         computeParamList(paramlist->children[2]);
    //     } 
    // }
};
unordered_map<string, shared_ptr<Procedure>> procedures_symbol_table;


void printTree(shared_ptr<PNode> cur_node){
    string cur_rule = cur_node->name;
    istringstream iss {cur_rule};
    string lhs;
    iss >> lhs;
    cout << cur_node->name;
    
    
    cout << endl;
    
    for(size_t i = 0;i < cur_node->children.size();i++){
        printTree(cur_node->children.at(i));
    }
}

void addToProcedureTable(string key,shared_ptr<Procedure> value){
    if(procedures_symbol_table.count(key) > 0){ // already existed
        ERROR = 1;
        error_code_q.emplace("Duplicate symbol in procedure symbol table");
    } else {
        procedures_symbol_table[key] = value;
    }
}
shared_ptr<PNode> gen_tree(istream& input){
    
    auto node = make_shared<PNode>();
    string cur_rule;
    getline(input,cur_rule);

    node->name = cur_rule;
    string cur_token;
    istringstream iss {cur_rule};
    string lhs;
    iss >> lhs; // remove the lhs

    if(terminals.count(lhs)){
        return node; // if its a terminal return directly
    }

    int num_of_children = 0;
    while (iss >> cur_token){ 
        if(cur_token == ":"){
            break;
        }
        if(cur_token != ".EMPTY"){
            num_of_children++;
        }      
    }
    int counter = 0;

    while (counter < num_of_children){        
        node->children.emplace_back(gen_tree(input));
        counter++;
    }    
    return node;
}
int calcParamNum(shared_ptr<PNode> params){
    int count = 0;
    if(params->children.size() > 0){
        shared_ptr<PNode> paramList = params->children[0];
        while(paramList->children.size() > 1){
            count++;
            paramList = paramList->children[2];
        }
        count++; // take the last dcl into account
        return count;
    } else {
        return count;
    }

}
void build_symbol_table(shared_ptr<PNode> node, shared_ptr<Procedure> p=nullptr){
    shared_ptr<Procedure> cur_procedure = p;   
    string cur_rule = node->name;
    string cur_token;
    istringstream iss {cur_rule};
    string lhs;
    iss >> lhs; // remove the lhs

    if(terminals.count(lhs)){
        return;
    }

    int num_of_children = node->children.size();
    

    if (lhs == "main"){
        auto procedure = make_shared<Procedure>();          
        cur_procedure = procedure;
        cur_procedure->name = "main";
        cur_procedure->offset = 8;  
     
    } 
    if(lhs == "procedure" ){
        auto procedure = make_shared<Procedure>();          
        cur_procedure = procedure;
        // set procedure name
        shared_ptr<PNode> id= node->children[1];
        istringstream idName {id->name};
        string label;
        idName >> label;
        idName >> label;
        cur_procedure->name = label; 
        // set offset 
        shared_ptr<PNode> params = node->children[3];
        int param_num = calcParamNum(params);
        cur_procedure->param_num = param_num;
        cur_procedure->offset = 4 * param_num;  
    }

    int counter = 0;
    while (counter < num_of_children){        
        build_symbol_table(node->children[counter],cur_procedure);
        counter++;
    }

    

    if(lhs == "dcl"){
        shared_ptr<PNode> type = node->children[0];
        shared_ptr<PNode> id = node->children[1];
        // cout << cur_procedure->name << endl;
        istringstream extract {id->name};
        string temp;
        extract >> temp;
        extract >> temp;

        
        if(type->children.size() == 1){
            cur_procedure->addToSymbolTable(temp,BaseType::INT);

        } else {
            cur_procedure->addToSymbolTable(temp,BaseType::INTSTAR);
        }
        
        // for non main procedure offset depends on the number of params

    }

    if(lhs == "main"||lhs == "procedure"){ // main                  
        addToProcedureTable(cur_procedure->name,cur_procedure);
    } 


    
    
}
void pushReg(int num){
    cout << "sw $" << num <<" , -4( $30 ) " << endl;
    cout << "sub $30 , $30 , $4" << endl;   

}

void popReg(int num){
    cout << "add $30 , $30 , $4" << endl;
    cout << "lw $" << num << " , -4( $30 )" << endl;
}

void prologue(int type,BaseType initType=BaseType::NA){
    cout << "; begin prologue" << endl;
    if(type == 0){ // main        
        cout << ".import print" << endl;
        cout << ".import init" << endl;
        cout << ".import new" << endl;
        cout << ".import delete" << endl;        
        cout << "lis $20" << endl;
        cout << ".word 0xffff000c ; putchar" << endl;
        cout << "lis $21" << endl;
        cout << ".word 0xffff0004 ; getchar" << endl;
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        if(initType == BaseType::INT){
            pushReg(2);
            pushReg(31);
            cout << "add $2, $0, $0" << endl;
            cout << "lis $5" << endl;
            cout << ".word init" << endl;
            cout << "jalr $5" << endl;
            popReg(31);
            popReg(2);
        } else if (initType == BaseType::INTSTAR){
            pushReg(31);
            cout << "lis $5" << endl;
            cout << ".word init" << endl;
            cout << "jalr $5" << endl;
            popReg(31);
        } else{
            cerr << "WRONG type for prologue\n";
        }
        cout << "sw $1 , -4( $30 ) ; push parameter variable a" << endl;
        cout << "sub $30 , $30 , $4 ; update stack pointer" << endl;
        cout << "sw $2 , -4( $30 ) ; push parameter variable b" << endl;
        cout << "sub $30 , $30 , $4 ; update stack pointer" << endl;
        cout << "sub $29 , $30 , $4 ; set frame pointer ( after pushing parameters , before pushing non - parameter" << endl;
    } else {
        cout << "sub $29 , $30 , $4 ; set frame pointer ( after pushing parameters , before pushing non - parameter" << endl;
    }


    
    cout << "; end prologue" << endl;

}

void epilogue(int num_of_var){
    cout << "; begin epilogue" << endl;
    for(int i = 0; i < num_of_var;i++){
        cout << "add $30 , $30 , $4" << endl;
    }
    cout << "jr $31" << endl;
}


int CalcOffset(shared_ptr<PNode> cur_node,shared_ptr<Procedure> p=nullptr){
    shared_ptr<Procedure> cur_Procedure = p;
    string cur_rule = cur_node->name;
    istringstream iss {cur_rule};
    string lhs;
    string cur_token;
    iss >> lhs;
    if(lhs == "lvalue"){
        if(cur_node->children.size() == 1){
            shared_ptr<PNode> ID = cur_node->children[0];
            stringstream label {ID->name};
            string my_label;
            label >> my_label;
            label >> my_label;
            return p->value_symbol_table[my_label].second;            
        } else if(cur_node->children.size() == 2){
            
        } else {
            return CalcOffset(cur_node->children[1],p);
        }
    }
    cerr << "ERROR lvalue not detected" << endl;
    return 0;
}

/// @brief 
/// @param cur_node 
/// @param p 
/// @return 
void code_gen(shared_ptr<PNode> cur_node,shared_ptr<Procedure> p=nullptr){
    shared_ptr<Procedure> cur_Procedure = p;
    string cur_rule = cur_node->name;
    istringstream iss {cur_rule};
    string lhs;
    string cur_token;
    iss >> lhs;

    // cout << lhs << endl;
    if(lhs == "start"){
        code_gen(cur_node->children[1]);
    } else if(lhs == "procedures"){
        if(cur_node->children.size() == 1){
            code_gen(cur_node->children[0]);
        } else {
            code_gen(cur_node->children[1]); // generate wain first
            code_gen(cur_node->children[0]);
        }
        
        
    } else if(lhs == "procedure"){
        // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        shared_ptr<PNode> id = cur_node->children[1];
        shared_ptr<PNode> params = cur_node->children[3];
        shared_ptr<PNode> dcls = cur_node->children[6];
        shared_ptr<PNode> statements = cur_node->children[7];
        shared_ptr<PNode> return_expr = cur_node->children[9];
        istringstream proID {id->name};
        string label;
        proID >> label;
        proID >> label;
        cur_Procedure = procedures_symbol_table[label];
        cout << "MY" << cur_Procedure->name << ":" << endl;
        prologue(1);
        code_gen(dcls,cur_Procedure);
        code_gen(statements,cur_Procedure);

        code_gen(return_expr,cur_Procedure);
        epilogue(cur_Procedure->num_of_var - cur_Procedure->param_num);
        
    } else if(lhs == "main"){
        // cout << "I GOT TO main  BUDDY" << endl;
        shared_ptr<PNode> dcl1 = cur_node->children[3];
        shared_ptr<PNode> dcl1_type = dcl1->children[0];
        if(dcl1_type->children.size() == 2){
            prologue(0,BaseType::INTSTAR);
        } else {
            prologue(0,BaseType::INT);
        }
        

        shared_ptr<PNode> dcl2 = cur_node->children[5];
        shared_ptr<PNode> dcls = cur_node->children[8];
        shared_ptr<PNode> statements = cur_node->children[9];
        shared_ptr<PNode> return_expr = cur_node->children[11];
        
        // check type
        cur_Procedure = procedures_symbol_table["main"];   
        
        
        code_gen(dcls,cur_Procedure);
        code_gen(statements,cur_Procedure);
        
        code_gen(return_expr,cur_Procedure);

        // cout << cur_Procedure->num_of_var << endl;
        epilogue(cur_Procedure->num_of_var);


    } else if(lhs == "params"){
        // int children_size = cur_node->children.size();
        // if(children_size != 0){
        //     code_gen(cur_node->children[0],cur_Procedure);
        // } 
    } else if(lhs == "paramlist"){
        // int children_size = cur_node->children.size();
        // code_gen(cur_node->children[0],cur_Procedure);
        // if(children_size > 1){
        //     code_gen(cur_node->children[2],cur_Procedure);
        // } 
    } else if(lhs == "statements"){
        for(shared_ptr<PNode> statement:cur_node->children){
            code_gen(statement,cur_Procedure);
        }
    } else if(lhs == "statement"){
        // statement → lvalue BECOMES expr SEMI
        // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
        // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
        // statement → PRINTLN LPAREN expr RPAREN SEMI
        // statement → PUTCHAR LPAREN expr RPAREN SEMI
        // statement → DELETE LBRACK RBRACK expr SEMI        
        string first_child_name = cur_node->children[0]->name;

         if(first_child_name == "IF if"){

            pushReg(5);
            int cur_if_counter = if_counter;
            if_counter++;
            shared_ptr<PNode> test = cur_node->children[2];
            shared_ptr<PNode> statements1 = cur_node->children[5];
            shared_ptr<PNode> statements2 = cur_node->children[9];
            code_gen(test,cur_Procedure);
            cout << "beq $3, $0, else" << cur_if_counter << endl;
            code_gen(statements1,cur_Procedure);
            cout << "beq $0, $0, endif" << cur_if_counter << endl;
            cout << "else" << cur_if_counter << ":" << endl;
            code_gen(statements2,cur_Procedure);
            cout << "endif" << cur_if_counter << ":" << endl;
            popReg(5);

        } else if(first_child_name == "WHILE while"){
            shared_ptr<PNode> test = cur_node->children[2];
            shared_ptr<PNode> statements = cur_node->children[5];
            pushReg(5);

            int cur_while_counter = while_counter;
            while_counter++;
            cout << "loop" << cur_while_counter <<":" << endl;            
            code_gen(test,cur_Procedure);
            cout << "beq $3, $0, endWhile" << cur_while_counter << endl;            
            code_gen(statements,cur_Procedure);
            cout << "beq $0, $0, loop" << cur_while_counter << endl;
            cout << "endWhile" << cur_while_counter <<":"<< endl;     
            popReg(5);
        } else if(first_child_name == "PUTCHAR putchar" ){
            
            shared_ptr<PNode> expr = cur_node->children[2];
            code_gen(expr,cur_Procedure);
            
            cout << "sw $3, 0($20)" << endl;         
        } else if(first_child_name == "PRINTLN println"){
            shared_ptr<PNode> expr = cur_node->children[2];
            pushReg(1);
            code_gen(expr,cur_Procedure);
            cout << "add $1, $3, $0" << endl;            
            pushReg(31);
            cout << "lis $5" << endl;
            cout << ".word print" << endl;
            cout << "jalr $5" << endl;
            popReg(31);
            popReg(1);

        } else if(first_child_name == "DELETE delete"){
            shared_ptr<PNode> expr = cur_node->children[3];
            code_gen(expr,cur_Procedure);
            cout << "beq $3, $11, MYskipDelete"<< delete_counter << endl;
            pushReg(1);
            cout << "add $1, $3, $0" << endl;
            pushReg(31);
            cout << "lis $5" << endl;
            cout << ".word delete" << endl;
            cout << "jalr $5" << endl;
            popReg(31);
            popReg(1);
            cout << "MYskipDelete" << delete_counter<<":" << endl;     
            delete_counter++;
	        
            
        } else{ //
            shared_ptr<PNode> lvalue = cur_node->children[0];
            shared_ptr<PNode> expr = cur_node->children[2];
            while(lvalue->children.size() == 3){
                lvalue = lvalue->children[1];
            }
            
            if( lvalue->children.size() == 1){
                shared_ptr<PNode> id = lvalue->children[0];                
                stringstream label {id->name};
                string my_label;
                label >> my_label;
                label >> my_label;
                int offset = cur_Procedure->value_symbol_table[my_label].second;
                code_gen(expr,cur_Procedure);
                cout << "sw $3, " << offset << "($29)" << "; changing value at offset " << offset << endl;  
            } else if( lvalue->children.size() == 2){
                shared_ptr<PNode> factor = lvalue->children[1];
                code_gen(factor,cur_Procedure);
                // cout << "lw $3, 0($3)" << endl;
                pushReg(3);
                code_gen(expr,cur_Procedure);
                popReg(5);             
                cout << "sw $3, 0($5)" << endl;

            } else {
                cerr << "problematic decompition of lvalue\n";
            }  
            // int offset = CalcOffset(cur_node->children[0],cur_Procedure);            
            
                
        }
    } else if(lhs == "NUM"){
        // cout << "FACTOR'S name IS num"<< endl;
        iss >> cur_token;
        cout << "lis $3" << endl;
        cout << ".word " << cur_token << endl;
        
    } else if(lhs == "NULL"){
        // cout << "FACTOR'S name IS null"<< endl;
        cout << "lis $3" << endl;
        cout << ".word " << "1" << endl;
     
    } else if(lhs == "test"){
        // test → expr EQ expr
        // test → expr NE expr
        // test → expr LT expr
        // test → expr LE expr
        // test → expr GE expr
        // test → expr GT expr
        shared_ptr<PNode> expr1 = cur_node->children[0];
        string op = cur_node->children[1]->name;
        shared_ptr<PNode> expr2 = cur_node->children[2];
        
        istringstream leftiss  {expr1->name};
        string temp;
        while(leftiss >> temp){
            if(temp == ":"){
                break;
            } 
        }
        leftiss >> temp;
        string left_type = temp;


        code_gen(expr1,cur_Procedure);
        pushReg(3);
        code_gen(expr2,cur_Procedure);
        popReg(5);
        
        if(op == "EQ =="){
            cout << "beq $3, $5, 2" << endl;
            cout << "add $3, $0, $0" << endl;
            cout << "beq $0, $0, 1" << endl;
            cout << "add $3, $0, $11 ; earlier we have set $11 = 1" << endl;
        } else if(op == "NE !="){
            cout << "beq $3, $5, 2" << endl;
            cout << "add $3, $0, $0" << endl;
            cout << "beq $0, $0, 1" << endl;
            cout << "add $3, $0, $11 ; earlier we have set $11 = 1" << endl;
            cout << "sub $3, $11, $3" << endl;
        } else if(op == "LT <"){
            if(left_type == "int"){
                cout << "slt $3,$5,$3" << endl;
            } else {
                cout << "sltu $3,$5,$3" << endl;
            }
        } else if(op == "LE <="){
            if(left_type == "int"){
                cout << "slt $3,$3,$5" << endl;
            } else {
                cout << "sltu $3,$5,$3" << endl;
            }
            cout << "sub $3, $11, $3" << endl;
        } else if(op == "GT >"){
            if (left_type == "int"){
                cout << "slt $3,$3,$5" << endl;
            } else {
                cout << "sltu $3,$3,$5" << endl;
            }
        } else{ // >= 
            if (left_type == "int"){
                cout << "slt $3,$5,$3" << endl;
            } else {
                cout << "sltu $3,$5,$3" << endl;
            }
            cout << "sub $3, $11, $3" << endl;
        }
        


    } else if(lhs == "dcls"){
        //dcls .EMPTY
        //dcls → dcls dcl BECOMES NUM SEMI
        //dcls → dcls dcl BECOMES NULL SEMI 
        if(cur_node->children.size() > 0){
            shared_ptr<PNode> dcls = cur_node->children[0];
            code_gen(dcls,cur_Procedure);

            shared_ptr<PNode> dcl = cur_node->children[1];

            shared_ptr<PNode> id = dcl->children[1];
            istringstream label {id->name};
            string var;
            label >> var;
            label >> var;
            BaseType varType = cur_Procedure->value_symbol_table[var].first;
            if(varType == BaseType::INT){
                shared_ptr<PNode> num = cur_node->children[3];
                istringstream val {num->name};
                string temp;  
                val >> temp;
                val >> temp;
                int offset = cur_Procedure->value_symbol_table[var].second;
                cout << "lis $3 ; load initial value for " << "var" << endl;
                cout << ".word " << temp << endl;
                cout << "sw $3 , " << offset <<  "( $29 ) ;" << endl;
                cout << "sub $30 , $30 , $4 ; update stack pointer" << endl;     
            } else if(varType == BaseType::INTSTAR){
                shared_ptr<PNode> null = cur_node->children[3];
                int offset = cur_Procedure->value_symbol_table[var].second;
                cout << "lis $3 ; load initial value for " << "var" << endl;
                cout << ".word " << "1" << endl;
                cout << "sw $3 , " << offset <<  "( $29 ) ;" << endl;
                cout << "sub $30 , $30 , $4 ; update stack pointer" << endl;
            }
            
        }
    } else if(lhs == "expr"){
        // expr → expr PLUS term
        // expr → expr MINUS term
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            code_gen(cur_node->children[0],cur_Procedure);
        } else {
            shared_ptr<PNode> left = cur_node->children[0];
            shared_ptr<PNode> right = cur_node->children[2];
            istringstream leftiss  {left->name};
            istringstream rightiss  {right->name};
            string temp;
            while(leftiss >> temp){
                if(temp == ":"){
                    break;
                } 
            }
            leftiss >> temp;
            string left_type = temp;

            while(rightiss >> temp){
                if(temp == ":"){
                    break;
                } 
            }
            rightiss >> temp;
            string right_type = temp;
            code_gen(left,cur_Procedure);
            pushReg(3);
            code_gen(right,cur_Procedure);
            popReg(5);
            if(cur_node->children[1]->name == "PLUS +"){
                if(left_type == "int" && right_type == "int"){
                    
                } else if(left_type == "int*" && right_type == "int"){
                    cout << "mult $3 , $4 ; ptr on the left" << endl;
                    cout  << "mflo $3" << endl;
                } else {
                    cout << "mult $5 , $4 ; $4 always has the value 4" << endl;
                    cout << "mflo $5" << endl;     
                }
                cout << "add $3, $3, $5" << endl;
            } else {
                if(left_type == "int" && right_type == "int"){
                    cout << "sub $3, $5, $3" << endl;
                } else if(left_type == "int*" && right_type == "int"){
                    cout << "mult $3 , $4 ; $4 always has the value 4" << endl;
                    cout  << "mflo $3" << endl;
                    cout << "sub $3, $5, $3" << endl;
                } else {
                    cout << "sub $3, $5, $3" << endl;
                    cout << "div $3, $4" << endl;
                    cout << "mflo $3"<< endl;
                }
            }





        }
        
        
        
        
        

    } else if(lhs == "term"){
        // term → term STAR factor
        // term → term SLASH factor
        // term → term PCT factor
        // term → factor
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            code_gen(cur_node->children[0],cur_Procedure);
        } else {
            string op = cur_node->children[1]->name;

            code_gen(cur_node->children[0],cur_Procedure);
            pushReg(3);
            code_gen(cur_node->children[2],cur_Procedure);
            popReg(5);
            if ( op == "STAR *"){
                cout << "mult $3, $5" << endl;
                cout << "mflo $3" << endl;
            } else if(op == "SLASH /"){
                cout << "div $5, $3" << endl;
                cout << "mflo $3" << endl;
            } else {
                cout << "div $5, $3" << endl;
                cout << "mfhi $3" << endl;
            }
        }

    } else if(lhs == "factor"){  // oh fk
        // cout << "IM AT factor " << cur_token << endl;
        // factor → NUM
        // factor → NULL
        // factor → ID

        // factor → AMP lvalue
        // factor → STAR factor

        // factor → LPAREN expr RPAREN  
        // factor → ID LPAREN RPAREN
        // factor → GETCHAR LPAREN RPAREN

        // factor → ID LPAREN arglist RPAREN

        // factor → NEW INT LBRACK expr RBRACK        
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            code_gen(cur_node->children[0],cur_Procedure);
        
        }else if(childrenSize == 2){
            if (cur_node->children[0]->name == "STAR *"){
                code_gen(cur_node->children[1],cur_Procedure);
                cout << "lw $3 , 0($3) ; $3 now contains the value at the address" << endl;
            } else { // AMP lvalue
                shared_ptr<PNode> lvalue = cur_node->children[1];
                while(lvalue->children.size() == 3){
                    lvalue = lvalue->children[1];
                }
                if( lvalue->children.size() == 1){
                    shared_ptr<PNode> id = lvalue->children[0];
                    
                    stringstream label {id->name};
                    string my_label;
                    label >> my_label;
                    label >> my_label;
                    int offset = cur_Procedure->value_symbol_table[my_label].second;
                    cout << "lis $3" << endl;
                    cout << ".word "<< offset << endl;
                    cout << "add $3, $29, $3" << endl;

                } else if( lvalue->children.size() == 2){
                    shared_ptr<PNode> factor = lvalue->children[1];
                    code_gen(factor,cur_Procedure);
                } else {
                    cerr << "problematic decompition of lvalue\n";
                }                
            }      
        
        } else if(childrenSize == 3){
            if(cur_node->children[0]->name == "LPAREN ("){
                code_gen(cur_node->children[1],cur_Procedure);
            } else if(cur_node->children[0]->name == "GETCHAR getchar"){ // getchar     
                cout << "lw $3, 0($21)" << endl; 
            } else {
                shared_ptr<PNode> id =  cur_node->children[0];
                string id_rule = id->name;  
                istringstream idName {id_rule};
                string label;
                idName >> label;
                idName >> label;

                pushReg(29);
                pushReg(31);
                cout<< "lis $5"<< endl;
                cout<< ".word "<< "MY" << label << endl;
                cout<< "jalr $5"<< endl;
                popReg(31);
                popReg(29);
            }
                     
        } else if(childrenSize == 4){
            // cout << "I AM HERE HEY!" << endl;
            shared_ptr<PNode> id =  cur_node->children[0];
            string id_rule = id->name;
            istringstream idName {id_rule};
            string label;
            idName >> label;
            idName >> label;//label stores procedure name
            shared_ptr<Procedure> callee = procedures_symbol_table[label];
            pushReg(29);
            pushReg(31);

            shared_ptr<PNode> argList =  cur_node->children[2];
            while(argList->children.size() > 1){
                shared_ptr<PNode> expr = argList->children[0];
                code_gen(expr,cur_Procedure);
                pushReg(3);
                argList = argList->children[2];
            }
            shared_ptr<PNode> expr = argList->children[0];
            code_gen(expr,cur_Procedure);
            pushReg(3);

            cout<< "lis $5"<< endl;
            cout<< ".word "<< "MY" << label << endl;
            cout<< "jalr $5"<< endl;        

            for(int i = 0; i < callee->param_num;i++){
                cout << "add $30, $30, $4;"<< "param_num: "<< i << endl;
            }
            popReg(31);
            popReg(29);
            
        } else if (childrenSize == 5){
            shared_ptr<PNode> expr =  cur_node->children[3];
            code_gen(expr,cur_Procedure);
            
            pushReg(1);
            pushReg(31);
            cout << "add $1, $3, $0" << endl;
            cout << "lis $5" << endl;
            cout << ".word new" << endl;
            cout << "jalr $5"<< endl;
            
            cout << "bne $3, $0, 1" << endl;
            cout << "add $3, $11, $0 ; if new failed, $3 = 0, so we set $3 = 1 = NULL" << endl;
            

            popReg(31);
            popReg(1);
        } else {
            cerr << "ERROR\n";
        }          

            
    
    } else if(lhs == "ID"){ // oh fk
        iss >> cur_token;
        // cout << "IM AT ID " << cur_token << endl;
        int offset = cur_Procedure->value_symbol_table[cur_token].second;
        cout << "lw $3 , " << offset << "( $29 )" << endl;       

    
    } else if (lhs == "dcl"){
        code_gen(cur_node->children[1],cur_Procedure);

    } else if (lhs == "lvalue"){
        // lvalue → ID
        // lvalue → STAR factor
        // lvalue → LPAREN lvalue RPAREN
        // int childrenSize = cur_node->children.size();
        // if(childrenSize == 1){
        //     cur_node->type = code_gen(cur_node->children[0],cur_Procedure);
        // } else if(childrenSize == 2){ // must dereference a ptr
        //     if(code_gen(cur_node->children[1],cur_Procedure) != BaseType::INTSTAR){
        //         ERROR = 1;
        //         error_code_q.emplace("Dereferencing a non ptr type");
        //         return BaseType::NA;
        //     } else {
        //         cur_node->type = BaseType::INT;
        //     }
        // } else {
        //     cur_node->type = code_gen(cur_node->children[1],cur_Procedure);
        // }

    } else {

    }
    

    

    
}


int main() {
    istream& input = cin;
    shared_ptr<PNode> parse_tree_node = gen_tree(input);
    build_symbol_table(parse_tree_node,nullptr);
    // printTree(parse_tree_node);
    code_gen(parse_tree_node,nullptr);
    // cerr << symbol_table_address_counter << endl;
}



