// REFERENCES:
// https://www.youtube.com/watch?v=8QrlSSgRC3s 
// https://www.youtube.com/watch?v=E4ZmJZfGYq4
// https://www.youtube.com/watch?v=hjY1btZ7UD4 

//Postfix++ Calculator 
const readline = require('readline');

//Stack implementation 
class Stack {
    constructor() {
        this.items = [];
        this.topIndex = -1; // Points to the top element, -1 indicates empty stack
    }

    /**
     * Push an item onto the stack
     * @param {*} item - Item to push
     */
    push(item) {
        this.topIndex++;
        this.items[this.topIndex] = item;
    }

    /**
     * Pop an item from the stack
     * @returns {*} The popped item
     * @throws {Error} If stack is empty
     */
    pop() {
        if (this.isEmpty()) {
            throw new Error('Stack underflow: Cannot pop from empty stack');
        }
        const item = this.items[this.topIndex];
        this.items[this.topIndex] = undefined; // Clear the reference to prevent memory leaks
        this.topIndex--;
        return item;
    }

    /**
     * look at top item without removing it
     * @returns {*} The top item, or null if stack is empty
     */
    peek() {
        if (this.isEmpty()) {
            return null;
        }
        return this.items[this.topIndex];
    }

    /**
     * Check if stack is empty
     * @returns {boolean} True if empty, false if not
     */
    isEmpty() {
        return this.topIndex === -1;
    }

    /**
     * Get stack size
     * @returns {number} Number of items in stack
     */
    size() {
        return this.topIndex + 1;
    }

    /**
     * Get all items as array
     * @returns {Array} Copy of stack contents from bottom to top
     */
    toArray() {
        const result = [];
        for (let i = 0; i <= this.topIndex; i++) {
            result[i] = this.items[i];
        }
        return result;
    }

    /**
     * Clear the stack
     */
    clear() {
        this.items = [];
        this.topIndex = -1;
    }
}

/**
 * Symbol Table implementation with INSERT, SEARCH, DELETE functionality
 */
class SymbolTable {
    constructor() {
        // Using parallel arrays to store keys and values
        this.keys = [];
        this.values = [];
        this.size = 0;
    }

    /**
     * INSERT algorithm - Add or update a key-value pair
     * @param {string} key - The variable name
     * @param {*} value - The value to store
     * @returns {boolean} True if successful
     */
    insert(key, value) {
        // First, search if key already exists
        const existingIndex = this._findIndex(key);
        
        if (existingIndex !== -1) {
            // Key exists, update the value
            this.values[existingIndex] = value;
            return true;
        }

        // Key doesn't exist, add new entry
        this.keys[this.size] = key;
        this.values[this.size] = value;
        this.size++;
        return true;
    }

    /**
     * SEARCH algorithm - Find a value by key
     * Time Complexity: O(n) linear search
     * @param {string} key - The key to search for
     * @returns {*} The value if found, undefined if not found
     */
    search(key) {
        const index = this._findIndex(key);
        if (index !== -1) {
            return this.values[index];
        }
        return undefined;
    }

    /**
     * DELETE algorithm - Remove a key-value pair
     * @param {string} key - The key to delete
     * @returns {boolean} True if deleted, false if not found
     */
    delete(key) {
        const index = this._findIndex(key);
        if (index === -1) {
            return false; // Key not found
        }

        // Shift all elements after the deleted index to the left
        for (let i = index; i < this.size - 1; i++) {
            this.keys[i] = this.keys[i + 1];
            this.values[i] = this.values[i + 1];
        }

        // Clear the last elements and decrease size
        this.keys[this.size - 1] = undefined;
        this.values[this.size - 1] = undefined;
        this.size--;
        return true;
    }

    /**
     * Check if a key exists in the symbol table
     * @param {string} key - The key to check
     * @returns {boolean} True if key exists, false if not
     */
    has(key) {
        return this._findIndex(key) !== -1;
    }

    /**
     * Helper method to find the index of a key using linear search
     * @param {string} key - The key to find
     * @returns {number} Index of key, or -1 if not found
     */
    _findIndex(key) {
        for (let i = 0; i < this.size; i++) {
            if (this.keys[i] === key) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Get all key-value pairs
     * @returns {Array<Array>} Array of [key, value] pairs
     */
    getEntries() {
        const result = [];
        for (let i = 0; i < this.size; i++) {
            result[i] = [this.keys[i], this.values[i]];
        }
        return result;
    }

    /**
     * Get the number of entries
     * @returns {number} Number of entries in the symbol table
     */
    getSize() {
        return this.size;
    }

    /**
     * Clear all entries
     */
    clear() {
        this.keys = [];
        this.values = [];
        this.size = 0;
    }
}

class PostfixCalculator {
    // Constants for better maintainability
    static BINARY_OPERATORS = new Set(['+', '-', '*', '/', '^', '%', 'mod']);
    static UNARY_OPERATORS = new Set(['sqrt', 'log', 'ln', 'sin', 'cos', 'tan', '!', 'neg']);
    static CONSTANTS = new Map([
        ['pi', Math.PI],
        ['e', Math.E]
    ]);
    
    static VARIABLE_PATTERN = /^[A-Z]$/;
    // Updated pattern to support scientific notation
    static NUMBER_PATTERN = /^-?\d+(\.\d+)?([eE][+-]?\d+)?$/;
    static MAX_SAFE_NUMBER = Number.MAX_SAFE_INTEGER;
    static MIN_SAFE_NUMBER = Number.MIN_SAFE_INTEGER;

    constructor() {
        this.stack = new Stack();
        this.symbolTable = new SymbolTable();
        this.lastOperationWasAssignment = false; // Track assignment operations
    }

    /**
     * Evaluates a postfix expression
     * @param {string} expression - The postfix expression to evaluate
     * @returns {Array<number>} The current stack state
     * @throws {Error} If expression is invalid or evaluation fails
     */
    evaluate(expression) {
        if (typeof expression !== 'string') {
            throw new Error('Invalid expression: Must be a string');
        }

        const tokens = this._tokenize(expression);
        
        if (tokens.length === 0) {
            return this.stack.toArray();
        }

        this.lastOperationWasAssignment = false;

        for (const token of tokens) {
            this._processToken(token);
        }

        return this.stack.toArray();
    }

    /**
     * Tokenize the input expression
     * @param {string} expression - The expression to tokenize
     * @returns {Array<string>} Array of tokens
     * @private
     */
    _tokenize(expression) {
        return expression.trim().split(/\s+/).filter(token => token.length > 0);
    }

    /**
     * Processes a single token
     * @param {string} token - The token to process
     * @throws {Error} If token is invalid
     * @private
     */
    _processToken(token) {
        if (this._isValidNumber(token)) {
            const num = parseFloat(token);
            this._validateNumberRange(num);
            this.stack.push(num);
        } else if (PostfixCalculator.BINARY_OPERATORS.has(token)) {
            this._handleBinaryOperator(token);
        } else if (PostfixCalculator.UNARY_OPERATORS.has(token)) {
            this._handleUnaryOperator(token);
        } else if (PostfixCalculator.CONSTANTS.has(token)) {
            this.stack.push(PostfixCalculator.CONSTANTS.get(token));
        } else if (token === '=') {
            this._handleAssignment();
        } else if (this._isValidVariable(token)) {
            this._handleVariable(token);
        } else {
            throw new Error(`Invalid token: '${token}'. Valid tokens: numbers, variables (A-Z), operators (+,-,*,/,^,%,mod,sqrt,log,ln,sin,cos,tan,!,neg), constants (pi,e), assignment (=)`);
        }
    }

    /**
     * Checks if a token is a properly formatted number
     * @param {string} token - The token to validate
     * @returns {boolean} True if valid number format
     * @private
     */
    _isValidNumber(token) {
        if (!PostfixCalculator.NUMBER_PATTERN.test(token)) {
            return false;
        }
        const num = parseFloat(token);
        return !isNaN(num) && isFinite(num);
    }

    /**
     * Checks if number is within safe range
     * @param {number} num - The number to validate
     * @throws {Error} If number is outside safe range
     * @private
     */
    _validateNumberRange(num) {
        if (num > PostfixCalculator.MAX_SAFE_NUMBER || num < PostfixCalculator.MIN_SAFE_NUMBER) {
            throw new Error(`Number ${num} is outside safe range`);
        }
    }

    /**
     * Checks if a token is a valid variable name (A-Z only)
     * @param {string} token - The token to validate
     * @returns {boolean} True if valid variable name
     * @private
     */
    _isValidVariable(token) {
        return PostfixCalculator.VARIABLE_PATTERN.test(token);
    }

    /**
     * Handles binary operators
     * @param {string} operator - The operator to handle
     * @throws {Error} If insufficient operands or invalid operation
     * @private
     */
    _handleBinaryOperator(operator) {
        if (this.stack.size() < 2) {
            throw new Error(`Insufficient operands for operator '${operator}': Need 2, have ${this.stack.size()}`);
        }

        const b = this.stack.pop();
        const a = this.stack.pop();

        // Resolve variables to their values
        const aValue = this._resolveValue(a);
        const bValue = this._resolveValue(b);

        if (typeof aValue !== 'number' || typeof bValue !== 'number') {
            throw new Error(`Cannot apply operator '${operator}' to non-numeric values`);
        }

        let result;
        switch (operator) {
            case '+':
                result = aValue + bValue;
                break;
            case '-':
                result = aValue - bValue;
                break;
            case '*':
                result = aValue * bValue;
                break;
            case '/':
                if (bValue === 0) {
                    throw new Error('Division by zero is not allowed');
                }
                result = aValue / bValue;
                break;
            case '^':
                result = Math.pow(aValue, bValue);
                if (!isFinite(result)) {
                    throw new Error(`Power operation resulted in invalid number: ${aValue}^${bValue}`);
                }
                break;
            case '%':
            case 'mod':
                if (bValue === 0) {
                    throw new Error('Modulo by zero is not allowed');
                }
                result = aValue % bValue;
                break;
        }

        this._validateNumberRange(result);
        this.stack.push(result);
    }

    /**
     * Handles unary operators
     * @param {string} operator - The operator to handle
     * @throws {Error} If insufficient operands or invalid operation
     * @private
     */
    _handleUnaryOperator(operator) {
        if (this.stack.size() < 1) {
            throw new Error(`Insufficient operands for operator '${operator}': Need 1, have ${this.stack.size()}`);
        }

        const a = this.stack.pop();
        const aValue = this._resolveValue(a);

        if (typeof aValue !== 'number') {
            throw new Error(`Cannot apply operator '${operator}' to non-numeric value`);
        }

        let result;
        switch (operator) {
            case 'sqrt':
                if (aValue < 0) {
                    throw new Error('Square root of negative number is not allowed');
                }
                result = Math.sqrt(aValue);
                break;
            case 'log':
                if (aValue <= 0) {
                    throw new Error('Logarithm of non-positive number is not allowed');
                }
                result = Math.log10(aValue);
                break;
            case 'ln':
                if (aValue <= 0) {
                    throw new Error('Natural logarithm of non-positive number is not allowed');
                }
                result = Math.log(aValue);
                break;
            case 'sin':
                result = Math.sin(aValue);
                break;
            case 'cos':
                result = Math.cos(aValue);
                break;
            case 'tan':
                result = Math.tan(aValue);
                break;
            case '!':
                if (aValue < 0 || !Number.isInteger(aValue)) {
                    throw new Error('Factorial is only defined for non-negative integers');
                }
                if (aValue > 170) {
                    throw new Error('Factorial argument too large (maximum 170)');
                }
                result = this._factorial(aValue);
                break;
            case 'neg':
                result = -aValue;
                break;
        }

        if (!isFinite(result)) {
            throw new Error(`Operation '${operator}' resulted in invalid number`);
        }

        this._validateNumberRange(result);
        this.stack.push(result);
    }

    /**
     * Calculate factorial
     * @param {number} n - The number to calculate factorial for
     * @returns {number} The factorial result
     * @private
     */
    _factorial(n) {
        if (n === 0 || n === 1) return 1;
        let result = 1;
        for (let i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    /**
     * Converts variables to their stored value
     * @param {*} value - The value to resolve
     * @returns {number} The resolved numeric value
     * @throws {Error} If variable is undefined
     * @private
     */
    _resolveValue(value) {
        if (typeof value === 'string') {
            if (this.symbolTable.has(value)) {
                return this.symbolTable.search(value);
            } else {
                throw new Error(`Undefined variable: '${value}'`);
            }
        }
        return value;
    }

    /**
     * Handles variable assignment
     * @throws {Error} If assignment is invalid
     * @private
     */
    _handleAssignment() {
        if (this.stack.size() < 2) {
            throw new Error('Assignment requires variable name and value on stack');
        }

        // pop in reverse order: value comes off first, then variable
        const value = this.stack.pop();
        const variableName = this.stack.pop();

        // The variable name should be a string and valid
        if (typeof variableName !== 'string' || !this._isValidVariable(variableName)) {
            throw new Error(`Invalid variable name: '${variableName}'. Must be A-Z`);
        }

        // The value should be a number (could be result of previous calculation)
        const resolvedValue = this._resolveValue(value);
        if (typeof resolvedValue !== 'number') {
            throw new Error(`Cannot assign non-numeric value: '${value}'`);
        }

        // Store in symbol table using INSERT algorithm
        this.symbolTable.insert(variableName, resolvedValue);
        
        // Mark that the last operation was an assignment
        this.lastOperationWasAssignment = true;
        
        // Assignment operator consumes both operands and produces no result
    }

    /**
     * Handles variable references
     * @param {string} variableName - The variable name to handle
     * @private
     */
    _handleVariable(variableName) {
        // Always push the variable name itself onto the stack
        // The resolution to values happens only when needed by operators
        this.stack.push(variableName);
    }

    /**
     * Get the top stack value for output without modifying the stack
     * This method returns the top value of the stack, resolving variables to their values.
     * If the stack is empty, it returns null.
     * If the top value is a variable name, it checks the symbol table and returns its value or 'undefined' if not found.
     * If the top value is a number, it returns that number.
     * @returns {number|string|null} The result to display, or null if stack is empty or last operation was assignment
     */
    getTopResult() {
        // Don't show result if last operation was assignment
        if (this.lastOperationWasAssignment) {
            return null;
        }
        
        const stackArray = this.stack.toArray();
        
        if (stackArray.length === 0) {
            return null; // No result for empty stack
        }
        
        const topValue = stackArray[stackArray.length - 1];
        
        if (typeof topValue === 'string') {
            // This is a variable name - check if it exists
            if (this.symbolTable.has(topValue)) {
                return this.symbolTable.search(topValue);
            } else {
                return 'undefined';
            }
        }
        
        return topValue;
    }

    /**
     * Delete a variable from the symbol table
     * @param {string} variableName - Variable to delete
     * @returns {boolean} True if deleted successfully, false if not found
     * @throws {Error} If variable name is invalid
     */
    deleteVariable(variableName) {
        if (!this._isValidVariable(variableName)) {
            throw new Error(`Invalid variable name: '${variableName}'. Must be A-Z`);
        }
        return this.symbolTable.delete(variableName);
    }

    /**
     * Resets the calculator state
     */
    reset() {
        this.stack.clear();
        this.symbolTable.clear();
        this.lastOperationWasAssignment = false;
    }

    /**
     * Format the current stack for display
     * Stack is displayed with top element on the LEFT
     * @returns {string} Formatted stack representation
     */
    formatStack() {
        const stackArray = this.stack.toArray();
        if (stackArray.length === 0) {
            return '[]';
        }
        // Reverse the array so top of stack appears on the left
        const reversed = stackArray.slice().reverse();
        return `[${reversed.join(' ')}]`;
    }

    /**
     * Display symbol table contents
     */
    showVariables() {
        if (this.symbolTable.getSize() === 0) {
            console.log('No variables defined');
            return;
        }
        console.log('Variables:');
        const entries = this.symbolTable.getEntries();
        for (let i = 0; i < entries.length; i++) {
            const [name, value] = entries[i];
            console.log(`  ${name} = ${value}`);
        }
    }

    /**
     * Get symbol table entries
     * @returns {Array<Array>} Array of [key, value] pairs
     */
    getSymbolTableEntries() {
        return this.symbolTable.getEntries();
    }

    /**
     * Get current stack size
     * @returns {number} Number of items on stack
     */
    getStackSize() {
        return this.stack.size();
    }
}

/**
 * Enhanced interactive mode with error recovery
 */
function startInteractiveMode() {
    const calc = new PostfixCalculator();
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        prompt: '> '
    });

    console.log('Postfix++ Calculator');
    console.log('Enter expressions/commands:');
    console.log('');
    console.log('Basic: "1 1 +", "A 5 =", "A B *"');
    console.log('');
    console.log('Advanced: "2 3 ^", "25 sqrt", "5 !", "pi 2 *", "45 sin"');
    console.log('');
    console.log('Commands:\n HELP: .help,\n CHECK VARIABLES: .vars,\n STACK: .stack,\n CLEAR: .clear,\n DELETE: .delete <var>,\n SEARCH: .get <var>,\n QUIT: CRTL + C, .Q');
    console.log('');
    console.log('Note: Angles for trig functions are in radians');
    console.log('');

    rl.prompt();

    rl.on('line', (input) => {
        const line = input.trim();

        if (line.startsWith('.')) {
            const parts = line.split(' ');
            const command = parts[0];

            switch (command) {
                case '.help':
                    console.log('Commands:');
                    console.log('  .help         - Show this help');
                    console.log('  .vars         - Show all variables');
                    console.log('  .get <var>    - Search for value of a variable');
                    console.log('  .stack        - Show current stack');
                    console.log('  .clear        - Clear calculator');
                    console.log('  .delete <var> - Delete a variable');
                    console.log('  .Q, CTRL+C - Exit calculator');
                    console.log('');
                    console.log('Operators:');
                    console.log('  Binary: +, -, *, /, ^, %, mod');
                    console.log('  Unary: sqrt, log, ln, sin, cos, tan, !, neg');
                    console.log('  Constants: pi, e');
                    console.log('  Assignment: =');
                    break;

                case '.vars':
                    calc.showVariables();
                    break;

                case '.get':
                    if (parts.length > 1) {
                        const varName = parts[1];
                        if (!/^[A-Z]$/.test(varName)) {
                            console.log(`Error: Invalid variable name: '${varName}'. Must be A-Z`);
                        } else if (calc.symbolTable.has(varName)) {
                            console.log(`${varName} = ${calc.symbolTable.search(varName)}`);
                        } else {
                            console.log(`Variable '${varName}' is not defined`);
                        }
                    } else {
                        console.log('Usage: .get <variable_name>');
                    }
                    break;

                case '.stack':
                    console.log('Stack:', calc.formatStack());
                    break;

                case '.clear':
                    calc.reset();
                    console.log('Calculator cleared');
                    break;

                case '.delete':
                    if (parts.length > 1) {
                        const varName = parts[1];
                        try {
                            if (calc.deleteVariable(varName)) {
                                console.log(`Variable '${varName}' deleted`);
                            } else {
                                console.log(`Variable '${varName}' not found`);
                            }
                        } catch (error) {
                            console.log(`Error: ${error.message}`);
                        }
                    } else {
                        console.log('Usage: .delete <variable_name>');
                    }
                    break;

                case '.Q':
                    console.log('Goodbye!');
                    rl.close();
                    return;

                default:
                    console.log('Unknown command. Type .help for available commands');
            }
        } else if (line === '') {
            // Empty line, no output 
        } else {
            // Enhanced error recovery: continue execution after errors in interactive mode
            try {
                calc.evaluate(line);
                const result = calc.getTopResult();
                if (result !== null) {
                    console.log(result);
                }
                // no output for operations that leave empty stack or assignments
            } catch (error) {
                console.log(`Error: ${error.message}`);
            }
        }

        rl.prompt();
    });

    rl.on('close', () => {
        console.log('\nGoodbye!');
        process.exit(0);
    });

    // Enhanced error recovery: Handle SIGINT (Ctrl+C) gracefully
    process.on('SIGINT', () => {
        console.log('\n\nUse .Q to exit or continue with calculations...');
        rl.prompt();
    });
}

/**
 * Command line mode 
 */
function evaluateCommandLine(expression) {
    const calc = new PostfixCalculator();
    try {
        calc.evaluate(expression);
        console.log(calc.formatStack());

        // If there's exactly one number on the stack, also output just that number
        const stackArray = calc.stack.toArray();
        if (stackArray.length === 1 && typeof stackArray[0] === 'number') {
            console.log(stackArray[0]);
        }
    } catch (error) {
        console.error(`Error: ${error.message}`);
        process.exit(1);
    }
}

// Main execution logic
if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        startInteractiveMode();
    } else {
        const expression = args.join(' ');
        evaluateCommandLine(expression);
    }
}

module.exports = PostfixCalculator;