# Budget-Tracking-System

This command-line budget tracking system is written in C and allows users to manage income and expense entries from a text-based financial database.

## Features

- Read and parse entries from a file passed via command-line argument  
- Display all entries in a formatted table  
- Compute total income, total expenses, needs vs. wants breakdown, and net balance  
- Sort entries by ID, date, amount, or description  
- Add new income/expense entries with automatic ID increment  
- Modify existing entry amounts  
- Filter transactions by month and year  
- Clean screen and menu-driven interface with user prompts  
- Memory allocation using `malloc`, `free`, and modular design across `.c` and `.h` files  
- Error handling for invalid file input and negative amounts

To compile the program:

```bash
gcc -o budgetTracker main.c budget.c data.c ordering.c
