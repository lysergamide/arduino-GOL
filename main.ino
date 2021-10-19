/**
 * An implementation of Conway's Game of Life on a 8x8 led matrix
 * 
 * 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 * 2. Any live cell with two or three live neighbours lives on to the next generation.
 * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
 * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 * 
 */

// 8x8, ergo...
const int ROWS = 8;
const int COLS = 8;

// array of row pin numbers:
const int row[ROWS] = { 0, 5, A7, 3, A0, A6, A1, A4 };
// array of column pin numbers:
const int col[COLS] = { 4, A2, A3, 1, A5, 2, 6, 7 };
// bool array representing the state of our world
bool world[ROWS][COLS] = { false };
// used to keep track of how much time has elapsed
unsigned long lastTime = 0;
// track pause/play state
volatile bool paused = false;
// track if step button was pressed;
volatile bool step = false;

void setup()
{
    // set pins to INPUT for interrupts
    pinMode(21, INPUT);  // pause button
    pinMode(20, INPUT);  // step button

    attachInterrupt(digitalPinToInterrupt(21), togglePause, RISING);
    attachInterrupt(digitalPinToInterrupt(20), setStep, RISING);

    // set matrix pins to OUTPUT
    for (int i = 0; i < 8; i++) {
        // initialize the pins as outputs
        pinMode(col[i], OUTPUT);
        pinMode(row[i], OUTPUT);
        // a LED is on when its column is LOW and its row is HIGH
        // start with everything turned off
        digitalWrite(col[i], HIGH);
        digitalWrite(row[i], LOW);
    }

    // fill the world randomly
    // we read from an unconnected pin as a source of randomness
    randomSeed(analogRead(13));
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            world[i][j] = (bool)(random(2));
}

void loop()
{
    unsigned long time = millis();
    // update the world every second
    if (time - lastTime >= 1000) {
        lastTime = time;
        updateWorld();
    }
    dispWorld();
    delay(5);
}

// toggle paused state
void togglePause()
{
    paused = !paused;
}
// step but only if we're paused
void setStep()
{
    if (paused)
        step = true;
}

/**
 * Turn light on for living cells, turn off dead ones
 * We turn LEDs on row by row (scanning)
 * The arduino does this so fast that it is not percieved as 
 * The row must be HIGH and col LOW for a LED to turn on
 */
void dispWorld()
{
    for (int r = 0; r < ROWS; r++) {
        // set current row to high
        digitalWrite(row[r], HIGH);
        for (int c = 0; c < COLS; c++) {
            // set current col LOW if the cell is alive
            if (world[r][c])
                digitalWrite(col[c], LOW);
            // set col back to HIGH
            digitalWrite(col[c], HIGH);
        }
        // set row back to LOW when we're done scanning it
        digitalWrite(row[r], LOW);
    }
}

/**
 * Helper function, counts neighbors at a given position
 * @param row row or y coord
 * @param col column or x coord
 */

int countNeighbors(int r, int c)
{
    int neighbors = 0;

    // for every row before and after r, inclusive
    for (int i = r - 1; i <= r + 1; i++) {
        // for ever column before and after c, inclusive
        for (int j = c - 1; j <= c + 1; j++) {
            // don't count the cel itself
            if (i == r && j == c)
                continue;
            // handling neighbors that 'wrap' around
            int y = i >= 0 ? i % ROWS : ROWS - 1;
            int x = j >= 0 ? j % COLS : COLS - 1;
            // add one to neighbors if the cell is alive
            neighbors += world[y][x];
        }
    }

    return neighbors;
}

void updateWorld()
{
    // we can't mutate the cells until after we decide if they should die or not
    // so this temp array will hold the next state until then
    bool nextWorld[ROWS][COLS] = { false };

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            // We can reduce the rules to three cases
            switch (countNeighbors(r, c)) {
            case 2:  // the cell stays the same when it has two neighbors
                nextWorld[r][c] = world[r][c];
                break;
            case 3:  // the cell is always alive when it has three neighbors
                nextWorld[r][c] = true;
                break;
            default:  // in all other cases the cell dies
                nextWorld[r][c] = false;
                break;
            }
        }
    }

    // update the world when we're finished
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            world[r][c] = nextWorld[r][c];
}
