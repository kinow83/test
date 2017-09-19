
#include <stdio.h>
#include <stdlib.h>
class Person {
private:
    char *name;
    int age
public:
    Person(char *name, int age) {
        this.name = strdup(name);
        this.age = age
    }
    ~Person() {
        free(this.name)
        free(this.age)
    }
    void show() {
        printf("name: %s\n", this.name);
        printf("age : %s\n", this.age);
    }
}