// This is an example xc source file

struct Counter {
    int value;
    int step;
}

Counter :: void setStep(int a) {
    if (a < 0) {
        return; 
    }
    self.step = a;
}

Counter :: void reset(void) {
    self.value = 0;
}

Counter :: void increment(void) {
    self.value += self.step;
}

Counter :: void decrement(void) {
    self.value -= self.step;
}

bool isEven(int value) {
    return value % 2 == 0;
}

int main(void) {
    Counter c;

    c.reset();
    c.setStep(2);

    // c.value == 0
    // c.step == 2

    for (int i = 0; i < 20; ++i) {
        c.increment();
    }

    // c.value == 40

    if (isEven(c.value)) {
        c.setStep(7);
    } else {
        c.setStep(4);
    }

    // c.step == 7

    while (c.value < 89) {
        c.increment(); // c.value == 47, 54, 61, 68, 75, 82, 89
    }

    return c.value;
}
