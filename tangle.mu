# example program: constructing recipes out of order
#
# We construct a factorial function with separate base and recursive cases.
# Compare factorial.mu.
#
# This isn't a very tasteful example, just a simple demonstration of
# possibilities.

recipe factorial [
  local-scope
  n:number <- next-ingredient
  {
    +base-case
  }
  +recursive-case
]

after +base-case [
  # if n=0 return 1
  zero?:boolean <- equal n, 0
  break-unless zero?
  reply 1
]

after +recursive-case [
  # return n * factorial(n - 1)
  x:number <- subtract n, 1
  subresult:number <- factorial x
  result:number <- multiply subresult, n
  reply result
]

recipe main [
  1:number <- factorial 5
  $print [result: ], 1:number, [
]
]
