//: Support literal non-integers.
//: '3.14159:literal' is ugly, so we'll just say '3.14159' for non-integers.

:(scenarios load)
:(scenario noninteger_literal)
recipe main [
  1:number <- copy 3.14159
]
+parse:   ingredient: {name: "3.14159", properties: ["3.14159": "literal-number"]}

:(after "reagent::reagent(string s)")
  if (is_noninteger(s)) {
    name = s;
    types.push_back(0);
    properties.push_back(pair<string, vector<string> >(name, vector<string>()));
    properties.back().second.push_back("literal-number");
    set_value(to_double(s));
    return;
  }

:(code)
bool is_noninteger(const string& s) {
  return s.find_first_not_of("0123456789-.") == string::npos
      && s.find('.') != string::npos;
}

double to_double(string n) {
  char* end = NULL;
  // safe because string.c_str() is guaranteed to be null-terminated
  double result = strtod(n.c_str(), &end);
  assert(*end == '\0');
  return result;
}