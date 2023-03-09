
# token define

## literal


# grammer

## assignment

assignment returns notiong, so `a=b=c` is illegal

## complex literal

1. designated initialized literal
   `Vector2{.x: 0.1, .y: 0.2}`, `[]i64{1: 12, 5: 10}`, `Vector2{"x": 0.1, "y": 0.2}` or `[]f64{x+y: 12}`
2. list initialized literal
   `[]i64{1, 2, 3, 4}`

after `type Vector2 struct{x f64; y f64}`, `Vector2{.x: 1, .y: 2}` and `Vector2{"x": 1, "y": 2}` are equivalent(string literal only, `var sym string = "x", Vector2{sym: 1}` are not allowed)

## member access

after `type Vector2 struct{x f64; y f64};var v := Vector2{.x: 1, .y: 2}`, `v["x"]` and `v.x` are equivalent.

## function definition

1. base function definition:
   `func foo(a i32, b i32):i32->a + b`
   1. additionally, if returned expression is a Block expr, `->` can omitted like
      `func foo(a i32, b i32):i32{1}`
   2. if no return value, the definition is like
      `func foo(a i32){}` 
      at present, no-returned function is useless except member function due to closure not supported.
   above 2 syntactic sugar is available for definition types down.
2. member function definition:
   `func add(a i32)(b i32):i32->a+b`
3. user defined infix operator:
   `func is infix(a i32, b i32):i32->a==b`
4. operator overload
   `func +(a Vector3, b Vector3):Vector3->Vector3{.x:a.x+b.x, .y:a.y+b.y, .z:a.z+b.z}`

### member function definition

## other

### tokenized symbol

`(+)(1, 2) //return 3`