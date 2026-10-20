#include <stdio.h>

#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400
#define MIN_TICKETS 1
#define MAX_TICKETS 20
#define DISCOUNT_LIMIT 5
#define DISCOUNT_RATE 10

typedef enum {
    STANDARD = 1,
    PREMIUM = 2,
    RECLINER = 3
} TicketCategory;

typedef struct
{
    char customerName[50];
    int tickets;
    TicketCategory category;
} BookingInput;

typedef struct
{
    float subtotal;
    float discount;
    float finalBill;
} Bill;

BookingInput booking;
Bill bill;

// Function declarations
int validateName(char name[]);
void takeCategory();
int takeInputs();
int calculateSubtotal();
int calculateDiscount();
void calculateFinalBill();
void displayBookingDetails();

// Function to validate customer name
int validateName(char name[])
{
    int i=0;
    if (name[0]=='\0')
    return 0;
    while(name[i]!='\0')
    {
        if(!((name[i]>='A' && name[i]<='Z') || (name[i]>='a' && name[i]<='z')))
        return 0;
        i++;
    }
    return 1;
}

// Function to take ticket category
void takeCategory()
{
    int choice;
    while(1)
    {
        scanf("%d", &choice);

        switch (choice)
        {
            case STANDARD:
                booking.category = STANDARD;
                return 1;

            case PREMIUM:
                booking.category = PREMIUM;
                return 1;

            case RECLINER:
                booking.category = RECLINER;
                return 1;

            default:
                printf("Invalid category! Enter 1, 2, or 3.\n");
        }
    }
}

// Function to take user inputs
int takeInputs()
{
    while(1)
    {
        scanf("%49s", booking.customerName);

        if (validateName(booking.customerName))
        break;

        printf("Invalid name!\n");
    }

    // Input and validate number of tickets
    while(1)
    {
        scanf("%d", &booking.tickets);

        if (booking.tickets>=MIN_TICKETS && booking.tickets<=MAX_TICKETS)
        break;

        printf("Invalid ticket count! Enter between 1 and 20.\n");
    }

    // Input ticket category
    takeCategory();
    return 1;
}

// Function to calculate subtotal
int calculateSubtotal()
{
    int ticketPrice;
    switch (booking.category)
    {
        case STANDARD:
            ticketPrice = STANDARD_PRICE;
            break;

        case PREMIUM:
            ticketPrice = PREMIUM_PRICE;
            break;

        case RECLINER:
            ticketPrice = RECLINER_PRICE;
            break;

        default:
            return 0;
    }

    bill.subtotal = booking.tickets*ticketPrice;
    return 1;
}

// Function to calculate discount
int calculateDiscount()
{
    if (booking.tickets>=DISCOUNT_LIMIT)
    bill.discount = bill.subtotal * DISCOUNT_RATE / 100.0;
    else
    bill.discount = 0;
    return 1;
}

// Function to calculate final bill
void calculateFinalBill()
{
    bill.finalBill = bill.subtotal-bill.discount;
}

// Function to display booking details
void displayBookingDetails()
{
    printf("Customer: %s\n", booking.customerName);
    printf("Ticket Category: ");
    switch (booking.category)
    {
        case STANDARD:
            printf("Standard\n");
            break;

        case PREMIUM:
            printf("Premium\n");
            break;

        case RECLINER:
            printf("Recliner\n");
            break;
    }
    printf("Tickets: %d\n", booking.tickets);
    printf("Subtotal: %.2f\n", bill.subtotal);
    printf("Discount: %.2f\n", bill.discount);
    printf("Final Bill: %.2f\n", bill.finalBill);
}

int main()
{
    if(!takeInputs())
    return 1;
    if(!calculateSubtotal())
    return 1;
    if(!calculateDiscount())
    return 1;
    calculateFinalBill();
    displayBookingDetails();
    return 0;
}
