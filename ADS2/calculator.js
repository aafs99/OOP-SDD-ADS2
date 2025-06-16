#!/usr/bin/env node

/**
 * Postfix++ Calculator - Interactive Terminal Version
 * A stack-based calculator for evaluating postfix arithmetic expressions with variables
 * Usage: node calculator.js [expression] or run interactively
 */

const readline = require('readline');
const fs = require('fs');
const path = require('path');

class PostfixCalculator {
    constructor() {
        this.stack = [];
        this.symbolTable = new Map();
        this.operators = new Set(['+', '-', '*', '/']);
        this.validVariable = /^[A-Z]$/;
    }

    /**
     * Evaluates a postfix expression
     * @param {string} expression - The postfix expression to evaluate
     * @returns {Array<number>} The current stack state
     */
    evaluate(expression) {
        if (!expression || typeof expression !== 'string') {
            throw new Error('Invalid expression: must be a non-empty string');
        }

        const tokens = expression.trim().split(/\s+/).filter(token => token.length > 0);
        
        if (tokens.length === 0) {
            return [...this.stack];
        }

        for (const token of tokens) {
            this._processToken(token);
        }

        return [...this.stack];
    }

    /**
     * Processes a single token
     * @param {string} token - The token to process
     * @private
     */
    _processToken(token) {
        if (this._isNumber(token)) {
            this.stack.push(parseFloat(token));
        }
        else if (this.operators.has(token)) {
            this._handleOperator(token);
        }
        else if (token === '=') {
            this._handleAssignment();
        }
        else if (this._isValidVariable(token)) {
            this._handleVariable(token);
        }
        else {
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
     * Checks if a token is a valid variable name (A-Z only)
     */
    _isValidVariable(token) {
        return this.validVariable.test(token);
    }

    /**
     * Handles arithmetic operators
     */
    _handleOperator(operator) {
        if (this.stack.length < 2) {
            throw new Error(`Insufficient operands for operator '${operator}'. Need 2, have ${this.stack.length}.`);
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
            return this.symbolTable.get(value);
        }
        if (typeof value === 'string') {
            throw new Error(`Undefined variable: '${value}'`);
        }
        return value;
    }

    /**
     * Handles variable assignment (=)
     */
    _handleAssignment() {
        if (this.stack.length < 2) {
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

        // Store in symbol table
        this.symbolTable.set(variableName, value);
    }

    /**
     * Handles variable references
     */
    _handleVariable(variableName) {
        if (this.symbolTable.has(variableName)) {
            // Variable exists, push its value
            const value = this.symbolTable.get(variableName);
            this.stack.push(value);
        } else {
            // New variable name for potential assignment
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
     * Format the current stack for display
     */
    formatStack() {
        if (this.stack.length === 0) {
            return '[]';
        }
        return `[${this.stack.join(' ')}]`;
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

/**
 * Load only variables (symbol table) from file, not the stack
 */
function loadVariables() {
    const stateFile = path.join(process.cwd(), '.calculator_variables.json');
    const symbolTable = new Map();
    
    try {
        if (fs.existsSync(stateFile)) {
            const savedState = JSON.parse(fs.readFileSync(stateFile, 'utf8'));
            return new Map(savedState.symbolTable || []);
        }
    } catch (error) {
        // Start fresh if state file is corrupted
    }
    
    return symbolTable;
}

/**
 * Save only variables (symbol table) to file, not the stack
 */
function saveVariables(symbolTable) {
    const stateFile = path.join(process.cwd(), '.calculator_variables.json');
    const stateToSave = {
        symbolTable: Array.from(symbolTable.entries())
    };
    
    try {
        fs.writeFileSync(stateFile, JSON.stringify(stateToSave));
    } catch (error) {
        // Ignore save errors
    }
}

/**
 * Clear saved variables file
 */
function clearVariables() {
    const stateFile = path.join(process.cwd(), '.calculator_variables.json');
    try {
        if (fs.existsSync(stateFile)) {
            fs.unlinkSync(stateFile);
        }
    } catch (error) {
        // Ignore errors
    }
}

/**
 * Interactive REPL mode
 */
function startInteractiveMode() {
    const calc = new PostfixCalculator();
    calc.symbolTable = loadVariables();
    
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        prompt: '> '
    });

    console.log('Postfix++ Calculator - Interactive Mode');
    console.log('Enter expressions or commands:');
    console.log('  Examples: "1 1 +", "A 5 =", "A B *"');
    console.log('  Commands: .help, .vars, .stack, .clear, .quit');
    console.log('');
    
    rl.prompt();

    rl.on('line', (input) => {
        const line = input.trim();
        
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
                    clearVariables();
                    console.log('Calculator cleared');
                    break;
                case '.quit':
                    saveVariables(calc.symbolTable);
                    console.log('Goodbye!');
                    rl.close();
                    return;
                default:
                    console.log('Unknown command. Type .help for available commands.');
            }
        } else if (line === '') {
            console.log(calc.formatStack());
        } else {
            try {
                calc.evaluate(line);
                console.log(calc.formatStack());
                saveVariables(calc.symbolTable);
            } catch (error) {
                console.log(`Error: ${error.message}`);
            }
        }
        
        rl.prompt();
    });

    rl.on('close', () => {
        saveVariables(calc.symbolTable);
        console.log('\nGoodbye!');
        process.exit(0);
    });
}

/**
 * Command line mode - evaluate single expression
 */
function evaluateCommandLine(expression) {
    const calc = new PostfixCalculator();
    // Load only variables, not the stack (stack starts fresh each time)
    calc.symbolTable = loadVariables();
    
    try {
        calc.evaluate(expression);
        console.log(calc.formatStack());
        
        // If there's exactly one number on the stack, also output just that number
        if (calc.stack.length === 1 && typeof calc.stack[0] === 'number') {
            console.log(calc.stack[0]);
        }
        
        // Save variables for future use
        saveVariables(calc.symbolTable);
        
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