# A Simple Web Server in C 

A basic web server in C that serves files from a specific directory and simulates rolling a Dungeons & Dragons d20 die when accessing a particular endpoint.

### Features: 

* Any URL should map to the current `src` directory and files that lie within. For example:

  ```
  http://localhost:8080/index.html
  ```

  should serve the file

  ```
  ./index.html
  ```
  
* If you request:
  ```
  http://localhost:8080/d20.html
  ```
  
  This will return a random number between 1 and 20. Simulating rolling a dungeons and dragons d20 die. 
  
### Configuration: 

* The default server root is the `src` directory. To change the root directory, modify the `SERVER_ROOT` value in the main file.
* The `src` directory contains `Work.html` and `index.html` for testing purposes. You can replace or add any other HTML files to this directory.

### To Build and Run:

1. There is a `Makefile` provided. On the command line, type `make` to build the server.

2. Type `./server` to run the server.

3. Open your web browser and navigate to `http://localhost:8080/` followed by the file name (eg. index.html, Work.html or d20.html).

### 

