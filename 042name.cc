//: A big convenience high-level languages provide is the ability to name memory
//: locations. In mu, a transform called 'transform_names' provides this
//: convenience.

:(scenario transform_names)
def main [
  x:number <- copy 0
]
+name: assign x 1
+mem: storing 0 in location 1

:(scenarios transform)
:(scenario transform_names_fails_on_use_before_define)
% Hide_errors = true;
def main [
  x:number <- copy y:number
]
+error: main: use before set: 'y'
# todo: detect conditional defines

:(after "Transform.push_back(compute_container_sizes)")
Transform.push_back(transform_names);  // idempotent

:(before "End Globals")
map<recipe_ordinal, map<string, int> > Name;

//: the Name map is a global, so save it before tests and reset it for every
//: test, just to be safe.
:(before "End Globals")
map<recipe_ordinal, map<string, int> > Name_snapshot;
:(before "End save_snapshots")
Name_snapshot = Name;
:(before "End restore_snapshots")
Name = Name_snapshot;

:(code)
void transform_names(const recipe_ordinal r) {
  recipe& caller = get(Recipe, r);
  trace(9991, "transform") << "--- transform names for recipe " << caller.name << end();
//?   cerr << "--- transform names for recipe " << caller.name << '\n';
  bool names_used = false;
  bool numeric_locations_used = false;
  map<string, int>& names = Name[r];
  // store the indices 'used' so far in the map
  int& curr_idx = names[""];
  ++curr_idx;  // avoid using index 0, benign skip in some other cases
  for (int i = 0; i < SIZE(caller.steps); ++i) {
    instruction& inst = caller.steps.at(i);
    // End transform_names(inst) Special-cases
    // map names to addresses
    for (int in = 0; in < SIZE(inst.ingredients); ++in) {
      reagent& ingredient = inst.ingredients.at(in);
      // Begin transform_names Ingredient Special-cases(ingredient, inst, caller)
      if (is_disqualified(ingredient, inst, caller.name)) continue;
      if (is_numeric_location(ingredient)) numeric_locations_used = true;
      if (is_named_location(ingredient)) names_used = true;
      if (is_integer(ingredient.name)) continue;
      if (!already_transformed(ingredient, names)) {
        raise << maybe(caller.name) << "use before set: '" << ingredient.name << "'\n" << end();
        return;
      }
      int v = lookup_name(ingredient, r);
      if (v >= 0) {
        ingredient.set_value(v);
        // Done Placing Ingredient(ingredient, inst, caller)
      }
      else {
        raise << maybe(caller.name) << "can't find a place to store '" << ingredient.name << "'\n" << end();
        return;
      }
    }
    for (int out = 0; out < SIZE(inst.products); ++out) {
      reagent& product = inst.products.at(out);
      // Begin transform_names Product Special-cases(product, inst, caller)
      if (is_disqualified(product, inst, caller.name)) continue;
      if (is_numeric_location(product)) numeric_locations_used = true;
      if (is_named_location(product)) names_used = true;
      if (is_integer(product.name)) continue;
      if (names.find(product.name) == names.end()) {
        trace(9993, "name") << "assign " << product.name << " " << curr_idx << end();
        names[product.name] = curr_idx;
        curr_idx += size_of(product);
      }
      int v = lookup_name(product, r);
      if (v >= 0) {
        product.set_value(v);
        // Done Placing Product(product, inst, caller)
      }
      else {
        raise << maybe(caller.name) << "can't find a place to store '" << product.name << "'\n" << end();
        return;
      }
    }
  }
  if (names_used && numeric_locations_used)
    raise << maybe(caller.name) << "mixing variable names and numeric addresses\n" << end();
}

bool is_disqualified(/*mutable*/ reagent& x, const instruction& inst, const string& recipe_name) {
  if (!x.type) {
    raise << maybe(recipe_name) << "missing type for '" << x.original_string << "' in '" << inst.original_string << "'\n" << end();
    return true;
  }
  if (is_raw(x)) return true;
  if (is_literal(x)) return true;
  // End is_disqualified Cases
  if (x.initialized) return true;
  return false;
}

bool already_transformed(const reagent& r, const map<string, int>& names) {
  return contains_key(names, r.name);
}

int lookup_name(const reagent& r, const recipe_ordinal default_recipe) {
  return Name[default_recipe][r.name];
}

type_ordinal skip_addresses(type_tree* type) {
  while (type && is_compound_type_starting_with(type, "address"))
    type = type->right;
  if (!type) return -1;  // error handled elsewhere
  return root_type(type)->value;
}

int find_element_name(const type_ordinal t, const string& name, const string& recipe_name) {
  const type_info& container = get(Type, t);
  for (int i = 0; i < SIZE(container.elements); ++i)
    if (container.elements.at(i).name == name) return i;
  raise << maybe(recipe_name) << "unknown element '" << name << "' in container '" << get(Type, t).name << "'\n" << end();
  return -1;
}

bool is_numeric_location(const reagent& x) {
  if (is_literal(x)) return false;
  if (is_raw(x)) return false;
  if (x.name == "0") return false;  // used for chaining lexical scopes
  return is_integer(x.name);
}

bool is_named_location(const reagent& x) {
  if (is_literal(x)) return false;
  if (is_raw(x)) return false;
  if (is_special_name(x.name)) return false;
  return !is_integer(x.name);
}

// all names here should either be disqualified or also in bind_special_scenario_names
bool is_special_name(const string& s) {
  if (s == "_") return true;
  if (s == "0") return true;
  // End is_special_name Cases
  return false;
}

:(scenario transform_names_supports_containers)
def main [
  x:point <- merge 34, 35
  y:number <- copy 3
]
+name: assign x 1
# skip location 2 because x occupies two locations
+name: assign y 3

:(scenario transform_names_supports_static_arrays)
def main [
  x:array:number:3 <- create-array
  y:number <- copy 3
]
+name: assign x 1
# skip locations 2, 3, 4 because x occupies four locations
+name: assign y 5

:(scenario transform_names_passes_dummy)
# _ is just a dummy result that never gets consumed
def main [
  _, x:number <- copy 0, 1
]
+name: assign x 1
-name: assign _ 1

//: an escape hatch to suppress name conversion that we'll use later
:(scenarios run)
:(scenario transform_names_passes_raw)
% Hide_errors = true;
def main [
  x:number/raw <- copy 0
]
-name: assign x 1
+error: can't write to location 0 in 'x:number/raw <- copy 0'

:(scenarios transform)
:(scenario transform_names_fails_when_mixing_names_and_numeric_locations)
% Hide_errors = true;
def main [
  x:number <- copy 1:number
]
+error: main: mixing variable names and numeric addresses

:(scenario transform_names_fails_when_mixing_names_and_numeric_locations_2)
% Hide_errors = true;
def main [
  x:number <- copy 1
  1:number <- copy x:number
]
+error: main: mixing variable names and numeric addresses

:(scenario transform_names_does_not_fail_when_mixing_names_and_raw_locations)
def main [
  x:number <- copy 1:number/raw
]
-error: main: mixing variable names and numeric addresses
$error: 0

:(scenario transform_names_does_not_fail_when_mixing_names_and_literals)
def main [
  x:number <- copy 1
]
-error: main: mixing variable names and numeric addresses
$error: 0

//:: Support element names for containers in 'get' and 'get-location' and 'put'.
//: (get-location is implemented later)

:(scenario transform_names_transforms_container_elements)
def main [
  p:address:point <- copy 0
  a:number <- get *p:address:point, y:offset
  b:number <- get *p:address:point, x:offset
]
+name: element y of type point is at offset 1
+name: element x of type point is at offset 0

:(before "End transform_names(inst) Special-cases")
// replace element names of containers with offsets
if (inst.name == "get" || inst.name == "get-location" || inst.name == "put") {
  //: avoid raising any errors here; later layers will support overloading new
  //: instructions with the same names (static dispatch), which could lead to
  //: spurious errors
  if (SIZE(inst.ingredients) < 2)
    break;  // error raised elsewhere
  if (!is_literal(inst.ingredients.at(1)))
    break;  // error raised elsewhere
  if (inst.ingredients.at(1).name.find_first_not_of("0123456789") != string::npos) {
    // since first non-address in base type must be a container, we don't have to canonize
    type_ordinal base_type = skip_addresses(inst.ingredients.at(0).type);
    if (base_type == -1)
      break;  // error raised elsewhere
    if (contains_key(Type, base_type)) {  // otherwise we'll raise an error elsewhere
      inst.ingredients.at(1).set_value(find_element_name(base_type, inst.ingredients.at(1).name, get(Recipe, r).name));
      trace(9993, "name") << "element " << inst.ingredients.at(1).name << " of type " << get(Type, base_type).name << " is at offset " << no_scientific(inst.ingredients.at(1).value) << end();
    }
  }
}

//: this test is actually illegal so can't call run
:(scenarios transform)
:(scenario transform_names_handles_containers)
def main [
  a:point <- copy 0/unsafe
  b:number <- copy 0/unsafe
]
+name: assign a 1
+name: assign b 3

//:: Support variant names for exclusive containers in 'maybe-convert'.

:(scenarios run)
:(scenario transform_names_handles_exclusive_containers)
def main [
  12:number <- copy 1
  13:number <- copy 35
  14:number <- copy 36
  20:point, 22:boolean <- maybe-convert 12:number-or-point/unsafe, p:variant
]
+name: variant p of type number-or-point has tag 1
+mem: storing 1 in location 22
+mem: storing 35 in location 20
+mem: storing 36 in location 21

:(before "End transform_names(inst) Special-cases")
// convert variant names of exclusive containers
if (inst.name == "maybe-convert") {
  if (SIZE(inst.ingredients) != 2) {
    raise << maybe(get(Recipe, r).name) << "exactly 2 ingredients expected in '" << inst.original_string << "'\n" << end();
    break;
  }
  assert(is_literal(inst.ingredients.at(1)));
  if (inst.ingredients.at(1).name.find_first_not_of("0123456789") != string::npos) {
    // since first non-address in base type must be an exclusive container, we don't have to canonize
    type_ordinal base_type = skip_addresses(inst.ingredients.at(0).type);
    if (base_type == -1)
      raise << maybe(get(Recipe, r).name) << "expected an exclusive-container in '" << inst.original_string << "'\n" << end();
    if (contains_key(Type, base_type)) {  // otherwise we'll raise an error elsewhere
      inst.ingredients.at(1).set_value(find_element_name(base_type, inst.ingredients.at(1).name, get(Recipe, r).name));
      trace(9993, "name") << "variant " << inst.ingredients.at(1).name << " of type " << get(Type, base_type).name << " has tag " << no_scientific(inst.ingredients.at(1).value) << end();
    }
  }
}
