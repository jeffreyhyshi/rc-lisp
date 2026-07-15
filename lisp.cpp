#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

enum lisp_object_type {
	NUMBER,
	STRING,
	SYMBOL,
	LIST
};

struct lisp_object {
	lisp_object_type type;

	string string_or_symbol;
	double number;
	vector<lisp_object> list;

	lisp_object(lisp_object_type t) {
		type = t;
	}

	friend ostream& operator<<(ostream& os, const lisp_object& a) {
		switch (a.type) {
			case NUMBER:
				os << a.number;
				break;
			case STRING:
				os << "\"" << a.string_or_symbol << "\"";
				break;
			case SYMBOL:
				os << a.string_or_symbol;
				break;
			case LIST:
				os << "[";
				for (int i = 0; i < a.list.size(); i++) {
					os << a.list[i];
					if (i < a.list.size() - 1) {
						os << ", ";
					}
				}
				os << "]";
				break;
		}
		return os;
	}
};

lisp_object read_string(string& line, size_t& pos);
lisp_object read_quoted_symbol(string& line, size_t& pos);
lisp_object read_number_or_symbol(string&line, size_t& pos);
lisp_object read_atom(string& line, size_t& pos);
lisp_object read_list(string& line, size_t& pos);

lisp_object read_string(string& line, size_t& pos) {
	size_t start_pos = pos;
	for (size_t escapedquote_pos = line.find("\\\"", pos);
		escapedquote_pos != string::npos;
		escapedquote_pos = line.find("\\\"", pos)) {
		pos = escapedquote_pos + 2;
		if (line.find('\"', pos) < line.find("\\\"", pos)) {
			break;
		}
	}
	if (line.find('\"', pos) != string::npos) {
		pos = line.find('\"', pos);
		lisp_object result(lisp_object_type::STRING);
		result.string_or_symbol = line.substr(start_pos, pos - start_pos);
		pos += 1;
		return result;
	} else {
		throw runtime_error("Expected a closing quotation mark.");
	}
}

lisp_object read_quoted_symbol(string& line, size_t& pos) {
	size_t next_break = line.find_first_of(" )", pos);

	lisp_object quote_symbol(lisp_object_type::SYMBOL);
	quote_symbol.string_or_symbol = "quote";

	if (next_break == pos) {
		throw runtime_error("Expected a symbol after single quote.");
	} else if (line.at(pos) == '(') {
		pos += 1;
		lisp_object quoted_symbol = read_list(line, pos);

		lisp_object result(lisp_object_type::LIST);
		result.list.push_back(quote_symbol);
		result.list.push_back(quoted_symbol);

		return result;
	} else {
		lisp_object quoted_symbol(lisp_object_type::SYMBOL);
		quoted_symbol.string_or_symbol = line.substr(pos, next_break - pos);
		pos = next_break;

		lisp_object result(lisp_object_type::LIST);
		result.list.push_back(quote_symbol);
		result.list.push_back(quoted_symbol);

		return result;
	}
}

lisp_object read_number_or_symbol(string& line, size_t& pos) {
	size_t next_break = line.find_first_of(" )", pos);
	if (next_break == string::npos) {
		next_break = line.length();
	}
	string number_or_symbol_str = line.substr(pos, next_break - pos);
	try {
		double num = stod(number_or_symbol_str);
		lisp_object result_num(lisp_object_type::NUMBER);
		result_num.number = num;
		pos = next_break;
		return result_num;
	} catch (out_of_range oor) {
		throw runtime_error("Number out of supported range.");
	} catch (...) {
		lisp_object result_symbol(lisp_object_type::SYMBOL);
		result_symbol.string_or_symbol = number_or_symbol_str;
		pos = next_break;
		return result_symbol;
	}
}

lisp_object read_atom(string& line, size_t& pos) {
	if (line.at(pos) == '\"') {
		pos += 1;
		return read_string(line, pos);
	} else if (line.at(pos) == '\'') {
		pos += 1;
		return read_quoted_symbol(line, pos);
	} else {
		return read_number_or_symbol(line, pos);
	}
}

lisp_object read_list(string& line, size_t& pos) {
	lisp_object result_list(lisp_object_type::LIST);

	while (pos < line.length()) {
		size_t next = line.find_first_not_of(' ', pos);
		if (next == string::npos){
			throw runtime_error("Expected a non-whitespace character.");
		} else if (line.at(next) == '(') {
			pos = next + 1;
			result_list.list.push_back(read_list(line, pos));
		} else if (line.at(next) == ')') {
			pos = next + 1;
			return result_list;
		} else {
			pos = next;
			result_list.list.push_back(read_atom(line, pos));
		}
	}

	throw runtime_error("Expected a closing parenthesis.");
}

lisp_object read(string& line) {
	size_t first_paren = line.find('(');
	if (first_paren != string::npos) {
		size_t pos = line.find_first_not_of(' ', first_paren + 1);
		if (pos == string::npos) {
			throw runtime_error("Expected a non-whitespace character.");
		}
		lisp_object result = read_list(line, pos);
		if (line.find_first_not_of(' ', pos) != string::npos) {
			throw runtime_error("Unexpected tokens after last matched parenthesis.");
		}
		return result;
	} else {
		// A single atom is also valid lisp
		size_t pos = 0;
		return read_atom(line, pos);
	}
}

void repl(istream& input, bool is_file) {
	string line;
	if (!is_file) {
		cout << "> ";
	}
	while (getline(input, line)) {
		if (is_file) {
			cout << "> " << line << "\n";
		}
		cout << read(line) << "\n";
		if (!is_file) {
			cout << "> ";
		}
	}
}

int main(int argc, char* argv[]) {
	// If an argument is provided, assume it is a filename. Otherwise, use stdin for lines of LISP
	if (argc > 1) {
		ifstream file_in(argv[1]);
		if (!file_in.is_open()) {
			cout << "Could not open file " << argv[1] << "\n";
			return 1;
		}
		repl(file_in, true);
	} else {
		repl(cin, false);
	}
}