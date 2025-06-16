#!/usr/bin/env node

/**
 * Postfix++ Calculator - Interactive Terminal Version
 * A stack-based calculator for evaluating postfix arithmetic expressions with variables
 * Usage: node calculator.js [expression] or run interactively
 */

const readline = require('readline');

class PostfixCalculator {
    constructor() {
        /**
         * The main stack for operands and intermediate results (persistent)
         * @type {Array<number|string>}
         */
        this.stack = [];

        /**
         * Symbol table for variable storage (A-Z only as per specification) (persistent)
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
    }

    /**
     * Evaluates a single line of Postfix++ code
     * Stack and symbol table persist between calls
     * @param {string} expression - The postfix expression to evaluate
     * @returns {Array<number>} The current stack state
     */
    evaluate(expression) {
        if (!expression || typeof expression !== 'string') {
            throw new Error('Invalid expression: must be a non-empty string');
        }

        // Split expression into tokens, handling multiple spaces
        const tokens = expression.trim().split(/\s+/).filter(token => token.length > 0);
        
        if (tokens.length === 0) {
            return [...this.stack]; // Return copy of current stack
        }

        // Process each token
        for (let i = 0; i < tokens.length; i++) {
            const token = tokens[i];
            this._processToken(token);
        }

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
        return !isNaN(num) && isFinite(num);
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
        } else {
            // New variable for potential assignment
            this.stack.push(variableName);
        }
    }

    /**
     * Resets the calculator state
     */
    reset() {
        this.stack = [];
        this.symbolTable.clear();
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

    /**
     * Format the current stack for display
     * @returns {string} Formatted stack
     */
    formatStack() {
        if (this.stack.length === 0) {
            return '[]';
        }
        return `[${this.stack.join(' ')}]`;
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
    console.log('  - Use postfix notation: "1 1 +" gives 2');
    console.log('  - Variables: "A 5 =" assigns 5 to A');
    console.log('  - Special commands: .help, .vars, .stack, .clear, .quit');
    console.log('');
    
    rl.prompt();

    rl.on('line', (input) => {
        const line = input.trim();
        
        // Handle special commands
        if (line.startsWith('.')) {
            switch (line) {
                case '.help':
                    console.log('Commands:');
                    console.log('  .help  - Show this help');
                    console.log('  .vars  - Show variables');
                    console.log('  .stack - Show current stack');
                    console.log('  .clear - Clear calculator');
                    console.log('  .quit  - Exit calculator');
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
                case '.quit':
                    console.log('Goodbye!');
                    rl.close();
                    return;
                default:
                    console.log('Unknown command. Type .help for available commands.');
            }
        } else if (line === '') {
            // Empty line, just show current stack
            console.log(calc.formatStack());
        } else {
            // Process expression
            try {
                calc.evaluate(line);
                console.log(calc.formatStack());
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
        if (calc.stack.length === 1 && typeof calc.stack[0] === 'number') {
            console.log(calc.stack[0]);
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
        // No arguments - start interactive mode
        startInteractiveMode();
    } else {
        // Arguments provided - evaluate as single expression
        const expression = args.join(' ');
        evaluateCommandLine(expression);
    }
}

// Export for use as module
module.exports = PostfixCalculator;