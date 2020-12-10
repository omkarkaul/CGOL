# Conway's Game of Life

Reasonably efficient implmentation using sets instead of arrays in CPP.  

Provide a initial state using a text file called ```{some_name_here}.txt```. An example is provided, known as glider gun.  

Compile with ```g++ main.cpp -o main.o```  
Run with ```./main.o {some_name_here}.txt -i {number of iterations} -v```   
&nbsp;   


# Parallel Conway's Game of Life

Parallelised version of CGOL using the taskflow library, acquired here: https://github.com/taskflow/taskflow.   

Provide a initial state using a text file called ```{some_name_here}.txt```. An example is provided, known as glider gun.  

Compile with ```g++ parallel_cgol.cpp -o parallel_cgol.o```  
Run with ```./parallel_cgol.o {some_name_here}.txt -i {number of iterations} -v```   

Enjoy the magic