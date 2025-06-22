//Postfix++ Calculator - Modified Version with Custom Stack and Symbol Table
const readline = require('readline');

/**
 * Custom Stack implementation using only arrays (no built-in push/pop)
 */
class Stack {
    constructor() {
        this.items = [];
        this.topIndex = -1; // Points to the top element, -1 means empty
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
     */
    pop() {
        if (this.isEmpty()) {
            throw new Error('Stack underflow: cannot pop from empty stack');
        }
        const item = this.items[this.topIndex];
        this.items[this.topIndex] = undefined; // Clear the reference
        this.topIndex--;
        return item;
    }

    /**
     * Peek at the top item without removing it
     * @returns {*} The top item
     */
    peek() {
        if (this.isEmpty()) {
            return null;
        }
        return this.items[this.topIndex];
    }

    /**
     * Check if stack is empty
     * @returns {boolean} True if empty
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
     * Get all items as array (for display purposes)
     * @returns {Array} Copy of stack contents
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
 * Custom Symbol Table implementation with explicit INSERT, SEARCH, DELETE algorithms
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
     * @returns {boolean} True if key exists
     */
    has(key) {
        return this._findIndex(key) !== -1;
    }

    /**
     * Private helper method to find the index of a key
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
     * @returns {Array} Array of [key, value] pairs
     */
    entries() {
        const result = [];
        let resultIndex = 0;
        for (let i = 0; i < this.size; i++) {
            result[resultIndex] = [this.keys[i], this.values[i]];
            resultIndex++;
        }
        return result;
    }

    /**
     * Get the number of entries
     * @returns {number} Number of entries
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
    constructor() {
        this.stack = new Stack();
        this.symbolTable = new SymbolTable();
        this.operators = new Set(['+', '-', '*', '/']);
        this.validVariable = /^[A-Z]$/;
    }

    /**
     * Evaluates a postfix expression - MODIFIED to not reset state
     * @param {string} expression - The postfix expression to evaluate
     * @returns {Array<number>} The current stack state
     */
    evaluate(expression) {
        if (!expression || typeof expression !== 'string') {
            throw new Error('Invalid expression: must be a non-empty string');
        }

        const tokens = expression.trim().split(/\s+/).filter(token => token.length > 0);
        
        if (tokens.length === 0) {
            return this.stack.toArray();
        }

        for (const token of tokens) {
            this._processToken(token);
        }

        return this.stack.toArray();
    }

    /**
     * Processes a single token
     * @param {string} token - The token to process
     * @private
     */
    _processToken(token) {
        if (this._isNumber(token)) {
            this.stack.push(parseFloat(token));
        } else if (this.operators.has(token)) {
            this._handleOperator(token);
        } else if (token === '=') {
            this._handleAssignment();
        } else if (this._isValidVariable(token)) {
            this._handleVariable(token);
        } else {
            throw new Error(`Invalid token: '${token}'. Variables must be A-Z.`);
        }
    }

    /**
     * Checks if a token is a valid number
     */
    _isNumber(token) {
        const num = parseFloat(token);
        return !isNaN(num) && isFinite(num);
    }

    /**
     * Checks if a token is a valid variable name (A-Z only) - ENFORCED
     */
    _isValidVariable(token) {
        return this.validVariable.test(token);
    }

    /**
     * Handles arithmetic operators
     */
    _handleOperator(operator) {
        if (this.stack.size() < 2) {
            throw new Error(`Insufficient operands for operator '${operator}'. Need 2, have ${this.stack.size()}.`);
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
        }

        this.stack.push(result);
    }

    /**
     * Resolves a value (converts variables to their stored values)
     */
    _resolveValue(value) {
        if (typeof value === 'string' && this.symbolTable.has(value)) {
            return this.symbolTable.search(value);
        }
        if (typeof value === 'string') {
            throw new Error(`Undefined variable: '${value}'`);
        }
        return value;
    }

    /**
     * Handles variable assignment (=) - MODIFIED behavior
     */
    _handleAssignment() {
        if (this.stack.size() < 2) {
            throw new Error('Assignment requires variable name and value on stack');
        }

        const value = this.stack.pop();
        const variableName = this.stack.pop();

        // The value should be a number (could be result of previous calculation)
        if (typeof value !== 'number') {
            throw new Error(`Cannot assign non-numeric value: ${value}`);
        }

        // The variable name should be a string and valid
        if (typeof variableName !== 'string' || !this._isValidVariable(variableName)) {
            throw new Error(`Invalid variable name: '${variableName}'. Must be A-Z.`);
        }

        // Store in symbol table using INSERT algorithm
        this.symbolTable.insert(variableName, value);
        
        // MODIFIED: Don't return anything to stack after assignment
        // The assignment operator should only update the variable, not leave anything on stack
    }

    /**
     * Handles variable references
     */
    _handleVariable(variableName) {
        if (this.symbolTable.has(variableName)) {
            // Variable exists, push its value using SEARCH algorithm
            const value = this.symbolTable.search(variableName);
            this.stack.push(value);
        } else {
            // New variable name for potential assignment
            this.stack.push(variableName);
        }
    }

    /**
     * Get the final result for output
     * @returns {string|number} The result to display
     */
    getResult() {
        const stackArray = this.stack.toArray();
        
        if (stackArray.length === 0) {
            return '';
        }
        
        if (stackArray.length === 1) {
            if (typeof stackArray[0] === 'string') {
                // This is a variable name - check if it exists
                if (this.symbolTable.has(stackArray[0])) {
                    return this.symbolTable.search(stackArray[0]);
                } else {
                    return 'undefined';
                }
            } else {
                // Numeric result
                return stackArray[0];
            }
        }
        
        // Multiple items on stack - shouldn't happen in normal calculator use
        return stackArray[stackArray.length - 1];
    }

    /**
     * Delete a variable from the symbol table - IMPLEMENTED
     * @param {string} variableName - Variable to delete
     * @returns {boolean} True if deleted successfully
     */
    deleteVariable(variableName) {
        if (!this._isValidVariable(variableName)) {
            throw new Error(`Invalid variable name: '${variableName}'. Must be A-Z.`);
        }
        return this.symbolTable.delete(variableName);
    }

    /**
     * Resets the calculator state
     */
    reset() {
        this.stack.clear();
        this.symbolTable.clear();
    }

    /**
     * Format the current stack for display
     */
    formatStack() {
        const stackArray = this.stack.toArray();
        if (stackArray.length === 0) {
            return '[]';
        }
        return `[${stackArray.join(' ')}]`;
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
        const entries = this.symbolTable.entries();
        for (let i = 0; i < entries.length; i++) {
            const [name, value] = entries[i];
            console.log(`  ${name} = ${value}`);
        }
    }

    /**
     * Get symbol table entries (for display purposes)
     */
    getSymbolTableEntries() {
        return this.symbolTable.entries();
    }
}

/**
 * Interactive REPL mode
 */
function startInteractiveMode() {
    const calc = new PostfixCalculator();
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        prompt: '> '
    });

    console.log('Postfix++ Calculator - Interactive Mode');
    console.log('Enter expressions or commands:');
    console.log('  Examples: "1 1 +", "A 5 =", "A B *"');
    console.log('  Commands: .help, .vars, .stack, .clear, .delete <var>, .quit');
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
                    console.log('  .vars         - Show variables');
                    console.log('  .stack        - Show current stack');
                    console.log('  .clear        - Clear calculator');
                    console.log('  .delete <var> - Delete a variable');
                    console.log('  .quit         - Exit calculator');
                    break;

                case '.vars':
                    calc.showVariables();
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
                                // Don't output anything for successful delete
                            } else {
                                console.log(`Variable ${varName} not found`);
                            }
                        } catch (error) {
                            console.log(`Error: ${error.message}`);
                        }
                    } else {
                        console.log('Usage: .delete <variable_name>');
                    }
                    break;

                case '.quit':
                    console.log('Goodbye!');
                    rl.close();
                    return;

                default:
                    console.log('Unknown command. Type .help for available commands.');
            }
        } else if (line === '') {
            // Empty line - don't output anything
        } else {
            try {
                calc.evaluate(line);
                const result = calc.getResult();
                if (result !== '') {
                    console.log(result);
                }
                // Clear stack after showing result to prevent accumulation
                calc.stack.clear();
            } catch (error) {
                console.log(`Error: ${error.message}`);
                // Clear stack on error too
                calc.stack.clear();
            }
        }

        rl.prompt();
    });

    rl.on('close', () => {
        console.log('\nGoodbye!');
        process.exit(0);
    });
}

/**
 * Command line mode - evaluate single expression
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