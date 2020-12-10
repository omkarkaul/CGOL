//
//  main.cpp
//  CGOL
//
//  Created by Omkar on 26/11/20.
//
//  RULES:
//  - Live cell with 2 or 3 live neighbours lives
//  - Dead cell with 3 live neighbours becomes alive
//  - All other live cells die, all other dead cells stay dead
//

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_set>
#include <time.h>
#include <mutex>
#include "taskflow/taskflow/taskflow.hpp"

int COLS = 0;
int ROWS = 0;

tf::Taskflow taskflow;
tf::Executor executor;

std::mutex mutex;

//////////////////////////////////////////////////////////////////////
// FROM https://stackoverflow.com/a/29855973 ////////////////////////
struct VectorHash {
    size_t operator()(const std::vector<int>& v) const {
        std::hash<int> hasher;
        size_t seed = 0;
        for (int i : v) {
            seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }
        return seed;
    }
};

using int_vector_set = std::unordered_set<std::vector<int>, VectorHash>;
//////////////////////////////////////////////////////////////////////

int mod(int a, int b) {
    // real modulo function, not crappy C++ remainder %
    
    int r = a % b;
    return r < 0 ? r + abs(b) : r;
}

std::vector<std::vector<char>> read_input_file(std::string filename) {
    // reads and input state from input.txt and builds the cells matrix
    
    std::vector<std::vector<char>> cells;

    // open file
    std::ifstream file;
    file.open("./" + filename);

    // exit if file doesn't exist
    if(!file.is_open()) {
      perror("Error open");
      exit(EXIT_FAILURE);
    }

    // read input file line by line
    std::string line_buffer;
    while(std::getline(file, line_buffer)) {
        std::string input_line(line_buffer); // write acquired line buffer into input line
        std::vector<char> row(input_line.size(), '-'); // init row with all dead cells ('-') from input line
        
        ROWS++; // increment num of rows
        COLS = (int)row.size(); // set number of cols to length of row (they're all the same)
        
        // loop through chars in input line, if any cell is alive ('X') then copy this to the row
        for(int i=0; i<input_line.size(); i++) {
            char c = input_line[i];
            if(c == 'X') {
                row[i] = 'X';
            }
        }
        
        // append row to main vector
        cells.push_back(row);
    }
    
    return cells;
}

void pretty_print(std::vector<std::vector<char>> cells) {
    // CGOL HELPER FUNCTION
    // pretty print the cells matrix
    
    for(std::vector<char> row : cells) {
        for(char c : row) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int_vector_set get_neighbours(std::vector<std::vector<char>> cells, std::vector<int> coord) {
    // CGOL HELPER FUNCTION
    // gets all neighbours of a specific state, for a particular cell
    // states : 'X' (alive), '-' (dead), NULL (both, default)
    // modulo coordinate lookup by the number of cols/rows to support wrap around
    // i.e. ((i-1)%ROWS, ((j+1)%COLS) for NORTHWEST NEIGHBOUR
    
    int_vector_set neighbours;
    int i = coord[0];
    int j = coord[1];
    
    // both alive and dead
    if(cells[mod(i-1,ROWS)][mod(j,COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i-1,ROWS), mod(j,COLS) });} // NORTH NEIGHBOUR
    if(cells[mod(i-1,ROWS)][mod((j+1),COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i-1,ROWS),mod(j+1,COLS)}); } // NORTHWEST NEIGHBOUR
    if(cells[mod(i, ROWS)][mod(j+1, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i,ROWS), mod(j+1, COLS)}); } // EAST NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j+1, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j+1, COLS)}); } // SOUTHEAST NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j, COLS)}); } // SOUTH NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j-1, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j-1, COLS)}); } // SOUTHWEST NEIGHBOUR
    if(cells[mod(i, ROWS)][mod(j-1, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i,ROWS), mod(j-1, COLS)}); } // WEST NEIGHBOUR
    if(cells[mod(i-1, ROWS)][mod(j-1, COLS)] == 'X' or '-') {neighbours.insert(std::vector<int>{mod(i-1,ROWS), mod(j-1, COLS)}); } // NORTHWEST NEIGHBOUR
}

int_vector_set get_neighbours(std::vector<std::vector<char>> cells, std::vector<int> coord, char state) {
    // CGOL HELPER FUNCTION
    // gets all neighbours of a specific state, for a particular cell
    // states : 'X' (alive), '-' (dead), NULL (both, default)
    // modulo coordinate lookup by the number of cols/rows to support wrap around
    // i.e. ((i-1)%ROWS, ((j+1)%COLS) for NORTHWEST NEIGHBOUR
    
    int_vector_set neighbours;
    int i = coord[0];
    int j = coord[1];
    
    
    // either alive or deadr
    if(cells[mod(i-1,ROWS)][mod(j,COLS)] == state) {neighbours.insert(std::vector<int>{mod(i-1,ROWS), mod(j,COLS) });} // NORTH NEIGHBOUR
    if(cells[mod(i-1,ROWS)][mod((j+1),COLS)] == state) {neighbours.insert(std::vector<int>{mod(i-1,ROWS),mod(j+1,COLS)}); } // NORTHWEST NEIGHBOUR
    if(cells[mod(i, ROWS)][mod(j+1, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i,ROWS), mod(j+1, COLS)}); } // EAST NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j+1, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j+1, COLS)}); } // SOUTHEAST NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j, COLS)}); } // SOUTH NEIGHBOUR
    if(cells[mod(i+1, ROWS)][mod(j-1, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i+1,ROWS), mod(j-1, COLS)}); } // SOUTHWEST NEIGHBOUR
    if(cells[mod(i, ROWS)][mod(j-1, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i,ROWS), mod(j-1, COLS)}); } // WEST NEIGHBOUR
    if(cells[mod(i-1, ROWS)][mod(j-1, COLS)] == state) {neighbours.insert(std::vector<int>{mod(i-1,ROWS), mod(j-1, COLS)}); } // NORTHWEST NEIGHBOUR
    
    return neighbours;
}

int_vector_set get_all_living_cells(std::vector<std::vector<char>> cells) {
    // CGOL HELPER FUNCTION
    // spawn thread for each row in cells, parallelly walk through
    // each row and populate hashset of alive cells with 2 member 
    // int vector representing coords of each live cell
    int_vector_set alive;
    
    taskflow.for_each_index(0, (int)cells.size(), 1, [&] (int i) {
        std::vector<char> row = cells[i];
        for(int j=0; j<row.size(); j++) {
            if(cells[i][j] == 'X') {       
                mutex.lock();
                alive.insert(std::vector<int>{i,j});
                mutex.unlock();
            }
        }
    });
    
    executor.run(taskflow).wait();
    taskflow.clear();

    return alive;
}

int_vector_set get_dead_of_interest(std::vector<std::vector<char>> cells, int_vector_set alive) {
    // CGOL HELPER FUNCTION
    // walk through alive cells and populate hashset of dead
    // cells OF INTEREST (i.e. dead cells that neighbour currently
    // live cells, because all other dead stay dead
    int_vector_set dead_of_interest;

    for(std::vector<int> coord : alive) {
        int_vector_set neighbours = get_neighbours(cells, coord, '-');
        for(std::vector<int> neighbour : neighbours) {
            dead_of_interest.insert(neighbour);
        }
    }
    
    return dead_of_interest;
}

std::vector<std::vector<char>> perform_update(std::vector<std::vector<char>> cells, int_vector_set new_alive, int_vector_set new_dead) {
    // birth new live cells
    for(std::vector<int> coord : new_alive) { cells[coord[0]][coord[1]] = 'X'; }
    // kill other cells
    for(std::vector<int> coord : new_dead) { cells[coord[0]][coord[1]] = '-'; }
    
    return cells;
}

void CGOL(std::vector<std::vector<char>> cells, int iterations, bool verbose) {
    // performs CGOL
    
    pretty_print(cells);

    // do main work
    while(iterations > 0) {
        iterations--;
            
        int_vector_set alive_cells = get_all_living_cells(cells);
        int_vector_set dead_of_interest = get_dead_of_interest(cells, alive_cells);

        int_vector_set new_alive_cells;
        int_vector_set live_cells_to_die;

        // RULE 1 (updates new_alive_cells)
        for(std::vector<int> live_coord : alive_cells) {
            int num_of_live_neighbours = (int)get_neighbours(cells, live_coord, 'X').size();
            if(2 <= num_of_live_neighbours && num_of_live_neighbours <= 3) { new_alive_cells.insert(live_coord); }
        }
        
        // RULE 2 (updates new_alive_cells)
        for(std::vector<int> relevant_dead_coord : dead_of_interest) {
            if(get_neighbours(cells, relevant_dead_coord, 'X').size() == 3) { new_alive_cells.insert(relevant_dead_coord); }
        }
        
        // RULE 3 (updates live_cells_to_die)
        for(std::vector<int> live_cell : alive_cells) {
            if(new_alive_cells.find(live_cell) == new_alive_cells.end()) {
                live_cells_to_die.insert(live_cell);
            }
        }
        
        cells = perform_update(cells, new_alive_cells, live_cells_to_die);
        
        if(verbose) { pretty_print(cells); }
    }
    
    if(!verbose) { pretty_print(cells); }
}

int main(int argc, char* argv[]) {
    int iterations = 1;
    bool verbose = false;
    std::string filename;
    
    if(argc >= 2) {
        filename = argv[1];
    }

    // check if iterations are passed as an arg
    if(argc >=3) {
        if(std::string(argv[1]) == "-i") {
            iterations = std::atoi(argv[2]);
        }
    }
    
    // get number of iterations
    if(argc >= 4) {
        iterations = std::atoi(argv[3]);
    }

    // check if verbose mode is passed as an arg
    if(argc >= 5) {
        if(std::string(argv[4]) == "-v") {
            verbose = true;
        }
    }
    
    std::vector<std::vector<char>> cells = read_input_file(filename); // generate char matrix from input file

    time_t start = clock();
    CGOL(cells, iterations, verbose); // perform CGOL
    time_t end = clock();

    std::cout << "EXECUTION TIME: " << (double)(end-start)/CLOCKS_PER_SEC << "s" << std::endl;

    return 0;
}
