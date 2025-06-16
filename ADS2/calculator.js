/**
 * Postfix++ Calculator
 */
class PostfixCalculator {
    constructor() {
        /**
         * The main stack for operands and intermediate results
         * @type {Array<number|string>}
         */
        this.stack = [];

        /**
         * Symbol table for variable storage (A-Z only as per specification)
         * @type {Map<string, number>}
         */
        this.symbolTable = new Map();

        /**
         * Set of supported arithmetic operators
         * @type {Set<string>}
         */
        this.operators = new Set(['+', '-', '*', '/']);

        /**
         * Valid variable names (A-Z only for limited memory devices)
         * @type {RegExp}
         */
        this.validVariable = /^[A-Z]$/;

        /**
         * History of operations for debugging/logging
         * @type {Array<Object>}
         */
        this.operationHistory = [];
    }

    /**
     * Evaluates a single line of Postfix++ code
     * @param {string} expression - The postfix expression to evaluate
     * @returns {Array<number>} The current stack state
     */
    evaluateLine(expression) {
        if (!expression || typeof expression !== 'string') {
            throw new Error('Invalid expression: must be a non-empty string');
        }

        // Split expression into tokens, handling multiple spaces
        const tokens = expression.trim().split(/\s+/).filter(token => token.length > 0);
        
        if (tokens.length === 0) {
            return [...this.stack]; // Return copy of current stack
        }

        console.log(`> ${expression}`);
        
        // Process each token
        for (let i = 0; i < tokens.length; i++) {
            const token = tokens[i];
            this._processToken(token);
            this._logState(token);
        }

        console.log(`[${this.stack.join(' ')}]`);
        return [...this.stack]; // Return copy of stack
    }

    /**
     * Processes a single token
     * @param {string} token - The token to process
     * @private
     */
    _processToken(token) {
        // Check if token is a number
        if (this._isNumber(token)) {
            this.stack.push(parseFloat(token));
            this._recordOperation('PUSH_NUMBER', token, [...this.stack]);
        }
        // Check if token is an operator
        else if (this.operators.has(token)) {
            this._handleOperator(token);
        }
        // Check if token is assignment operator
        else if (token === '=') {
            this._handleAssignment();
        }
        // Check if token is a valid variable name
        else if (this._isValidVariable(token)) {
            this._handleVariable(token);
        }
        else {
            throw new Error(`Invalid token: '${token}'. Variables must be A-Z.`);
        }
    }

    /**
     * Checks if a token is a valid number
     * @param {string} token - Token to check
     * @returns {boolean}
     * @private
     */
    _isNumber(token) {
        const num = parseFloat(token);
        return !isNaN(num) && isFinite(num) && token.toString() === num.toString();
    }

    /**
     * Checks if a token is a valid variable name (A-Z only)
     * @param {string} token - Token to check
     * @returns {boolean}
     * @private
     */
    _isValidVariable(token) {
        return this.validVariable.test(token);
    }

    /**
     * Handles arithmetic operators
     * @param {string} operator - The operator (+, -, *, /)
     * @private
     */
    _handleOperator(operator) {
        if (this.stack.length < 2) {
            throw new Error(`Insufficient operands for operator '${operator}'. Need 2, have ${this.stack.length}.`);
        }

        const b = this.stack.pop(); // Second operand
        const a = this.stack.pop(); // First operand

        // Ensure both operands are numbers
        if (typeof a !== 'number' || typeof b !== 'number') {
            throw new Error(`Cannot apply operator '${operator}' to non-numeric values: ${a}, ${b}`);
        }

        let result;
        switch (operator) {
            case '+':
                result = a + b;
                break;
            case '-':
                result = a - b;
                break;
            case '*':
                result = a * b;
                break;
            case '/':
                if (b === 0) {
                    throw new Error('Division by zero is not allowed');
                }
                result = a / b;
                break;
            default:
                throw new Error(`Unknown operator: ${operator}`);
        }

        this.stack.push(result);
        this._recordOperation('OPERATOR', `${a} ${operator} ${b} = ${result}`, [...this.stack]);
    }

    /**
     * Handles variable assignment (=)
     * @private
     */
    _handleAssignment() {
        if (this.stack.length < 2) {
            throw new Error('Assignment requires variable name and value on stack');
        }

        const value = this.stack.pop();
        const variableName = this.stack.pop();

        // Validate assignment
        if (typeof value !== 'number') {
            throw new Error(`Cannot assign non-numeric value: ${value}`);
        }

        if (typeof variableName !== 'string' || !this._isValidVariable(variableName)) {
            throw new Error(`Invalid variable name: '${variableName}'. Must be A-Z.`);
        }

        // Store in symbol table
        this.symbolTable.set(variableName, value);
        this._recordOperation('ASSIGNMENT', `${variableName} = ${value}`, [...this.stack]);
    }

    /**
     * Handles variable references
     * @param {string} variableName - The variable name
     * @private
     */
    _handleVariable(variableName) {
        if (this.symbolTable.has(variableName)) {
            // Variable exists, push its value
            const value = this.symbolTable.get(variableName);
            this.stack.push(value);
            this._recordOperation('VARIABLE_USE', `${variableName} (${value})`, [...this.stack]);
        } else {
            // New variable for potential assignment
            this.stack.push(variableName);
            this._recordOperation('VARIABLE_NAME', variableName, [...this.stack]);
        }
    }

    /**
     * Records operation for history/debugging
     * @param {string} type - Type of operation
     * @param {string} details - Operation details
     * @param {Array} stackState - Stack state after operation
     * @private
     */
    _recordOperation(type, details, stackState) {
        this.operationHistory.push({
            type,
            details,
            stackState: [...stackState],
            symbolTable: new Map(this.symbolTable),
            timestamp: new Date()
        });
    }

    /**
     * Logs current state (simplified for interactive use)
     * @param {string} token - Current token
     * @private
     */
    _logState(token) {
        const stackDisplay = this.stack.map(item => 
            typeof item === 'string' ? `"${item}"` : item
        ).join(', ');
        console.log(`  Token: ${token} -> Stack: [${stackDisplay}]`);
    }

    /**
     * Clears the calculator state
     */
    clear() {
        this.stack = [];
        this.symbolTable.clear();
        this.operationHistory = [];
        console.log('Calculator cleared');
    }

    /**
     * Gets the current state of the calculator
     * @returns {Object} Current state
     */
    getState() {
        return {
            stack: [...this.stack],
            symbolTable: Object.fromEntries(this.symbolTable),
            stackSize: this.stack.length,
            variableCount: this.symbolTable.size
        };
    }

    /**
     * Gets the top value from the stack without removing it
     * @returns {number|string|undefined} Top stack value
     */
    peek() {
        return this.stack.length > 0 ? this.stack[this.stack.length - 1] : undefined;
    }

    /**
     * Interactive REPL-style evaluation
     * @param {string} input - User input
     * @returns {string} Formatted response
     */
    interactive(input) {
        try {
            const result = this.evaluateLine(input);
            if (result.length === 0) {
                return '[]';
            } else if (result.length === 1 && typeof result[0] === 'number') {
                return `[${result[0]}]`;
            } else {
                return `[${result.join(' ')}]`;
            }
        } catch (error) {
            return `Error: ${error.message}`;
        }
    }

    /**
     * Display symbol table contents
     */
    showVariables() {
        if (this.symbolTable.size === 0) {
            console.log('No variables defined');
            return;
        }
        
        console.log('Variables:');
        for (const [name, value] of this.symbolTable) {
            console.log(`  ${name} = ${value}`);
        }
    }
}

// === Demo and Testing ===

function runDemo() {
    console.log('=== Postfix++ Calculator Demo ===\n');
    
    const calc = new PostfixCalculator();
    
    console.log('--- Stage A: Basic Arithmetic ---');
    calc.evaluateLine("3 4 5 + *");
    console.log('Expected: [27]\n');
    
    console.log('--- Stage B: Variables and Assignment ---');
    calc.evaluateLine("A 2 =");
    calc.evaluateLine("B 3 =");
    calc.evaluateLine("A B *");
    console.log('Expected: [6]');
    calc.showVariables();
    console.log();
    
    console.log('--- Interactive Examples ---');
    console.log('> C 10 =');
    console.log(calc.interactive("C 10 ="));
    
    console.log('> A C +');
    console.log(calc.interactive("A C +"));
    
    console.log('> D A B + =');
    console.log(calc.interactive("D A B + ="));
    
    console.log('\n--- Final State ---');
    console.log('Stack:', calc.getState().stack);
    calc.showVariables();
}

function runInteractiveSimulation() {
    console.log('\n=== Interactive Session Simulation ===');
    const calc = new PostfixCalculator();
    
    const commands = [
        "A 2 =",
        "B 3 =", 
        "A B *"
    ];
    
    commands.forEach(cmd => {
        console.log(`> ${cmd}`);
        const result = calc.interactive(cmd);
        console.log(result);
    });
}

// Run demonstrations
if (typeof module !== 'undefined' && require.main === module) {
    runDemo();
    runInteractiveSimulation();
}

// Export for use as module
if (typeof module !== 'undefined') {
    module.exports = PostfixCalculator;
}