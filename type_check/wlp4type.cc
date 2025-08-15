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
                                    "PLUS","MINUS","STAR","SLASH","PCT","AMP","NEW","LBRACK","RBRACK",
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
    unordered_map<string,BaseType> value_symbol_table;
    void addToSymbolTable(string key, BaseType value){
        if (this->value_symbol_table.count(key) > 0){
            ERROR = 1;
            error_code_q.emplace("Repeated symbol in symbol table");
        } else {
            this->value_symbol_table[key] = value;
            // cout << "I am in symbol table ";
            // printType(value);
            // cout << endl;
        }
    }
    void computeParamList(shared_ptr<PNode> paramlist){
        int num_of_children = paramlist->children.size();
        shared_ptr<PNode> dcl = paramlist->children[0];
        shared_ptr<PNode> type = dcl->children[0];
        int param_type = type->children.size();
        if(param_type == 1){
            paramList.emplace_back(BaseType::INT);
        } else {
            paramList.emplace_back(BaseType::INTSTAR);
        }

        if(num_of_children != 1){
            computeParamList(paramlist->children[2]);
        } 
    }
};
unordered_map<string, shared_ptr<Procedure>> procedures_symbol_table;




void printType(BaseType bt){
    if(bt == BaseType::INT){
        cout << " : " << "int";
    } else if (bt == BaseType::INTSTAR){
        cout << " : " << "int*";
    } else {
        // cout << " : " << "I fked up";
    }
}
void printTree(shared_ptr<PNode> cur_node){
    string cur_rule = cur_node->name;
    istringstream iss {cur_rule};
    string lhs;
    iss >> lhs;
    cout << cur_node->name;
    
    if(lhs == "ID"||lhs == "expr"||lhs == "term"||lhs == "factor"||lhs == "NUM"||lhs == "NULL"||lhs=="lvalue"){
        printType(cur_node->type);
    } 
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
shared_ptr<PNode> build_tree(istream& input, shared_ptr<Procedure> p){
    shared_ptr<Procedure> cur_procedure = p;
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
        if(cur_token != ".EMPTY"){
            num_of_children++;
        }      
    }
    int counter = 0;

    if (lhs == "procedure" || lhs == "main"){

        auto procedure = make_shared<Procedure>();          
        cur_procedure = procedure;
        if(lhs == "procedure"){
            std::streampos original_pos = input.tellg();
            std::string line1, line2;    
            // Try reading two lines
            getline(input, line1); // 1st line (we ignore or store if needed)
            getline(input, line2); // 2nd line (this is the one we want to preview)
            input.clear();                // Clear any eof flags
            input.seekg(original_pos);
            procedure->name = line2;
        }
     
    }
    while (counter < num_of_children){        
        node->children.emplace_back(build_tree(input,cur_procedure));
        counter++;
    }
    if(lhs == "dcl"){
        shared_ptr<PNode> type = node->children[0];
        shared_ptr<PNode> id = node->children[1];
        if(type->children.size() == 1){
            cur_procedure->addToSymbolTable(id->name,BaseType::INT);
        } else {
            cur_procedure->addToSymbolTable(id->name,BaseType::INTSTAR);
        }
    } 

 
    if(lhs == "main"){ // main     
        cur_procedure->name =  lhs;          
        addToProcedureTable(node->name,cur_procedure);
    } 
    if (lhs == "procedure") {
        shared_ptr<PNode> id = node->children[1];
        shared_ptr<PNode> params = node->children[3];
        if(params->children.size() != 0){
            shared_ptr<PNode> paramlist = params->children[0];
            cur_procedure->computeParamList(paramlist);
        }
        cur_procedure->name =    id->name;         
        addToProcedureTable(id->name,cur_procedure);
        // cerr << id->name <<" added to procedure symbol table " << endl;
    }      
    
    if(lhs == "factor"){
        // factor → ID LPAREN RPAREN
        // factor → ID LPAREN arglist RPAREN
        // factor → GETCHAR LPAREN RPAREN
        if(node->name == "factor ID LPAREN RPAREN" || node->name == "factor ID LPAREN arglist RPAREN"){
            shared_ptr<PNode> id = node->children[0];
            if(cur_procedure->value_symbol_table.count(id->name) > 0){
                ERROR = 1;
                error_code_q.emplace("Improper use of symbol");
            } else if(procedures_symbol_table.count(id->name) == 0 && id->name != cur_procedure->name ){// recursion case
                // cerr << node->name <<endl;
                // cerr << node->children[0]->name <<endl;
                ERROR = 1;
                error_code_q.emplace("Not in procedure symbol table(yet)");
            } else {
                // check later
            }
        } 

    } 
    
    return node;
}


/// @brief 
/// @param cur_node 
/// @param p 
/// @return 
BaseType type_check(shared_ptr<PNode> cur_node,shared_ptr<Procedure> p=nullptr){
    shared_ptr<Procedure> cur_Procedure = p;
    string cur_rule = cur_node->name;
    istringstream iss {cur_rule};
    string lhs;
    string cur_token;
    iss >> lhs;
    // cout << lhs << endl;
    if(lhs == "start"){
        type_check(cur_node->children[1]);
    } else if(lhs == "procedures"){
        for(shared_ptr<PNode> procedure:cur_node->children){
            type_check(procedure);
        }
    } else if(lhs == "procedure"){
        // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        shared_ptr<PNode> id = cur_node->children[1];
        shared_ptr<PNode> params = cur_node->children[3];
        shared_ptr<PNode> dcls = cur_node->children[6];
        shared_ptr<PNode> statements = cur_node->children[7];
        shared_ptr<PNode> return_expr = cur_node->children[9];
        
        //check type


        cur_Procedure = procedures_symbol_table[id->name];
        type_check(params,cur_Procedure);
        if(type_check(return_expr,cur_Procedure) != BaseType::INT){
            ERROR = 1;
            error_code_q.emplace("Wrong return type");
            return BaseType::NA;
        }
        type_check(dcls,cur_Procedure);
        type_check(statements,cur_Procedure);
        
    } else if(lhs == "main"){
        // cout << "I GOT TO main  BUDDY" << endl;
        shared_ptr<PNode> dcl1 = cur_node->children[3];
        shared_ptr<PNode> dcl2 = cur_node->children[5];
        shared_ptr<PNode> dcls = cur_node->children[8];
        shared_ptr<PNode> statements = cur_node->children[9];
        shared_ptr<PNode> return_expr = cur_node->children[11];

        // check type

        cur_Procedure = procedures_symbol_table[cur_node->name];
        type_check(dcl1,cur_Procedure);
        if(type_check(dcl2,cur_Procedure) != BaseType::INT ||type_check(return_expr,cur_Procedure) != BaseType::INT ){
            ERROR = 1;
            error_code_q.emplace("Second declaration of main is not int or return type is not int");
            return BaseType::NA;
        }         
        type_check(dcls,cur_Procedure);
        type_check(statements,cur_Procedure);
    } else if(lhs == "params"){
        int children_size = cur_node->children.size();
        if(children_size != 0){
            type_check(cur_node->children[0],cur_Procedure);
        } 
    } else if(lhs == "paramlist"){
        int children_size = cur_node->children.size();
        type_check(cur_node->children[0],cur_Procedure);
        if(children_size > 1){
            type_check(cur_node->children[2],cur_Procedure);
        } 
    } else if(lhs == "statements"){
        for(shared_ptr<PNode> statement:cur_node->children){
            type_check(statement,cur_Procedure);
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
            shared_ptr<PNode> test = cur_node->children[2];
            shared_ptr<PNode> statements1 = cur_node->children[5];
            shared_ptr<PNode> statements2 = cur_node->children[9];
            type_check(test,cur_Procedure);
            type_check(statements1,cur_Procedure);
            type_check(statements2,cur_Procedure);
        } else if(first_child_name == "WHILE while"){
            shared_ptr<PNode> test = cur_node->children[2];
            shared_ptr<PNode> statements = cur_node->children[5];
            type_check(test,cur_Procedure);
            type_check(statements,cur_Procedure);

        } else if(first_child_name == "PUTCHAR putchar" || first_child_name == "PRINTLN println"){
            shared_ptr<PNode> expr = cur_node->children[2];
            if(type_check(expr,cur_Procedure) != BaseType::INT){
                ERROR = 1;
                error_code_q.emplace("Wrong type of param for putchar/println");
                return BaseType::NA;
            }
        } else if(first_child_name == "DELETE delete"){
            shared_ptr<PNode> expr = cur_node->children[3];
            if(type_check(expr,cur_Procedure) != BaseType::INTSTAR){
                ERROR = 1;
                error_code_q.emplace("Wrong type of param for delete");
                return BaseType::NA;
            }
        } else{
            BaseType lvalue_type = type_check(cur_node->children[0],cur_Procedure);
            BaseType expr_type = type_check(cur_node->children[2],cur_Procedure);
            if(lvalue_type == BaseType::INT && expr_type == BaseType::INT){

            } else if(lvalue_type == BaseType::INTSTAR && expr_type == BaseType::INTSTAR){

            } else {
                ERROR = 1;
                error_code_q.emplace("LHS AND RHS of a statement are not the appropriate");
                return BaseType::NA;
            }

        }
    } else if(lhs == "test"){
        // test → expr EQ expr
        // test → expr NE expr
        // test → expr LT expr
        // test → expr LE expr
        // test → expr GE expr
        // test → expr GT expr
        shared_ptr<PNode> expr1 = cur_node->children[0];
        shared_ptr<PNode> expr2 = cur_node->children[2];
        BaseType left = type_check(expr1,cur_Procedure);
        BaseType right = type_check(expr2,cur_Procedure);
        if(left == BaseType::INT && right == BaseType::INT){

        } else if(left == BaseType::INTSTAR && right == BaseType::INTSTAR){

        } else {
            ERROR = 1;
            error_code_q.emplace("LHS and RHS of a test are not the same");
            return BaseType::NA;
        }

    } else if(lhs == "dcls"){
        //dcls .EMPTY
        //dcls → dcls dcl BECOMES NUM SEMI
        //dcls → dcls dcl BECOMES NULL SEMI 
        if(cur_node->children.size() > 0){
            shared_ptr<PNode> dcls = cur_node->children[0];
            shared_ptr<PNode> dcl = cur_node->children[1];
            shared_ptr<PNode> factor = cur_node->children[3];
            // cout << "FACTOR'S name IS "<< endl;
            type_check(dcls,cur_Procedure);
            if(type_check(dcl,cur_Procedure) != type_check(factor,cur_Procedure)){                
                ERROR = 1;
                error_code_q.emplace("LHS and RHS of a declaration are not the same");
                return BaseType::NA;
            }
        }

    } else if(lhs == "NUM"){
        // cout << "FACTOR'S name IS num"<< endl;
        cur_node->type = BaseType::INT;
    } else if(lhs == "NULL"){
        // cout << "FACTOR'S name IS null"<< endl;
        cur_node->type = BaseType::INTSTAR;
     
    } else if(lhs == "expr" ){
        // expr → expr PLUS term
        // expr → expr MINUS term
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            cur_node->type = type_check(cur_node->children[0],cur_Procedure);
        } else if(cur_node->children[1]->name == "PLUS +"){
            BaseType left = type_check(cur_node->children[0],cur_Procedure);
            BaseType right = type_check(cur_node->children[2],cur_Procedure);
            if ( left == BaseType::INT && right == BaseType::INT){
                cur_node->type = BaseType::INT;
            } else if ((left == BaseType::INTSTAR && right == BaseType::INT) ||
                       (left == BaseType::INT && right == BaseType::INTSTAR)){
                cur_node->type = BaseType::INTSTAR;
            } else {
                ERROR = 1;
                error_code_q.emplace("type error for operation PLUS ");
                return BaseType::NA;
            }
        } else {
            BaseType left = type_check(cur_node->children[0],cur_Procedure);
            BaseType right = type_check(cur_node->children[2],cur_Procedure);
            if ( left == BaseType::INTSTAR && right == BaseType::INT){
                cur_node->type = BaseType::INTSTAR;
            } else if ((left == BaseType::INTSTAR && right == BaseType::INTSTAR) ||
                       (left == BaseType::INT && right == BaseType::INT)){
                cur_node->type = BaseType::INT;
            } else {
                ERROR = 1;
                error_code_q.emplace("type error for operation MINUS ");
                return BaseType::NA;
            }
        }       

    } else if(lhs == "term"){
        // term → term STAR factor
        // term → term SLASH factor
        // term → term PCT factor
        // term → factor
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            cur_node->type = type_check(cur_node->children[0],cur_Procedure);
        } else {
            BaseType left = type_check(cur_node->children[0],cur_Procedure);
            BaseType right = type_check(cur_node->children[2],cur_Procedure);
            if ( left == BaseType::INT && right == BaseType::INT){
                cur_node->type = BaseType::INT;
            } else {
                ERROR = 1;
                error_code_q.emplace("type error for operation ");
                return BaseType::NA;
            }
        }
        // cur_node->type = type_check(cur_node->children[0],cur_Procedure);
    } else if(lhs == "factor"){  // oh fk

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
            cur_node->type = type_check(cur_node->children[0],cur_Procedure);
        } else if(childrenSize == 2){
            if (cur_node->children[0]->name == "STAR *"){
                if(type_check(cur_node->children[1],cur_Procedure) != BaseType::INTSTAR){
                    ERROR = 1;
                    error_code_q.emplace("wrong type for star factor");
                    return BaseType::NA;
                }
                cur_node->type = BaseType::INT;
            } else { // AMP lvalue
                if(type_check(cur_node->children[1],cur_Procedure) != BaseType::INT){
                    ERROR = 1;
                    error_code_q.emplace("wrong type for amp lvalue");
                    return BaseType::NA;
                }
                cur_node->type = BaseType::INTSTAR;
            }      
        }  else if(childrenSize == 3){
            if(cur_node->children[0]->name == "LPAREN ("){
                cur_node->type = type_check(cur_node->children[1],cur_Procedure);
            } else if(cur_node->children[0]->name == "GETCHAR getchar"){
                cur_node->type = BaseType::INT;
            } else {
                shared_ptr<PNode> id =  cur_node->children[0];
                if (cur_Procedure->value_symbol_table.count(id->name) > 0){
                    ERROR = 1;
                    error_code_q.emplace("Improper use of symbol");
                    return BaseType::NA;
                }else if(procedures_symbol_table.count(id->name) == 0){
                    ERROR = 1;
                    error_code_q.emplace("Procedure does not exist");
                    return BaseType::NA;
                } else if(procedures_symbol_table[id->name]->paramList.size() != 0){
                    ERROR = 1;
                    error_code_q.emplace("Wrong number of params");
                    return BaseType::NA;
                } else {
                    cur_node->type = BaseType::INT;
                }
            }            
        } else if(childrenSize == 4){
            // cout << "I AM HERE HEY!" << endl;
            shared_ptr<PNode> id =  cur_node->children[0];
            shared_ptr<PNode> argList =  cur_node->children[2];
            if (cur_Procedure->value_symbol_table.count(id->name) > 0){
                ERROR = 1;
                error_code_q.emplace("Improper use of symbol");
                return BaseType::NA;
            }else if(procedures_symbol_table.count(id->name) == 0){
                ERROR = 1;
                error_code_q.emplace("Procedure does not exist");
                return BaseType::NA;
            } else {  
                shared_ptr<Procedure> called_procedure = procedures_symbol_table[id->name];
                int called_procedure_param_size = called_procedure->paramList.size();
                if(called_procedure_param_size == 0){
                    ERROR = 1;
                    error_code_q.emplace("Need Param!");
                    return BaseType::NA;
                }
                shared_ptr<PNode> cur = argList;
                int index = 0;
                while (cur->children.size() > 1 && index < called_procedure_param_size - 1) {
                    if (type_check(cur->children[0],cur_Procedure) != called_procedure->paramList[index]) {
                        ERROR = 1;
                        error_code_q.emplace("Wrong type of param!!!!!!");
                        return BaseType::NA;
                    }
                    cur = cur->children[2]; 
                    
                    index++;
                    // cerr  << cur->children.size() << endl;
                    // cerr << index  << endl;
                    // cerr << called_procedure_param_size - 1  << endl;
                }
                // cerr << cur_node->children[0]->name << endl;
                // cerr << cur_Procedure->name << endl;
                // cerr  << cur->children.size() << endl;
                // cerr << index  << endl;
                // cerr << called_procedure_param_size - 1  << endl;
                if(cur->children.size() == 1 && index == called_procedure_param_size - 1){
                    if(type_check(cur->children[0],cur_Procedure) != called_procedure->paramList[index]){
                        ERROR = 1;
                        error_code_q.emplace("Wrong type of param!!");
                        return BaseType::NA;
                    }
                    cur_node->type = BaseType::INT;
                } else {
                    ERROR = 1;
                    error_code_q.emplace("Wrong number of param!!!");
                    return BaseType::NA;
                }
            }             

            // cur_node->type = type_check(cur_node->children[1],cur_Procedure);
        } else if (childrenSize == 5){
            if(type_check(cur_node->children[3],cur_Procedure) != BaseType::INT){
                ERROR = 1;
                error_code_q.emplace("wrong type for NEW INT LBRACK expr RBRACK");
                return BaseType::NA;
            }
            cur_node->type = BaseType::INTSTAR;
        }  
    
    } else if(lhs == "ID"){ // oh fk
        if(cur_Procedure->value_symbol_table.count(cur_node->name) > 0){
            // cout << "I AM HERE CHECKING symbol TABLE " << cur_node->name << endl;
            cur_node->type = cur_Procedure->value_symbol_table[cur_node->name];
        } else if(procedures_symbol_table.count(cur_node->name)) { //is a func
            // cout << "I AM HERE CHECKING PROCEDURE TABLE" << endl;
            cur_node->type = BaseType::FUNC;
        } else {
            ERROR = 1;
            error_code_q.emplace("Symbol doesnt exist in both symbol table");
            return BaseType::NA;
        }
        

    
    } else if (lhs == "dcl"){
        cur_node->type = type_check(cur_node->children[1],cur_Procedure);

    } else if (lhs == "lvalue"){
        // lvalue → ID
        // lvalue → STAR factor
        // lvalue → LPAREN lvalue RPAREN
        int childrenSize = cur_node->children.size();
        if(childrenSize == 1){
            cur_node->type = type_check(cur_node->children[0],cur_Procedure);
        } else if(childrenSize == 2){ // must dereference a ptr
            if(type_check(cur_node->children[1],cur_Procedure) != BaseType::INTSTAR){
                ERROR = 1;
                error_code_q.emplace("Dereferencing a non ptr type");
                return BaseType::NA;
            } else {
                cur_node->type = BaseType::INT;
            }
        } else {
            cur_node->type = type_check(cur_node->children[1],cur_Procedure);
        }

    } else {

    }
    return cur_node->type;

}

int main() {
    istream& input = cin;
    shared_ptr<PNode> parse_tree_node = build_tree(input,nullptr);
    if (ERROR == 1){
        cerr << "ERROR: " << error_code_q.front() << endl;
        
    } else {

        type_check(parse_tree_node);
        if (ERROR == 1){
            cerr << "ERROR: " << error_code_q.front() << endl;
        } else {
            printTree(parse_tree_node);
        }
    }
    
    
}




