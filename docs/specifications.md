# The XC Language Specifications
*Last updated on February 23, 2024*

## Introduction
**XC** is a statically typed, compiled, procedural programming language designed for general-purpose use.

<hr/>

## Grammar Notation
The syntax for this language will be described using a variant of [Extended Backus Naur Form (EBNF)](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) in this document.

### The variant of EBNF goes as follows:
```ebnf
<non-terminal> =    (* Defines a production rule for a non-terminal. *)
    ...             (* Terminal, non-terminals, and symbols (see below) are specified here. *)
    ;               (* Indicates the end of the production rule. *)
```

### Notation Symbols:
```ebnf
| Or
() Grouping
* 0 or more occurrences
+ 1 or more occurrences
? Optional (0 or 1 occurrences)
"" Literal
```

For a comprehensive and detailed syntax specification, refer to the [*grammar.ebnf*](grammar.ebnf) file located in the `docs/` directory.

<hr />

## Lexical Elements

### Comments
Comments are text that are ignored by the compiler. They are used to provide an explanation, description, or context to the reader of the source file. Comments are either single-line or multi-line.

 * **Single-line**: Begin with `//` and extends to the end of the line or file
 * **Multi-line**: Begin with `/*` and continue until the first `*/` or to the end of the file.

### Numbers
Numbers can take the form of either a integer literal (whole numbers) or a float-point literal (number with a decimal point). 

 * **Integers** consist of a sequence of digits. By default, integer literals are defined with the type `int` (32-bits).
    > :notebook: **Note**: There is a special case for the number `0`. It could either stand alone (e.g. `0`) or can be followed `b`, `o`, `x` to represent a binary, octal, or hexadecimal value respectively. Having superfluous `0`s, or not providing the proper digits after `b`, `o`, `x` will report an error.
    ```ebnf
    <integer-literal> =
        <decimal>
        | <binary>
        | <octal>
        | <hexadecimal>
        ;
    ```
    ```ebnf
    <decimal> =
        "0"
        | <non-zero-digit> <digit>*
        ;

    <non-zero-digit> =
        "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
        ;

    <digit> =
        "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
        ;
    ```
    ```ebnf
    <binary> =
        "0b" <binary-digit>+
        ;

    <binary-digit> =
        "0" | "1"
        ;
    ```
    ```ebnf
    <octal> =
        "0o" <octal-digit>+
        ;

    <octal-digit> =
        "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7"
        ;
    ```
    ```ebnf
    <hexadecimal> =
        "0x" <hexadecimal-digit>+
        ;

    <hexadecimal-digit> =
        "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
        | "a" | "b" | "c" | "d" | "e" | "f" | "A" | "B" | "C" | "D" | "E" | "F"
        ;
    ```
 * **Floating-point** values consist of an integer literal (in base-10) followed by a `.` and another literal literal (also in base-10)
    ```ebnf
    <float> =
        <decimal> "." <decimal>
        ;

    (* See above for the definition for a decimal *)
    ```

### Identifiers
Identifiers is a sequences of one or or letters, digit, or underscores. Refer to the list below for reserved words.
> :notebook: **Note**: The first character of an identifier cannot be a digit. 
```ebnf
<identifier> =
    ("_" | <letter>) ("_" | <letter> | <digit>)*
    ;
```

### Reserved Words
The following words are reserved keywords and may not be used as user-defined identifiers.
```
bool    break   byte    continue    
else    enum    false   float
for     if      int     long
null    return  short   struct
true    void    while
```
> :warning: **Note**: This is not a complete list, more reserved words may be introduced in the future!

### Operators and Punctuation symbols
The following symbols are used to represent operators and punctuation.
```
(    )    {   }   [   ]   ;   :   ::
.    ,    =   +   -   *   /   %   &
^    |    ~   <   >   !   ==  +=  -=
*=   /=   %=  &=  ^=  |=  <=  >=  !=
<<=  >>=  &&  ||  <<  >>
```

### Operator Precedence
The following table lists out the precedence of the operators found in the XC language. Operators are ranked from highest to lowest precedence.

<!-- I am sorry for this table.  -->
| **Operator**                                                                     | **Description**                                                                                                                                                                                                |
| :--------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `.`<br />`()`<br />`[]`<br />                                                | Structure member access<br />Function Call<br />Array Indexing                                                                                                                                             |
| `type()`                                                                     | Type casting                                                                                                                                                                                               |
| `++` `--`                                                                    | Postfix increment, Postfix decrement                                                                                                                                                                       |
| `++` `--`<br />`-`<br />`!`<br />`~`<br/>`&`                                 | Prefix increment, Prefix decrement<br />Negation<br />Logical NOT<br />Bitwise complement<br />Reference-of                                                                                                |
| `*` `/` `%`                                                                  | Multiplication, Division, Modulus                                                                                                                                                                          |
| `+` `-`                                                                      | Addition, Subtraction                                                                                                                                                                                      |
| `<<` `>>`                                                                    | Bitwise left-shift, Bitwise right-shift                                                                                                                                                                    |
| `<` `<=`<br />`>` `>=`                                                       | Relational less than, Relational less than or equal to<br />Relational greater than, Relational greater than or equal to                                                                                   |
| `==` `!=`                                                                    | Relational equality, Relational inequality                                                                                                                                                                 |
| `&`                                                                          | Bitwise AND                                                                                                                                                                                                |
| `^`                                                                          | Bitwise XOR                                                                                                                                                                                                |
| `\|`                                                                         | Bitwise OR                                                                                                                                                                                                 |
| `&&`                                                                         | Logical AND                                                                                                                                                                                                |
| `^^`                                                                         | Logical XOR                                                                                                                                                                                                |
| `\|\|`                                                                       | Logical OR                                                                                                                                                                                                 |
| `=`<br />`+=` `-=`<br />`*=` `/=` `%=`<br />`\|=` `^=` `&=`<br />`<<=` `>>=` | Simple assignment<br />Assignment by sum and difference<br />Assignment by product, quotient, remainder<br />Assignment by bitwise OR, XOR, AND<br />Assignment by bitwise left-shift, bitwise right-shift |

<hr />

## Types
Every variable and value is associated with a type. The type of a value dictates it size in memory and determines which operations can be performed on the value.

### Void
The void type, denoted by the keyword `void`, is used to indicate the absence of a specific type. It represent having no value. It can be used in two cases:
 * As a ***function's return type***, to indicate that the function will not return a value.
 * As a ***function's parameter***, to indicate that the function will not accept any arguments.

### Boolean
The boolean type, denoted by the keyword `bool`, is used to indicate a boolean value, which can either contain a `true` or `false` value. They are store as 8-bits.

Boolean values can be defined using the literal keywords `true` and `false`.

#### Operations on boolean values
Here is a list of operations and their corresponding operators that can be done with boolean values.

 * Logical NOT (`!`)
 * Logical OR (`||`)
 * Logical XOR (`^^`)
 * Logical AND (`&&`)
 * Relational EQUALITY (`==`)
 * Relational INEQUALITY (`!=`)

> :notebook: **Note**: These operations require both operands to be of boolean type for them to function properly!

### Numeric
Numeric values consist of integers and floating-point values.

#### Integer
Integers represent whole numbers. All integers are [*two's complement*](https://en.wikipedia.org/wiki/Two%27s_complement) signed integers.
| **Type** | **Size** | **Minimum Value**    | **Maximum Value**    |
| :------- | :------- | :------------------- | :------------------- |
| `byte`   | 8-bits   | -128                 | 127                  |
| `short`  | 16-bits  | -32768               | -32767               |
| `int`    | 32-bits  | -2147483648          | -2147483647          |
| `long`   | 64-bits  | -9223372036854775808 | -9223372036854775807 |

#### Floating-Point
Floating-Point represent numbers with a decimal point.
| **Type** | **Size** |
| :------- | :------- |
| `float`  | 32-bits  |
| `double` | 64-bits  |

#### Operations on numeric values
Here is a list of operations and their corresponding operators that can be done with numeric values.

 * Arithmetic
   * Addition (`+`)
   * Subtraction (`-`)
   * Multiplication (`*`)
   * Division (`/`)
   * Modulus (`%`)
   * Negation (`-`)
 * Bitwise
   * Left-shift (`<<`)
   * Right-shift (`>>`)
   * OR (`|`)
   * XOR (`^`)
   * AND (`&`)
   * Complement (`~`)
 * Relational
   * Equality (`==`)
   * Inequality (`!=`)
   * Less than (`<`)
   * Less than or equal to (`<=`)
   * Greater than (`>`)
   * Greater than or equal to (`>=`)

> :notebook: **Note**: Bitwise operators and the arithmetic modulus operator may only be used for integer values. When using arithmetic operators with integer and floating-point values the resulting value will be a floating-point. 

### Array
An array is a sequence of element of the same type. The length of the array of a fixed and positive size.

```ebnf
(* The array type is built within the `type` production rule *)
<type> =
    "&"? (<builtin-type> | <identifier>) <dimensions>*
    ;

<array> =
    "[" <expression-list>? "]"
    | <type> "[" <integer-literal> "]"
    ;
```

#### Example of arrays
```c
int main(void) {

    // ...

    int[] arr = [2, 4, 5, 6, 8];
    int arr_length = arr.length; // 5

    int[] arr2;
    arr2 = [1, 2, 3, 4];
    arr2_length = arr2.length; // 4

    int[] arr3 = int[3];

    arr3[0] = 3;
    arr3[1] = 2;
    arr3[2] = 1;

    // ...

    return 0;
}
```

#### Operations on arrays
There are 2 operations that can be done on arrays.

 * Member access (`.`)
   * All arrays will have a single member called `length`. Like the name suggest, length represents the size of the array.
 * Indexing (`[]`)
   * This is used to access the element at a given position. The first element is at index **0**.

### Struct
Struct types are a way of grouping related data together into a single structure (hence the name).

```ebnf
<structure> =
    "struct" <identifier> "{" <variable-declaration-statement> "}"
    ;
```

Struct types can have member functions.
```ebnf
(* where `identifier` is the struct name *)
<structure-member-function> =
    <identifier> "::" <function>
    ;
```

#### Example of struct
```c
struct Counter {
    int count;
}

Counter :: void reset(void) {
    self.count = 0;
}

Counter :: void increment(int step) {
    self.count += step;
}

int main(void) {

    // ...

    Counter c;

    c.reset();
    c.increment(20);
    c.increment(4);

    // ...

    return 0;
}
```

#### Operations on struct
 * Member access (`.`)

### Enum
Enum types consists of a set of named constants.

```ebnf
<enumerator> =
    "enum" <identifier> "{" <identifier-list> "}"
    ;
```

#### Example of enum
```c
enum Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
}

int main(void) {

    // ...

    Direction dir;

    dir = NORTH;

    // ...

    return 0;
}
```

<hr />

## Functions

Grammar to define a function:
```ebnf
<function> =
    ("void" | <type>) <identifier> "(" ("void" | <parameter-list>) ")" <block-statement>
    ;
```

#### Example of functions
```c

int main(void) {

    // ...

    int x = 42;
    int y = 59;

    int z = add(42, 59); // 101

    // ...

}

// assuming `a` and `b` are both positive values
int add(int a, int b) {
    if (a < b) {
        return add(b, a);
    } else if (b != 0) {
        return add(incr(a), decr(b));
    }
}

int incr(int v) {
    return v + 1;
}

int decr(int v) {
    return v - 1;
}
```

<hr />

## Statements

### Variable Declaration
```ebnf
<variable-declaration-statement> =
    <variable-declarator> (";" | "=" <expression-statement>)
    ;

<variable-declarator> =
    <type> <identifier>
    ;
```

#### Example of variable declaration
```c
int main(void) {
    
    // ...

    bool flag = true;
    
    byte binary42 = 0b00101010;

    bool[] this_a_bool_array = [true, true, false, true, false];

    int my_special_num = 23;

    &int num_ref = &my_special_num;

    num_ref = 84; // my_special_num == 84 now.

    // ...

    return 0;
}
```

### Conditional 
```ebnf
(* Where `<expression>` needs to evaluate to a boolean value *)
<conditional-statement> =
    "if" "(" <expression> ")" <block-statement> ("else" (<conditional-statement> | <block-statement>))?
    ;
```

#### Examples of conditionals
```c
int main(void) {

    // ...

    int value = 40;

    if (value > 20) {
        value /= 2;
    } else if (isEven(value)) {
        value *= 2;
    } else {
        value *= value;
    }

    // ...

    return 0;
}

bool isEven(int v) {
    if (v % 2 == 0) {
        return true;
    } else {
        return false;
    }
}
```

### Iteration
While loop:
```ebnf
<while-iteration> =
    "while" "(" <expression> ")" <block-statement>
    ;
```

#### Examples of iteration
```c
int main(void) {

    // ...

    int i = 0;
    while (i < 20) { // does something 20 times

        // ...

        ++i;
    }
    
    // ...

    return 0;
}
``` 

<hr />