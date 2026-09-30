# Railway Reservation System in C

A console-based Railway Reservation System developed using C programming. This project simulates important railway booking operations such as passenger reservation, automatic seat allocation, PNR generation, ticket cancellation, waiting-list management, and file-based data storage.

## Features

- Passenger booking
- Automatic seat allocation
- Unique PNR generation
- PNR-based ticket search
- Train selection
- Travel date management
- Sleeper and AC class selection
- Fare calculation with GST
- Ticket cancellation
- Automatic seat release
- Waiting-list management
- Automatic waiting-list promotion
- Passenger modification
- Admin login and dashboard
- Train management
- File-based data persistence
- Input validation

## Technologies Used

- C Programming
- Structures
- Linked Lists
- Dynamic Memory Allocation
- File Handling
- Functions
- String Handling

## Data Structures Used

### Linked List

Linked lists are used to store passenger booking information dynamically.

### Waiting List

A separate linked-list structure is used to manage passengers when all seats are occupied.

## How the System Works

1. The user selects an operation from the main menu.
2. Passenger details are entered.
3. The user selects a train and travel date.
4. The system checks seat availability for that specific train and date.
5. An available seat is automatically allocated.
6. A unique PNR number is generated.
7. The fare and GST are calculated.
8. The booking information is saved to a file.
9. If all seats are occupied, the passenger is added to the waiting list.
10. When a confirmed booking is cancelled, a waiting-list passenger can be automatically promoted.

## Admin Login

```text
Username: admin
Password: railway123
