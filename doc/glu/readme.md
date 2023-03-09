
# token define

## literal


# grammer

## complex literal

1. designated initialized literal
   `Vector2{.x: 0.1, .y: 0.2}`, `Vector2{"x": 0.1, "y": 0.2}` or `[]f64{x+y: 12}`
2. list initialized literal
   `[]i64{1, 2, 3, 4}`

after `type Vector2 struct{x f64; y f64}`, `Vector2{.x: 1, .y: 2}` and `Vector2{"x": 1, "y": 2}` are equivalent.

## member access

after `type Vector2 struct{x f64; y f64};var v := Vector2{.x: 1, .y: 2}`, `v["x"]` and `v.x` are equivalent.

## function definition

1. base function definition:
   `func foo(a i32, b i32):i32->a + b`
   1. additionally, if returned expression is a Block expr, `->` can omitted like
        `func foo(a i32, b i32):i32{1}`

### member function definition

## other

### tokenized symbol

`(+)(1, 2) //return 3`