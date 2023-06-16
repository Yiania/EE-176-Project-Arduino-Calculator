#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Number of rows and columns in the keypad
const byte numRows = 4; // 4 rows in the keypad
const byte numCols = 4; // 4 columns

// Pin configuration for rows and columns
byte rowPins[numRows] = {9, 8, 7, 6}; // Pinout for rows 1 to 4
byte colPins[numCols] = {5, 4, 3, 2}; // Pinout for columns 1 to 4

// Keypad key mapping
char keymap[numRows][numCols] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'C', '0', '=', '/'},
};

// Create a Keypad object with the specified parameters
Keypad kpd = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

// Initialization variables and flags
boolean decimalInput = false; // Flag to track decimal input
boolean shiftPressed = false; // Flag to track shift key press
boolean presentValue = false; // Flag to track the current value being entered
boolean negPressed = false; // Flag to track negative sign key press
boolean final = false; // Flag to indicate if the final value is being entered
String num1, num2; // Strings to store the first and second numbers
double answer; // Variable to store the calculated answer
char op; // Variable to store the current operation

int numLength1 = 0; // Length of num1 string
int numLength2 = 0; // Length of num2 string
int ansLength = 0; // Length of the answer string

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("   Calculator   ");
  delay(1000);
  lcd.clear();
}

void loop() {
  char key = kpd.getKey();
  
  // Check if a key is pressed
  if (key != NO_KEY) {
    handleButtonPress(key); // Call the function to handle the button press
    Serial.println(key);
  }
}

// Function to handle the button press
void handleButtonPress(char button) {
  if (button == 'C') {
    shiftPressed = true;
    return;
  }
  
  if (shiftPressed) {
    // Handle shift key combinations
    if (button == '=') {
      clearCalculator(); // Clear the calculator
      shiftPressed = false;
      return;
    } else if (button == '-') {
      negPressed = true;
      shiftPressed = false;
      return;
    } else if (button == '/') {
      decimalInput = true;
      shiftPressed = false;
      return;
    } else if (button == '*') {
      // Handle delete key
      if (presentValue == false) {
        num1 = num1.substring(0, num1.length() - 1);
        numLength1 = num1.length();
        lcd.clear();
        lcd.setCursor(16 - numLength1, 1);
        lcd.print(num1);
      } else {
        num2 = num2.substring(0, num2.length() - 1);
        numLength2 = num2.length();
        lcd.clear();
        lcd.setCursor(16 - numLength1, 0);
        lcd.print(num1);
        lcd.setCursor(0, 1);
        lcd.print(op);
        lcd.setCursor(16 - numLength2, 1);
        lcd.print(num2);
        final = true;
      }
      
      shiftPressed = false;
      return;
    } else if (button == '+') {
      handleEngineeringNotation(); // Handle engineering notation
      shiftPressed = false;
      return;
    } else {
      shiftPressed = false;
    }
  }
  
  switch (button) {
    case '0' ... '9':
      // Handle numeric input
      if (presentValue == false) {
        if (decimalInput) {
          num1 += '.';
          lcd.setCursor(16 - numLength1, 1);
          
          decimalInput = false;
          shiftPressed = false;
          negPressed = false;
        }
        if (negPressed) {
          num1 = num1 + '-';
          lcd.setCursor(16 - numLength1, 1);
          
          decimalInput = false;
          shiftPressed = false;
          negPressed = false;
        }
        
        num1 = num1 + button;
        numLength1 = num1.length();
        lcd.setCursor(16 - numLength1, 1);
        lcd.print(num1);
        negPressed = false;
      } else {
        if (decimalInput) {
          num2 += '.';
          lcd.setCursor(16 - numLength2, 1);
          
          decimalInput = false;
          shiftPressed = false;
          negPressed = false;
        }
        if (negPressed) {
          num2 = num2 + '-';
          lcd.setCursor(16 - numLength2, 1);
          
          decimalInput = false;
          shiftPressed = false;
          negPressed = false;
        }
        
        num2 = num2 + button;
        numLength2 = num2.length();
        lcd.clear();
        lcd.setCursor(16 - numLength1, 0);
        lcd.print(num1);
        lcd.setCursor(0, 1);
        lcd.print(op);
        lcd.setCursor(16 - numLength2, 1);
        lcd.print(num2);
        final = true;
        negPressed = false;
      }
      break;
    
    case '/':
    case '+':
    case '-':
    case '*':
    case '=':
      // Handle arithmetic operations and calculate the result
      if (presentValue == false && button == '=') {
        // Ignore '=' at the start of the program
        return;
      }
      
      if (presentValue == false) {
        presentValue = true;
        op = button;
        lcd.clear();
        lcd.setCursor(16 - numLength1, 0);
        lcd.print(num1);
        lcd.setCursor(0, 1);
        lcd.print(op);
      } else if (presentValue == false && button != NO_KEY && (button == '/' || button == '*' || button == '-' || button == '+')) {
        if (presentValue == false) {
          presentValue = true;
          op = button;
          lcd.clear();
          lcd.setCursor(16 - numLength1, 0);
          lcd.print(num1);
          lcd.setCursor(0, 1);
          lcd.print(op);
        }
      } else if ((final == true && button != NO_KEY) || (final == true && (button == '/' || button == '*' || button == '-' || button == '+') || button == '=')) {
        if (op == '+') {
          answer = num1.toDouble() + num2.toDouble();
        } else if (op == '-') {
          answer = num1.toDouble() - num2.toDouble();
        } else if (op == '*') {
          answer = num1.toDouble() * num2.toDouble();
        } else if (op == '/') {
          if (num2.toFloat() == 0) {
            handleMathError(); // Display math error when dividing by zero
            return;
          }
          answer = num1.toDouble() / num2.toDouble();
        }
        
        String answerString;
        if (answer - static_cast<int>(answer) == 0) {
          answerString = String(static_cast<int>(answer));
        } else {
          answerString = String(answer, 6); //6 decimal places
        }
        ansLength = answerString.length();
        lcd.clear();
        lcd.setCursor(16 - ansLength, 0);
        lcd.print(answerString);
        
        // Update the variables for the next operation
        num1 = answerString;
        num2 = "";
        numLength1 = ansLength;
        numLength2 = 0;
        op = button;
        lcd.setCursor(0, 1);
        lcd.print(op);
        final = true;
      }
      break;
  }
}


void handleEngineeringNotation() {
  // Handle engineering notation
  if (presentValue == false) {
    num1 = useEngineeringNotation(num1);
    if (negPressed) {
      negPressed = false;
      num1 = num1 + "-";
      
      shiftPressed = false;
    }
    numLength1 = num1.length();
    Serial.println(num1);
    lcd.clear();
    lcd.setCursor(16 - numLength1, 1);
    lcd.print(num1);
    negPressed = false;
  } else {
    num2 = useEngineeringNotation(num2);
    if (negPressed) {
      negPressed = false;
      num2 = num2 + "-";
      
      shiftPressed = false;
    }
    numLength2 = num2.length();
    lcd.clear();
    lcd.setCursor(16 - numLength1, 0);
    lcd.print(num1);
    lcd.setCursor(0, 1);
    lcd.print(op);
    lcd.setCursor(16 - numLength2, 1);
    lcd.print(num2);
    final = true;
    decimalInput = false; // reset decimal input flag
    negPressed = false;
  }
}


String useEngineeringNotation(String number) {
  // Convert the number to engineering notation format
  double value = number.toDouble();

  // Format the number with the exponent
  char formattedNumber[10];
  dtostrf(value, -6, 0, formattedNumber); 

  String formattedString = formattedNumber;
  formattedString.trim(); // Remove leading/trailing spaces
  formattedString.replace(".000000", ""); // Remove insignificant zeros after decimal
  formattedString += "E";

  return formattedString;
}

void clearCalculator() {
  // Clear the calculator variables and display
  lcd.clear();
  presentValue = false;
  final = false;
  num1 = "";
  num2 = "";
  answer = 0;
  op = ' ';
  decimalInput = false; // reset decimal input flag
  negPressed = false;
}

void handleMathError() {
  // Display math error and reset the calculator
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Math Error!");
  delay(1000);
  lcd.clear();
  lcd.print("RESETTING...");
  delay(2000);
  clearCalculator();
}
