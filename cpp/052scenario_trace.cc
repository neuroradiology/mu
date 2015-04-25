//: Support a scenario [ ... ] form at the top level so we can start creating
//: scenarios in mu just like we do in C++.

:(before "End scenario Fields")
vector<pair<string, string> > trace_checks;
vector<pair<string, string> > trace_negative_checks;

:(before "End Scenario Command Handlers")
else if (scenario_command == "trace") {
  handle_scenario_trace_directive(inner, x);
}

:(before "End Scenario Checks")
check_trace_contents(Scenarios[i]);
check_trace_negative_contents(Scenarios[i]);

:(code)
void handle_scenario_trace_directive(istream& in, scenario& out) {
  if (next_word(in) != "should") {
    raise << "'trace' directive inside scenario must continue 'trace should'\n";
  }
  string s = next_word(in);
  if (s == "not") {
    handle_scenario_trace_negative_directive(in, out);
    return;
  }
  if (s != "contain") {
    raise << "'trace' directive inside scenario must continue 'trace should [not] contain'\n";
  }
  skip_bracket(in, "'trace' directive inside scenario must begin with 'trace should contain ['\n");
  while (true) {
    skip_whitespace_and_comments(in);
    if (in.eof()) break;
    if (in.peek() == ']') break;
    string curr_line;
    getline(in, curr_line);
    istringstream tmp(curr_line);
    tmp >> std::noskipws;
    string label = slurp_until(tmp, ':');
    if (tmp.get() != ' ') {
      raise << "'trace' directive inside scenario should contain lines of the form 'label: message', instead got " << curr_line;
      continue;
    }
    string message = slurp_rest(tmp);
    out.trace_checks.push_back(pair<string, string>(label, message));
    trace("scenario") << "trace: " << label << ": " << message << '\n';
  }
  skip_whitespace(in);
  assert(in.get() == ']');
}

void handle_scenario_trace_negative_directive(istream& in, scenario& out) {
  // 'not' already slurped
  if (next_word(in) != "contain") {
    raise << "'trace' directive inside scenario must continue 'trace should not contain'\n";
  }
  skip_bracket(in, "'trace' directive inside scenario must begin with 'trace should not contain ['\n");
  while (true) {
    skip_whitespace_and_comments(in);
    if (in.eof()) break;
    if (in.peek() == ']') break;
    string curr_line;
    getline(in, curr_line);
    istringstream tmp(curr_line);
    tmp >> std::noskipws;
    string label = slurp_until(tmp, ':');
    if (tmp.get() != ' ') {
      raise << "'trace' directive inside scenario should contain lines of the form 'label: message', instead got " << curr_line;
      continue;
    }
    string message = slurp_rest(tmp);
    out.trace_negative_checks.push_back(pair<string, string>(label, message));
    trace("scenario") << "trace: " << label << ": " << message << '\n';
  }
  skip_whitespace(in);
  assert(in.get() == ']');
}

string slurp_rest(istream& in) {
  ostringstream out;
  char c;
  while (in >> c) {
    out << c;
  }
  return out.str();
}

void check_trace_contents(const scenario& s) {
  if (s.trace_checks.empty()) return;
  ostringstream contents;
  for (size_t i = 0; i < s.trace_checks.size(); ++i) {
    contents << s.trace_checks[i].first << ": " << s.trace_checks[i].second << "";
  }
  CHECK_TRACE_CONTENTS(contents.str());
}

void check_trace_negative_contents(const scenario& s) {
  for (size_t i = 0; i < s.trace_negative_checks.size(); ++i) {
    if (trace_count(s.trace_negative_checks[i].first, s.trace_negative_checks[i].second) > 0) {
      raise << "trace shouldn't contain " << s.trace_negative_checks[i].first << ": " << s.trace_negative_checks[i].second << '\n';
      Passed = false;
    }
  }
}