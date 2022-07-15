# Wave Function Collapse
This is my C++ implementation of the [Wave Function Collapse](https://en.wikipedia.org/wiki/Wave_function_collapse) algorithm, using the OneLoneCoder's [Pixel Game Engine](https://github.com/OneLoneCoder/olcPixelGameEngine).

## How to Start? (Linux)
```console
$ git clone https://github.com/T0nd0Tara/Wave-Function-Collapse
$ cd Wave-Function-Collapse
$ cp tiles/basic/* tiles/
$ ./run.sh
```

## rules.txt structures
First line is the dimensions of the wanted grid seperated with a space  
` 
grid_x grid_y
`

### set grid cells
set cell: `s <x> <y> <cell_num>`  
set row: `sr <row_num> <cell_num>`  
set col: `sc <col_num> <cell_num>`  

### instructions
To add a cell instruction first write the cell number
And the next 4 lines should be the available connection types for [UP, RIGHT, DOWN, LEFT] in that order
  
For example the lines
```
0
1 2
3
4
5
```
Would set the 0'th cell's connection types to
UP   : 1 and 2
RIGHT: 3
DOWN : 4
LEFT : 5

Each cell can connect to his neighbor if they have matching connection types

### comments
Any line that starts with `\\ ` is a comment

**CAUTION: The program will not see `\\ ` in a middle of a line!**
