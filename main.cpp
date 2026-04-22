#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stack>
using namespace std;

// You will need these forward references.
class Expr;
class Stmt;

// Runtime Global Variables
int pc;              // program counter
vector<string> lexemes;
vector<string> tokens;
vector<string>::iterator lexitr;
vector<string>::iterator tokitr;
map<string, string> symbolvalues;   // map of variables and their values
map<string, string> symboltable; // map of variables to datatype (i.e. sum t_integer)
vector<Stmt *> insttable;      // table of instructions
map<string,int> precMap;


// Runtime Global Methods
				// prints vartable, instable, symboltable

bool isOperator(string lex) {
    return lex == "+" ||
            lex == "*" ||
            lex == "-" ||
            lex == "/" ||
            lex == "%" ||
            lex == "and" ||
            lex == "or" ||
            lex == "<" ||
            lex == "<=" ||
            lex == ">" ||
            lex == ">=" ||
            lex == "==" ||
            lex == "!=";
}

// Classes Stmt and Expr
// You are allowed to add methods if needed. You should NOT need to add member variables.

class Expr{ // expressions are evaluated!
public:
	virtual string toString() = 0;
	virtual ~Expr(){}
};

class StringExpr : public Expr{
public:
    virtual string eval() = 0;
};

class IntExpr : public Expr{
public:
    virtual int eval() = 0;
};

class StringConstExpr : public StringExpr {
private:
    string value;
public:
    StringConstExpr(string val) {
        value = val;
    }

    ~StringConstExpr() {}

    string eval() {return value;}

    string toString() {return "String Constant Expression: " + value;}
};

class StringIDExpr : public StringExpr {
private:
    string id;

public:
    StringIDExpr(string val) {
        id = val;
    }

    ~StringIDExpr() {}

    string eval() {return symbolvalues[id];}

    string toString() {return "String ID Expression: " + id + ", " + symbolvalues[id];}
};

class StringPostFixExpr : public Expr {
private:
    vector<string> expr;
    vector<string> exprtoks;

    string* applyOper(string* a, string* b, string oper) {
        if (oper == "+") return new string(*a + *b);
        else if (oper == "and") return (a && b) ? new string() : nullptr;
        else if (oper == "or") return (a || b) ? new string() : nullptr;
        else if (oper == "<") return (*a < *b) ? new string() : nullptr;
        else if (oper == "<=") return (*a <= *b) ? new string() : nullptr;
        else if (oper == ">") return (*a > *b) ? new string() : nullptr;
        else if (oper == ">=") return (*a >= *b) ? new string() : nullptr;
        else if (oper == "==") return (*a == *b) ? new string() : nullptr;
        else if (oper == "!=") return (*a != *b) ? new string() : nullptr;
    	return nullptr;
    }

public:
    StringPostFixExpr() {}

    ~StringPostFixExpr() {}

    void add(string tok, string lex) {
        expr.push_back(lex);
        exprtoks.push_back(tok);
    }

    string* eval() {
        vector<string>::iterator expritr = expr.begin();
        vector<string>::iterator tokitr = exprtoks.begin();
        stack<string*> oprandStk;

        string* a;
        string* b;
        while (tokitr != exprtoks.end()) {
            if (*tokitr == "t_text") oprandStk.push(new string(*expritr));
            else if (*tokitr == "t_id") oprandStk.push(new string(symbolvalues[*expritr]));
            else {
                b = oprandStk.top(); oprandStk.pop();
                a = oprandStk.top(); oprandStk.pop();
                oprandStk.push(applyOper(a, b, *expritr));
                delete b;
                delete a;
            }
            expritr++; tokitr++;
        }
        return oprandStk.top();
    }

    string toString() {
        string res = "String Postfix Expression\n";

        res += "Expression Terms Vector:\n";
        for (int i = 0; i < expr.size(); i++) {
            res += expr[i];
            res += "\n";
        }

        res += "Expression Tokens Vector:\n";
        for (int i = 0; i < exprtoks.size(); i++) {
            res += exprtoks[i];
            res += "\n";
        }

        res += "Evaluation Result:\n";
        string* val = eval();
        res += *val;
        delete val;

        return res;
    }
};

class IntConstExpr : public IntExpr {
private:
    int value;

public:
    IntConstExpr(int val) {
        value = val;
    }

    ~IntConstExpr() {}

    int eval() {return value;}

    string toString() {return "Integer Constant Expression: " + value;}
};

class IntIDExpr : public IntExpr {
private:
    string id;

public:
    IntIDExpr(string val) {
        id = val;
    }

    ~IntIDExpr() {}

    int eval() {return stoi(symbolvalues[id]);}

    string toString() {return "Integer ID Expression: " + id + ", " + symbolvalues[id];}
};

class IntPostFixExpr : public IntExpr {
private:
    vector<string> expr;

    int applyOperator(int a, int b, string oper) {
        if (oper == "+") return a + b;
        else if (oper == "*") return a * b;
        else if (oper == "-") return a - b;
        else if (oper == "/") return a / b;
        else if (oper == "%") return a % b;
        else if (oper == "and") return a && b;
        else if (oper == "or") return a || b;
        else if (oper == "<") return a < b;
        else if (oper == "<=") return a <= b;
        else if (oper == ">") return a > b;
        else if (oper == ">=") return a >= b;
        else if (oper == "==") return a == b;
        else if (oper == "!=") return a != b;
        else {
            cout << "In IntPostFixExpr::applyOperator, unexpected operator" << endl;
            exit(-1);
        }
    }

	bool isNumber(string lex) {
        for (int i = 0; i < lex.length(); i++) {
            if (!isdigit(lex[i])) return false;
        }
        return true;
    }

public:
    IntPostFixExpr() {}

    ~IntPostFixExpr() {}

    void add(string lex) {
        expr.push_back(lex);
    }

    int eval() {
        vector<string>::iterator expritr = expr.begin();
        stack<string> oprandStk;

        int a;
        int b;
        while (expritr != expr.end()) {
            if (isNumber(*expritr)) oprandStk.push(*expritr);
            else if (isOperator(*expritr)) {
                b = stoi(oprandStk.top()); oprandStk.pop();
                a = stoi(oprandStk.top()); oprandStk.pop();
                oprandStk.push(to_string(applyOperator(a, b, *expritr)));
            }
            else oprandStk.push(symbolvalues[*expritr]);

            expritr++;
        }
        return stoi(oprandStk.top());
    }

    string toString() {
        string res = "Integer Postfix Expression\n";

        res += "Expression Vector:\n";
        for (int i = 0; i < expr.size(); i++) {
            res += expr[i];
            res += "\n";
        }

        res += "Evaluation Result:\n";
        res += to_string(eval());
        return res;
    }
};

class Stmt{ // statements are executed!
private:
	string _name;
public:
	Stmt(string name) {
		_name = name;
	}
	virtual ~Stmt(){};
	virtual string toString() {
		return _name;
	}
	virtual void execute() = 0;
};

class AssignStmt : public Stmt{
private:
	string _var;
	Expr* _p_expr;
public:
	AssignStmt(string var, Expr *expr) : Stmt("s_assign") {
		_var = var;
		_p_expr = expr;
	}
	~AssignStmt() {
		if (_p_expr != nullptr) {
			delete _p_expr;
		}
	}
	string toString() {
		return Stmt::toString() + " var " + _var + " expression: " + _p_expr->toString();
	}
	void execute() {
		if (symboltable[_var] == "t_string") {
			StringExpr* s_expr = dynamic_cast<StringExpr*>(_p_expr);
			if (s_expr != nullptr) symbolvalues[_var] =s_expr->eval();
			pc++;
		}
		if (symboltable[_var] == "t_integer") {
			IntExpr* i_expr = dynamic_cast<IntExpr*>(_p_expr);
			if (i_expr != nullptr) symbolvalues[_var] = to_string(i_expr->eval());
			pc++;
		}
		// Throw error - Invalid Expression
	}
};

class InputStmt : public Stmt{
private:
	string _var;
public:
	InputStmt(string var) : Stmt("t_input") {
		_var = var;
	}
	string toString() {
		return Stmt::toString() + " var: " + _var;
	}
	void execute() {
		string t;
		cin >> t;
		symbolvalues[_var] = t;
		pc++;
	}
};

class StrOutStmt : public Stmt{
private:
	string _value;
public:
	StrOutStmt(string value) : Stmt("t_output_str") {
		_value = value;
	}
	string toString() {
		return Stmt::toString() + " value: " + _value;
	}
	void execute() {
		cout << _value << endl;
		pc++;
	}
};

class IntOutStmt : public Stmt{
private:
	int _value;
public:
	IntOutStmt(int value) : Stmt("t_output_int") {
		_value = value;
	}
	~IntOutStmt() {}
	string toString() {
		return Stmt::toString() + " value: " + to_string(_value);
	}
	void execute() {
		cout << _value << endl;
		pc++;
	}
};

class IDOutStmt : public Stmt{
private:
	string _var;
public:
	IDOutStmt(string var) : Stmt("t_output_id") {
		_var = var;
	}
	~IDOutStmt() {}
	string toString() {
		return Stmt::toString() + " var: " + _var;
	}
	void execute() {
		cout << symbolvalues[_var] << endl;
		pc++;
	}
};

class IfStmt : public Stmt{
private:
	Expr* _p_expr;
	int _elsetarget;
public:
	IfStmt(Expr *expr, int elsetarget) : Stmt("t_if") {
		_p_expr = expr;
		_elsetarget = elsetarget;
	}
	~IfStmt() {
		if (_p_expr != nullptr) {
			delete _p_expr;
		}
	}
	void setTarget(int target) {
		_elsetarget = target;
	}
	string toString() {
		return Stmt::toString() + " expression " + _p_expr->toString() + " elsetarget: " + to_string(_elsetarget);
	}
	void execute() {
		IntExpr* i_expr = dynamic_cast<IntExpr*>(_p_expr);
		if (i_expr != nullptr) {
			if (i_expr->eval() == 0) {
				pc = _elsetarget;
			} else {
				pc++;
			}
		}
		StringPostFixExpr* s_pfexpr = dynamic_cast<StringPostFixExpr*>(_p_expr);
		if (s_pfexpr != nullptr) {
			if (s_pfexpr->eval() == nullptr) {
				pc = _elsetarget;
			} else {
				pc++;
			}
		}
		StringExpr* s_expr = dynamic_cast<StringExpr*>(_p_expr);
		if (s_expr != nullptr) {
			pc++;
		}
		// Throw error - Invalid Expression
	}
};

class WhileStmt : public Stmt{
private:
	Expr* _p_expr;
	int _elsetarget;
public:
	WhileStmt(Expr *expr, int elsetarget) : Stmt("t_while") {
		_p_expr = expr;
		_elsetarget = elsetarget;
	}
	~WhileStmt() {
		delete _p_expr;
	}
	void setTarget(int target) {
		_elsetarget = target;
	}
	string toString() {
		return Stmt::toString() + " expression " + _p_expr->toString() + " elsetarget: " + to_string(_elsetarget);
	}
	void execute() {
		IntExpr* i_expr = dynamic_cast<IntExpr*>(_p_expr);
		if (i_expr != nullptr) {
			if (i_expr->eval() == 0) {
				pc = _elsetarget;
			} else {
				pc++;
			}
		}
		StringPostFixExpr* s_pfexpr = dynamic_cast<StringPostFixExpr*>(_p_expr);
		if (s_pfexpr != nullptr) {
			if (s_pfexpr->eval() == nullptr) {
				pc = _elsetarget;
			} else {
				pc++;
			}
		}
		StringExpr* s_expr = dynamic_cast<StringExpr*>(_p_expr);
		if (s_expr != nullptr) {
			pc++;
		}
	}
};

class GoToStmt : public Stmt{
private:
	int _target;
public:
	GoToStmt(int target) : Stmt("t_goto") {
		_target = target;
	}
	~GoToStmt() {}
	void setTarget(int target) {
		_target = target;
	}
	string toString() {
		return Stmt::toString() + " target: " + to_string(_target);
	}
	void execute() {
		pc = _target;
	}
};

class Compiler{
private:
		void buildStmt() {
		if (*tokitr == "t_if") buildIf();
		else if (*tokitr == "t_while") buildWhile();
		else if (*tokitr == "t_id") {
			tokitr++; lexitr++;
			if (*tokitr == "s_assign") {
				tokitr--; lexitr--;
				buildAssign();
			}
			else {
				tokitr--; lexitr--;
			}
		}
		else if (*tokitr == "t_input") buildInput();
		else if (*tokitr == "t_output") buildOutput();
	}

	void buildIf() {
		tokitr++; lexitr++; // move past t_if
		tokitr++; lexitr++; // move past (
		Expr* expr = buildExpr();
		IfStmt* ifstmt = new IfStmt(expr, 0); // set target later
		insttable.push_back(ifstmt);
		tokitr++; lexitr++; // move past )
		tokitr++; lexitr++; // move past t_then
		while (*tokitr != "t_else" && *tokitr != "t_end") {
			buildStmt();
		}
		if (*tokitr == "t_else") {
			GoToStmt* gotostmt = new GoToStmt(0); // set target later
			insttable.push_back(gotostmt);
			// set target for ifstmt
			ifstmt->setTarget(insttable.size());
			tokitr++; lexitr++; // move past t_else
			while (*tokitr != "t_end") {
				buildStmt();
			}
			gotostmt->setTarget(insttable.size());
		}
		else {
			ifstmt->setTarget(insttable.size());
		}
		tokitr++; lexitr++; // move past t_end
		tokitr++; lexitr++; // move past t_if
	}


	void buildWhile() {
		tokitr++; lexitr++; // move past t_while
		tokitr++; lexitr++; // move past (
		int startWhile = insttable.size(); // use for goto
		Expr* expr = buildExpr();
		WhileStmt* whilestmt = new WhileStmt(expr, 0); // set target later
		insttable.push_back(whilestmt);
		tokitr++; lexitr++; // move past )
		tokitr++; lexitr++; // move past t_loop
		while (*tokitr != "t_end") {
			buildStmt();
		}
		GoToStmt* gotostmt = new GoToStmt(startWhile);
		insttable.push_back(gotostmt);
		whilestmt->setTarget(insttable.size());
		tokitr++; lexitr++; // move past t_end
		tokitr++; lexitr++; // move past t_loop
	}

	void buildAssign() {
		string var = *tokitr;
		tokitr++; lexitr++; // move past var
		tokitr++; lexitr++; // move past =
		Expr* expr = buildExpr();
		AssignStmt* assign = new AssignStmt(var, expr);
		insttable.push_back(assign);
		tokitr++; lexitr++; // move past ;
	}

	void buildInput() {
		tokitr++; lexitr++; // move past t_input
		tokitr++; lexitr++; // move past s_lparen
		InputStmt* input = new InputStmt(*lexitr);
		insttable.push_back(input);
		tokitr++; lexitr++; // move past t_id
		tokitr++; lexitr++; // move past s_rparen
	}

	void buildOutput() {
		cout << "building output" << endl;
		tokitr++; lexitr++; // move past t_output
		tokitr++; lexitr++; // move past s_lparen
		if (*tokitr == "t_text") {
			StrOutStmt* strOut = new StrOutStmt(*lexitr);
			insttable.push_back(strOut);
		} else if (*tokitr == "t_number") {
			int value = stoi(*lexitr);
			IntOutStmt* intOut = new IntOutStmt(value);
			insttable.push_back(intOut);
		} else if (*tokitr == "t_id") {
			IDOutStmt* idOut = new IDOutStmt(*lexitr);
			insttable.push_back(idOut);
		}
		tokitr++; lexitr++; // move past output var
		tokitr++; lexitr++; // move past s_rparen
	}

    Expr* buildExpr() {
        Expr* ex_ptr = nullptr;

        bool isOp;

        if (tokitr != tokens.end()) {
            if (*tokitr == "t_text" || (*tokitr == "t_id" && symboltable[*lexitr] == "t_string")) {
                lexitr++;
                isOp = isOperator(*lexitr);
                lexitr--;

                if (isOp) {
                    StringPostFixExpr(ex_ptr);
                }
                else {
                    if (*tokitr == "t_text") ex_ptr = new StringConstExpr(*lexitr);
                    else ex_ptr = new StringIDExpr(*lexitr);
                    tokitr++; lexitr++;
                }
            }

            else if (*tokitr == "t_number" || (*tokitr == "t_id" && symboltable[*lexitr] == "t_integer")) {
                lexitr++;
                isOp = isOperator(*lexitr);
                lexitr--;

                if (isOp) {
                    IntPostFixExpr(ex_ptr);
                }
                else {
                    if (*tokitr == "t_number") ex_ptr = new IntConstExpr(stoi(*lexitr));
                    else ex_ptr = new IntIDExpr(*lexitr);
                    tokitr++; lexitr++;
                }
            }
        }

        if (ex_ptr == nullptr) {
            cout << "error in buildExpr: token didn't match any potential expressions" << endl;
            exit(-1);
        }
        return ex_ptr;
	}

	void buildStringPostFix(StringPostFixExpr*& ptr) {
        // creates a new String Postfix Expression and points the argument ptr to it
        ptr = new StringPostFixExpr();

        stack<string> operStk;
        stack<string> tokStk;

        bool isId = *tokitr == "t_id";
        bool isText = *tokitr == "t_text";
        bool isOper = false;
        while (isId || isText || isOper) {
            if (isOper) {
                while (operStk.size() > 0 && precMap[*lexitr] <= precMap[operStk.top()]) {
                    ptr->add(tokStk.top(), operStk.top());
                    tokStk.pop(); operStk.pop();
                }
                tokStk.push(*tokitr); operStk.push(*lexitr);
            }
            else ptr->add(*tokitr, *lexitr);

            tokitr++; lexitr++;
            isId = *tokitr == "t_id";
            isText = *tokitr == "t_text";
            isOper = isOperator(*lexitr);
        }
        while (operStk.size() > 0) {
            ptr->add(tokStk.top(), operStk.top());
            tokStk.pop(); operStk.pop();
        }
	}

	void buildIntPostFix(IntPostFixExpr*& ptr) {
        // creates a new Integer Postfix Expression and points the argument ptr to it
        ptr = new IntPostFixExpr();

        stack<string> operStk;

        bool isId = *tokitr == "t_id";
        bool isNumber = *tokitr == "t_number";
        bool isOper = false;
        while (isId || isNumber || isOper) {
            if (isOper) {
                while (operStk.size() > 0 && precMap[*lexitr] <= precMap[operStk.top()]) {
                    ptr->add(operStk.top()); operStk.pop();
                }
                operStk.push(*lexitr);
            }
            else ptr->add(*lexitr);

            tokitr++; lexitr++;
            isId = *tokitr == "t_id";
            isNumber = *tokitr == "t_number";
            isOper = isOperator(*lexitr);
        }
        while (operStk.size() > 0) {
            ptr->add(operStk.top()); operStk.pop();
        }
	}

	// headers for populate methods may not change
	void populateTokenLexemes(istream& infile) {
		string tok, lex;
		string line;
		while (getline(infile, line)) {
			int spaceIdx = line.find(' ');
			string tok = line.substr(0, spaceIdx);
			string lex = line.substr(spaceIdx + 1);
			tokens.push_back(tok);
			lexemes.push_back(lex);
		}
	}

	void populateSymbolTable(istream& infile) {
		string var, type;
		string line;
		while (getline(infile, line)) {
			int spaceIdx = line.find(' ');
			string var = line.substr(0, spaceIdx);
			string type = line.substr(spaceIdx + 1);
			symboltable[var] = type;
			if (type == "t_integer") {
				symbolvalues[var] = "0";
			}else {
				symbolvalues[var] = "";
			}
		}
	}

public:
	Compiler(){}

	// headers may not change
	Compiler(istream& source, istream& symbols){
		// build precMap - include logical, relational, arithmetic operators
		precMap["*"] = 1;
		precMap["/"] = 1;
		precMap["%"] = 1;
		precMap["+"] = 2;
		precMap["-"] = 2;
		precMap["<"] = 3;
		precMap[">"] = 3;
		precMap["<="] = 3;
		precMap[">="] = 3;
		precMap["=="] = 3;
		precMap["and"] = 4;
		precMap["or"] = 5;

		populateTokenLexemes(source);
		populateSymbolTable(symbols);

		tokitr = tokens.begin();
		lexitr = lexemes.begin();
	}

	// The compile method is responsible for getting the instruction
	// table built.  It will call the appropriate build methods.
	bool compile() {
		while (tokitr != tokens.end()) {
			buildStmt();
			tokitr++; lexitr++;
		}
		cout << "compile method done" << endl;
		return true;
	}

	// The run method will execute the code in the instruction
	// table.
	void run() {
		while (pc < insttable.size()) {
			insttable[pc]->execute();
		}
		for (int i = 0; i < insttable.size(); i++) {
			delete insttable[i];
		}
	}
};

void dump() {
	cout << "Symbol table:" << endl;
	for (const auto& [variable, type] : symboltable) {
		cout << variable << " : " << type << endl;
	}
	cout << "Symbol values:" << endl;
	for (const auto& [variable, type] : symbolvalues) {
		cout << variable << " : " << type << endl;
	}

	for (int x = 0; x < insttable.size(); x++) {
		cout << x << ": " << insttable[x]-> toString() << endl;
	}
}


int main(){
	ifstream source("test1A_data.txt");
	ifstream symbols("test1A_vars.txt");
	if (!source || !symbols) exit(-1);
	Compiler c(source, symbols);
	c.compile();
	// might want to call dump to check if everything built correctly
	dump();
	c.run();
	return 0;
}
