#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cctype>
#include <conio.h> // For getch() on Windows
#include <stdexcept>
#include <set>
#include <string>

using namespace std;

// ========== Data Structures ==========
struct Drink {
    int id;
    char name[50];
    char category[20];
    float price;
    int calories;
    char iceLevel[20];   // "Regular", "Less", "None"
    char sweetness[20];  // "Regular", "Less", "None"
    int iceChoice;   
    int sweetChoice;
    int quantity;
};

struct CartItem {
    Drink drink;
    CartItem* next;
};

struct OrderHistory {
    int orderId;
    time_t orderDate;
    float totalAmount;
    int itemCount;
    OrderHistory* next;
};

struct Customer {
    int id;
    char name[50];
    char email[100];
    char password[50];
    CartItem* cart;
    OrderHistory* orderHistory;
    bool isGuest;        
};

// ========== Queue Implementation ==========
class OrderQueue {
private:
    struct QueueNode {
        int orderId;
        QueueNode* next;
        
        QueueNode(int id) : orderId(id), next(nullptr) {}
    };
    
    QueueNode* front;
    QueueNode* rear;
    int count;
    
public:
    OrderQueue() : front(nullptr), rear(nullptr), count(0) {
        cout<<"Order queue initialized\n";
    }
    
    ~OrderQueue() {
        while (front != nullptr) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    void enqueue(int orderId) {
        QueueNode* newNode = new QueueNode(orderId);
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
        cout<<"Order #" << orderId << " added to queue\n";
    }
    
    int dequeue() {
        if (front == nullptr) {
            throw runtime_error("Queue is empty!");
        }
        
        QueueNode* temp = front;
        int orderId = front->orderId;
        front = front->next;
        
        if (front == nullptr) {
            rear = nullptr;
        }
        
        delete temp;
        count--;
        cout<<"Processing order #" << orderId << "\n";
        return orderId;
    }
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    int size() {
        return count;
    }
};

// ========== Global Variables ==========
const int MAX_QUANTITY = 20;
const int MAX_DRINKS = 50;
const int MAX_CUSTOMERS = 100;
Drink drinkMenu[MAX_DRINKS];
int drinkCount = 0;
Customer customers[MAX_CUSTOMERS];
int customerCount = 0;
Customer* currentCustomer = nullptr;
OrderQueue orderQueue;
int orderCounter = 1000;

// ========== Function Prototypes ==========
void loadDrinksFromFile();
void loadCustomers();
void saveCustomers();
void saveOrderToHistory(Customer* customer, float total);
void displayDashboard();
void viewAllProducts();
void startOrder();
bool viewCart();
void editCart();
void processPayment();
void loginOrRegister();
void viewOrderHistory();
void viewProfile();
void bubbleSortDrinks(int sortBy);
int binarySearchDrink(int id);
void addToCart(Drink drink);
void removeFromCart(int itemIndex);
void clearCart();
float calculateCartTotal();
void displayDrink(Drink d);
void pressAnyKey();
void initializeSystem(); 
int generateUniqueOrderId();

void initializeSystem() {
 
    customers[0].id = 0;
    strcpy(customers[0].name, "");      
    strcpy(customers[0].email, "");    
    strcpy(customers[0].password, ""); 
    customers[0].cart = nullptr;
    customers[0].orderHistory = nullptr;
    customers[0].isGuest = true;       
    customerCount = 1;                 
}

// ========== Main Function ==========
int main() {
	initializeSystem();
    loadDrinksFromFile();
    loadCustomers();
    
      if(!currentCustomer) {
        currentCustomer = &customers[0];
    }
    
    int choice;
    do {
        system("cls"); // Clear screen (Windows)
        displayDashboard();
        
        cout<<"\nEnter your choice: ";
        cin>>choice;
        
  			switch(choice) {
            case 1: viewAllProducts(); break;
            case 2: startOrder(); break;
            case 3: viewCart(); break;
            case 4: editCart(); break;
            case 5: processPayment(); break;
            case 6: loginOrRegister(); break;
            case 7: if(currentCustomer) viewOrderHistory(); break;
            case 8: if(currentCustomer) viewProfile(); break;
            case 0: 
                cout<<"Exiting...\n"; 
                currentCustomer = &customers[0];
                break;
            default: cout<<"Invalid choice!\n"; pressAnyKey();
        }
    } while(choice != 0);
    
    saveCustomers();
    return 0;
}

// ========== Core Function Implementations ==========
void loadDrinksFromFile() {
    ifstream file("mixue.txt");
    if (!file) {
        cerr << "Error opening drink menu file!\n";
        return;
    }
    
    while (file >> drinkMenu[drinkCount].name 
                >> drinkMenu[drinkCount].category 
                >> drinkMenu[drinkCount].price 
                >> drinkMenu[drinkCount].calories) {
        drinkMenu[drinkCount].id = drinkCount + 1;
        strcpy(drinkMenu[drinkCount].iceLevel, "Regular");
        strcpy(drinkMenu[drinkCount].sweetness, "Regular");
        drinkMenu[drinkCount].quantity = 1;
        drinkCount++;
    }
    file.close();
    cout<<"Loaded " << drinkCount << " drinks from file\n";
}

void loadCustomers() {
    ifstream file("customers.txt");
    if (!file) {
        cout << "No existing customer data\n";
        return;
    }
    
    // Initialize guest account
    customers[0].id = 0;
    strcpy(customers[0].name, "Guest");
    strcpy(customers[0].email, "guest@system");
    strcpy(customers[0].password, "");
    customers[0].cart = nullptr;
    customers[0].orderHistory = nullptr;
    customerCount = 1; // Guest is index 0

    // Load registered customers starting from index 1
    while (customerCount < MAX_CUSTOMERS && 
           file >> customers[customerCount].id) {
        file.ignore();
        file.getline(customers[customerCount].name, 50, ' ');
        file.getline(customers[customerCount].email, 100, ' ');
        file.getline(customers[customerCount].password, 50);
        
        customers[customerCount].cart = nullptr;
        customers[customerCount].orderHistory = nullptr;
        customerCount++;
    }
    file.close();
}

void saveCustomers() {
    ofstream file("customers.txt");
    if (!file) return;
    for (int i = 1; i < customerCount; i++) {
        file << customers[i].id << " "
             << customers[i].name << " "
             << customers[i].email << " "
             << customers[i].password << "\n";
    }
    file.close();
    cout<<"Customer data saved\n";
}

void displayDashboard() {
    system("cls");

    cout<<"|----------------------------------------------------------------------------|\n";
    cout<<"|                            MIXUE ICE CREAM & TEA                           |\n";
    cout<<"|                      Ice Cream & Tea Ordering System                       |\n";
    cout<<"|----------------------------------------------------------------------------|\n";

    if (currentCustomer) {
        cout<<"\n                            Welcome, " << currentCustomer->name << "!\n\n";
    } else {
        cout<<"\n                            Welcome, Guest!\n\n";
    };

    cout<<"|----------------------------- DASHBOARD ------------------------------------|\n";
    cout<<"|  [1] View All Products           | Browse our full range of drinks         |\n";
    cout<<"|  [2] Start New Order             | Begin selecting your favorite drinks    |\n";
    cout<<"|  [3] View Cart                   | See what you've added to your cart      |\n";
    cout<<"|  [4] Edit Cart                   | Change quantities or remove items       |\n";
    cout<<"|  [5] Payment                     | Proceed to checkout and pay             |\n";
    cout<<"|  [6] Login / Register            | Sign in or create a new account         |\n";

    if (currentCustomer && !currentCustomer->isGuest) {
        cout<<"|  [7] View Order History          | Review your previous purchases          |\n";
        cout<<"|  [8] View Profile                | Manage your account information         |\n";
    }

    cout<<"|  [0] Exit                        | Close the application                   |\n";
    cout<<"|----------------------------------------------------------------------------|\n";

    // Optional system stats
    printf("  Total Drinks: %-3d   |  Registered Users: %-3d   |  Active Queue: %d\n",
           drinkCount, customerCount - 1, orderQueue.size());
}



void viewAllProducts() {
    int choice;
    do {
        system("cls");
        cout<<"================ MIXUE PRODUCT MENU ================\n";
        cout<<"ID  | Name                    | Category   | Price  | Calories\n";
        cout<<"----+-------------------------+------------+--------+----------\n";

        for (int i = 0; i < drinkCount; i++) {
            printf("%-4d| %-24s| %-11s| RM%-5.2f| %-9d\n", 
                   i + 1, 
                   drinkMenu[i].name, 
                   drinkMenu[i].category, 
                   drinkMenu[i].price, 
                   drinkMenu[i].calories);
        }

        cout<<"\n================== OPTIONS ==================\n";
        cout<<"1. Sort by Price\n";
        cout<<"2. Sort by Calories\n";
        cout<<"3. Filter by Category\n";
        cout<<"4. Search by Name\n";
        cout<<"0. Back to Dashboard\n";
        cout<<"=============================================\n";
        cout<<"Enter your choice: ";
        cin>>choice;
        cout<<"\n";

        switch (choice) {
            case 1:
                bubbleSortDrinks(1);
                break;

            case 2:
                bubbleSortDrinks(2);
                break;

            case 3: {
                char categories[3][20] = {"Beverage", "Juice", "Tea"};

                cout<<"Available categories:\n";
                for (int i = 0; i < 3; i++) {
                    cout<<i + 1 << ". " << categories[i] << "\n";
                }

                int catChoice;
                cout<<"Select category (1-3): ";
                cin>>catChoice;

                if (catChoice >= 1 && catChoice <= 3) {
                    system("cls");
                    cout<<"========== FILTER: " << categories[catChoice - 1] << " ==========\n";
                    cout<<"ID  | Name                    | Category   | Price  | Calories\n";
                    cout<<"----+-------------------------+------------+--------+----------\n";

                    for (int i = 0; i < drinkCount; i++) {
                        if (strcmp(drinkMenu[i].category, categories[catChoice - 1]) == 0) {
                            printf("%-4d| %-24s| %-11s| RM%-5.2f| %-9d\n",
                                   i + 1,
                                   drinkMenu[i].name,
                                   drinkMenu[i].category,
                                   drinkMenu[i].price,
                                   drinkMenu[i].calories);
                        }
                    }
                } else {
                    cout<<"Invalid category choice!\n";
                }

                pressAnyKey();
                break;
            }

            case 4: {
                system("cls");
                cout<<"===== SEARCH BY NAME =====\n";
                cout<<"Enter product name to search: ";
                cin.ignore();
                char searchName[50];
                cin.getline(searchName, 50);

                bool found = false;

                cout<<"\nSearch Results:\n";
                cout<<"ID  | Name                    | Category   | Price  | Calories\n";
                cout<<"----+-------------------------+------------+--------+----------\n";

                for (int i = 0; i < drinkCount; i++) {
                    char nameLower[50], searchLower[50];
                    strcpy(nameLower, drinkMenu[i].name);
                    strcpy(searchLower, searchName);

                    for (int j = 0; nameLower[j]; j++) 
                        nameLower[j] = tolower(nameLower[j]);

                    for (int j = 0; searchLower[j]; j++) 
                        searchLower[j] = tolower(searchLower[j]);

                    if (strstr(nameLower, searchLower)) {
                        printf("%-4d| %-24s| %-11s| RM%-5.2f| %-9d\n",
                               i + 1,
                               drinkMenu[i].name,
                               drinkMenu[i].category,
                               drinkMenu[i].price,
                               drinkMenu[i].calories);
                        found = true;
                    }
                }

                if (!found) {
                    cout<<"No matching products found.\n";
                }

                pressAnyKey();
                break;
            }

            case 0:
                break;

            default:
                cout<<"Invalid choice!\n";
                pressAnyKey();
        }

    } while (choice != 0);
}

void startOrder() {
    int drinkChoice;
    char cont;

    do {
        system("cls");
        cout<<"========== MIXUE DRINK MENU ==========\n";
        cout<<"ID  | Name                    | Category   | Price  | Calories\n";
        cout<<"----+-------------------------+------------+--------+----------\n";

        for (int i = 0; i < drinkCount; i++) {
            printf("%-4d| %-24s| %-11s| RM%-5.2f| %-9d\n",
                   i + 1,
                   drinkMenu[i].name,
                   drinkMenu[i].category,
                   drinkMenu[i].price,
                   drinkMenu[i].calories);
        }

        cout<<"\nEnter Drink ID to order (1-" << drinkCount << "): ";
        cin>>drinkChoice;

        if (drinkChoice < 1 || drinkChoice > drinkCount) {
            cout<<"Invalid ID!\n";
            pressAnyKey();
            return;
        }

        Drink selected = drinkMenu[drinkChoice - 1];

        // Set quantity
        int qty;
        cout<<"\nEnter quantity (1-" << MAX_QUANTITY << "): ";
        cin>>qty;
        if (qty < 1 || qty > MAX_QUANTITY) qty = 1;
        selected.quantity = qty;

        // Set ice level
        cout<<"\nChoose Ice Level:\n";
        cout<<"1. Regular\n2. Less\n3. None\n";
        int ice;
        cout<<"Enter choice: ";
        cin>>ice;
        switch (ice) {
            case 2: strcpy(selected.iceLevel, "Less"); break;
            case 3: strcpy(selected.iceLevel, "None"); break;
            default: strcpy(selected.iceLevel, "Regular");
        }
        selected.iceChoice = ice;

        // Set sweetness
        cout<<"\nChoose Sweetness:\n";
        cout<<"1. Regular\n2. Less\n3. None\n";
        int sweet;
        cout<<"Enter choice: ";
        cin>>sweet;
        switch (sweet) {
            case 2: strcpy(selected.sweetness, "Less"); break;
            case 3: strcpy(selected.sweetness, "None"); break;
            default: strcpy(selected.sweetness, "Regular");
        }
        selected.sweetChoice = sweet;

        // Add to cart
        addToCart(selected);
        cout<<"Item added to cart!\n";

        cout<<"\nOrder another item? (y/n): ";
        cin>>cont;
    } while (tolower(cont) == 'y');
  
   if (tolower(cont) == 'n') {
        viewCart();
    }
}


void addToCart(Drink drink) {
    CartItem* newItem = new CartItem;
    newItem->drink = drink;
    newItem->next = nullptr;
    
    if (!currentCustomer) {
        // For guests, create a temporary cart
        if (!customers[0].cart) {
            customers[0].cart = newItem;
        } else {
            CartItem* current = customers[0].cart;
            while (current->next) {
                current = current->next;
            }
            current->next = newItem;
        }
    } else {
        if (!currentCustomer->cart) {
            currentCustomer->cart = newItem;
        } else {
            CartItem* current = currentCustomer->cart;
            while (current->next) {
                current = current->next;
            }
            current->next = newItem;
        }
    }
}

bool viewCart() {
    system("cls");
    cout<<"+--------------------------------------------------------------------------+\n";
    cout<<"|                             YOUR CART                                    |\n";
    cout<<"+--------------------------------------------------------------------------+\n";

    CartItem* cart = currentCustomer ? currentCustomer->cart : customers[0].cart;
    if (!cart) {
        cout<<"|                                                                          |\n";
        cout<<"| Your cart is currently empty.                                            |\n";
        cout<<"+--------------------------------------------------------------------------+\n";
        pressAnyKey();
        displayDashboard();
        return true;
    }

    int index = 1;
    float total = 0;
    CartItem* current = cart;

    cout<<"\n";
    cout<<"+-----+-------------------------------+------+-----------------------------+\n";
    cout<<"| No  | Drink Name                    | Qty  | Customization               |\n";
    cout<<"+-----+-------------------------------+------+-----------------------------+\n";

    while (current) {
        printf("| %-3d | %-29s | %-4d | Ice: %-7s Sweet: %-7s |\n",
               index,
               current->drink.name,
               current->drink.quantity,
               current->drink.iceLevel,
               current->drink.sweetness);
        total += current->drink.price * current->drink.quantity;
        current = current->next;
        index++;
    }

    cout<<"+-----+-------------------------------+------+-----------------------------+\n";

	char formattedTotal[50];
	snprintf(formattedTotal, sizeof(formattedTotal), "%.2f", total);
	string totalLine = "Total Amount: RM " + string(formattedTotal); 
	
    printf("| %-71s  |\n", totalLine.c_str());
    cout<<"+--------------------------------------------------------------------------+\n";


    cout<<"\n";
    cout<<"+----------------------+\n";
    cout<<"| 1. Proceed to Payment|\n";
    cout<<"| 2. Edit Cart         |\n";
    cout<<"| 3. Remove Item       |\n";
    cout<<"| 4. Clear Cart        |\n";
    cout<<"| 0. Back              |\n";
    cout<<"+----------------------+\n";

    int choice;
    cout<<"\nEnter choice: ";
    cin>>choice;

    switch(choice) {
        case 0: return false;
        case 1: processPayment(); break;
        case 2: editCart(); break;
        case 3: {
            int itemIndex;
            cout<<"Enter item number to remove: ";
            cin>>itemIndex;
            removeFromCart(itemIndex);
            break;
        }
        case 4: clearCart(); break;
        default:
            cout<<"Invalid choice.\n";
            pressAnyKey();
            break;
    }

    return true;
}



void removeFromCart(int itemIndex) {
    CartItem** cart = currentCustomer ? &(currentCustomer->cart) : &(customers[0].cart);
    
    if (itemIndex == 1) {
        CartItem* temp = *cart;
        *cart = (*cart)->next;
        delete temp;
    } else {
        CartItem* current = *cart;
        for (int i = 1; i < itemIndex - 1 && current; i++) {
            current = current->next;
        }
        
        if (current && current->next) {
            CartItem* temp = current->next;
            current->next = temp->next;
            delete temp;
        }
    }
    cout<<"Item removed from cart!\n";
    pressAnyKey();
    viewCart();
}

void clearCart() {
    CartItem** cart = currentCustomer ? &(currentCustomer->cart) : &(customers[0].cart);
    while (*cart) {
        CartItem* temp = *cart;
        *cart = (*cart)->next;
        delete temp;
    }
    cout<<"Cart cleared!\n";
    pressAnyKey();
}

void processPayment() {
    system("cls");
    cout<<"+--------------------------------------------------+\n";
    cout<<"|                    PAYMENT                       |\n";
    cout<<"+--------------------------------------------------+\n";
    
    CartItem* cart = currentCustomer ? currentCustomer->cart : customers[0].cart;
    if (!cart) {
        cout<<"| Your cart is empty!                              |\n";
        cout<<"+--------------------------------------------------+\n";
        pressAnyKey();
        displayDashboard();
        return;
    }
    
    float total = calculateCartTotal();
    float discount = 0;
    
   // Display cart items
CartItem* current = cart;
int index = 1;
cout<<"\n+--------------------------------------------------+\n";
cout<<"|                  CART ITEMS                      |\n";
cout<<"+--------------------------------------------------+\n";
while (current) {
    cout<<"| " << index++ << ". " << current->drink.name 
         << " (" << current->drink.quantity << "x)"
         << " - RM " << current->drink.price * current->drink.quantity << "\n";
    current = current->next;
}
cout<<"+--------------------------------------------------+\n";

cout<<"| SUBTOTAL: RM " << total << "\n";
cout<<"+--------------------------------------------------+\n";

    
    // Discount code (only for registered users)
    if (currentCustomer) {
        char discountCode[20];
        cout<<" Enter discount code (or 0 to skip): ";
        cin>>discountCode;
        
        if (strcmp(discountCode, "0") != 0) {
            // Check discount code (simplified)
            ifstream discountFile("discount.txt");
            if (discountFile) {
                char validCode[20];
                float discountPercent;
                discountFile >> validCode >> discountPercent;
                
                if (strcmp(discountCode, validCode) == 0) {
                    discount = total * (discountPercent / 100);
                    cout<<"| Discount applied: RM " << discount << "\n";
                } else {
                    cout<<"| Invalid discount code                            |\n";
                }
                discountFile.close();
            } else {
                cout<<"| No discount codes available                       |\n";
            }
        }
    }
    
    float finalTotal = total - discount;
    cout<<"| FINAL TOTAL: RM " << finalTotal << "\n";
    cout<<"+--------------------------------------------------+\n\n";
    
    cout<<"+-------------------- OPTIONS ---------------------+\n";
    cout<<"| 1. Confirm Payment                               |\n";
    cout<<"| 2. Edit Cart                                     |\n";
    cout<<"| 0. Cancel                                        |\n";
    cout<<"+--------------------------------------------------+\n";
    
    int choice;
    cout<<"Enter choice: ";
    cin>>choice;
    
    if (choice == 1) {
        cout<<"Processing payment...\n";
        
        // Simulate processing time
        for (int i = 3; i > 0; i--) {
            cout<<i << "... ";
            time_t start = time(0);
            while (time(0) - start < 1) {} // 1 second delay
        }
        
    // Generate unique order ID
  	    int orderId = generateUniqueOrderId();
        orderQueue.enqueue(orderId);
        int processedId = orderQueue.dequeue();
        
cout<<"\n\n+=================================================+\n";
cout<<"|               ORDER COMPLETE                    |\n";
cout<<"+=================================================+\n";
cout<<"| Order ID: #" << processedId << "\n";
cout<<"| Total   : RM " << finalTotal << "\n";
cout<<"| Items   :\n";

        
        current = cart;
        while (current) {
            cout<<"| - " << current->drink.name 
                 << " (" << current->drink.quantity << "x)\n";
            current = current->next;
        }
        cout<<"+--------------------------------------------------+\n";
        
        // Save order to history (for ALL users)
        saveOrderToHistory(currentCustomer ? currentCustomer : &customers[0], finalTotal);
        
        // Clear cart
        clearCart();
        
        pressAnyKey();
    }
    else if (choice == 2) {
        editCart(); 
        processPayment();
    }
    else if (choice == 0) {
        return;
    }
}


void saveOrderToHistory(Customer* customer, float total) {
    OrderHistory* newOrder = new OrderHistory;
    newOrder->orderId = orderCounter;
    newOrder->orderDate = time(0);
    newOrder->totalAmount = total;
    
    // Count items and build item details string
    int itemCount = 0;
    string itemDetails;
    CartItem* current = customer->cart;
    
    while (current) {
        itemCount += current->drink.quantity;
        
        char itemStr[200];
        snprintf(itemStr, sizeof(itemStr), "%s (%d) - %s ice, %s sweet - RM %.2f",
                current->drink.name,
                current->drink.quantity,
                current->drink.iceLevel,
                current->drink.sweetness,
                current->drink.price * current->drink.quantity);
        
        itemDetails += itemStr;
        current = current->next;
        
        // Add separator if more items
        if (current) {
            itemDetails += ", ";
        }
    }

    // Add to history (original unchanged)
    newOrder->next = customer->orderHistory;
    customer->orderHistory = newOrder;
    
    // Save to file in simplified format
    ofstream historyFile("order_history.txt", ios::app);
    if (historyFile) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&newOrder->orderDate));
        
        historyFile << (customer->id ? customer->id : 0) << "|"
                   << newOrder->orderId << "|"
                   << timeStr << "|"
                   << total << "|"
                   << itemCount << "|"
                   << itemDetails << "\n|\n";  
        
        historyFile.close();
        
        // Also print a simple confirmation message
        cout<<"Order #" << newOrder->orderId << " saved to history.\n";
    } else {
        cerr << "Error: Unable to open order_history.txt for writing!\n";
    }
}

void loginOrRegister() {
    system("cls");
    cout<<"+=============== ACCOUNT ===============+\n";
    cout<<"| 1. Login                             |\n";
    cout<<"| 2. Register                          |\n";
    cout<<"| 0. Back                              |\n";
    cout<<"+--------------------------------------+\n";
    
    int choice;
    cout<<"| Enter choice: ";
    cin>>choice;
    cout<<"+--------------------------------------+\n";
    
    if (choice == 1) {
        char email[100], password[50];
        cout<<"| Email: ";
        cin>>email;
        cout<<"| Password: ";
        cin>>password;
        cout<<"+--------------------------------------+\n";
        
        for (int i = 0; i < customerCount; i++) {
            if (strcmp(customers[i].email, email) == 0 && 
                strcmp(customers[i].password, password) == 0) {
                currentCustomer = &customers[i];
                cout<<"| Login successful! Welcome " << customers[i].name << "!\n";
                cout<<"+--------------------------------------+\n";
                pressAnyKey();
                return;
            }
        }
        cout<<"| Invalid email or password!            |\n";
        cout<<"+--------------------------------------+\n";
        pressAnyKey();
    }
    else if (choice == 2) {
        if (customerCount >= MAX_CUSTOMERS) {
            cout<<"+--------------------------------------+\n";
            cout<<"| Cannot register more users!          |\n";
            cout<<"+--------------------------------------+\n";
            pressAnyKey();
            return;
        }
        
        Customer newCustomer;
        newCustomer.id = 1000 + customerCount + 1;
        newCustomer.cart = nullptr;
        newCustomer.orderHistory = nullptr;
        
        cout<<"| Name: ";
        cin.ignore();
        cin.getline(newCustomer.name, 50);
        cout<<"+--------------------------------------+\n";
        
        // Email validation (original code unchanged)
        bool validEmail = false;
        do {
            cout<<"| Email (must end with @gmail.com etc): ";
            cin.getline(newCustomer.email, 100);
            cout<<"+--------------------------------------+\n";
            
            string emailStr(newCustomer.email);
            size_t atPos = emailStr.find('@');
            
            if (atPos != string::npos) {
                string domain = emailStr.substr(atPos);
                if (domain == "@gmail.com" || domain == "@yahoo.com" || 
                    domain == "@outlook.com" || domain == "@email.com") {
                    validEmail = true;
                    
                    for (int i = 0; i < customerCount; i++) {
                        if (strcmp(customers[i].email, newCustomer.email) == 0) {
                            cout<<"| This email is already registered!    |\n";
                            cout<<"+--------------------------------------+\n";
                            validEmail = false;
                            break;
                        }
                    }
                }
            }
            
            if (!validEmail) {
                cout<<"| Invalid email format or domain!      |\n";
                cout<<"+--------------------------------------+\n";
            }
        } while (!validEmail);
        
        // Password validation (original code unchanged)
        bool validPassword = false;
        do {
            cout<<"| Password (6-10 characters): ";
            cin.getline(newCustomer.password, 50);
            cout<<"+--------------------------------------+\n";
            
            int passLength = strlen(newCustomer.password);
            if (passLength >= 6 && passLength <= 10) {
                validPassword = true;
            } else {
                cout<<"| Password must be 6-10 characters!   |\n";
                cout<<"+--------------------------------------+\n";
            }
        } while (!validPassword);
        
        customers[customerCount] = newCustomer;
        currentCustomer = &customers[customerCount];
        customerCount++;
        
        // Create customer file (original code unchanged)
        ofstream customerFile(string(newCustomer.email) + ".txt");
        customerFile << "Customer ID: " << newCustomer.id << "\n";
        customerFile << "Name: " << newCustomer.name << "\n";
        customerFile << "Email: " << newCustomer.email << "\n";
        customerFile.close();
        
        cout<<"| Registration successful!           |\n";
        cout<<"+--------------------------------------+\n";
        pressAnyKey();
    }
}

void viewOrderHistory() {
    system("cls");
    cout<<"+=========== ORDER HISTORY ===========+\n";
    
    if (!currentCustomer->orderHistory) {
        cout<<"+-----------------------------------+\n";
        cout<<"| No order history found!           |\n";
        cout<<"+-----------------------------------+\n";
        pressAnyKey();
        return;
    }
    
    OrderHistory* current = currentCustomer->orderHistory;
while (current) {
    cout<<"+-----------------------------------+\n";
    cout<<"| Order #" << current->orderId << "\n";
    cout<<"| Date: " << ctime(&(current->orderDate));
    cout<<"| Items: " << current->itemCount << "\n";
    cout<<"| Total: RM " << current->totalAmount << "\n";
    cout<<"+-----------------------------------+\n";
    current = current->next;
}
pressAnyKey();
}

void viewProfile() {
    if (!currentCustomer || currentCustomer->isGuest) {
        cout<<"+-------------------------------------+\n";
        cout<<"| Guest session - no profile info     |\n";
        cout<<"+-------------------------------------+\n";
        pressAnyKey();
        return;
    }
    
    system("cls");
    cout<<"+============= YOUR PROFILE =============+\n";
    cout<<"| Name: " << currentCustomer->name << "\n";
    cout<<"| Email: " << currentCustomer->email << "\n";
    cout<<"| Member since: 2025\n";
    cout<<"+-------------------------------------+\n";
    cout<<"| 1. Change Name                     |\n";
    cout<<"| 2. Change Password                 |\n";
    cout<<"| 0. Back                            |\n";
    cout<<"+-------------------------------------+\n";
    
    int choice;
    cout<<"| Enter choice: ";
    cin>>choice;
    cin.ignore(); // Clear input buffer
    
    if (choice == 1) {
        cout<<"| New name: ";
        char newName[50];
        cin.getline(newName, 50);
        strcpy(currentCustomer->name, newName);
        cout<<"+-----------------------------+\n";
        cout<<"| Name updated!              |\n";
        cout<<"+-----------------------------+\n";
        pressAnyKey();
    }
    else if (choice == 2) {
        char currentPwd[50], newPwd[50], confirmPwd[50];
        
        // Verify current password
        cout<<"| Current password: ";
        cin.getline(currentPwd, 50);
        
        if (strcmp(currentPwd, currentCustomer->password) != 0) {
            cout<<"+-----------------------------+\n";
            cout<<"| Incorrect current password!|\n";
            cout<<"+-----------------------------+\n";
            pressAnyKey();
            return;
        }
        
        // Get new password with validation
        bool valid = false;
        do {
            cout<<"| New password (6-10 chars): ";
            cin.getline(newPwd, 50);
            
            if (strlen(newPwd) < 6 || strlen(newPwd) > 10) {
                cout<<"| Password must be 6-10 characters!\n";
                continue;
            }
            
            cout<<"| Confirm new password: ";
            cin.getline(confirmPwd, 50);
            
            if (strcmp(newPwd, confirmPwd) != 0) {
                cout<<"| Passwords don't match!\n";
            } else {
                valid = true;
            }
        } while (!valid);
        
        // Update password
        strcpy(currentCustomer->password, newPwd);
        cout<<"+-----------------------------+\n";
        cout<<"| Password updated!          |\n";
        cout<<"+-----------------------------+\n";
        pressAnyKey();
    }
}

// ========== Algorithm Implementations ==========
void bubbleSortDrinks(int sortBy) {
    bool swapped;
    for (int i = 0; i < drinkCount - 1; i++) {
        swapped = false;
        for (int j = 0; j < drinkCount - i - 1; j++) {
            bool shouldSwap = false;
            
            if (sortBy == 1) { // Price
                shouldSwap = drinkMenu[j].price > drinkMenu[j+1].price;
            } 
            else if (sortBy == 2) { // Calories
                shouldSwap = drinkMenu[j].calories > drinkMenu[j+1].calories;
            }
            
            if (shouldSwap) {
                Drink temp = drinkMenu[j];
                drinkMenu[j] = drinkMenu[j+1];
                drinkMenu[j+1] = temp;
                swapped = true;
            }
        }
        if (!swapped) break;
    }
    cout<<"Sort completed!\n";
    pressAnyKey();
}

int binarySearchDrink(int id) {
    int left = 0;
    int right = drinkCount - 1;
    
    // First sort by ID for binary search
    bubbleSortDrinks(0); // Simplified
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (drinkMenu[mid].id == id) {
            return mid;
        }
        
        if (drinkMenu[mid].id < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

// ========== Helper Functions ==========
void displayDrink(Drink d) {
    cout<<d.name << " (" << d.category << ")";
    cout<<"   Price: RM " << d.price;
    cout<<"   Calories: " << d.calories << "\n\n";
}

float calculateCartTotal() {
    float total = 0;
    CartItem* cart = currentCustomer ? currentCustomer->cart : customers[0].cart;
    CartItem* current = cart;
    
    while (current) {
        total += current->drink.price * current->drink.quantity;
        current = current->next;
    }
    return total;
}

void pressAnyKey() {
    cout<<"\nPress any key to continue...";
    getch(); // Windows-specific, use cin.get() for cross-platfoRM 
}

// ========== Friend Functions ==========
class CustomerSystem {
private:
    int systemId;
public:
    CustomerSystem() : systemId(2025) {}
    
    friend void displaySystemInfo(CustomerSystem sys);
};

void displaySystemInfo(CustomerSystem sys) {
    cout<<"Mixue Ordering System (ID: " << sys.systemId << ")\n";
}

void editCart() {
    int returnToView = 1;

    do {
        system("cls");
        cout<<"+=================================================+\n";
        cout<<"|                  EDIT YOUR CART                 |\n";
        cout<<"+=================================================+\n";

        CartItem* cart = currentCustomer ? currentCustomer->cart : customers[0].cart;
        if (!cart) {
            cout<<"| Your cart is empty!                             |\n";
            cout<<"+=================================================+\n";
            pressAnyKey();
            displayDashboard();
            return;
        }

        int index = 1;
        CartItem* current = cart;
		cout<<"\n+-----+-------------------------------+----------+------------+\n";
		cout<<"| No. | Item                          | Quantity | Total (RM) |\n";
		cout<<"+-----+-------------------------------+----------+------------+\n";
        while (current) {
        printf("| %-3d | %-29s | %-8d | %-10.2f |\n",
           index++,
           current->drink.name,
           current->drink.quantity,  
           current->drink.price * current->drink.quantity);
   		   current = current->next;
			}
		cout<<"+-----+-------------------------------+----------+------------+\n";

cout<<"\nEnter item number to edit (0 to return): ";
int choice;
cin>>choice;

        if (choice == 0) {
            viewCart();
            break;
        }

        if (choice > 0 && choice <= index - 1) {
            current = cart;
            for (int i = 1; i < choice && current; i++) {
                current = current->next;
            }

            if (current) {
                system("cls");
                cout<<"+=====================================+\n";
                cout<<"| Editing: " << current->drink.name << "\n";
                cout<<"+=====================================+\n";
                cout<<"1. Change quantity\n";
                cout<<"2. Change customization\n";
                cout<<"3. Remove item\n";
                cout<<"0. Back to cart\n";
                cout<<"Enter your choice: ";
                int editChoice;
                cin>>editChoice;

                switch (editChoice) {
                    case 1: {
                        int newQty;
                        cout<<"Enter new quantity (0-" << MAX_QUANTITY << "): ";
                        cin>>newQty;
                        if (newQty > 0 && newQty <= MAX_QUANTITY) {
                            current->drink.quantity = newQty;
                            cout<<"Quantity updated!" << endl;
                        } else if (newQty == 0) {
                            removeFromCart(choice);
                            cout<<"Item removed!" << endl;
                        } else {
                            cout<<"Invalid quantity! Max is " << MAX_QUANTITY << endl;
                        }
                        pressAnyKey();
                        break;
                    }
                    case 2: {
                        int customChoice;
                        do {
                            system("cls");
                            cout<<"+==============================+\n";
                            cout<<"| Customization Options        |\n";
                            cout<<"+==============================+\n";
                            cout<<"Current: " << current->drink.iceLevel << " ice, "
                                 << current->drink.sweetness << " sweet\n\n";
                            cout<<"1. Change ice level\n";
                            cout<<"2. Change sweetness\n";
                            cout<<"0. Finish customization\n";
                            cout<<"Enter choice: ";
                            cin>>customChoice;

                            if (customChoice == 1) {
                                cout<<"\n1. Regular Ice\n2. Less Ice\n3. No Ice\n";
                                int iceChoice;
                                cout<<"Enter choice: ";
                                cin>>iceChoice;
                                if (iceChoice >= 1 && iceChoice <= 3) {
                                    const char* levels[] = {"Regular", "Less", "None"};
                                    strcpy(current->drink.iceLevel, levels[iceChoice - 1]);
                                }
                            } else if (customChoice == 2) {
                                cout<<"\n1. Regular Sweet\n2. Less Sweet\n3. No Sugar\n";
                                int sweetChoice;
                                cout<<"Enter choice: ";
                                cin>>sweetChoice;
                                if (sweetChoice >= 1 && sweetChoice <= 3) {
                                    const char* levels[] = {"Regular", "Less", "None"};
                                    strcpy(current->drink.sweetness, levels[sweetChoice - 1]);
                                }
                            }
                        } while (customChoice != 0);
                        cout<<"Customization updated!" << endl;
                        break;
                    }
                    case 3: {
                        removeFromCart(choice);
                        cout<<"Item removed!" << endl;
                        pressAnyKey();
                        break;
                    }
                    case 0:
                        break;
                    default:
                        cout<<"Invalid choice!" << endl;
                        pressAnyKey();
                }
            }
        } else {
            cout<<"Invalid selection!" << endl;
            pressAnyKey();
        }

        cart = currentCustomer ? currentCustomer->cart : customers[0].cart;
        if (!cart) {
            returnToView = 0;
            break;
        }
    } while (returnToView);

    if (returnToView) {
        viewCart();
    }
}


void showCartInterface() {
    while(viewCart()) {
    }
}

int generateUniqueOrderId() {
    ifstream historyFile("order_history.txt");
    set<int> existingIds;
    string line;
    
    // Read existing order IDs from file
    while (getline(historyFile, line)) {
        if (line.find('|') != string::npos) {
            size_t firstPipe = line.find('|');
            size_t secondPipe = line.find('|', firstPipe + 1);
            if (firstPipe != string::npos && secondPipe != string::npos) {
                string idStr = line.substr(firstPipe + 1, secondPipe - firstPipe - 1);
                try {
                    int id = stoi(idStr);
                    existingIds.insert(id);
                } catch (...) {
                    // Ignore invalid entries
                }
            }
        }
    }
    historyFile.close();
    
    // Find the next available ID
    int newId = orderCounter;
    while (existingIds.find(newId) != existingIds.end()) {
        newId++;
        if (newId > 9999) {
            newId = 1001;  // Wrap around if we reach 9999
        }
        // Safety check to prevent infinite loop
        if (newId == orderCounter) {
            cerr << "Error: All order IDs are in use!\n";
            return -1;
        }
    }
    
    orderCounter = newId + 1;  // Update the counter for next order
    if (orderCounter > 9999) {
        orderCounter = 1001;
    }
    
    return newId;
}
