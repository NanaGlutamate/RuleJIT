
# token define

## literal


# grammer

## assignment

assignment returns notiong, so `a=b=c` is illegal

## control flow

1. if:
2. while:
   labeled while:

## complex literal

1. designated initialized literal
   `Vector2{.x: 0.1, .y: 0.2}`, `[]i64{1: 12, 5: 10}`, `Vector2{"x": 0.1, "y": 0.2}` or `[]f64{x+y: 12}`
2. list initialized literal
   `[]i64{1, 2, 3, 4}`

after `type Vector2 struct{x f64; y f64}`, `Vector2{.x: 1, .y: 2}` and `Vector2{"x": 1, "y": 2}` are equivalent(string literal only, `var sym string = "x", Vector2{sym: 1}` are not allowed)

## member access

after `type Vector2 struct{x f64; y f64};var v := Vector2{.x: 1, .y: 2}`, `v["x"]` and `v.x` are equivalent.

## function definition

only support top-level function definitions for now.

1. base function definition:
   `func foo(a i32, b i32):i32->a + b`
   1. additionally, if returned expression is a Block expr, `->` can omitted like
      `func foo(a i32, b i32):i32{1}`
   2. if no return value, the definition is like
      `func foo(a i32){}` 
   3. TODO: type inferrence
      `func foo(a i32)->a`
   above statements is suitable for definition types followed.
2. member function definition:
   `func add(a i32)(b i32):i32->a+b`
   `func add(a *i32)(b i32):i32->a+b`
3. user defined infix operator:
   `func is infix(a i32, b i32):i32->a==b`
4. operator overload
   `func +(a Vector3, b Vector3):Vector3->Vector3{.x:a.x+b.x, .y:a.y+b.y, .z:a.z+b.z}`

## function overloading

automatically. overload by param type TODO: or return type

## operator overloading

can overloading infix and unary operators.

1. normal operator: straight forward.
   `func +(v1 Vector3, v2 Vector3):Vector3->Vector3{v1.x+v2.x,v1.y+v2.y,v1.z+v2.z}`
2. special operator: overloading infix operator with short-circuilt logic like `&&` and `||` which can only seen as a marco rather than a function will change it's behaviour, so at present it's not supported.
3. member access:
   `func (tar Vector3, name const string):f64{match(name){}}`

## lambda and closure

TODO: lambda
TODO: directly called lambda

## captures

TODO: value capture by default; can be explicitlly decleared like
   `func add(i i32):i32{func[var p *i32=&i](){p<-*p+1}();i}`

## scope

a expression which is

1. a block,
2. a branch,
3. or a loop

will create a new scope

## extern declearation

1. extern type declearation
   `extern type u64 as Vector3 struct {x f64; y f64; z f64;}`
   to use type check with outer resource, which will use u64 as a resource handler.
   extern type do not have any function or operation unless overloaded.
2. extern function declearation
   1. normal
      `extern func sin(f64, f64):f64`
   2. rename declearation
      `extern func vadd(u64, u64):u64 as +(Vector3, Vector3):Vector3`
      `extern func memberAccess(u64, u64):f64 as .(Vector3, const string):f64`
   call to extern function will be handled by C function of that name.

## other

### type check

1. Anonymous structs are automatically given names that are not duplicated, so anonymous structs are defined differently in different places, but different instance types of anonymous structs defined in the same place are the same
   `var a struct{i i32} = struct{i i32}{.i: 0} // wrong, use tuple instead`
2. member function call to assignable expression will automatically call it's pointer's member function if possible.
3. TODO: auto type tramsform guide
   `type i64(i32) = to64 // where to64 is func(i32):i64`

### tokenized symbol

TODO: `(+)(1, 2) //return 3`