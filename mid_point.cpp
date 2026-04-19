int x = 0;
int y = 100;
int d = 1 - 100;

while (x < y) {
    if (d < 0)
        d += 2*x + 3;
    else {
        d += 2*(x - y) + 5;
        y--;
    }
    x++;
}