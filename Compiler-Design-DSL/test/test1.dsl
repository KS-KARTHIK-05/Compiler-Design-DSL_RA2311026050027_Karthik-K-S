// test1.dsl - Comprehensive test for Simple Imperative DSL
// Tests: declarations, assignments, if/else, while, print, arithmetic

// Variable declarations
int x;
int y;
int result;
bool flag;

// Assignments
x = 10;
y = 3;
flag = true;

// Arithmetic
result = x + y * 2;
print(result);

// If-else
if (x > y) {
    result = x - y;
    print(result);
} else {
    result = y - x;
    print(result);
}

// While loop with condition
int counter;
counter = 0;
while (counter < 5) {
    print(counter);
    counter = counter + 1;
}

// Nested if
if (flag) {
    if (x >= 10) {
        print(x);
    }
}

// Logical operators
bool check;
check = (x > 5) && (y < 10);
if (check) {
    print(x);
}

// Print string
print("Hello from DSL!");
