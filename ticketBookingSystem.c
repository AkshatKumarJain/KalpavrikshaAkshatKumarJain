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

// Global variables
char customerName[50];
int tickets, choice, ticketPrice;
TicketCategory category;
float subtotal, discount, finalBill;

// Function declarations
int validateName(char name[]);
void takeCategory();
void takeInputs();
void calculateSubtotal();
void calculateDiscount();
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
    while(1)
    {
        printf("Enter ticket category (1-Standard, 2-Premium, 3-Recliner): ");
        scanf("%d", &choice);

        switch (choice)
        {
            case STANDARD:
                category = STANDARD;
                ticketPrice = STANDARD_PRICE;
                break;

            case PREMIUM:
                category = PREMIUM;
                ticketPrice = PREMIUM_PRICE;
                break;

            case RECLINER:
                category = RECLINER;
                ticketPrice = RECLINER_PRICE;
                break;

            default:
                printf("Invalid category! Enter 1, 2, or 3.\n");
                continue;
        }
        break;
    }
}

// Function to take user inputs
void takeInputs()
{
    while(1)
    {
        printf("Enter customer name: ");
        scanf("%49s", customerName);

        if (validateName(customerName))
        break;

        printf("Invalid name!\n");
    }

    // Input and validate number of tickets
    while(1)
    {
        printf("Enter number of tickets: ");
        scanf("%d", &tickets);

        if (tickets>=MIN_TICKETS && tickets<=MAX_TICKETS)
        break;

        printf("Invalid ticket count! Enter between 1 and 20.\n");
    }

    // Input ticket category
    takeCategory();
}

// Function to calculate subtotal
void calculateSubtotal()
{
    subtotal = tickets*ticketPrice;
}

// Function to calculate discount
void calculateDiscount()
{
    if (tickets>=DISCOUNT_LIMIT)
    discount = subtotal*DISCOUNT_RATE/100.0;
    else
    discount = 0;
}

// Function to calculate final bill
void calculateFinalBill()
{
    finalBill = subtotal-discount;
}

// Function to display booking details
void displayBookingDetails()
{
    printf("MOVIE TICKET BOOKING DETAILS\n");
    printf("Customer: %s\n", customerName);
    printf("Number of Tickets: %d\n", tickets);
    printf("Ticket Category: ");
    switch (category)
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
    printf("Subtotal: Rs. %.2f\n", subtotal);
    printf("Discount: Rs. %.2f\n", discount);
    printf("Final Bill: Rs. %.2f\n", finalBill);
}

int main()
{
    takeInputs();
    calculateSubtotal();
    calculateDiscount();
    calculateFinalBill();
    displayBookingDetails();
}
