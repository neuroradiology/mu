//: Arithmetic primitives

:(before "End Primitive Recipe Declarations")
ADD,
:(before "End Primitive Recipe Numbers")
Recipe_ordinal["add"] = ADD;
:(before "End Primitive Recipe Type Checks")
case ADD: {
  for (long long int i = 0; i < SIZE(inst.ingredients); ++i) {
    if (!is_mu_number(inst.ingredients.at(i))) {
      raise << Recipe[r].name << ": 'add' requires number ingredients, but got " << inst.ingredients.at(i).original_string << '\n' << end();
      goto finish_checking_instruction;
    }
  }
  if (SIZE(inst.products) > 1) {
    raise << Recipe[r].name << ": 'add' yields exactly one product in '" << inst.to_string() << "'\n" << end();
    break;
  }
  if (!inst.products.empty() && !is_mu_number(inst.products.at(0))) {
    raise << Recipe[r].name << ": 'add' should yield a number, but got " << inst.products.at(0).original_string << '\n' << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case ADD: {
  double result = 0;
  for (long long int i = 0; i < SIZE(ingredients); ++i) {
    result += ingredients.at(i).at(0);
  }
  products.resize(1);
  products.at(0).push_back(result);
  break;
}

:(scenario add_literal)
recipe main [
  1:number <- add 23, 34
]
+mem: storing 57 in location 1

:(scenario add)
recipe main [
  1:number <- copy 23
  2:number <- copy 34
  3:number <- add 1:number, 2:number
]
+mem: storing 57 in location 3

:(scenario add_multiple)
recipe main [
  1:number <- add 3, 4, 5
]
+mem: storing 12 in location 1

:(scenario add_checks_type)
% Hide_warnings = true;
recipe main [
  1:number <- add 2:boolean, 1
]
+warn: main: 'add' requires number ingredients, but got 2:boolean

:(scenario add_checks_return_type)
% Hide_warnings = true;
recipe main [
  1:address:number <- add 2, 2
]
+warn: main: 'add' should yield a number, but got 1:address:number

:(before "End Primitive Recipe Declarations")
SUBTRACT,
:(before "End Primitive Recipe Numbers")
Recipe_ordinal["subtract"] = SUBTRACT;
:(before "End Primitive Recipe Type Checks")
case SUBTRACT: {
  if (inst.ingredients.empty()) {
    raise << Recipe[r].name << ": 'subtract' has no ingredients\n" << end();
    break;
  }
  for (long long int i = 0; i < SIZE(inst.ingredients); ++i) {
    if (is_raw(inst.ingredients.at(i))) continue;  // offsets in tests
    if (!is_mu_number(inst.ingredients.at(i))) {
      raise << Recipe[r].name << ": 'subtract' requires number ingredients, but got " << inst.ingredients.at(i).original_string << '\n' << end();
      goto finish_checking_instruction;
    }
  }
  if (SIZE(inst.products) > 1) {
    raise << Recipe[r].name << ": 'subtract' yields exactly one product in '" << inst.to_string() << "'\n" << end();
    break;
  }
  if (!inst.products.empty() && !is_mu_number(inst.products.at(0))) {
    raise << Recipe[r].name << ": 'subtract' should yield a number, but got " << inst.products.at(0).original_string << '\n' << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case SUBTRACT: {
  double result = ingredients.at(0).at(0);
  for (long long int i = 1; i < SIZE(ingredients); ++i)
    result -= ingredients.at(i).at(0);
  products.resize(1);
  products.at(0).push_back(result);
  break;
}

:(scenario subtract_literal)
recipe main [
  1:number <- subtract 5, 2
]
+mem: storing 3 in location 1

:(scenario subtract)
recipe main [
  1:number <- copy 23
  2:number <- copy 34
  3:number <- subtract 1:number, 2:number
]
+mem: storing -11 in location 3

:(scenario subtract_multiple)
recipe main [
  1:number <- subtract 6, 3, 2
]
+mem: storing 1 in location 1

:(before "End Primitive Recipe Declarations")
MULTIPLY,
:(before "End Primitive Recipe Numbers")
Recipe_ordinal["multiply"] = MULTIPLY;
:(before "End Primitive Recipe Type Checks")
case MULTIPLY: {
  for (long long int i = 0; i < SIZE(inst.ingredients); ++i) {
    if (!is_mu_number(inst.ingredients.at(i))) {
      raise << Recipe[r].name << ": 'add' requires number ingredients, but got " << inst.ingredients.at(i).original_string << '\n' << end();
      goto finish_checking_instruction;
    }
  }
  if (SIZE(inst.products) > 1) {
    raise << Recipe[r].name << ": 'multiply' yields exactly one product in '" << inst.to_string() << "'\n" << end();
    break;
  }
  if (!inst.products.empty() && !is_mu_number(inst.products.at(0))) {
    raise << Recipe[r].name << ": 'multiply' should yield a number, but got " << inst.products.at(0).original_string << '\n' << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case MULTIPLY: {
  double result = 1;
  for (long long int i = 0; i < SIZE(ingredients); ++i) {
    result *= ingredients.at(i).at(0);
  }
  products.resize(1);
  products.at(0).push_back(result);
  break;
}

:(scenario multiply_literal)
recipe main [
  1:number <- multiply 2, 3
]
+mem: storing 6 in location 1

:(scenario multiply)
recipe main [
  1:number <- copy 4
  2:number <- copy 6
  3:number <- multiply 1:number, 2:number
]
+mem: storing 24 in location 3

:(scenario multiply_multiple)
recipe main [
  1:number <- multiply 2, 3, 4
]
+mem: storing 24 in location 1

:(before "End Primitive Recipe Declarations")
DIVIDE,
:(before "End Primitive Recipe Numbers")
Recipe_ordinal["divide"] = DIVIDE;
:(before "End Primitive Recipe Type Checks")
case DIVIDE: {
  if (inst.ingredients.empty()) {
    raise << Recipe[r].name << ": 'divide' has no ingredients\n" << end();
    break;
  }
  for (long long int i = 0; i < SIZE(inst.ingredients); ++i) {
    if (!is_mu_number(inst.ingredients.at(i))) {
      raise << Recipe[r].name << ": 'divide' requires number ingredients, but got " << inst.ingredients.at(i).original_string << '\n' << end();
      goto finish_checking_instruction;
    }
  }
  if (SIZE(inst.products) > 1) {
    raise << Recipe[r].name << ": 'divide' yields exactly one product in '" << inst.to_string() << "'\n" << end();
    break;
  }
  if (!inst.products.empty() && !is_mu_number(inst.products.at(0))) {
    raise << Recipe[r].name << ": 'divide' should yield a number, but got " << inst.products.at(0).original_string << '\n' << end();
    break;
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case DIVIDE: {
  double result = ingredients.at(0).at(0);
  for (long long int i = 1; i < SIZE(ingredients); ++i)
    result /= ingredients.at(i).at(0);
  products.resize(1);
  products.at(0).push_back(result);
  break;
}

:(scenario divide_literal)
recipe main [
  1:number <- divide 8, 2
]
+mem: storing 4 in location 1

:(scenario divide)
recipe main [
  1:number <- copy 27
  2:number <- copy 3
  3:number <- divide 1:number, 2:number
]
+mem: storing 9 in location 3

:(scenario divide_multiple)
recipe main [
  1:number <- divide 12, 3, 2
]
+mem: storing 2 in location 1

//: Integer division

:(before "End Primitive Recipe Declarations")
DIVIDE_WITH_REMAINDER,
:(before "End Primitive Recipe Numbers")
Recipe_ordinal["divide-with-remainder"] = DIVIDE_WITH_REMAINDER;
:(before "End Primitive Recipe Type Checks")
case DIVIDE_WITH_REMAINDER: {
  if (SIZE(inst.ingredients) != 2) {
    raise << current_recipe_name() << ": 'divide-with-remainder' requires exactly two ingredients, but got '" << current_instruction().to_string() << "'\n" << end();
    break;
  }
  if (!is_mu_number(inst.ingredients.at(0)) || !is_mu_number(inst.ingredients.at(1))) {
    raise << current_recipe_name() << ": 'divide-with-remainder' requires number ingredients, but got '" << current_instruction().to_string() << "'\n" << end();
    break;
  }
  if (SIZE(inst.products) > 2) {
    raise << Recipe[r].name << ": 'divide-with-remainder' yields two products in '" << inst.to_string() << "'\n" << end();
    break;
  }
  for (long long int i = 0; i < SIZE(inst.products); ++i) {
    if (!is_mu_number(inst.products.at(i))) {
      raise << Recipe[r].name << ": 'divide-with-remainder' should yield a number, but got " << inst.products.at(i).original_string << '\n' << end();
      goto finish_checking_instruction;
    }
  }
  break;
}
:(before "End Primitive Recipe Implementations")
case DIVIDE_WITH_REMAINDER: {
  products.resize(2);
  long long int a = static_cast<long long int>(ingredients.at(0).at(0));
  long long int b = static_cast<long long int>(ingredients.at(1).at(0));
  if (b == 0) {
    raise << current_recipe_name() << ": divide by zero in '" << current_instruction().to_string() << "'\n" << end();
    break;
  }
  long long int quotient = a / b;
  long long int remainder = a % b;
  // very large integers will lose precision
  products.at(0).push_back(quotient);
  products.at(1).push_back(remainder);
  break;
}

:(scenario divide_with_remainder_literal)
recipe main [
  1:number, 2:number <- divide-with-remainder 9, 2
]
+mem: storing 4 in location 1
+mem: storing 1 in location 2

:(scenario divide_with_remainder)
recipe main [
  1:number <- copy 27
  2:number <- copy 11
  3:number, 4:number <- divide-with-remainder 1:number, 2:number
]
+mem: storing 2 in location 3
+mem: storing 5 in location 4

:(scenario divide_with_decimal_point)
recipe main [
  1:number <- divide 5, 2
]
+mem: storing 2.5 in location 1

:(scenario divide_by_zero)
recipe main [
  1:number <- divide 4, 0
]
+mem: storing inf in location 1

:(scenario divide_by_zero_2)
% Hide_warnings = true;
recipe main [
  1:number <- divide-with-remainder 4, 0
]
# integer division can't return floating-point infinity
+warn: main: divide by zero in '1:number <- divide-with-remainder 4, 0'

:(code)
inline bool scalar(const vector<long long int>& x) {
  return SIZE(x) == 1;
}
inline bool scalar(const vector<double>& x) {
  return SIZE(x) == 1;
}